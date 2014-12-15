
//--------------------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------------------
class CImageDataBlackWhite;
class CImageDataRGBA;

//--------------------------------------------------------------------------------------------------------------
// Platform interface
//--------------------------------------------------------------------------------------------------------------
namespace Platform
{
    bool Init ();
    void Shutdown ();

    bool LoadImageFile (const wchar_t* fileName, CImageDataBlackWhite& imageData);

    bool SaveImageFile (const wchar_t* fileName, const CImageDataRGBA& imageData);

    void ReportError (const char* format, ...);
};