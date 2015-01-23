//--------------------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA;
struct SSettings;
enum class ETextureFilter;

//--------------------------------------------------------------------------------------------------------------
void Decode (const CImageDataRGBA& src, float frame, CImageDataRGBA& dest, bool debugColors, const SSettings& settings);