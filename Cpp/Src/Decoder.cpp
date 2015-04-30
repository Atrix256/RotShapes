#include "Decoder.h"
#include "CImageData.h"
#include "SSettings.h"
#include <vector>
#include <atomic>
#include <thread>


#define _USE_MATH_DEFINES
#include <math.h>

//--------------------------------------------------------------------------------------------------------------
void DecodeRowMT(size_t iy, const CImageDataRGBA& src, float frame, CImageDataRGBA& dest, bool debugColors, const SSettings& settings, float OffsetX, float OffsetY)
{
    std::array<float, 4> srcPixel;

    const size_t width = dest.GetWidth();
    const size_t height = dest.GetHeight();
    const size_t stride = dest.GetStride();

    const float centerX = (float)width / 2.0f;
    const float centerY = (float)height / 2.0f;
    const float maxDist = settings.m_shortDist
        ? (float)std::max(width, height) / 2.0f
        : sqrtf(centerX*centerX + centerY*centerY);

    unsigned char* pixels = dest.GetPixelBuffer() + stride*iy;

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
}

//--------------------------------------------------------------------------------------------------------------
void DecodeInternal (const CImageDataRGBA& src, float frame, CImageDataRGBA& dest, bool debugColors, const SSettings& settings, float OffsetX, float OffsetY)
{
    const size_t width = dest.GetWidth();
    const size_t height = dest.GetHeight();

    // spin up all available cores to help do decoding work.  one work item = one row of decoded data
    std::atomic<size_t> nextRow(static_cast<size_t>(-1));
    auto numThreads = std::thread::hardware_concurrency();
    numThreads = std::max(numThreads, static_cast<decltype(numThreads)>(1));
    std::vector<std::thread> threads;
    for (size_t i = 0, c = numThreads; i < c; ++i)
    {
        threads.push_back(
            std::thread(
                [&]()
                {
                    size_t row = ++nextRow;
                    while (row < height)
                    {
                        DecodeRowMT(row, src, frame, dest, debugColors, settings, OffsetX, OffsetY);
                        row = ++nextRow;
                    };
                }
            )
        );
    }
    for_each(threads.begin(), threads.end(), [](std::thread& t) { t.join(); });

    const float centerX = (float)width / 2.0f;
    const float centerY = (float)height / 2.0f;
    const float maxDist = settings.m_shortDist
        ? (float)std::max(width, height) / 2.0f
        : sqrtf(centerX*centerX + centerY*centerY);

    // draw the radial pixels if we should
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
    // if not doing AA, decode the frame normally
    if (!settings.m_decoding.m_useAA)
    {
        DecodeInternal(src, frame, dest, debugColors, settings, 0.0f, 0.0f);
        return;
    }

    // Do 4-rook anti aliasing!
    CImageDataRGBA decodedOffset[4];
    decodedOffset[0].AllocatePixels(dest.GetWidth(), dest.GetHeight());
    decodedOffset[1].AllocatePixels(dest.GetWidth(), dest.GetHeight());
    decodedOffset[2].AllocatePixels(dest.GetWidth(), dest.GetHeight());
    decodedOffset[3].AllocatePixels(dest.GetWidth(), dest.GetHeight());
    DecodeInternal(src, frame, decodedOffset[0], debugColors, settings,  1.0f / 8.0f,  3.0f / 8.0f);
    DecodeInternal(src, frame, decodedOffset[1], debugColors, settings,  3.0f / 8.0f, -1.0f / 8.0f);
    DecodeInternal(src, frame, decodedOffset[2], debugColors, settings, -1.0f / 8.0f, -3.0f / 8.0f);
    DecodeInternal(src, frame, decodedOffset[3], debugColors, settings, -3.0f / 8.0f,  1.0f / 8.0f);

    // Average the 4 samples per pixel to get the final AA'd image
    for (size_t y = 0, yc = dest.GetHeight(); y < yc; ++y)
    {
        for (size_t x = 0, xc = dest.GetWidth(); x < xc; ++x)
        {
            std::array<float, 4> blendedPixel;
            std::fill(blendedPixel.begin(), blendedPixel.end(), 0.0f);

            for (size_t i = 0; i < 4; ++i)
            {
                std::array<float, 4> offsetPixel;
                decodedOffset[i].GetPixel(x, y, offsetPixel);
                for (size_t pixel = 0; pixel < 4; ++pixel)
                {
                    blendedPixel[pixel] += offsetPixel[pixel] * 0.25f;
                }
            }

            dest.SetPixel(x, y, blendedPixel);
        }
    }
}
