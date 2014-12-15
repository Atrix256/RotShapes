#include "platform.h"
#include "CImageData.h"
#include <stdio.h>

void main (int argc, char **argv)
{
    // TODO: usage, and command line param parsing!
    // TODO: if we have problems with hard coded pixel formats, maybe we can do some sort of conversion where we create a bitmap with the format we have and convert it to the format the runtime code wants
    do
    {
        if (!Platform::Init())
            break;
    
        CImageDataBlackWhite sourceImageData;
        if (!Platform::LoadImageFile(L"../Assets/female_256.png", sourceImageData))
            Platform::ReportError("Could not load source image!");
        else
            Platform::ReportError("source image loaded");

        // TODO: init to the size based on command line params for encoding!
        // TODO: encode the image!
        // TODO: force the encoded image to always be png?
        CImageDataRGBA encodedImageData;
        encodedImageData.AllocatePixels(1,256);

        // TODO: TEMP
        memset(encodedImageData.GetPixelBuffer(), 128, encodedImageData.GetPixelBufferSize());

        if (!Platform::SaveImageFile(L"blah.png",encodedImageData))
            Platform::ReportError("Could not save encoded image!");
        else
            Platform::ReportError("encoded image saved");

    }
    while (0);

    Platform::Shutdown();
}