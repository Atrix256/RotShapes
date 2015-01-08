#include "Decoder.h"
#include "CImageData.h"

#define _USE_MATH_DEFINES
#include <math.h>

//--------------------------------------------------------------------------------------------------------------
bool Decode (const CImageDataRGBA& src, CImageDataRGBA& dest)
{
	size_t width = dest.GetWidth();
	size_t height = dest.GetHeight();
	size_t stride = dest.GetStride();

	const float centerX = (float)width / 2.0f;
	const float centerY = (float)height / 2.0f;
	const float hypotneuse = sqrtf(centerX*centerX + centerY*centerY);

	std::array<unsigned char, 4> srcPixel;
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
			assert(angle >= 0.0f && angle < (float)src.GetHeight());

			// calculate the distance in a range from 0 to 255 since that is how the distance is encoded
			float distNorm = (sqrt(x*x + y*y) / hypotneuse);
			assert(distNorm >= 0.0f && distNorm <= 255.0f);
			unsigned char dist = (unsigned char)(distNorm*255.0f);

			// get the source (encoded) pixel
			src.GetPixel(0, (size_t)angle, srcPixel);

			// image format is BGRA (defined in Platform::SaveImageFile() by necesity), but we want to encode
			// distances RGBA, so we flip the [2] and [0] index when both reading and writing pixel data
			pixel[3] = 255;
			if (dist < srcPixel[2])
			{
				pixel[2] = 0;
				pixel[1] = 0;
				pixel[0] = 0;
			}
			else if (dist < srcPixel[1])
			{
				pixel[2] = 255;
				pixel[1] = 0;
				pixel[0] = 0;
			}
			else if (dist < srcPixel[0])
			{
				pixel[2] = 0;
				pixel[1] = 255;
				pixel[0] = 0;
			}
			else if (dist < srcPixel[3])
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

			// TODO: the decoded image is screwed up for some reason, need to investigate why! could try loading it in a web
			// decoder and seeing if it decodes bad there too (if so, the encoded image is wrong, else the cpp decopder is wrong!)
			// could also be that something is sampling pixels incorrectly maybe...
			pixel += 4;
		}
		pixels+=stride;
	}

    return true;
}