#include <string>

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
		, m_bilinearFilter(false)
		, m_debugColors(false)
	{ }

	std::wstring	m_srcFile;
	std::wstring	m_destFile;
	size_t			m_width;
	size_t			m_height;
	bool			m_bilinearFilter;
	bool			m_debugColors;
};

struct SSettings
{
	SEncodingSettings m_encoding;
	SDecodingSettings m_decoding;
};