#include "Decoder.h"
#include "CImageData.h"
#include "SSettings.h"
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

//--------------------------------------------------------------------------------------------------------------
void DecodeInternal (const CImageDataRGBA& src, float frame, CImageDataRGBA& dest, bool debugColors, const SSettings& settings, float OffsetX, float OffsetY)
{
    size_t width = dest.GetWidth();
    size_t height = dest.GetHeight();
    size_t stride = dest.GetStride();

    const float centerX = (float)width / 2.0f;
    const float centerY = (float)height / 2.0f;
    const float maxDist = settings.m_shortDist
        ? (float)std::max(width, height) / 2.0f
        : sqrtf(centerX*centerX + centerY*centerY);

    std::array<float, 4> srcPixel;
    unsigned char* pixels = dest.GetPixelBuffer();
    for (size_t iy = 0; iy < height; ++iy)
    {
        float y = OffsetY + (float)iy - centerY;
        unsigned char* pixel = pixels;
        for (size_t ix = 0; ix < width; ++ix)
        {
            float x = OffsetX + (float)ix - centerX;

            // calculate the angle, as the y coordinate to sample the source data from
            float angle = atan2(y,x) / ((float)M_PI*2.0f);
            if (angle < 0.0f)
                angle += 1.0f;
            angle *= src.GetHeight();

            // calculate the distance in a range from 0 to 255 since that is how the distance is encoded
            float distNorm = (sqrt(x*x + y*y) / maxDist);
            if (settings.m_sqDist)
                distNorm *= distNorm;

            if (distNorm >= 1.0f)
                distNorm = 1.0f;

            float dist = distNorm*255.0f;

            // get the source (encoded) pixel
            switch (settings.m_decoding.m_textureFilter)
            {
                case ETextureFilter::e_filterNone:
                {
                    // add half a pixel to do proper rounding when not using filtering
                    src.GetPixelVanilla(frame, angle+0.5f, srcPixel);
                    break;
                }
                case ETextureFilter::e_filterBilinear:
                {
                    src.GetPixelBilinear(frame, angle, srcPixel);
                    break;
                }
                case ETextureFilter::e_filterSmart:
                {
                    src.GetPixelSmart(frame, angle, srcPixel);
                    break;
                }
                default:assert(false);
            }

            // image format is BGRA (defined in Platform::SaveImageFile() by necesity), but we want to encode
            // distances RGBA, so we flip the [2] and [0] index when both reading and writing pixel data
            pixel[3] = 255;
            if (dist < srcPixel[2])
            {
                if (debugColors)
                {
                    pixel[2] = 0;
                    pixel[1] = 0;
                    pixel[0] = 0;
                }
                else
                {
                    pixel[2] = 0;
                    pixel[1] = 0;
                    pixel[0] = 0;
                }
            }
            else if (dist < srcPixel[1])
            {
                if (debugColors)
                {
                    pixel[2] = 128;
                    pixel[1] = 0;
                    pixel[0] = 0;
                }
                else
                {
                    pixel[2] = 255;
                    pixel[1] = 255;
                    pixel[0] = 255;
                }
            }
            else if (dist < srcPixel[0])
            {
                if (debugColors)
                {
                    pixel[2] = 0;
                    pixel[1] = 128;
                    pixel[0] = 0;
                }
                else
                {
                    pixel[2] = 0;
                    pixel[1] = 0;
                    pixel[0] = 0;
                }
            }
            else if (dist < srcPixel[3])
            {
                if (debugColors)
                {
                    pixel[2] = 0;
                    pixel[1] = 0;
                    pixel[0] = 128;
                }
                else
                {
                    pixel[2] = 255;
                    pixel[1] = 255;
                    pixel[0] = 255;
                }
            }
            else
            {
                if (debugColors)
                {
                    pixel[2] = 128;
                    pixel[1] = 128;
                    pixel[0] = 128;
                }
                else
                {
                    pixel[2] = 0;
                    pixel[1] = 0;
                    pixel[0] = 0;
                }
            }

            pixel += 4;
        }
        pixels+=stride;
    }

    if (settings.m_decoding.m_showRadialPixels)
    {
        // draw each line for the angles
        int middlex = dest.GetWidth() / 2;
        int middley = dest.GetHeight() / 2;
        for (int i = src.GetHeight(); i >= 0; --i)
        {
            float angle = 2.0f*(float)M_PI*((float)(i+0.5f) / (float)(src.GetHeight()));
            int x = (int)(cos(angle)*maxDist);
            int y = (int)(sin(angle)*maxDist);
            dest.DrawLineClip(middlex, middley, middlex+x, middley+y, 0xFFFFFF00);
        }

        // draw the circles for the distances
        for (int i = 1; i <= 16; ++i)
        {
            float distNorm = ((float)i*16.0f)/256.0f;
            if (settings.m_sqDist)
                distNorm *= distNorm;
            int radius = (int)(maxDist*distNorm);
            dest.DrawCircleClip(middlex,middley,radius,0xFFFFFF00);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------
void Decode (const CImageDataRGBA& src, float frame, CImageDataRGBA& dest, bool debugColors, const SSettings& settings)
{
    // decode the frame
    DecodeInternal(src, frame, dest, debugColors, settings, 0.0f, 0.0f);

    // if not doing AA, we are done!
    if (!settings.m_decoding.m_useAA)
        return;

    // if we are doing AA, we are doing supersampling with a quincunx pattern, so we need to decode again
    // with half a pixel offset.

    // allocate pixels for all our samples
    CImageDataRGBA destOffset;
    destOffset.AllocatePixels(dest.GetWidth(), dest.GetHeight());
    DecodeInternal(src, frame, destOffset, debugColors, settings, 0.5f, 0.5f);

    // now, we need to combine the data from the two buffers quincunx style
    for (size_t y = 0, yc = dest.GetHeight(); y < yc; ++y)
    {
        for (size_t x = 0, xc = dest.GetWidth(); x < xc; ++x)
        {
            // get the center pixel
            std::array<float, 4> centerPixel;
            dest.GetPixel(x, y, centerPixel);

            // get the corner pixels
            std::array<std::array<float, 4>, 4> cornerPixels;
            if (x > 0)
                destOffset.GetPixel(x - 1, y, cornerPixels[0]);
            else
                cornerPixels[0] = centerPixel;

            if (y > 0)
                destOffset.GetPixel(x, y - 1, cornerPixels[1]);
            else
                cornerPixels[1] = centerPixel;

            if (x > 0 && y > 0)
                destOffset.GetPixel(x - 1, y - 1, cornerPixels[2]);
            else
                cornerPixels[2] = centerPixel;

            destOffset.GetPixel(x, y, cornerPixels[3]);

            // combine the center and corner pixels
            std::array<float, 4> blendedPixel;
            for (size_t i = 0; i < 4; ++i)
            {
                blendedPixel[i] = centerPixel[i] * 0.5f;
                std::for_each(
                    cornerPixels.begin(),
                    cornerPixels.end(),
                    [&](const std::array<float, 4>& cornerPixel)
                    {
                        blendedPixel[i] += cornerPixel[i] / 8.0f;
                    }
                );
            }

            //write the combined pixel out
            dest.SetPixel(x, y, blendedPixel);
        }
    }
}
