#pragma once
#pragma once
#include <afxwin.h>
#include <string>

using namespace std;

CString Utf8ToCString(const std::string& utf8Str);
std::string CStringToUtf8(const CString& cstr);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp);
