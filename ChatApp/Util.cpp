#include "pch.h"
#include "Util.h"
#include <vector>

using namespace std;

CString Utf8ToCString(const std::string& utf8Str)
{
	int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
	if (wideCharLen == 0) return CString();

	std::vector<WCHAR> wideCharBuf(wideCharLen);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideCharBuf.data(), wideCharLen);

	return CString(wideCharBuf.data());
}


std::string CStringToUtf8(const CString& cstr)
{
	if (cstr.IsEmpty()) return std::string();

	int utf8CharLen = WideCharToMultiByte(CP_UTF8, 0, cstr, -1, nullptr, 0, nullptr, nullptr);
	if (utf8CharLen == 0) return std::string();

	std::vector<CHAR> utf8CharBuf(utf8CharLen);
	WideCharToMultiByte(CP_UTF8, 0, cstr, -1, utf8CharBuf.data(), utf8CharLen, nullptr, nullptr);

	return std::string(utf8CharBuf.data());
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp)
{
	size_t totalSize = size * nmemb;
	userp->append((char*)contents, totalSize);
	return totalSize;
}