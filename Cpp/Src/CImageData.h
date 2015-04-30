#include <memory.h>
#include <assert.h>
#include <array>
#include <math.h>

class CImageDataRGBA
{
public:
    CImageDataRGBA() : m_pixels(0), m_width(0), m_height(0), m_stride(0) { }
    ~CImageDataRGBA() { Clear(); }

    void Clear()
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
    void SetPixels(unsigned int width, unsigned int height, unsigned int stride, unsigned char *pixels)
    {
        Clear();
        m_pixels = pixels;
        m_width = width;
        m_height = height;
        m_stride = stride;
    }

    void AllocatePixels(unsigned int width, unsigned int height)
    {
        Clear();
        m_width = width;
        m_height = height;

        unsigned int rowBits = 32 * m_width;
        m_stride = (rowBits / 8) + (rowBits & 8 ? 1 : 0);

        m_pixels = new unsigned char[m_stride * m_height];
        memset(m_pixels, 0, m_stride*m_height);
    }

    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    unsigned int GetStride() const { return m_stride; }

    unsigned int GetPixelBufferSize() const { return GetStride() * GetHeight(); }
    unsigned char* GetPixelBuffer() const { return m_pixels; }

    //--------------------------------------------------------------------------------------------------------------
    // PIXEL READS : get a float[4] with values 0.0f-255.0f
    //--------------------------------------------------------------------------------------------------------------
    void GetPixel (size_t x, size_t y, std::array<float, 4>& pixel) const
    {
        return GetPixel((float)x, (float)y, pixel);
    }

    void GetPixel (float x, float y, std::array<float, 4>& pixel) const
    {
        // mod x,y by width, height to do wrap texture mode
        size_t xx = (size_t)x;
        size_t yy = (size_t)y;
        unsigned char *pixelPointer = GetPixelBuffer() + (yy%GetHeight()) * GetStride() + (xx%GetWidth())*4;
        pixel[0] = (float)pixelPointer[0];
        pixel[1] = (float)pixelPointer[1];
        pixel[2] = (float)pixelPointer[2];
        pixel[3] = (float)pixelPointer[3];
    }

    template <typename FILTERX, typename FILTERY>
    void GetPixelFiltered (float x, float y, std::array<float, 4>& pixel, const FILTERX& filterX, const FILTERY& filterY) const
    {
        float y1 = floor(y);
        float x1 = floor(x);
        float fractX = x - x1;
        float fractY = y - y1;

        // blend the left pixel
        std::array<float, 4> left;
        {
            std::array<float, 4> p1;
            std::array<float, 4> p2;
            GetPixel(x1, y1, p1);
            GetPixel(x1, y1 + 1, p2);
            filterY(p2, p1, left, fractY);
        }

        // blend the right pixel
        std::array<float, 4> right;
        {
            std::array<float, 4> p1;
            std::array<float, 4> p2;
            GetPixel(x1 + 1, y1, p1);
            GetPixel(x1 + 1, y1 + 1, p2);
            filterY(p2, p1, right, fractY);
        }

        //blend the left and right pixels
        filterX(right, left, pixel, fractX);
    }

    void GetPixelVanilla (float x, float y, std::array<float, 4>& pixel) const
    {
        GetPixelFiltered(x, y, pixel, PixelBlendLinear, PixelBlendNone);
    }

    void GetPixelBilinear (float x, float y, std::array<float, 4>& pixel) const
    {
        GetPixelFiltered(x, y, pixel, PixelBlendLinear, PixelBlendLinear);
    }

    void GetPixelSmart (float x, float y, std::array<float, 4>& pixel) const
    {
        GetPixelFiltered(x, y, pixel, PixelBlendLinear, PixelBlendSmart);
    }
    
    static void PixelBlendSmart (const std::array<float, 4>& a, const std::array<float, 4>& b, std::array<float, 4>& c, float weight)
    {
        for (int i = 0; i < a._EEN_SIZE; ++i)
        {
            float dist = abs((float)a[i] - (float)b[i]);
            if (dist > 20.0f)
            {
                if (weight > 0.5f)
                    c[i] = a[i];
                else
                    c[i] = b[i];
            }
            else
                c[i] = (float)a[i] * weight + (float)b[i] * (1.0f-weight);
        }
    }

    static void PixelBlendLinear (const std::array<float, 4>& a, const std::array<float, 4>& b, std::array<float, 4>& c, float weight)
    {
        for (int i = 0; i < a._EEN_SIZE; ++i)
        {
            c[i] = a[i] * weight + b[i] * (1.0f - weight);
        }
    }

    static void PixelBlendNone (const std::array<float, 4>& a, const std::array<float, 4>& b, std::array<float, 4>& c, float weight)
    {
        c = weight > 0.5f ? a : b;
    }

    //--------------------------------------------------------------------------------------------------------------
    // PIXEL WRITES: unsigned int 0xAARRGGBB
    //--------------------------------------------------------------------------------------------------------------

    void SetPixel (float x, float y, std::array<float, 4>& pixel) const
    {
        SetPixel((size_t)x, (size_t)y, pixel);
    }

    void SetPixel (size_t x, size_t y, std::array<float, 4>& pixel) const
    {
        // mod x,y by width, height to do wrap texture mode
        unsigned char *pixelPointer = GetPixelBuffer() + (y%GetHeight()) * GetStride() + (x%GetWidth()) * 4;
        pixelPointer[0] = (unsigned char)pixel[0];
        pixelPointer[1] = (unsigned char)pixel[1];
        pixelPointer[2] = (unsigned char)pixel[2];
        pixelPointer[3] = (unsigned char)pixel[3];
    }

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

    //--------------------------------------------------------------------------------------------------------------
    // IMAGE COPIES
    //--------------------------------------------------------------------------------------------------------------
    void DrawImageData (size_t x, size_t y, const CImageDataRGBA& src)
    {
        unsigned int drawWidth = GetWidth() - x;
        unsigned int drawHeight = GetHeight() - y;
        drawWidth = std::min(drawWidth, src.GetWidth());
        drawHeight = std::min(drawHeight, src.GetHeight());

        for (unsigned int iy = 0; iy < drawHeight; ++iy)
        {
            for (unsigned int ix = 0; ix < drawWidth; ++ix)
            {
                std::array<float, 4> srcPixel;
                src.GetPixel(ix, iy, srcPixel);

                unsigned int destPixel =
                    ((unsigned int)srcPixel[3]) << 24 |
                    ((unsigned int)srcPixel[2]) << 16 |
                    ((unsigned int)srcPixel[1]) << 8 |
                    ((unsigned int)srcPixel[0]);

                DrawPixelClip(x + ix, y + iy, destPixel);
            }
        }
    }

private:
    unsigned char *m_pixels;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
};
