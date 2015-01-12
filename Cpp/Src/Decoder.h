//--------------------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA;
enum class ETextureFilter;

//--------------------------------------------------------------------------------------------------------------
bool Decode(const CImageDataRGBA& src, CImageDataRGBA& dest, bool debugColors, ETextureFilter textureFilter);