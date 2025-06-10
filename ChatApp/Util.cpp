#include "pch.h"
#include "Util.h"
#include <vector>

CString Utf8ToCString(const std::string& utf8Str)
{
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (wideCharLen == 0) return CString();

    std::vector<WCHAR> wideCharBuf(wideCharLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideCharBuf.data(), wideCharLen);

    return CString(wideCharBuf.data());
}
CString FixUtf8InWideCString(const CString& input)
{
    const wchar_t* wideStr = input.GetString();

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    if (utf8Len == 0) return CString();

    std::vector<char> utf8Buf(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Buf.data(), utf8Len, NULL, NULL);

    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Buf.data(), -1, NULL, 0);
    if (wideCharLen == 0) return CString();

    std::vector<WCHAR> wideBuf(wideCharLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8Buf.data(), -1, wideBuf.data(), wideCharLen);

    return CString(wideBuf.data());
}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp)
{
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}