#pragma once
#include "nlohmann/json.hpp"
#include "Util.h"

using json = nlohmann::json;
class ImageItem {
public:
    CString fileName;
    CString url;
    CString id;

    static ImageItem FromJson(const json& j) {
        ImageItem item;
        item.fileName = j.contains("FileName") ? Utf8ToCString(j["FileName"].get<std::string>()) : _T("");
        item.url = j.contains("urlImage") ? Utf8ToCString(j["urlImage"].get<std::string>()) : _T("");
        item.id = j.contains("_id") ? Utf8ToCString(j["_id"].get<std::string>()) : _T("");
        return item;
    }
};

