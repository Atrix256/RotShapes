#include "Decoder.h"
#include "CImageData.h"
#include "SSettings.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

//--------------------------------------------------------------------------------------------------------------
bool Decode (const CImageDataRGBA& src, CImageDataRGBA& dest, bool debugColors, const SSettings& settings)
{
	size_t width = dest.GetWidth();
	size_t height = dest.GetHeight();
	size_t stride = dest.GetStride();

	const float centerX = (float)width / 2.0f;
	const float centerY = (float)height / 2.0f;
	const float maxDist = settings.m_shortDist
		? (float)max(width, height)/2.0f
		: sqrtf(centerX*centerX + centerY*centerY);

	array<float, 4> srcPixel;
	unsigned char* pixels = dest.GetPixelBuffer();
	for (size_t iy = 0; iy < height; ++iy)
	{
		float y = (float)iy - centerY;
		unsigned char* pixel = pixels;
		for (size_t ix = 0; ix < width; ++ix)
		{
			float x = (float)ix - centerX;

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
					src.GetPixel(0, (size_t)(angle+0.5f), srcPixel);
					break;
				}
				case ETextureFilter::e_filterBilinear:
				{
					src.GetPixelBilinear(0.0f, angle, srcPixel);
					break;
				}
				case ETextureFilter::e_filterSmart:
				{
					src.GetPixelSmart(0.0f, angle, srcPixel);
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
					pixel[2] = 255;
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
					pixel[1] = 255;
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
					pixel[0] = 255;
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
					pixel[2] = 255;
					pixel[1] = 255;
					pixel[0] = 255;
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

    return true;
}