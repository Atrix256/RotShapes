#include "platform.h"
#include "CImageData.h"
#include <stdio.h>
#include <stdarg.h>

#include <Windows.h>
#include <Wincodecsdk.h>
#include <comdef.h>

#pragma comment(lib,"windowscodecs.lib")

// WIC COM interface
static IWICImagingFactory *s_factory = NULL;

namespace Platform
{
    // prototypes
    const char* GetMessageForHresult(HRESULT hr);
    static void ReportErrorHRESULT (HRESULT, const char* format, ...);

    //--------------------------------------------------------------------------------------------------------------
    bool Init ()
    {
        // init com and create the imaging factory
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (!SUCCEEDED(hr))
        {
            ReportErrorHRESULT(hr, __FUNCTION__" Failed: CoInitializeEx failed");
            return false;
        }
            
        hr = CoCreateInstance(CLSID_WICImagingFactory,NULL, CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&s_factory));
        if (!SUCCEEDED(hr))
        {
            ReportErrorHRESULT(hr, __FUNCTION__" Failed: CoCreateInstance CLSID_WICImagingFactory failed");
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
    bool LoadImageFile (const wchar_t* fileName, CImageDataRGBA& imageData, bool convertToBlackWhite)
    {
        // taken from http://msdn.microsoft.com/en-us/library/ee719794(v=vs.85).aspx
        // and http://msdn.microsoft.com/en-us/library/windows/desktop/ee719661%28v=vs.85%29.aspx
        
        bool ret = true;
        IWICBitmapDecoder* decoder = NULL;
        IWICBitmapFrameDecode* frame = NULL;
        IWICBitmap* bitmap = NULL;
        IWICBitmapSource * convertedBitmap = NULL;
		IWICBitmapSource * convertedBitmap2 = NULL;
        unsigned char* pixels = NULL;

        do {
            // Create the decoder.
            HRESULT hr = s_factory->CreateDecoderFromFilename(fileName, NULL, GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                &decoder);
            if (!SUCCEEDED(hr) || !decoder)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: CreateDecoderFromFilename with file '%s'", fileName);
                ret = false;
                break;
            }

            UINT frameCount;
            hr = decoder->GetFrameCount(&frameCount);
            if (!SUCCEEDED(hr) || frameCount != 1)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: frame count = ", frameCount);
                ret = false;
                break;
            }

            // get the first frame
            hr = decoder->GetFrame(0, &frame);
            if (!SUCCEEDED(hr) || !frame)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not get frame");
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
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not create bitmap");
                ret = false;
                break;
            }

            // get bitmap height / width
            UINT width, height;
            hr = bitmap->GetSize(&width, &height);
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not get image size");
                ret = false;
                break;
            }

            // get the pixel format
            WICPixelFormatGUID pixelFormat; 
            hr = bitmap->GetPixelFormat(&pixelFormat);
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not get pixel format");
                ret = false;
                break;
            }

			// if we need black and white, make it so
			if (convertToBlackWhite)
			{
				// convert to black and white 1 bit per pixel
				hr = WICConvertBitmapSource(GUID_WICPixelFormatBlackWhite, bitmap, &convertedBitmap);
				if (!SUCCEEDED(hr) || !convertedBitmap)
				{
					ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not convert image to black and white");
					ret = false;
					break;
				}

				// convert back to 32 bit color
				hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, convertedBitmap, &convertedBitmap2);
				if (!SUCCEEDED(hr) || !convertedBitmap2)
				{
					ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not convert image from black and white");
					ret = false;
					break;
				}
			}

            UINT stride = width*4;
            UINT bufferSize = stride*height;
            pixels = new unsigned char[bufferSize];
            WICRect rc = {0, 0, width, height};
			if (convertToBlackWhite)
				hr = convertedBitmap2->CopyPixels(&rc,stride,bufferSize,pixels);
			else
				hr = bitmap->CopyPixels(&rc,stride,bufferSize,pixels);
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: could not copy pixel data");
                ret = false;
                break;
            }

            // give the pixels to the image  data
            imageData.SetPixels(width, height, stride, pixels);
            pixels = NULL;
        }
        while(0);

        // free the frame and decoder etc
        if (convertedBitmap2)
            convertedBitmap2->Release();

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
    bool SaveImageFile (const wchar_t* fileName, const CImageDataRGBA& imageData)
    {
        // info taken from http://msdn.microsoft.com/en-us/library/windows/desktop/ff973956.aspx
        // and http://msdn.microsoft.com/en-us/library/windows/desktop/ee690141%28v=vs.85%29.aspx
        
        bool ret = true;
        IWICBitmapEncoder* encoder = NULL;
        IWICStream* stream = NULL;
        IWICBitmapFrameEncode* frameEncode = NULL;

        do {
            HRESULT hr = s_factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
            if (!SUCCEEDED(hr) || encoder == NULL)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: CreateEncoder");
                ret = false;
                break;
            }

            hr = s_factory->CreateStream(&stream);
            if (!SUCCEEDED(hr) || stream == NULL)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: CreateStream");
                ret = false;
                break;
            }

            hr = stream->InitializeFromFilename(fileName, GENERIC_WRITE);
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: stream InitializeFromFilename");
                ret = false;
                break;
            }

            hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: encoder initialize ");
                ret = false;
                break;
            }

            // create a new frame for our image data
            hr = encoder->CreateNewFrame(&frameEncode, nullptr);
            if (!SUCCEEDED(hr) || !frameEncode)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: encoder initialize ");
                ret = false;
                break;
            }

            // initialize the encoder
            hr = frameEncode->Initialize(NULL);
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: frame initialize ");
                ret = false;
                break;
            }

            // set the size of the frame
            hr = frameEncode->SetSize(imageData.GetWidth(), imageData.GetHeight());
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: set frame size");
                ret = false;
                break;
            }

            // set the pixel format of the frame
            WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppBGRA;  //(GUID_WICPixelFormat32bppRGBA ??)
            hr = frameEncode->SetPixelFormat(&pixelFormat);
			if (!SUCCEEDED(hr) || pixelFormat != GUID_WICPixelFormat32bppBGRA)
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: set frame pixel format");
                ret = false;
                break;
            }

            // copy the pixels
            hr = frameEncode->WritePixels(imageData.GetHeight(), imageData.GetStride(), imageData.GetPixelBufferSize(), imageData.GetPixelBuffer());
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: frame write pixels");
                ret = false;
                break;
            }

            // commit the frame
            hr = frameEncode->Commit();
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: frame commit");
                ret = false;
                break;
            }

            // commit the encoder
            hr = encoder->Commit();
            if (!SUCCEEDED(hr))
            {
                ReportErrorHRESULT(hr,__FUNCTION__" Failed: encoder commit");
                ret = false;
                break;
            }
        }
        while(0);

        if (frameEncode)
            frameEncode->Release();

        if (stream)
            stream->Release();

        if (encoder)
            encoder->Release();

        return ret;
    }

    //--------------------------------------------------------------------------------------------------------------
    void ReportError (const char* format, ...)
    {
        va_list args;
        va_start (args, format);
        vprintf (format, args);
        va_end (args);
        printf("\n");
    }

    //--------------------------------------------------------------------------------------------------------------
    static void ReportErrorHRESULT (HRESULT hr, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n%s\n",GetMessageForHresult(hr));
    }

    //--------------------------------------------------------------------------------------------------------------
    static const char* GetMessageForHresult (HRESULT hr) {
        static char retString[1024];
        _com_error error(hr);
        sprintf_s(retString, 1024, "Error 0x%08x: %ls", hr, error.ErrorMessage());
        return retString;
    }

};