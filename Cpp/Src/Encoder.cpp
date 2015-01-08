#include "Encoder.h"
#include "CImageData.h"
#include <vector>
#include <array>
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
static bool operator == (const SVector2& A, const SVector2& B)
{
	return A.x == B.x && A.y == B.y;
}

//--------------------------------------------------------------------------------------------------------------
static float TriArea (const SVector2& A, const SVector2& B, const SVector2& C)
{
	return abs((A.x - C.x) * (B.y - C.y) - (A.y - C.y) * (B.x - C.x))*0.5f;
}

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
		, c_radialPixelCount(dest.GetHeight() << 8)
		, c_angleCount(dest.GetHeight())
		, c_centerX((float)src.GetWidth() / 2.0f)
		, c_centerY((float)src.GetHeight() / 2.0f)
		, c_maxX(src.GetWidth()-1.0f)
		, c_maxY(src.GetHeight()-1.0f)
		, c_hypotneuse(sqrtf(c_centerX*c_centerX+c_centerY*c_centerY))
		, c_arcSizeRadians(((float)M_PI*2.0f)/((float)c_angleCount))
		, c_halfArcSizeRadians(((float)M_PI)/((float)c_angleCount))
    {
        assert(dest.GetWidth() == 1);
    }

	typedef vector<bool>			TRadialPixels;
	typedef vector<unsigned char>	TAngleRange;
	typedef vector<TAngleRange>		TAngleRanges;

	//----------------------------------------------------------------------------------------------------------
    bool Encode ()
	{
		// make a thread slot for each core we have available, making sure to at least have 1 thread
		auto numThreads = thread::hardware_concurrency();
		numThreads = max(numThreads, static_cast<decltype(numThreads)>(1));
		vector<thread> threads(numThreads);

		// make some storage for our radial pixels and angle ranges
		TRadialPixels radialPixels(c_radialPixelCount);
		TAngleRanges angleRanges(c_angleCount);

		// Calculate our radial pixels using as many threads as we have cores for and wait for them to finish
		{
			atomic<size_t> nextPixel(static_cast<size_t>(-1));
			for_each(threads.begin(), threads.end(), [&] (thread& t)
				{
					t = thread([&] () { CalcRadialPixelsMT(radialPixels, nextPixel); });
				}
			);
			for_each(threads.begin(), threads.end(), [] (thread& t) { t.join(); });
		}

		// we now have the color of each radial pixel, we need to use those to make black/white distances for
		// each angle, so that we can encode those distances as R,G,B,A
		{
			atomic<size_t> nextAngle(static_cast<size_t>(-1));
			for_each(threads.begin(), threads.end(), [&](thread& t)
				{
				t = thread([&]() { CalcAngleRangesMT(radialPixels, angleRanges, nextAngle); });
				}
			);
			for_each(threads.begin(), threads.end(), [](thread& t) { t.join(); });
		}

		// image format is BGRA (defined in Platform::SaveImageFile() by necesity), but we want to encode
		// distances RGBA, so we flip the [2] and [0] index
		unsigned char *pixelBuffer = m_dest.GetPixelBuffer();
		size_t stride = m_dest.GetStride();
		for_each(angleRanges.begin(), angleRanges.end(), [&pixelBuffer,stride] (const TAngleRange& range) {
				pixelBuffer[2] = range.size() > 1 ? range[1] : 255;
				pixelBuffer[1] = range.size() > 2 ? range[2] : pixelBuffer[2];
				pixelBuffer[0] = range.size() > 3 ? range[3] : pixelBuffer[1];
				pixelBuffer[3] = range.size() > 4 ? range[4] : pixelBuffer[0];
				pixelBuffer += stride;
			}
		);

        // return success
        return true;
    }

