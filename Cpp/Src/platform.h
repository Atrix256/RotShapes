#include <vector>

//--------------------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA;

//--------------------------------------------------------------------------------------------------------------
// Platform interface
//--------------------------------------------------------------------------------------------------------------
namespace Platform
{
    bool Init ();
    void Shutdown ();

    bool LoadImageFile (const wchar_t* fileName, CImageDataRGBA& imageData, bool convertToBlackWhite);

    bool SaveImageFile (const wchar_t* fileName, const CImageDataRGBA& imageData);

	bool SameAnimatedImageFile(const wchar_t* fileName, const std::vector<CImageDataRGBA>& frames, unsigned int fps);

    void ReportError (const char* format, ...);
};