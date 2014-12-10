#include "platform.h"
#include "CImageData.h"
#include <stdio.h>
#include <stdarg.h>

#include <Windows.h>
#include <Wincodecsdk.h>

#pragma comment(lib,"windowscodecs.lib")

// WIC COM interface
static IWICImagingFactory *s_factory = NULL;

namespace Platform
{
    //--------------------------------------------------------------------------------------------------------------
    bool Init ()
    {
        // init com and create the imaging factory
        if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        {
            ReportError(__FUNCTION__" Failed: CoInitializeEx failed");
            return false;
        }
            
        if (!SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory,NULL, CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&s_factory))))
        {
            ReportError(__FUNCTION__" Failed: CoCreateInstance CLSID_WICImagingFactory failed");
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------------------
    void Shutdown ()
    {
        if (s_factory)
        {
            s_factory->Release();
            s_factory=NULL;
        }

        CoUninitialize();
    }

    //--------------------------------------------------------------------------------------------------------------
    bool LoadImageFileBlackWhite (const wchar_t* fileName, CImageDataBlackWhite& imageData)
    {
        // taken from http://msdn.microsoft.com/en-us/library/ee719794(v=vs.85).aspx
        // and http://msdn.microsoft.com/en-us/library/windows/desktop/ee719661%28v=vs.85%29.aspx
        
        bool ret = true;
        IWICBitmapDecoder* decoder = NULL;
        IWICBitmapFrameDecode* frame = NULL;
        IWICBitmap* bitmap = NULL;
        IWICBitmapSource * convertedBitmap = NULL;
        unsigned char* pixels = NULL;

        do {
            // Create the decoder.
            HRESULT hr = s_factory->CreateDecoderFromFilename(fileName, NULL, GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                &decoder);
            if (!SUCCEEDED(hr) || !decoder)
            {
                ReportError(__FUNCTION__" Failed: CreateDecoderFromFilename with file '%s'", fileName);
                ret = false;
                break;
            }

            UINT frameCount;
            hr = decoder->GetFrameCount(&frameCount);
            if (!SUCCEEDED(hr) || frameCount != 1)
            {
                ReportError(__FUNCTION__" Failed: frame count = ", frameCount);
                ret = false;
                break;
            }

            // get the first frame
            hr = decoder->GetFrame(0, &frame);
            if (!SUCCEEDED(hr) || !frame)
            {
                ReportError(__FUNCTION__" Failed: could not get frame");
                ret = false;
                break;
            }

            // make a bitmap
            hr = s_factory->CreateBitmapFromSource(
              frame,          // Create a bitmap from the image frame
              WICBitmapCacheOnDemand,  // Cache metadata when needed
              &bitmap);              // Pointer to the bitmap
            if (!SUCCEEDED(hr) || !bitmap)
            {
                ReportError(__FUNCTION__" Failed: could not create bitmap");
                ret = false;
                break;
            }

            // get bitmap height / width
            UINT width, height;
            hr = bitmap->GetSize(&width, &height);
            if (!SUCCEEDED(hr))
            {
                ReportError(__FUNCTION__" Failed: could not get image size");
                ret = false;
                break;
            }

            // get the pixel format
            WICPixelFormatGUID pixelFormat; 
            hr = bitmap->GetPixelFormat(&pixelFormat);
            if (!SUCCEEDED(hr))
            {
                ReportError(__FUNCTION__" Failed: could not get pixel format");
                ret = false;
                break;
            }

            // convert to black and white 1 bit per pixel
            hr = WICConvertBitmapSource(GUID_WICPixelFormatBlackWhite, bitmap, &convertedBitmap);
            if (!SUCCEEDED(hr) || !convertedBitmap)
            {
                ReportError(__FUNCTION__" Failed: could not convert image to black and white");
                ret = false;
                break;
            }

            UINT stride = width / 8;
            if (width % 8)
                stride++;
        
            UINT bufferSize = stride*height;
            pixels = new unsigned char[bufferSize];

            WICRect rc = {0, 0, width, height};
            hr = convertedBitmap->CopyPixels(&rc,stride,bufferSize,pixels);
            if (!SUCCEEDED(hr))
            {
                ReportError(__FUNCTION__" Failed: could not copy pixel data");
                ret = false;
                break;
            }

            // give the pixels to the image  data
            imageData.SetPixels(width, height, stride, pixels);
            pixels = NULL;

        }
        while(0);

        // free the frame and decoder etc
        if (convertedBitmap)
            convertedBitmap->Release();

        if (bitmap)
            bitmap->Release();

        if (frame)
            frame->Release();

        if (decoder)
            decoder->Release();

        if (pixels)
            delete[] pixels;

        return ret;
    }

    //--------------------------------------------------------------------------------------------------------------
    void ReportError (const char* format, ...)
    {
        va_list args;
        va_start (args, format);
        vprintf (format, args);
        va_end (args);
    }
};