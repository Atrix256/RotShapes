#include <memory.h>
#include <assert.h>
#include <array>
#include <math.h>

using namespace std;

class CImageDataRGBA
{
public:
    CImageDataRGBA () : m_pixels(0), m_width(0), m_height(0), m_stride(0) { }
    ~CImageDataRGBA() { Clear(); }

    void Clear ()
    {
        if (m_pixels)
        {
            delete[] m_pixels;
            m_pixels = 0;
        }

        m_width = 0;
        m_height = 0;
        m_stride = 0;
    }

    // this function takes ownership over the pixels memory and is responsible for freeing it
    void SetPixels (unsigned int width, unsigned int height, unsigned int stride, unsigned char *pixels)
    {
        Clear();
        m_pixels = pixels;
        m_width = width;
        m_height = height;
        m_stride = stride;
    }

    void AllocatePixels (unsigned int width, unsigned int height)
    {
        Clear();
        m_width = width;
        m_height = height;

        unsigned int rowBits = 32 * m_width;
        m_stride = (rowBits / 8) + (rowBits & 8 ? 1 : 0);

        m_pixels = new unsigned char[m_stride * m_height];
        memset(m_pixels, 0, m_stride*m_height);
    }

    unsigned int GetWidth () const { return m_width; }
    unsigned int GetHeight () const { return m_height; }
    unsigned int GetStride () const { return m_stride; }

    unsigned int GetPixelBufferSize() const { return GetStride() * GetHeight(); }
    unsigned char* GetPixelBuffer () const { return m_pixels; }

	void GetPixel(size_t x, size_t y, array<float, 4>& pixel) const
	{
		// mod x,y by width, height to do wrap texture mode
		unsigned char *pixelPointer = GetPixelBuffer() + (y%GetHeight()) * GetStride() + (x%GetWidth())*4;
		pixel[0] = (float)pixelPointer[0];
		pixel[1] = (float)pixelPointer[1];
		pixel[2] = (float)pixelPointer[2];
		pixel[3] = (float)pixelPointer[3];
	}

	void GetPixelBilinear(float x, float y, array<float, 4>& pixel) const
	{
		float fractX = x - floor(x);
		float fractY = y - floor(y);
		float fractXOpp = 1.0f - fractX;
		float fractYOpp = 1.0f - fractY;

		size_t y1 = (size_t)floor(y);
		size_t y2 = y1 + 1;

		size_t x1 = (size_t)floor(x);
		size_t x2 = x1 + 1;

		array<float, 4> p1;
		array<float, 4> p2;
		array<float, 4> p3;
		array<float, 4> p4;

		GetPixel(x1,y1,p1);
		GetPixel(x2,y1,p2);
		GetPixel(x1,y2,p3);
		GetPixel(x2,y2,p4);

		pixel[0] = (p1[0] * fractXOpp + p2[0] * fractX) * fractYOpp + (p3[0] * fractXOpp + p4[0] * fractX) * fractY;
		pixel[1] = (p1[1] * fractXOpp + p2[1] * fractX) * fractYOpp + (p3[1] * fractXOpp + p4[1] * fractX) * fractY;
		pixel[2] = (p1[2] * fractXOpp + p2[2] * fractX) * fractYOpp + (p3[2] * fractXOpp + p4[2] * fractX) * fractY;
		pixel[3] = (p1[3] * fractXOpp + p2[3] * fractX) * fractYOpp + (p3[3] * fractXOpp + p4[3] * fractX) * fractY;
	}

	void GetPixelSmart(float x, float y, array<float, 4>& pixel) const
	{
		// TODO: re-arrange the math in GetPixelBilinear to match the math below for easier readability etc (flip order of X, Y multiplies in blend!)
		// always do bilinear filtering on x axis (time) but only do filtering on y axis (angle) if the distances aren't too large!
		// do that determination per channel.
		float fractX = x - floor(x);
		float fractY = y - floor(y);
		float fractXOpp = 1.0f - fractX;
		float fractYOpp = 1.0f - fractY;

		size_t y1 = (size_t)floor(y);
		size_t y2 = y1 + 1;

		size_t x1 = (size_t)floor(x);
		size_t x2 = x1 + 1;

		array<float, 4> p1;
		array<float, 4> p2;
		array<float, 4> p3;
		array<float, 4> p4;

		GetPixel(x1, y1, p1);
		GetPixel(x2, y1, p2);
		GetPixel(x1, y2, p3);
		GetPixel(x2, y2, p4);

		// this lambda will smart blend two vertical (angular difference) pixels
		auto lambda = [](array<float, 4>& pixel, const array<float, 4>& a, const array<float, 4>& b, float weightA, float weightB)
		{
			// for each channel
			for (int i = 0; i < 4; ++i)
			{
				// TODO: play around with thresholds, or expose as a parameter
				// TODO: try rejecting the pixel outright if any channels are rejected!
				// TODO: may want to take more pixel samples to fit a curve or something?
				float dist = abs((float)a[i] - (float)b[i]);
				if (dist > 10.0f)
				{
					if (weightA > 0.5f)
						pixel[i] = a[i];
					else
						pixel[i] = b[i];
				}
				else
					pixel[i] = (float)a[i] * weightA + (float)b[i] * weightB;
			}
		};

		// smart blend the lower x valued pixels
		array<float, 4> left;
		lambda(left, p1, p3, fractYOpp, fractY);

		// smart blend the higher x valued pixels
		array<float, 4> right;
		lambda(right, p2, p4, fractYOpp, fractY);

		// linear blend across the x axis
		pixel[0] = left[0] * fractXOpp + right[0] * fractX;
		pixel[1] = left[1] * fractXOpp + right[1] * fractX;
		pixel[2] = left[2] * fractXOpp + right[2] * fractX;
		pixel[3] = left[3] * fractXOpp + right[3] * fractX;
	}

	void DrawPixel (int x, int y, unsigned int color)
	{
		// TODO: put this up by getpixel if it's staying around. maybe don't do texture wrap?
		// mod x,y by width, height to do wrap texture mode
		unsigned int *pixelPointer = (unsigned int*)(GetPixelBuffer() + (y%GetHeight()) * GetStride() + (x%GetWidth())*4);
		pixelPointer[0] = color;
	}

	void DrawLine (int x1, int y1, int x2, int y2, unsigned int color)
	{
		// TODO: other octants, bounds checking, pixel pointer more friendly pixel writes.
		static_assert(sizeof(color) == 4, "wrongly assuming sizeof(unsigned int) is 32 bits!");

		int dx = x2 - x1;
		int dy = y2 - y1;

		int D = 2*dy-dx;
		int y = y1;
		
		DrawPixel(x1, y1, color);
		for (int x = x1+1; x < x2; ++x)
		{
			if (D > 0)
			{
				y = y+1;
				D += (2*dy-2*dx);
			}
			else
			{
				D += 2*dy;
			}
			DrawPixel(x,y,color);
		}

		/*
		plotLine(x0,y0, x1,y1)
		  dx=x1-x0
		  dy=y1-y0

		  D = 2*dy - dx
		  plot(x0,y0)
		  y=y0

		  for x from x0+1 to x1
			if D > 0
			  y = y+1
			  plot(x,y)
			  D = D + (2*dy-2*dx)
			else
			  plot(x,y)
			  D = D + (2*dy)*/
	}

private:
    unsigned char *m_pixels;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
};
