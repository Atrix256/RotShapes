#include "platform.h"
#include "CImageData.h"
#include "Encoder.h"
#include "Decoder.h"
#include <stdio.h>

void main (int argc, char **argv)
{

	// TODO: option for bilinear filtering
	// TODO: do we need to handle texture wrap?
	// TODO: option for hypotneuse vs not?

    // TODO: usage, and command line param parsing!
    // TODO: if we have problems with hard coded pixel formats, maybe we can do some sort of conversion where we create a bitmap with the format we have and convert it to the format the runtime code wants
	// TODO: add unencoding support too and all the other features like layering and animation?
    do
    {
        if (!Platform::Init())
            break;
    
        CImageDataRGBA sourceImageData;
        if (!Platform::LoadImageFile(L"../Assets/female_256.png", sourceImageData))
		{
            Platform::ReportError("Could not load source image!");
			break;
		}
        Platform::ReportError("source image loaded");

		// TODO: make it spit out the b&w source, the intermediate, and the encoded.
		// TODO: keep command line param for it to spit out b&w translated source!
        // TODO: init to the size based on command line params for encoding!
        // TODO: force the encoded image to always be png extension and type?
        CImageDataRGBA encodedImageData;
        encodedImageData.AllocatePixels(1,256);
        
        // encode the image
        if (!Encode(sourceImageData, encodedImageData))
		{
            Platform::ReportError("Could not encode image!");
			break;
		}
        Platform::ReportError("Image encoded");

		// save the encoded image
        if (!Platform::SaveImageFile(L"encoded.png",encodedImageData))
		{
            Platform::ReportError("Could not save encoded image!");
			break;
		}
        Platform::ReportError("encoded image saved");

		// decode the image
        CImageDataRGBA decodedImageData;
        decodedImageData.AllocatePixels(256,256);
		if (!Decode(encodedImageData, decodedImageData))
		{
			Platform::ReportError("Could not decode image!");
			break;
		}
		Platform::ReportError("Image decoded");

		// save the decoded image
        if (!Platform::SaveImageFile(L"decoded.png",decodedImageData))
		{
            Platform::ReportError("Could not save encoded image!");
			break;
		}
        Platform::ReportError("encoded image saved");
    }
    while (0);

    Platform::Shutdown();
}