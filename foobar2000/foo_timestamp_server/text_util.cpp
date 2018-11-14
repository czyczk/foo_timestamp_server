#include "stdafx.h"
#include "text_util.h"

unsigned char text_util::from_hex(unsigned char ch)
{
	if (ch <= '9' && ch >= '0')
		ch -= '0';
	else if (ch <= 'f' && ch >= 'a')
		ch -= 'a' - 10;
	else if (ch <= 'F' && ch >= 'A')
		ch -= 'A' - 10;
	else
		ch = 0;
	return ch;
}

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

std::string text_util::to_utf8(const std::wstring & wstr) {
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), nullptr, 0, nullptr, nullptr);
	std::string utf8_str(utf8_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &utf8_str[0], utf8_size, nullptr, nullptr);

	return utf8_str;
}

std::string text_util::to_utf8(const std::string & str) {
	int size = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, str.c_str(), str.length(), nullptr, 0);
	std::wstring utf16_str(size, '\0');
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, str.c_str(), str.length(), &utf16_str[0], size);

	return to_utf8(utf16_str);
}

std::string text_util::url_decode(const std::string & str) {
	using namespace std;
	string result;
	string::size_type i;
	for (i = 0; i < str.size(); ++i) {
		if (str[i] == '+') {
			result += ' ';
		} else if (str[i] == '%' && str.size() > i + 2) {
			const unsigned char ch1 = from_hex(str[i + 1]);
			const unsigned char ch2 = from_hex(str[i + 2]);
			const unsigned char ch = (ch1 << 4) | ch2;
			result += ch;
			i += 2;
		} else {
			result += str[i];
		}
	}
	return result;
}
