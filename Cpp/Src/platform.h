
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

    bool LoadImageFile (const wchar_t* fileName, CImageDataRGBA& imageData);

    bool SaveImageFile (const wchar_t* fileName, const CImageDataRGBA& imageData);

    void ReportError (const char* format, ...);
};