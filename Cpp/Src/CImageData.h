// TODO: common interface? or maybe a template class with specializations?
// TODO: intro comments in header for file name and description
// TODO: stride should be implicit maybe.  maybe if it's a templated class, stride can be a compile time constant based on template params!

//--------------------------------------------------------------------------------------------------------------

#include <memory.h>
#include <assert.h>

template <unsigned int CHANNELS, unsigned int BITSPERCHANNEL>
class CImageData
{
public:
    CImageData () : m_pixels(0), m_width(0), m_height(0), m_stride(0) { }
    ~CImageData() { Clear(); }

    static const unsigned int c_pixelBits = CHANNELS * BITSPERCHANNEL;

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

        unsigned int rowBits = c_pixelBits * m_width;
        m_stride = (rowBits / 8) + (rowBits & 8 ? 1 : 0);

        m_pixels = new unsigned char[m_stride * m_height];
        memset(m_pixels, 0, m_stride*m_height);
    }

    unsigned int GetWidth () const { return m_width; }
    unsigned int GetHeight () const { return m_height; }
    unsigned int GetStride () const { return m_stride; }

    unsigned int GetPixelBufferSize() const { return GetStride() * GetHeight(); }
    unsigned char* GetPixelBuffer () const { return m_pixels; }

private:
    unsigned char *m_pixels;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
};

//--------------------------------------------------------------------------------------------------------------
class CImageDataBlackWhite : public CImageData<1,1>
{
public:
	bool GetPixel (size_t x, size_t y) const
	{
		assert(x < GetWidth());
		assert(y < GetHeight());
		unsigned char *pixelRow = GetPixelBuffer() + y * GetStride();
		return (pixelRow[x/8] & (1 << (x%8))) != 0;
	}

private:
};

//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA: public CImageData<4,8>
{
public:
private:
};
