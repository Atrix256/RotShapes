#include "platform.h"
#include "CImageData.h"
#include "Encoder.h"
#include "Decoder.h"
#include "SSettings.h"
#include <stdio.h>

bool ParseCommandLine (SSettings& settings, int argc, wchar_t **argv)
{
	int index = 1;
	while (index < argc)
	{
		if (!_wcsicmp(argv[index], L"-encode"))
		{
			++index;
			if (index >= argc)
			{
				Platform::ReportError("no source file specified for encoding");
				return false;
			}
			settings.m_encoding.m_srcFile = argv[index];
			++index;
			if (index >= argc)
			{
				Platform::ReportError("no destination file specified for encoding");
				return false;
			}
			settings.m_encoding.m_destFile = argv[index];
			++index;
			if (index >= argc || swscanf_s(argv[index],L"%u",&settings.m_encoding.m_angles) !=1)
			{
				Platform::ReportError("no angle count specified for encoding");
				return false;
			}
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-decode"))
		{
			++index;
			if (index >= argc)
			{
				Platform::ReportError("no source file specified for decoding");
				return false;
			}
			settings.m_decoding.m_srcFile = argv[index];
			++index;
			if (index >= argc)
			{
				Platform::ReportError("no destination file specified for decoding");
				return false;
			}
			settings.m_decoding.m_destFile = argv[index];
			++index;
			if (index >= argc || swscanf_s(argv[index], L"%u", &settings.m_decoding.m_width) != 1)
			{
				Platform::ReportError("no width given for decoding");
				return false;
			}
			++index;
			if (index >= argc || swscanf_s(argv[index], L"%u", &settings.m_decoding.m_height) != 1)
			{
				Platform::ReportError("no height given for decoding");
				return false;
			}
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-bw"))
		{
			++index;
			if (index >= argc)
			{
				Platform::ReportError("no file specified for black/white converted source image");
				return false;
			}
			settings.m_encoding.m_convertedFile = argv[index];
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-bilinear"))
		{
			settings.m_decoding.m_textureFilter = ETextureFilter::e_filterBilinear;
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-smartfilter"))
		{
			settings.m_decoding.m_textureFilter = ETextureFilter::e_filterSmart;
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-shortdist"))
		{
			settings.m_shortDist = true;
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-sqdist"))
		{
			settings.m_sqDist = true;
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-showradialpixels"))
		{
			settings.m_decoding.m_showRadialPixels = true;
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-debugcolors"))
		{
			++index;
			if (index >= argc)
			{
				Platform::ReportError("no file specified for debug colors decoded image");
				return false;
			}
			settings.m_decoding.m_debugColorsFile = argv[index];
			++index;
		}
		else
		{
			Platform::ReportError("Unknown command line option: %ls", argv[index]);
			return false;
		}
	}
	return true;
}

void PrintUsage()
{
	Platform::ReportError("Dual Annulus Polar Shape Encoder/Decoder.");
	Platform::ReportError("Written by Alan Wolfe daspe@demofox.org");
	Platform::ReportError("\nUsage:");
	Platform::ReportError("  -encode <source> <destination> <angles>");
	Platform::ReportError("    encode the <source> file with <angles> angles and save it as <destination>.");
	Platform::ReportError("  -decode <source> <destination> <width> <height>");
	Platform::ReportError("    decode the <source> file into an image that is <width> x <height> in\n    resolution and saves it as <destination>.");
	Platform::ReportError("\nEncoding Options:");
	Platform::ReportError("  -bw <filename>");
	Platform::ReportError("    Save the source image converted to black & white to <filename>.");
	Platform::ReportError("\nDecoding Options:");
	Platform::ReportError("  -debugcolors <filename>");
	Platform::ReportError("    decode the encoded regions as black, red, green, blue, white and save it as\n    <filename>.");
	Platform::ReportError("  -bilinear");
	Platform::ReportError("    Use bilinear filtering when decoding image.");
	Platform::ReportError("  -smartfilter");
	Platform::ReportError("    Use bilinear filtering when decoding image on the x axis (time), but only\n    bilinear filter on the y axis (angles) if there isn't too large of a\n    discontinuity.");
	Platform::ReportError("  -showradialpixels");
	Platform::ReportError("    This option will show the radial pixel boundaries in the decoded images.");
	Platform::ReportError("\nFormat Options:");
	Platform::ReportError("  -shortdist");
	Platform::ReportError("    By default, the maximum distance encodable is the length of the hypotneuse.\n    This option makes the max distance the greater of width or height.  This\n    gives more precision but rounds off the corners.");
	Platform::ReportError("  -sqdist");
	Platform::ReportError("    Store squared distance instead of regular distance.\n");
}

int wmain (int argc, wchar_t **argv)
{
	// TODO: showradialpixels should draw lines and circles (bressenham) after the fact, not do distance tests!
	// TODO: maybe an option for showradialpixels to only show the distances, or only the angles, or both
	// TODO: work on smart filtering more, possibly expose threshold as a command line parameter!
	// TODO: print out encoding and decoding options while we do the work
	// TODO: make it so we can use all the threads again
	// TODO: force the encoded image (and other images?) to always be png extension and type somehow?
	// TODO: other features like layering and animation for decoding?
	// TODO: look through all files for todos
	// TODO: make a verb to combine encoded images (for animations / sprite sheets). maybe one to split them apart too.
	// TODO: for decoding, let them specify frame number of source image as a float (used as X texture coordinate)
	// TODO: option for a single 32 bit distance for encoding & decoding!
	// TODO: option for smoothstep?
	// TODO: distance seems to round up from left corner.  should round based on center i think
	// TODO: make asserts happen in release too!
	// TODO: test non square images
	/* TODO:
	* maybe have a thing when bilinear filtering that throws out info when distances are too far.
	 * maybe try some kind of custom filtering to ditch that data
	 * maybe try to do some curve fitting between the pixels? like grab the last pixel and the next pixel and curve fit? (while throwing out data that is too far!)

	* maybe try one of the averages (like, geometric, or something)
	*/
	// TODO: combine bilinear and smartfilter into one option: -filter smart/bilinear. wait on this til animation is working
	// TODO: make a thing where if there is only one argument, if the image has width > 1, it does encoding, else it does decoding? for easy drag / drop usage?

	// show usage if we aren't given enough command line options
	if (argc <= 1)
	{
		PrintUsage();
		return 0;
	}

	// get our settings from the command line if we can, bail out if we can't
	SSettings settings;
	if (!ParseCommandLine(settings, argc, argv))
		return 0;

	// make sure we have a file to encode or decode
	if ((settings.m_encoding.m_srcFile.length() == 0 || settings.m_encoding.m_destFile.length() == 0) &&
		(settings.m_decoding.m_srcFile.length() == 0 || settings.m_decoding.m_destFile.length() == 0))
	{
		Platform::ReportError("No files specified for encoding or decoding.");
		PrintUsage();
		return 0;
	}

	// do our work!
    do
    {
        if (!Platform::Init())
            break;

		// encode an image if we should
		if (settings.m_encoding.m_srcFile.length() > 0 && settings.m_encoding.m_destFile.length() > 0)
		{
			CImageDataRGBA sourceImageData;
			if (!Platform::LoadImageFile(settings.m_encoding.m_srcFile.c_str(), sourceImageData, true))
			{
				Platform::ReportError("Could not load source file for encoding: %ls", settings.m_encoding.m_srcFile.c_str());
				break;
			}
			Platform::ReportError("encoding source image loaded: %ls", settings.m_encoding.m_srcFile.c_str());

			// save the source image if we should
			if (settings.m_encoding.m_convertedFile.length() > 0)
			{
				if (!Platform::SaveImageFile(settings.m_encoding.m_convertedFile.c_str(),sourceImageData))
				{
					Platform::ReportError("Could not save converted source image: %ls", settings.m_encoding.m_convertedFile.c_str());
					break;
				}
				Platform::ReportError("converted source image saved: %ls", settings.m_encoding.m_convertedFile.c_str());
			}

			CImageDataRGBA encodedImageData;
			encodedImageData.AllocatePixels(1,settings.m_encoding.m_angles);

			// encode the image
			if (!Encode(sourceImageData, encodedImageData, settings))
			{
				Platform::ReportError("Could not encode image!");
				break;
			}
			Platform::ReportError("Image encoded");

			// save the encoded image
			if (!Platform::SaveImageFile(settings.m_encoding.m_destFile.c_str(),encodedImageData))
			{
				Platform::ReportError("Could not save encoded image: %ls", settings.m_encoding.m_destFile.c_str());
				break;
			}
			Platform::ReportError("encoded image saved: %ls", settings.m_encoding.m_destFile.c_str());
		}

		// decode an image if we should
		if (settings.m_decoding.m_srcFile.length() > 0 && settings.m_decoding.m_destFile.length() > 0)
		{
			// load the encoded image
			CImageDataRGBA sourceImageData;
			if (!Platform::LoadImageFile(settings.m_decoding.m_srcFile.c_str(), sourceImageData, false))
			{
				Platform::ReportError("Could not load source file for decoding: %ls", settings.m_decoding.m_srcFile.c_str());
				break;
			}
			Platform::ReportError("decoding source image loaded: %ls", settings.m_decoding.m_srcFile.c_str());

			// decode the image
			CImageDataRGBA decodedImageData;
			decodedImageData.AllocatePixels(settings.m_decoding.m_width,settings.m_decoding.m_height);
			if (!Decode(sourceImageData, decodedImageData, false, settings))
			{
				Platform::ReportError("Could not decode image!");
				break;
			}
			Platform::ReportError("Image decoded");

			// save the decoded image
			if (!Platform::SaveImageFile(settings.m_decoding.m_destFile.c_str(), decodedImageData))
			{
				Platform::ReportError("Could not save decoded image: %ls", settings.m_decoding.m_destFile.c_str());
				break;
			}
			Platform::ReportError("decoded image saved: %ls", settings.m_decoding.m_destFile.c_str());

			// do a debug color decoding if we should
			if (settings.m_decoding.m_debugColorsFile.length() > 0)
			{
				// decode the image
				if (!Decode(sourceImageData, decodedImageData, true, settings))
				{
					Platform::ReportError("Could not decode image for debug colors!");
					break;
				}
				Platform::ReportError("Image decoded for debug colors");

				// save the decoded image
				if (!Platform::SaveImageFile(settings.m_decoding.m_debugColorsFile.c_str(), decodedImageData))
				{
					Platform::ReportError("Could not save debug colors decoded image: %ls", settings.m_decoding.m_debugColorsFile.c_str());
					break;
				}
				Platform::ReportError("debug colors decoded image saved: %ls", settings.m_decoding.m_debugColorsFile.c_str());
			}
		}
    }
    while (0);

    Platform::Shutdown();

	return 0;
}