#include "platform.h"
#include "CImageData.h"
#include <stdio.h>

void main (int argc, char **argv)
{
    // TODO: usage, and command line param parsing!
    do
    {
        if (!Platform::Init())
            break;
    
        CImageDataBlackWhite sourceImageData;
        if (!Platform::LoadImageFileBlackWhite(L"../Assets/female_256.png", sourceImageData))
            Platform::ReportError("Could not load image!");
        else
            Platform::ReportError("image loaded");
        
        // TODO: init to the size based on command line params for encoding!
        CImageDataRGBA encodedImageData;
        //encodedImageData.AllocatePixels();

    }
    while (0);

    Platform::Shutdown();
}