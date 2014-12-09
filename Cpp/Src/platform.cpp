#include "platform.h"
#include <stdio.h>

#include <Windows.h>
#include <Wincodecsdk.h>

// WIC COM interface
static IWICImagingFactory *s_factory = NULL;

#pragma comment(lib,"windowscodecs.lib")

namespace Platform
{
    bool Init ()
    {
        // init com and create the imaging factory
        return SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) && 
            SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory,NULL, CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&s_factory)));
    }

    void Shutdown ()
    {
        if (s_factory)
        {
            s_factory->Release();
            s_factory=NULL;
        }

        CoUninitialize();
    }

    bool LoadImageFile (const wchar_t* fileName, CImageData& imageData)
    {
        // taken from http://msdn.microsoft.com/en-us/library/ee719794(v=vs.85).aspx
        // and http://msdn.microsoft.com/en-us/library/windows/desktop/ee719661%28v=vs.85%29.aspx
        
        // Create the decoder.
        IWICBitmapDecoder *decoder = NULL;
        HRESULT hr = s_factory->CreateDecoderFromFilename(fileName, NULL, GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            &decoder);
        if (!SUCCEEDED(hr) || !decoder)
            return false;

        // get the first frame
        IWICBitmapFrameDecode* frame = NULL;
        hr = decoder->GetFrame(0, &frame);
        if (!SUCCEEDED(hr) || !frame)
            return false;

        // make a bitmap
        IWICBitmap* bitmap = NULL;
        hr = s_factory->CreateBitmapFromSource(
          frame,          // Create a bitmap from the image frame
          WICBitmapCacheOnDemand,  // Cache metadata when needed
          &bitmap);              // Pointer to the bitmap
        if (!SUCCEEDED(hr) || !bitmap)
            return false;

        // get bitmap height / width
        UINT width, height;
        hr = bitmap->GetSize(&width, &height);
        if (!SUCCEEDED(hr))
            return false;

        // get the pixel format
        WICPixelFormatGUID pixelFormat; 
        hr = bitmap->GetPixelFormat(&pixelFormat);
        if (!SUCCEEDED(hr))
            return false;

        // convert to black and white 1 bit per pixel
        IWICBitmapSource * convertedBitmap = NULL;
        hr = WICConvertBitmapSource(GUID_WICPixelFormatBlackWhite, bitmap, &convertedBitmap);
        if (!SUCCEEDED(hr) || !convertedBitmap)
            return false;

        // TODO: copy the pixels to a CImageData
        UINT stride = width / 8;
        if (width % 8)
            stride++;
        
        UINT bufferSize = stride*height;
        unsigned char* pixels = new unsigned char[bufferSize];

        WICRect rc = {0, 0, width, height};
        hr = convertedBitmap->CopyPixels(&rc,stride,bufferSize,pixels);
        if (!SUCCEEDED(hr))
            return false;

        // free the frame and decoder etc
        convertedBitmap->Release();
        bitmap->Release();
        frame->Release();
        decoder->Release();

        delete[] pixels;

        // TODO: return error if multiple frames found? or what?
        // TODO: better error handling
        // TODO: some templated wrapper object that destroys COM objects at object destructiuon time!

        return true;
    }
};