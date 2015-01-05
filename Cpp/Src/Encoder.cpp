#include "Encoder.h"
#include "CImageData.h"
#include <vector>
#include <array>
#include <assert.h>
#include <atomic>
#include <thread>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;


//--------------------------------------------------------------------------------------------------------------
struct SVector2
{
	SVector2 (float px, float py) : x(px), y(py) { }
	float x;
	float y;
};

//--------------------------------------------------------------------------------------------------------------
struct SThreadData
{
	vector<SVector2>	m_polygon;
	vector<SVector2>	m_polygonTemp;
};

//--------------------------------------------------------------------------------------------------------------
class CEncodedPixelData
{
public:
    CEncodedPixelData (const CImageDataBlackWhite& src, CImageDataRGBA& dest)
        : m_src(src)
        , m_dest(dest)
		, c_bucketCount(dest.GetHeight() << 8)
		, c_angleCount(dest.GetHeight())
		, c_centerX((float)src.GetWidth() / 2.0f)
		, c_centerY((float)src.GetHeight() / 2.0f)
		, c_maxX(src.GetWidth()-1.0f)
		, c_maxY(src.GetHeight()-1.0f)
		, c_hypotneuse(sqrtf((float)src.GetWidth()*(float)src.GetWidth()+(float)src.GetHeight()*(float)src.GetHeight()))
		, c_arcSizeRadians(((float)M_PI*2.0f)/((float)c_angleCount))
		, c_halfArcSizeRadians(((float)M_PI)/((float)c_angleCount))
    {
        assert(dest.GetWidth() == 1);
    }

	typedef vector<bool> TBucketType;
	typedef TBucketType::size_type TSizeType;

	//----------------------------------------------------------------------------------------------------------
    bool Encode ()
	{
		// make some storage for our buckets.  size = angles * distances and there are
		// 256 distances since they are stored in an 8 bit channel.
		TBucketType buckets(c_bucketCount);

		// reset our atomic integer which the threads use to know what work to do
		atomic<TSizeType> nextBucket(static_cast<TSizeType>(-1));

		// make a thread pool that calls our encoding thread function
		auto numThreads = thread::hardware_concurrency();
		vector<thread> threads(numThreads);
		for (decltype(numThreads) i = 0; i < numThreads; ++i)
			threads[i] = thread([&] () { CalcBucketsOverlapMT(buckets, nextBucket); });

		// wait for our threads to be done
		for (decltype(numThreads) i = 0; i < numThreads; ++i)
			threads[i].join();

		// now the buckets have the color (black/white) for each radial pixel
		// TODO: make sure there are less than 4 ranges by removing the smallest ranges first
		// TODO: convert to RGBA!

		// TODO: TEMP
        memset(m_dest.GetPixelBuffer(), 128, m_dest.GetPixelBufferSize());

        // return success
        return true;
    }

private:

	//----------------------------------------------------------------------------------------------------------
	template <typename TestValidFN, typename IntersectPointFN>
	void ClipPolygonByPlane (SThreadData& threadData, TestValidFN TestValid, IntersectPointFN IntersectPoint) const
	{
		// if no points (due to previous culling), nothing to do here!
		if (threadData.m_polygon.size() == 0)
			return;

		// start with a fresh slate
		threadData.m_polygonTemp.clear();

		// for each line segment...
		SVector2 *lastPoint = &(*threadData.m_polygon.rbegin());
		bool lastPointValid = TestValid(*lastPoint);;
		for (size_t i = 0, c = threadData.m_polygon.size(); i < c; ++i)
		{
			// get the current point
			SVector2 *currentPoint = &threadData.m_polygon[i];
			bool currentPointValid = TestValid(*currentPoint);

			if (currentPointValid)
			{
				// if the last point is invalid, but the current point is valid, add the intersection point
				if (!lastPointValid)
					threadData.m_polygonTemp.push_back(IntersectPoint(*lastPoint, *currentPoint));

				// if the current point is valid, add it
				threadData.m_polygonTemp.push_back(*currentPoint);
			}
			// current point invalid, last point valid
			// add the point in last->current that intersects the edge
			else if (lastPointValid)
			{
				threadData.m_polygonTemp.push_back(IntersectPoint(*lastPoint, *currentPoint));
			}

			// make the current point into the last point for the next go around
			lastPoint = currentPoint;
			lastPointValid = currentPointValid;
		}

		// make our temp work the new polygon data
		threadData.m_polygon = threadData.m_polygonTemp;
	}

