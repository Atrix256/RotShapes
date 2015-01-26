#include <string>

using namespace std;

enum class ETextureFilter
{
	e_filterNone,
	e_filterBilinear,
	e_filterSmart
};

enum class EAAMethod
{
	e_AANone,
	e_AASmoothStep,
	e_AASmoothStepGradient
};

struct SEncodingSettings
{
	SEncodingSettings()
		: m_angles(256)
	{ }

	wstring	m_srcFile;
	wstring	m_destFile;
	size_t	m_angles;
	wstring	m_convertedFile;
};

struct SDecodingSettings
{
	SDecodingSettings()
		: m_width(256)
		, m_height(256)
		, m_textureFilter(ETextureFilter::e_filterNone)
		, m_showRadialPixels(false)
		, m_AAMethod(EAAMethod::e_AANone)
		, m_AAParam(0.0f)
	{ }

	wstring			m_srcFile;
	wstring			m_destFile;
	size_t			m_width;
	size_t			m_height;
	ETextureFilter	m_textureFilter;
	wstring			m_debugColorsFile;
	bool			m_showRadialPixels;
	EAAMethod		m_AAMethod;
	float			m_AAParam;
};

struct SCombineSetings
{
	wstring			m_srcFileA;
	wstring			m_srcFileB;
	wstring			m_destFile;
};

struct SAnimateSettings
{
	SAnimateSettings()
		: m_fps(30)
		, m_seconds(1.0f)
		, m_animate(false)
	{ }

	wstring			m_destGifFile;
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