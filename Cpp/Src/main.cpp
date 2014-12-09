#include "platform.h"
#include "CImageData.h"
#include <stdio.h>

void main (int argc, char **argv)
{
    // TODO: usage, and command line param parsing!
    // TODO: error handling for if init fails
    Platform::Init();
    
    CImageData imageData;
    if (!Platform::LoadImageFile(L"../Assets/female_256.png", imageData))
        printf("Could not load image!");
    else
        printf("image loaded");

    Platform::Shutdown();
}