	//----------------------------------------------------------------------------------------------------------
	float CalcTriangleOverlapMT (SThreadData& threadData, float AX, float AY, float BX, float BY, float CX, float CY) const
	{
		// get our bounding box to be able to know which pixels we need to test this triangle against
		float minX = floor(min(AX,min(BX,CX)));
		float minY = floor(min(AY,min(BY,CY)));
		float maxX = floor(max(AX,max(BX,CX)))+1.0f;
		float maxY = floor(max(AY,max(BY,CY)))+1.0f;

		// clip the bounding box to the source image dimensions
		minX = max(minX, 0.0f);
		minY = max(minY, 0.0f);
		maxX = min(maxX, c_maxX);
		maxY = min(maxY, c_maxY);

		// TODO: make sure the above works correct when entire triangle is out of bounds
		// like if both y's are negative, it'll get pushed to zero and the <= below will make it test a pixel!

		// for each pixel in the bounding box
		int sx = (int)minX;
		int sy = (int)minY;
		int ex = (int)maxX;
		int ey = (int)maxY;
		float triangleTotal = 0.0f;
		for (int iy = sy; iy <= ey; ++iy)
		{
			for (int ix = sx; ix <= ex; ++ix)
			{
				// make the source polygon
				threadData.m_polygon.clear();
				threadData.m_polygon.push_back(SVector2(AX,AY));
				threadData.m_polygon.push_back(SVector2(BX,BY));
				threadData.m_polygon.push_back(SVector2(CX,CY));

				// TODO: remove this debug code
				/*
				char temp[512];
				sprintf(temp, "polygon (%f,%f) (%f,%f) (%f,%f)",
					threadData.m_polygon[0].x, threadData.m_polygon[0].y,
					threadData.m_polygon[1].x, threadData.m_polygon[1].y,
					threadData.m_polygon[2].x, threadData.m_polygon[2].y);
				*/

				// clip polygon by minx
				ClipPolygonByPlane(
					threadData,
					[=] (const SVector2& point) -> bool {return point.x >= (float)ix;},
					[=] (const SVector2& A, const SVector2& B) -> SVector2
					{
						float percent = ((float)ix - A.x) / (B.x - A.x);
						float y = (B.y - A.y) * percent + A.y;
						return SVector2((float)ix,y);
					}
				);

				// clip polygon by maxx
				ClipPolygonByPlane(
					threadData,
					[=] (const SVector2& point) -> bool {return point.x <= (float)(ix+1);},
					[=] (const SVector2& A, const SVector2& B) -> SVector2
					{
						float percent = ((float)(ix+1) - A.x) / (B.x - A.x);
						float y = (B.y - A.y) * percent + A.y;
						return SVector2((float)(ix+1),y);
					}
				);

				// clip polygon by miny
				ClipPolygonByPlane(
					threadData,
					[=] (const SVector2& point) -> bool {return point.y >= (float)iy;},
					[=] (const SVector2& A, const SVector2& B) -> SVector2
					{
						float percent = ((float)iy - A.y) / (B.y - A.y);
						float x = (B.x - A.x) * percent + A.x;
						return SVector2(x,(float)iy);
					}
				);

				// clip polygon by maxy
				ClipPolygonByPlane(
					threadData,
					[=] (const SVector2& point) -> bool {return point.y <= (float)(iy+1);},
					[=] (const SVector2& A, const SVector2& B) -> SVector2
					{
						float percent = ((float)(iy+1) - A.y) / (B.y - A.y);
						float x = (B.x - A.x) * percent + A.x;
						return SVector2(x,(float)(iy+1));
					}
				);

				// todo: test the above! visualize in wolfram alpha or something

				// todo: clip by miny, maxy

				// TODO: clip triangle against pixel to get new shape
				// TODO: get area of shape via ear clipping
				// TODO: multiply by -1 if pixel black, add into triangleTotal.

				// TODO: remove this debug code
				/*
				if (threadData.m_polygon.size() == 4)
				{
					sprintf(temp, "polygon (%f,%f) (%f,%f) (%f,%f) (%f,%f)",
						threadData.m_polygon[0].x, threadData.m_polygon[0].y,
						threadData.m_polygon[1].x, threadData.m_polygon[1].y,
						threadData.m_polygon[2].x, threadData.m_polygon[2].y,
						threadData.m_polygon[3].x, threadData.m_polygon[3].y);
				}
				*/
				int ijkl = 0;
			}
		}

		// return the triangle overlap value
		return triangleTotal;
	}

