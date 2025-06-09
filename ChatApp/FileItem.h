#pragma once
#include "nlohmann/json.hpp"
#include "Util.h"

using json = nlohmann::json;
using namespace std;
class FileItem
{
public:
	CString fileName;
	CString url;
	CString id;

	static FileItem FromJson(const json& j) {
		FileItem item;
		item.fileName = j.contains("FileName") ? Utf8ToCString(j["FileName"].get<string>()) : _T("");
		item.url = j.contains("urlFile") ? Utf8ToCString(j["urlFile"].get<string>()) : _T("");
		item.id = j.contains("_id") ? Utf8ToCString(j["_id"].get<string>()) : _T("");
		return item;
	}
};

