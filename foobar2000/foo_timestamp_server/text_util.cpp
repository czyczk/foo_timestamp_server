#include "stdafx.h"
#include "text_util.h"

std::unique_ptr<std::wstring> text_util::to_wstring(const std::string & str)
{
	if (&str == nullptr) {
		return nullptr;
	}
	return std::make_unique<std::wstring>(str.begin(), str.end());
}

std::unique_ptr<CString> text_util::to_lpctstr(const std::string & str)
{
	if (&str == nullptr) {
		return nullptr;
	}
	auto result = std::make_unique<CString>();
	result->Format(_T("%s"), str);
	return result;
}
