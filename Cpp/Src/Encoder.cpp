#include "Encoder.h"
#include "CImageData.h"
#include <vector>
#include <array>
#include <windows.h>
#include <assert.h>
#include <ppl.h>

using namespace std;

//--------------------------------------------------------------------------------------------------------------
class CEncodedPixelData
{
public:
    CEncodedPixelData (const CImageDataBlackWhite& src, CImageDataRGBA& dest)
        : m_src(src)
        , m_dest(dest)
    {
        assert(dest.GetWidth() == 1);
        m_AngleDistancePixelColors.resize(dest.GetHeight());
    }

    bool Encode ()
    {
        // TODO: TEMP
        memset(m_dest.GetPixelBuffer(), 128, m_dest.GetPixelBufferSize());

        // for each angle, parallelized across worker threads
        Concurrency::parallel_for(
            (size_t)0,
            m_AngleDistancePixelColors.size(), 
            [=] (size_t angle) {
                array<bool,256> &distancePixelColors = m_AngleDistancePixelColors[angle];

                // for each distance of that angle, calculate the color of the pixel
                unsigned int distance = 0;
                for (auto itDist = distancePixelColors.begin(); itDist != distancePixelColors.end(); ++itDist, ++distance)
                    (*itDist) = CalculateAngleDistancePixelColor(angle, distance);
            }
        );

        // TODO: do the rest of the encoding

        // return success
        return true;
    }

private:

    //bool LineIntersectsSquare ()
    //{
    //}

    float PixelValueInCone (unsigned int x, unsigned int y, unsigned int angle, unsigned int distance) const
    {
        // a pixel that is not in the cone will return 0
        // else it will return 1 if the pixel is in the cone and is white
        // else it will return -1 if the pixel is in the cone and is black
        // TODO: we could calculate the area of the pixel inside the cone and multiply the return value by that

        // TODO: make the 2 lines of the cone and see if they intersect with the square? or need to contain it sadly, so intersection won't work.

        // TODO: maybe instead of looping by angle, distance, each pixel... instead loop through each pixel and...
        //   1) find the angle(s) it's part of (how?)
        //   2) find the min and max distance of that pixel (in 0-256 space). going by corners (or coners of intersected shape? sigh). need to make sure this is right.
        //   3) add or subtract 1 depending on black / white.
        //   4) when done with all pixels, convert floats to bools using >= 0 heuristic
        //   5) continue with encoding

        // TODO: finish this
        return 1.0f;
    }

    bool CalculateAngleDistancePixelColor (unsigned int angle, unsigned int distance) const
    {
        // TODO: yes, having a quadruple nested for loop is pretty bad for perf.  fix it up later.

        //For every pixel that is part of this angle and distance, add one if it is white, and subtract one
        //if it's black.  Possibly calculate area of the pixel overlap with this angle and distance and weight
        // it accordingly.
        float total = 0.0f;
        for (unsigned int y = 0, height = m_src.GetHeight(); y < height; ++y)
            for (unsigned int x = 0, width = m_src.GetWidth(); x < width; ++x)
                total += PixelValueInCone(y,x,angle,distance);

        // if the end result is positive, it's white, else it's black.
        return total >= 0.0f;
    }

private:
    vector<array<bool,256>>     m_AngleDistancePixelColors;
    const CImageDataBlackWhite& m_src;
    CImageDataRGBA&             m_dest;
};

//--------------------------------------------------------------------------------------------------------------
bool Encode (const CImageDataBlackWhite& src, CImageDataRGBA& dest)
{
    // create an encoder and encode our image
    CEncodedPixelData encoder(src, dest);
    return encoder.Encode();
}