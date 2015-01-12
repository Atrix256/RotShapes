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
	size_t			m_angles;
	std::wstring	m_convertedFile;
};

struct SDecodingSettings
{
	SDecodingSettings()
		:  m_width(256)
		, m_height(256)
		, m_textureFilter(ETextureFilter::e_filterNone)
	{ }

	std::wstring	m_srcFile;
	std::wstring	m_destFile;
	size_t			m_width;
	size_t			m_height;
	ETextureFilter	m_textureFilter;
	std::wstring	m_debugColorsFile;
};

struct SSettings
{
	SEncodingSettings m_encoding;
	SDecodingSettings m_decoding;
};