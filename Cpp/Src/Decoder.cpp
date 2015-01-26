#include "Decoder.h"
#include "CImageData.h"
#include "SSettings.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

//--------------------------------------------------------------------------------------------------------------
float DistanceFromPointToLine(float AX, float AY, float BX, float BY, float PX, float PY)
{
	// given a line defined by A and B, this tells you how far away the point P is from that line
	// from http://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line

	return abs((BY - AY)*PX - (BX - AX)*PY + BX*AY - BY*AX) /
			sqrt((BY-AY)*(BY-AY)+(BX-AX)*(BX-AX));
}

//--------------------------------------------------------------------------------------------------------------
float SmoothStep (float value, float a, float b)
{
	if (value < a)
		return 0.0f;
	else if (value > b)
		return 1.0f;

	float t = (value-a) / (b-a);
	return 3*t*t-2*t*t*t;
}

//--------------------------------------------------------------------------------------------------------------
void Decode (const CImageDataRGBA& src, float frame, CImageDataRGBA& dest, bool debugColors, const SSettings& settings)
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
	array<float, 4> srcPixelSlope;  // used for -smoothstepgradient
	const float h = -0.01f;
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

			bool distTooFar = false;
			if (distNorm >= 1.0f)
			{
				distTooFar = true;
				distNorm = 1.0f;
			}
			float dist = distNorm*255.0f;

			// get the source (encoded) pixel
			switch (settings.m_decoding.m_textureFilter)
			{
				case ETextureFilter::e_filterNone:
				{
					// add half a pixel to do proper rounding when not using filtering
					src.GetPixelVanilla(frame, angle+0.5f, srcPixel);

					// assume flat slope when not doing any filtering
					srcPixelSlope[0] = 0.0f;
					srcPixelSlope[1] = 0.0f;
					srcPixelSlope[2] = 0.0f;
					srcPixelSlope[3] = 0.0f;
					break;
				}
				case ETextureFilter::e_filterBilinear:
				{
					src.GetPixelBilinear(frame, angle, srcPixel);
					src.GetPixelBilinear(frame, angle + h, srcPixelSlope);
					break;
				}
				case ETextureFilter::e_filterSmart:
				{
					src.GetPixelSmart(frame, angle, srcPixel);
					src.GetPixelSmart(frame, angle + h, srcPixelSlope);
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
					unsigned char value = 255;
					if (settings.m_decoding.m_AAMethod != EAAMethod::e_AANone)
					{
						float distance = (settings.m_decoding.m_AAMethod == EAAMethod::e_AASmoothStepGradient)
							? DistanceFromPointToLine(angle, srcPixel[2], angle + h, srcPixelSlope[2], angle, dist)
							: dist - srcPixel[2];

						value = (unsigned char)(255.0f*SmoothStep(distance, 0, settings.m_decoding.m_AAParam*255.0f));
					}
						
					pixel[2] = value;
					pixel[1] = value;
					pixel[0] = value;
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
					unsigned char value = 0;
					if (settings.m_decoding.m_AAMethod != EAAMethod::e_AANone)
					{
						float distance = (settings.m_decoding.m_AAMethod == EAAMethod::e_AASmoothStepGradient)
							? DistanceFromPointToLine(angle, srcPixel[1], angle + h, srcPixelSlope[1], angle, dist)
							: dist - srcPixel[1];

						value = (unsigned char)(255.0f*(1.0f - SmoothStep(distance, 0, settings.m_decoding.m_AAParam*255.0f)));
					}

					pixel[2] = value;
					pixel[1] = value;
					pixel[0] = value;
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
					unsigned char value = 255;
					if (settings.m_decoding.m_AAMethod != EAAMethod::e_AANone)
					{
						float distance = (settings.m_decoding.m_AAMethod == EAAMethod::e_AASmoothStepGradient)
							? DistanceFromPointToLine(angle, srcPixel[0], angle + h, srcPixelSlope[0], angle, dist)
							: dist - srcPixel[0];

						value = (unsigned char)(255.0f*SmoothStep(distance, 0, settings.m_decoding.m_AAParam*255.0f));
					}

					pixel[2] = value;
					pixel[1] = value;
					pixel[0] = value;
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
					unsigned char value = 0;
					if (settings.m_decoding.m_AAMethod != EAAMethod::e_AANone)
					{
						float distance = (settings.m_decoding.m_AAMethod == EAAMethod::e_AASmoothStepGradient)
							? DistanceFromPointToLine(angle, srcPixel[3], angle + h, srcPixelSlope[3], angle, dist)
							: dist - srcPixel[3];

						value = (unsigned char)(255.0f*(1.0f - SmoothStep(distance, 0, settings.m_decoding.m_AAParam*255.0f)));
					}

					pixel[2] = value;
					pixel[1] = value;
					pixel[0] = value;
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