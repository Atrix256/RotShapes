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

	//--------------------------------------------------------------------------------------------------------------
	// PIXEL READS : get a float[4] with values 0.0f-255.0f
	//--------------------------------------------------------------------------------------------------------------
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
		size_t y1 = (size_t)floor(y);
		size_t x1 = (size_t)floor(x);

		// TODO: may have GetPixel return an rvalue reference perhaps?? don't need temporaries

		// blend the left pixel
		array<float, 4> left;
		{
			// get our 2 pixels
			array<float, 4> p1;
			array<float, 4> p2;
			GetPixel(x1, y1, p1);
			GetPixel(x1, y1+1, p2);

			// TODO: why does this seem backwards?
			PixelBlendLinear(p2, p1, left, fractY);
		}

		// blend the right pixel
		array<float, 4> right;
		{
			// get our 2 pixels
			array<float, 4> p1;
			array<float, 4> p2;
			GetPixel(x1+1, y1, p1);
			GetPixel(x1+1, y1 + 1, p2);

			// TODO: why does this seem backwards?
			PixelBlendLinear(p2, p1, right, fractY);
		}

		// TODO: why does this seem backwards?
		PixelBlendLinear(right, left, pixel, fractX);
	}

	void GetPixelSmart(float x, float y, array<float, 4>& pixel) const
	{
		// TODO: combine with the function above somehow, even if just internally they use the same function (with template param lambda passed in), but the api is split. maybe template enum value to specify which blend to use?
		// TODO: re-arrange the math in GetPixelBilinear to match the math below for easier readability etc (flip order of X, Y multiplies in blend!)
		// always do bilinear filtering on x axis (time) but only do filtering on y axis (angle) if the distances aren't too large!
		// do that determination per channel.
		// TODO: we may actually want smart filtering on x axis too... need to test and see
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

		// TODO: try rejecting a pixel if any of the channels are "bad"
		// TODO: try NOT rejecting the full pixel, but just the "bad" parts of it?
		// this lambda will smart blend two vertical (angular difference) pixels
		auto lambda = [](array<float, 4>& pixel, const array<float, 4>& a, const array<float, 4>& b, float weightA, float weightB)
		{
			// for each channel
			for (int i = 0; i < a._EEN_SIZE; ++i)
			{
				// TODO: play around with thresholds, or expose as a parameter
				float dist = abs((float)a[i] - (float)b[i]);
				if (dist > 20.0f)
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
	
	static void PixelBlendLinear(const array<float, 4>& a, const array<float, 4>& b, array<float, 4>& c, float weightA)
	{
		for (int i = 0; i < a._EEN_SIZE; ++i)
		{
			c[i] = a[i] * weightA + b[i] * (1.0f-weightA);
		}
	}

	//--------------------------------------------------------------------------------------------------------------
	// PIXEL WRITES: unsigned int 0xAARRGGBB
	//--------------------------------------------------------------------------------------------------------------
	void DrawPixelClip (int x, int y, unsigned int color)
	{
		if (x < 0 || y < 0 || x >= (int)GetWidth() || y >= (int)GetHeight())
			return;

		((unsigned int*)GetPixelBuffer())[y*GetWidth()+x]=color;
	}

	void DrawCircleClip (int xm, int ym, int r, unsigned int color)
	{
	   int x = -r, y = 0, err = 2-2*r; /* II. Quadrant */ 
	   do {
		  DrawPixelClip(xm-x, ym+y, color); /*   I. Quadrant */
		  DrawPixelClip(xm-y, ym-x, color); /*  II. Quadrant */
		  DrawPixelClip(xm+x, ym-y, color); /* III. Quadrant */
		  DrawPixelClip(xm+y, ym+x, color); /*  IV. Quadrant */
		  r = err;
		  if (r <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
		  if (r > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
	   } while (x < 0);
	}

	void DrawLineClip (int x0, int y0, int x1, int y1, unsigned int color)
	{
	   int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
	   int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	   int err = dx+dy, e2; /* error value e_xy */
 
	   for(;;){  /* loop */
		  DrawPixelClip(x0,y0,color);
		  if (x0==x1 && y0==y1) break;
		  e2 = 2*err;
		  if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		  if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	   }
	}

private:
    unsigned char *m_pixels;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
};
