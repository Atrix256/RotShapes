//--------------------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA;
struct SSettings;
enum class ETextureFilter;

//--------------------------------------------------------------------------------------------------------------
bool Decode(const CImageDataRGBA& src, CImageDataRGBA& dest, bool debugColors, const SSettings& settings);