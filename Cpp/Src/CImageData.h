// TODO: common interface? or maybe a template class with specializations?
// TODO: intro comments in header for file name and description
// TODO: stride should be implicit maybe.  maybe if it's a templated class, stride can be a compile time constant based on template params!

//--------------------------------------------------------------------------------------------------------------
template <unsigned int CHANNELS, unsigned int BITSPERCHANNEL>
class CImageData
{
public:
    CImageData () : m_pixels(0), m_width(0), m_height(0), m_stride(0) { }
    ~CImageData() { Clear(); }

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

    //void AllocatePixels ();

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
private:
};

//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA: public CImageData<4,8>
{
public:
private:
};
