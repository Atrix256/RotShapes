#include "platform.h"
#include "CImageData.h"
#include "Encoder.h"
#include "Decoder.h"
#include "SSettings.h"
#include <stdio.h>
#include <vector>

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
        else if (!_wcsicmp(argv[index], L"-combine"))
        {
            ++index;
            if (index >= argc)
            {
                Platform::ReportError("no source file A specified for combining");
                return false;
            }
            settings.m_combine.m_srcFileA = argv[index];
            ++index;
            if (index >= argc)
            {
                Platform::ReportError("no source file B specified for combining");
                return false;
            }
            settings.m_combine.m_srcFileB = argv[index];
            ++index;
            if (index >= argc)
            {
                Platform::ReportError("no destination file specified for combining");
                return false;
            }
            settings.m_combine.m_destFile = argv[index];
            ++index;
        }
        else if (!_wcsicmp(argv[index], L"-animate"))
        {
            settings.m_animate.m_animate = true;
            ++index;
            if (index >= argc)
            {
                Platform::ReportError("no dest gif file specified for animation");
                return false;
            }
            settings.m_animate.m_destGifFile = argv[index];
            ++index;
            if (index >= argc || swscanf_s(argv[index], L"%u", &settings.m_animate.m_fps) != 1)
            {
                Platform::ReportError("no fps given for animation");
                return false;
            }
            if (settings.m_animate.m_fps > 50)
                Platform::ReportError("Warning: chrome doesn't seem to support gifs greater than 50fps, and the gif\nformat doesn't support more than 100fps.");
            ++index;
            if (index >= argc || swscanf_s(argv[index], L"%f", &settings.m_animate.m_seconds) != 1)
            {
                Platform::ReportError("no seconds given for animation");
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
        else if (!_wcsicmp(argv[index], L"-aa"))
        {
            settings.m_decoding.m_useAA = true;
            ++index;
        }
        else if (!_wcsicmp(argv[index], L"-filterbilinear"))
        {
            settings.m_decoding.m_textureFilter = ETextureFilter::e_filterBilinear;
            ++index;
        }
        else if (!_wcsicmp(argv[index], L"-filtersmart"))
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
    Platform::ReportError("    encode the <source> file with <angles> angles and save it as <destination>.\n");
    Platform::ReportError("  -decode <source> <destination> <width> <height>");
    Platform::ReportError("    decode the <source> file into an image that is <width> x <height> in\n    resolution and saves it as <destination>.\n");
    Platform::ReportError("Encoding Options:");
    Platform::ReportError("  -bw <filename>");
    Platform::ReportError("    Save the source image converted to black & white to <filename>.\n");
    Platform::ReportError("Decoding Options:");
    Platform::ReportError("  -debugcolors <filename>");
    Platform::ReportError("    decode the encoded regions as black, red, green, blue, white and save it as\n    <filename>.\n");
    Platform::ReportError("  -filterbilinear");
    Platform::ReportError("    Use bilinear filtering when decoding image.\n");
    Platform::ReportError("  -filtersmart");
    Platform::ReportError("    Use bilinear filtering when decoding image on the x axis (time), but only\n    bilinear filter on the y axis (angles) if there isn't too large of a\n    discontinuity.\n");
    Platform::ReportError("  -showradialpixels");
    Platform::ReportError("    This option will show the radial pixel boundaries in the decoded images.\n    Every angle is drawn, but only every 16 distances.\n");
    Platform::ReportError("  -animate <destgiffile> <fps> <seconds>");
    Platform::ReportError("    By default, a multiframe encoded image will decode to a sheet of images.\n    When this option is specified, it makes an animated gif named\n    <destgiffile> of the animation happening over <seconds> seconds at <fps>");
    Platform::ReportError("    frames per second.  This option also assumes that the decoded filenames have    a %%i in them where you want a frame number, and will output all frames of\n    the animation used to make the gif.\n");
    Platform::ReportError("  -aa");
    Platform::ReportError("    If specified, will use 4-rook super sampled anti aliasing\n");
    Platform::ReportError("Format Options:");
    Platform::ReportError("  -shortdist");
    Platform::ReportError("    By default, the maximum distance encodable is the length of the hypotneuse.\n    This option makes the max distance the greater of width or height.  This\n    gives more precision but rounds off the corners.\n");
    Platform::ReportError("  -sqdist");
    Platform::ReportError("    Store squared distance instead of regular distance.\n");
    Platform::ReportError("Other Options:");
    Platform::ReportError("  -combine <sourceA> <sourceB> <destination>");
    Platform::ReportError("    This will combine the encoded frames of images <sourceA> and <sourceB> and\n    save the result as <destination>. Useful for animations or sprite sheets.\n    <sourceA> and <sourceB> must be the same height.\n");
}

void DoDecode (const SSettings& settings, const CImageDataRGBA& sourceImageData, bool debugColors)
{
    // for the case of multiple frames in the encoded image, and decoding it as a sheet, figure out
    // how many images wide and high (in a square) we are going to decode.
    unsigned int numFrames = sourceImageData.GetWidth();
    unsigned int tilesX = 1;
    unsigned int tilesY = 1;
    while (tilesX*tilesY < numFrames)
    {
        if (tilesX == tilesY)
            tilesX++;
        else
            tilesY++;
    }
    const std::wstring& outFile = debugColors ? settings.m_decoding.m_debugColorsFile : settings.m_decoding.m_destFile;

    // decode each frame to a destination image
    CImageDataRGBA decodedImageData;
    std::vector<CImageDataRGBA> decodedFrames;
    decodedFrames.resize(numFrames);
    for (unsigned int i = 0; i < numFrames; ++i)
        decodedFrames[i].AllocatePixels(settings.m_decoding.m_width,settings.m_decoding.m_height);

    decodedImageData.AllocatePixels(settings.m_decoding.m_width*tilesX,settings.m_decoding.m_height*tilesY);
    for (unsigned int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
    {
        // decode the frame
        Decode(sourceImageData, (float)frameIndex, decodedFrames[frameIndex], debugColors, settings);
        Platform::ReportError("Frame %i decoded", frameIndex);

        // copy it into it's location in the decoded image
        int x = (frameIndex % tilesX)*settings.m_decoding.m_width;
        int y = (frameIndex / tilesX)*settings.m_decoding.m_height;
        decodedImageData.DrawImageData(x,y,decodedFrames[frameIndex]);
    }

    // save the decoded image
    if (!Platform::SaveImageFile(outFile.c_str(), decodedImageData))
        Platform::ReportError("Could not save decoded image: %ls", outFile.c_str());
    else
        Platform::ReportError("decoded image saved: %ls", outFile.c_str());
}

void DoDecodeAnimate (const SSettings& settings, const CImageDataRGBA& sourceImageData, bool debugColors)
{
    // for the case of multiple frames in the encoded image, and decoding it as a sheet, figure out
    // how many images wide and high (in a square) we are going to decode.
    unsigned int numFrames = (unsigned int)((float)settings.m_animate.m_fps * settings.m_animate.m_seconds);
    const std::wstring& outFile = debugColors ? settings.m_decoding.m_debugColorsFile : settings.m_decoding.m_destFile;

    // decode each frame
    std::vector<CImageDataRGBA> decodedFrames;
    decodedFrames.resize(numFrames);
    for (unsigned int i = 0; i < numFrames; ++i)
        decodedFrames[i].AllocatePixels(settings.m_decoding.m_width,settings.m_decoding.m_height);

    for (unsigned int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
    {
        // calculate the frame value
        float frame = numFrames < 2 ? 0.0f: ((float)frameIndex / (float)(numFrames - 1)) * ((float)sourceImageData.GetWidth() - 1);

        // decode the frame
        Decode(sourceImageData, frame, decodedFrames[frameIndex], debugColors, settings);
        Platform::ReportError("Frame %i decoded", frameIndex);

        // save the decoded image
        wchar_t fileName[1024];
        swprintf_s(fileName, outFile.c_str(), frameIndex);

        // save the decoded image
        if (!Platform::SaveImageFile(fileName, decodedFrames[frameIndex]))
            Platform::ReportError("Could not save decoded frame: %ls", outFile.c_str());
    }
    Platform::ReportError("decoded animation frames saved");

    // save the animated gif if we aren't doing debug colors
    if (!debugColors) {
        if (!Platform::SameAnimatedImageFile(settings.m_animate.m_destGifFile.c_str(), decodedFrames, settings.m_animate.m_fps))
            Platform::ReportError("Could not save animated image: %ls", settings.m_animate.m_destGifFile.c_str());
        else
            Platform::ReportError("Saved animated image: %ls", settings.m_animate.m_destGifFile.c_str());
    }
}

int wmain (int argc, wchar_t **argv)
{
    
    /*
    // ===== TODOS =====

    ! yeah... i think we just need to manually fix up source images for now.  future work: improve encoding algorithm
    * expose "smart filter threshold" to be able to compare / contrast it in the paper

    ? how does 128 distances look vs 256?
     * meh.. not so good ):
     ? can we get more precision near center or something maybe?

    ! next the 0-127 version with the "blend or not" bit?  bilinear filtering out the window though ):

    ? maybe manually specify which things can and cannot be lerped between.  manual fix up when it goes wrong.
     * for animation, if < 0.5, use the last frame, else use the next frame?

    // ===== FEATURE TODOS =====

    ? maybe make it be able to spit out raw rawdial pixels captured, before cutting it down to N ranges.  to verify what it captured.

    ? maybe have a 0-127 version, where the top bit specifies whether you should blend to the next angle or not for that slot?
     * bummer: we cut our uniuque distance values in half.
     ? can we do it a different way, like if B < G, that means to not blend?
     * or we could like, make A be 4 bits, and make it a scaled delta from B? use the rest of the 4 bits from A as the blend flags?




    ! next most major feature remaining = blending between pie slices when we should, and not when we shouldn't.
     * could maybe have two modes of operation. One = like it is (heuristic per image?).  Other = 2 pixels per cone.

    * make a version that uses 2 pixels per cone, or uses one less channel for color, to have the "smart blend" threshold info be baked in or something?
    * or, maybe have 1 extra pixel to have tolarance info somehow?
    * both of these result in more pixel reads though ):
    ? maybe try to do some curve fitting between the pixels? like grab the last pixel and the next pixel and curve fit? (while throwing out data that is too far!)

    // TODO: work on smart filtering more, possibly expose threshold as a command line parameter?

    // TODO: expose smart filter threshold as a parameter

    * when encoding, instead of always just removing the smallest section, could make it higher priority to keep things continuous
    
    // ===== MAYBE FEATURES
    ? get rid of "web" implementation? kinda useless at this point it seems
    // TODO: option for a single 32 bit distance for encoding & decoding?
    // TODO: Layering and color tint decoding? for demos
    // TODO: or, maybe could get gradient from bilinear information and do something with that (continuity test? distance estimation?) probably better AA at least!
    // maybe try one of the averages (like, geometric, or something)
    // TODO: try maybe doing some curve fitting with smart filter if the current smart filter doesn't work out

    // ===== PROGRAM TODOS =====
    // TODO: distance seems to round up from left corner.  should round based on center i think
    // TODO: make asserts happen in release too!
    // TODO: force the encoded image (and other images?) to always be png extension and type somehow? (and gif for animated files)
    // TODO: async file io, so we can dump to disk and keep working?

    */

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
        (settings.m_decoding.m_srcFile.length() == 0 || settings.m_decoding.m_destFile.length() == 0) &&
        (settings.m_combine.m_srcFileA.length() == 0 || settings.m_combine.m_srcFileB.length() == 0 || settings.m_combine.m_srcFileA.length() == 0))
    {
        Platform::ReportError("No files specified for encoding, decoding or combining.");
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

            if (settings.m_animate.m_animate)
            {
                // decode image
                DoDecodeAnimate(settings, sourceImageData, false);
            
                // do a debug color decoding if we should
                if (settings.m_decoding.m_debugColorsFile.length() > 0)
                    DoDecodeAnimate(settings, sourceImageData, true);
            }
            else
            {
                // decode image
                DoDecode(settings, sourceImageData, false);
            
                // do a debug color decoding if we should
                if (settings.m_decoding.m_debugColorsFile.length() > 0)
                    DoDecode(settings, sourceImageData, true);
            }
        }

        // combine an image if we should
        if (settings.m_combine.m_srcFileA.length() > 0 && settings.m_combine.m_srcFileB.length() && settings.m_combine.m_destFile.length() > 0)
        {
            // load the source A image
            CImageDataRGBA sourceImageDataA;
            if (!Platform::LoadImageFile(settings.m_combine.m_srcFileA.c_str(), sourceImageDataA, false))
            {
                Platform::ReportError("Could not load source file A for combining: %ls", settings.m_combine.m_srcFileA.c_str());
                break;
            }
            Platform::ReportError("combining source image A loaded: %ls", settings.m_combine.m_srcFileA.c_str());

            // load the source B image
            CImageDataRGBA sourceImageDataB;
            if (!Platform::LoadImageFile(settings.m_combine.m_srcFileB.c_str(), sourceImageDataB, false))
            {
                Platform::ReportError("Could not load source file B for combining: %ls", settings.m_combine.m_srcFileB.c_str());
                break;
            }
            Platform::ReportError("combining source image B loaded: %ls", settings.m_combine.m_srcFileB.c_str());

            // make sure they are the same height
            if (sourceImageDataA.GetHeight() != sourceImageDataB.GetHeight())
            {
                Platform::ReportError("Could not combine images, height mismatch! %u vs %u", sourceImageDataA.GetHeight(), sourceImageDataB.GetHeight());
                break;
            }

            // combine the images
            CImageDataRGBA combinedImageData;
            combinedImageData.AllocatePixels(sourceImageDataA.GetWidth() + sourceImageDataB.GetWidth(), sourceImageDataA.GetHeight());
            combinedImageData.DrawImageData(0, 0, sourceImageDataA);
            combinedImageData.DrawImageData(sourceImageDataA.GetWidth(), 0, sourceImageDataB);

            // save the combined image
            if (!Platform::SaveImageFile(settings.m_combine.m_destFile.c_str(), combinedImageData))
            {
                Platform::ReportError("Could not save combined image: %ls", settings.m_combine.m_destFile.c_str());
                break;
            }
            Platform::ReportError("combined image saved: %ls", settings.m_combine.m_destFile.c_str());
        }
    }
    while (0);

    Platform::Shutdown();

    return 0;
}