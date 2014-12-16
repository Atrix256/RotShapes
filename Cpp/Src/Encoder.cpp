#include "Encoder.h"
#include "CImageData.h"
#include <vector>
#include <array>
#include <windows.h>
#include <assert.h>

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

        // TODO: can we parallelize this across worker threads?

        // for each angle
        unsigned int angle = 0;
        for (auto itAngle = m_AngleDistancePixelColors.begin(); itAngle != m_AngleDistancePixelColors.end(); ++itAngle, ++angle) {
            auto distancePixelColors = *itAngle;

            // for each distance of that angle, calculate the color of the pixel
            unsigned int distance = 0;
            for (auto itDist = distancePixelColors.begin(); itDist != distancePixelColors.end(); ++itDist, ++distance)
                (*itDist) = CalculateAngleDistancePixelColor(angle, distance);
        }

        // TODO: do the rest of the encoding

        // return success
        return true;
    }

private:
    bool CalculateAngleDistancePixelColor (unsigned int angle, unsigned int distance)
    {
        //For every pixel that is part of this angle and distance, add one if it is white, and subtract one
        //if it's black.  Possibly calculate area of the pixel overlap with this angle and distance and weight
        // it accordingly.

        // if the end result is positive, it's white, else it's black.

        //TODO: finish this!
        return true;
    }

private:
    vector<array<bool,256>>    m_AngleDistancePixelColors;
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