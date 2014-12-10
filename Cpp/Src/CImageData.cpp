
#include "CImageData.h"

//--------------------------------------------------------------------------------------------------------------
void CImageData::SetPixels (unsigned int width, unsigned int height, unsigned int stride, unsigned char *pixels)
{
    Clear();
    m_pixels = pixels;
    m_width = width;
    m_height = height;
    m_stride = stride;
}