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

	// TODO: option for bilinear filtering
	// TODO: option for hypotneuse vs not

	float centerX = (float)width / 2.0f;
	float centerY = (float)height / 2.0f;

	// TODO: i think hypotneuse should be half of the width and height distance! fix the other hypotneuse place in encoder.
	float hypotneuse = sqrtf(centerX*centerX+centerY*centerY);

	unsigned char* pixels = dest.GetPixelBuffer();
	for (size_t iy = 0; iy < height; ++iy)
	{
		float y = (float)iy - centerY;
		for (size_t ix = 0; ix < width; ++ix)
		{
			float x = (float)ix - centerX;
			float angle = atan2(y,x) / ((float)M_PI*2.0f);
			if (angle < 0.0f)
				angle += 1.0f;
			float dist = sqrt(x*x+y*y) / hypotneuse;
			int ijkl = 0;
			// TODO: better pointer traversal!
			// TODO: finish this!
			// TODO: sample the src pixel for this angle and distance and see if it should be white or black, and write the pixel as is appropriate
			pixels[ix*4+2] = 255;
			pixels[ix*4+1] = 255;
			pixels[ix*4+0] = 255;
			pixels[ix*4+3] = 255;
		}
		pixels+=stride;
	}

    return true;
}