	//----------------------------------------------------------------------------------------------------------
	float CalcAnnulusOverlapMT (SThreadData& threadData, float distMin, float distMax, float angleMin, float angleMax) const
	{
		// Given an anulus arc, this returns a value representing the count of white pixels minus
		// the total of black pixels, where each pixel is multiplied by it's area of overlap.
		// This is to be able to detect whether the bucket (polar "pixel") should be black or white.
		// It aproximates the annulus arc by breaking it into trapezoids and doing two triangle
		// tests per trapezoid
		/*
		arc length = 2 * pi * radius * arcRadians   = radius * arcRadians
		                              ------------
		                                 2 * pi 
		*/
		const float arcLength = distMax * (angleMax - angleMin);

		// we want to subdivide the arc so that there is as most 1 pixel length of arc for
		// each section.  Since we aproximate the arc with triangles, smaller arc lengths
		// are closer to reality.
		const unsigned int numSubDivisions = (unsigned int)floor(arcLength / 1.0f) +1;
		const float arcAngleDelta = (angleMax-angleMin)/((float)numSubDivisions);

		// for each subdivision...
		float bucketTotal = 0.0f;
		for (unsigned int i = 0; i < numSubDivisions; ++i)
		{
			// calculate the min and max angle of this subdivision
			const float angle1 = angleMin + arcAngleDelta * i;
			const float angle2 = angle1 + arcAngleDelta;

			// calculate the 4 corners of the trapezoid.
			//
			//  B--------C  
			//   \      /
			//    A----D
			//
			//      o
			//
			// A = MinAngleMinDist
			// B = MinAngleMaxDist
			// C = MaxAngleMaxDist
			// D = MaxAngleMinDist

			const float AX = c_centerX + cos(angle1) * distMin;
			const float AY = c_centerY + sin(angle1) * distMin;

			const float BX = c_centerX + cos(angle1) * distMax;
			const float BY = c_centerY + sin(angle1) * distMax;

			const float CX = c_centerX + cos(angle2) * distMax;
			const float CY = c_centerY + sin(angle2) * distMax;

			const float DX = c_centerX + cos(angle2) * distMin;
			const float DY = c_centerY + sin(angle2) * distMin;

			// break the trapezoid into two triangles (A,B,C) and (A,C,D)
			// and add their overlap pixel totals into our bucket totals
			bucketTotal += CalcTriangleOverlapMT(threadData,AX,AY,BX,BY,CX,CY);
			bucketTotal += CalcTriangleOverlapMT(threadData,AX,AY,CX,CY,DX,DY);
		}

		// return the total for this bucket
		return bucketTotal;
	}

	//----------------------------------------------------------------------------------------------------------
	void CalcBucketsOverlapMT (TBucketType& buckets, atomic<TSizeType>& nextBucket) const
	{
		// make our per thread data object
		SThreadData threadData;

		// grab buckets until we are out of range
		auto bucket = ++nextBucket;
		while (bucket < c_bucketCount)
		{
			// calculate the distance range
			auto bucketDistance = bucket & 0xFF;
			float distMin = c_hypotneuse * ((float)bucketDistance) / 256.0f;
			float distMax = c_hypotneuse * ((float)bucketDistance+1) / 256.0f;

			// calculate the angle range
			auto bucketAngle = bucket >> 8;
			float angleMin = ((float)bucketAngle) * c_arcSizeRadians - c_halfArcSizeRadians;
			float angleMax = ((float)bucketAngle) * c_arcSizeRadians + c_halfArcSizeRadians;

			if (bucketDistance == 255)
			{
				int ijkl = 0;
			}

			// generate the bucket
			buckets[bucket] = CalcAnnulusOverlapMT(threadData, distMin, distMax, angleMin, angleMax) > 0.0f;

			// go to the next bucket
			bucket = ++nextBucket;
		}
	}

private:
    const CImageDataBlackWhite& m_src;
    CImageDataRGBA&             m_dest;

	// some constants sharable across threads
	const TSizeType c_bucketCount;
	const TSizeType c_angleCount;

	const float c_centerX;
	const float c_centerY;
	const float c_maxX;
	const float c_maxY;
	const float c_hypotneuse;

	const float c_arcSizeRadians;
	const float c_halfArcSizeRadians;
};

//--------------------------------------------------------------------------------------------------------------
bool Encode (const CImageDataBlackWhite& src, CImageDataRGBA& dest)
{
    // create an encoder and encode our image
    CEncodedPixelData encoder(src, dest);
    return encoder.Encode();
}