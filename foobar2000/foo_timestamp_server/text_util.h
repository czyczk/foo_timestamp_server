#pragma once
namespace text_util {
	// Convert a std::string to a std::wstring
	std::unique_ptr<std::wstring> to_wstring(const std::string& str);
	// Convert a std::string to a ATL::CString or LPCTSTR
	std::unique_ptr<CString> to_lpctstr(const std::string& str);
}