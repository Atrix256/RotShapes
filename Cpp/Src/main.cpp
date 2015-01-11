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
		if (!_wcsicmp(argv[index], L"-bw"))
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
		else if (!_wcsicmp(argv[index], L"-encode"))
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
		else if (!_wcsicmp(argv[index], L"-bilinear"))
		{
			settings.m_decoding.m_bilinearFilter = true;
			++index;
		}
		else if (!_wcsicmp(argv[index], L"-debugcolors"))
		{
			settings.m_decoding.m_debugColors = true;
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
			if (index >= argc || swscanf_s(argv[index],L"%u",&settings.m_decoding.m_width) !=1)
			{
				Platform::ReportError("no width given for decoding");
				return false;
			}
			++index;
			if (index >= argc || swscanf_s(argv[index],L"%u",&settings.m_decoding.m_height) !=1)
			{
				Platform::ReportError("no height given for decoding");
				return false;
			}
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
	Platform::ReportError("Usage:");
	Platform::ReportError("\t-encode <source> <destination> <angles>");
	Platform::ReportError("\t\tencode the <source> file with <angles> angles and save it as <destination>.");
	Platform::ReportError("\t-decode <source> <destination> <width> <height>");
	Platform::ReportError("\t\tdecode the <source> file into an image that is <width> x <height> in resolution and saves it as <destination>.");
	Platform::ReportError("Encoding Options:");
	Platform::ReportError("\t-bw <filename>");
	Platform::ReportError("\t\tSave the source image converted to black & white to <filename>.");
	Platform::ReportError("Decoding Options:");
	Platform::ReportError("\t-debugcolors");
	Platform::ReportError("\t\tShow the encoded regions as black, red, green, blue, white.");
	Platform::ReportError("\t-bilinear");
	Platform::ReportError("\t\tUse bilinear filtering for decoding image.");
}

int wmain (int argc, wchar_t **argv)
{
	// TODO: force the encoded image (and other images?) to always be png extension and type somehow?
	// TODO: option for hypotneuse vs not?
	// TODO: option for squared distance vs not.
	// TODO: other features like layering and animation for decoding?
	// TODO: look through all files for todos
	// TODO: make a verb to combine encoded images (for animations / sprite sheets)
	// TODO: for decoding, let them specify frame number of source image?

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
			if (!Encode(sourceImageData, encodedImageData))
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
			if (!Decode(sourceImageData, decodedImageData, settings.m_decoding.m_debugColors, settings.m_decoding.m_bilinearFilter))
			{
				Platform::ReportError("Could not decode image!");
				break;
			}
			Platform::ReportError("Image decoded");

			// save the decoded image
			if (!Platform::SaveImageFile(settings.m_decoding.m_destFile.c_str(), decodedImageData))
			{
				Platform::ReportError("Could not save encoded image: %ls", settings.m_decoding.m_destFile.c_str());
				break;
			}
			Platform::ReportError("encoded image saved: %ls", settings.m_decoding.m_destFile.c_str());
		}
    }
    while (0);

    Platform::Shutdown();

	return 0;
}