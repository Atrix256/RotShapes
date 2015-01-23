//--------------------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------------------
class CImageDataRGBA;
struct SSettings;
enum class ETextureFilter;

//--------------------------------------------------------------------------------------------------------------
void Decode(const CImageDataRGBA& src, unsigned int frameIndex, CImageDataRGBA& dest, bool debugColors, const SSettings& settings);