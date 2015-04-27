#include <string>

enum class ETextureFilter
{
	e_filterNone,
	e_filterBilinear,
	e_filterSmart
};

struct SEncodingSettings
{
	SEncodingSettings()
		: m_angles(256)
	{ }

    std::wstring	m_srcFile;
    std::wstring	m_destFile;
	size_t	        m_angles;
    std::wstring	m_convertedFile;
};

struct SDecodingSettings
{
	SDecodingSettings()
		: m_width(256)
		, m_height(256)
		, m_textureFilter(ETextureFilter::e_filterNone)
		, m_showRadialPixels(false)
        , m_useAA(false)
	{ }

    std::wstring	m_srcFile;
    std::wstring	m_destFile;
	size_t			m_width;
	size_t			m_height;
	ETextureFilter	m_textureFilter;
    std::wstring	m_debugColorsFile;
	bool			m_showRadialPixels;
    bool            m_useAA;
};

struct SCombineSetings
{
    std::wstring	m_srcFileA;
    std::wstring	m_srcFileB;
    std::wstring	m_destFile;
};

struct SAnimateSettings
{
	SAnimateSettings()
		: m_fps(30)
		, m_seconds(1.0f)
		, m_animate(false)
	{ }

    std::wstring	m_destGifFile;
	bool			m_animate;
	unsigned int	m_fps;
	float			m_seconds;
};

struct SSettings
{
	SSettings()
		: m_shortDist(false)
		, m_sqDist(false)
	{ }

	SEncodingSettings	m_encoding;
	SDecodingSettings	m_decoding;
	SCombineSetings		m_combine;
	SAnimateSettings	m_animate;

	bool				m_shortDist;
	bool				m_sqDist;
};