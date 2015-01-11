#include <memory.h>
#include <assert.h>
#include <array>
#include <math.h>

//template <unsigned int CHANNELS, unsigned int BITSPERCHANNEL>
class CImageDataRGBA
{
public:
    CImageDataRGBA () : m_pixels(0), m_width(0), m_height(0), m_stride(0) { }
    ~CImageDataRGBA() { Clear(); }

    //static const unsigned int c_pixelBits = CHANNELS * BITSPERCHANNEL;

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

	void GetPixel(size_t x, size_t y, std::array<unsigned char, 4>& pixel) const
	{
		// mod x,y by width, height to do wrap texture mode
		unsigned char *pixelPointer = GetPixelBuffer() + (y%GetHeight()) * GetStride() + (x%GetWidth())*4;
		pixel[0] = pixelPointer[0];
		pixel[1] = pixelPointer[1];
		pixel[2] = pixelPointer[2];
		pixel[3] = pixelPointer[3];
	}

	void GetPixelBilinear(float x, float y, std::array<float, 4>& pixel) const
	{
		float fractX = x - floor(x);
		float fractY = y - floor(y);
		float fractXOpp = 1.0f - fractX;
		float fractYOpp = 1.0f - fractY;

		size_t y1 = (size_t)floor(y);
		size_t y2 = y1 + 1;

		size_t x1 = (size_t)floor(x);
		size_t x2 = x1 + 1;

		std::array<unsigned char, 4> p1;
		std::array<unsigned char, 4> p2;
		std::array<unsigned char, 4> p3;
		std::array<unsigned char, 4> p4;

		GetPixel(x1,y1,p1);
		GetPixel(x2,y1,p2);
		GetPixel(x1,y2,p3);
		GetPixel(x2,y2,p4);

		pixel[0] = ((float)p1[0] * fractXOpp + (float)p2[0] * fractX) * fractYOpp + ((float)p3[0] * fractXOpp + (float)p4[0] * fractX) * fractY;
		pixel[1] = ((float)p1[1] * fractXOpp + (float)p2[1] * fractX) * fractYOpp + ((float)p3[1] * fractXOpp + (float)p4[1] * fractX) * fractY;
		pixel[2] = ((float)p1[2] * fractXOpp + (float)p2[2] * fractX) * fractYOpp + ((float)p3[2] * fractXOpp + (float)p4[2] * fractX) * fractY;
		pixel[3] = ((float)p1[3] * fractXOpp + (float)p2[3] * fractX) * fractYOpp + ((float)p3[3] * fractXOpp + (float)p4[3] * fractX) * fractY;
	}

private:
    unsigned char *m_pixels;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
};
