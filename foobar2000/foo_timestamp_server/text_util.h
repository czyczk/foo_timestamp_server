#pragma once
namespace text_util {
	// Helper function to convert a hex char to hex value
	inline unsigned char from_hex(unsigned char ch);

	// Convert a std::string to a std::wstring
	std::unique_ptr<std::wstring> to_wstring(const std::string& str);
	// Convert a std::string to a ATL::CString or LPCTSTR
	std::unique_ptr<CString> to_lpctstr(const std::string& str);

	// Convert a UTF-16 encoded string to a UTF-8 encoded one
	std::string to_utf8(const std::wstring & wstr);
	// Convert an ANSI encoded string to a UTF-8 encoded one
	std::string to_utf8(const std::string & str);

	// Decode an escaped URI
	std::string url_decode(const std::string & str);
}