private:

	//----------------------------------------------------------------------------------------------------------
	void RemoveShortestRange (TAngleRange& range)
	{
		// find the shortest boundary that isn't the first black to white, or the last white to black
		size_t shortestLengthIndex = -1;
		unsigned char shortestLength = 255;
		for (size_t i = 2; i < range.size() - 1; ++i)
		{
			unsigned char length = range[i] - range[i - 1];

			if (length < shortestLength)
			{
				shortestLengthIndex = i;
				shortestLength = length;
			}
		}

		// remove the shortest range found.  That means we need to erase it and the next item to merge the last
		// range with the next range.
		assert(shortestLengthIndex != -1);
		range.erase(range.begin() + shortestLengthIndex - 1, range.begin() + shortestLengthIndex + 1);
	}

	//----------------------------------------------------------------------------------------------------------
	void CalcAngleRangesMT (const TRadialPixels &pixels, TAngleRanges &angles, atomic<size_t> &nextAngle)
	{
		// grab angles until we are out of range
		auto angle = ++nextAngle;
		while (angle < c_angleCount)
		{
			// get our angle range
			TAngleRange &range = angles[angle];
			
			// start with an explicit black value at the start
			range.clear();
			range.push_back(0);

			// loop through all distances in this angle, making a list of where the colors change
			bool white = false;
			size_t base = angle << 8;
			for (size_t dist = 0; dist < 256; ++dist)
			{
				if (pixels[base + dist] != white)
				{
					range.push_back(dist);
					white = !white;
				}				
			}

			// make sure there is a black value at the end
			if (white)
				range.push_back(255);

			// cut out the smallest ranges until we have 5 boundaries or fewer
			while (range.size() > 5)
				RemoveShortestRange(range);

			// go to the next pixel
			angle = ++nextAngle;
		}
	}

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

				// remove redundant points in the polygon
				for (auto i = threadData.m_polygon.size(); i > 1; --i)
				{
					if (threadData.m_polygon[i-1] == threadData.m_polygon[i-2])
						threadData.m_polygon.erase(threadData.m_polygon.begin()+(i-1));
				}

				// black pixels subtract from the total, white pixels add into the total
				float multiplier = m_src.GetPixel(ix, iy) ? 1.0f : -1.0f;

				// add area of polygon into triangleTotal, using ear clipping.  The polygon is garaunteed convex since it's
				// a triangle clipped to a square.  Also garaunteed to be in clockwise order.
				// If we have points 0...N, our first ear clipping triangle is points 0,1,2.
				// We'd then remove point 1 and handle the next triangle: 0,2,3
				// We'd then remove point 2 and handle the next triangle: 0,3,4
				// We simulate that without actually modifying the polygon array
				for (size_t i = 2, c = threadData.m_polygon.size(); i < c; ++i)
				{
					triangleTotal +=
						TriArea(threadData.m_polygon[0], threadData.m_polygon[i-1], threadData.m_polygon[i])
						* multiplier;
				}
			}
		}

		// return the triangle overlap value
		return triangleTotal;
	}

	//----------------------------------------------------------------------------------------------------------
	float CalcAnnulusOverlapMT (SThreadData& threadData, float distMin, float distMax, float angleMin, float angleMax) const
	{
		// Given an annulus arc, this returns a value representing the count of white pixels minus
		// the total of black pixels, where each pixel is multiplied by it's area of overlap.
		// This is to be able to detect whether the radial pixel should be black or white.
		// It approximates the annulus arc by breaking it into trapezoids and doing two triangle
		// tests per trapezoid
		/*
		arc length = 2 * pi * radius * arcRadians   = radius * arcRadians
		                              ------------
		                                 2 * pi 
		*/
		const float arcLength = distMax * (angleMax - angleMin);

		// we want to subdivide the arc so that there is as most 1 pixel length of arc for
		// each section.  Since we approximate the arc with triangles, smaller arc lengths
		// are closer to reality.
		const unsigned int numSubDivisions = (unsigned int)floor(arcLength / 1.0f) +1;
		const float arcAngleDelta = (angleMax-angleMin)/((float)numSubDivisions);

		// for each subdivision...
		float arcTotal = 0.0f;
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
			// and add their overlap pixel totals into our arc totals
			arcTotal += CalcTriangleOverlapMT(threadData,AX,AY,BX,BY,CX,CY);
			arcTotal += CalcTriangleOverlapMT(threadData,AX,AY,CX,CY,DX,DY);
		}

		// return the total for this arc
		return arcTotal;
	}

	//----------------------------------------------------------------------------------------------------------
	void CalcRadialPixelsMT (TRadialPixels& pixels, atomic<size_t>& nextPixel) const
	{
		// make our per thread data object
		SThreadData threadData;

		// grab pixels until we are out of range
		auto pixel = ++nextPixel;
		while (pixel < c_radialPixelCount)
		{
			// calculate the distance range
			auto pixelDistance = pixel & 0xFF;
			float distMin = c_hypotneuse * ((float)pixelDistance) / 256.0f;
			float distMax = c_hypotneuse * ((float)pixelDistance+1) / 256.0f;

			// calculate the angle range
			auto pixelAngle = pixel >> 8;
			float angleMin = ((float)pixelAngle) * c_arcSizeRadians - c_halfArcSizeRadians;
			float angleMax = ((float)pixelAngle) * c_arcSizeRadians + c_halfArcSizeRadians;

			// generate the pixel
			pixels[pixel] = CalcAnnulusOverlapMT(threadData, distMin, distMax, angleMin, angleMax) > 0.0f;

			// go to the next pixel
			pixel = ++nextPixel;
		}
	}

private:
    const CImageDataBlackWhite& m_src;
    CImageDataRGBA&             m_dest;

	// some constants sharable across threads
	const size_t c_radialPixelCount;
	const size_t c_angleCount;

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