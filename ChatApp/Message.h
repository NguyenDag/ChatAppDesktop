#pragma once
#include <vector>

#include "nlohmann/json.hpp"
#include "Util.h"
#include "FileItem.h"
#include "ImageItem.h"

using json = nlohmann::json;
using namespace std;

class Message
{
public:
	Message() : isSend(0), messageType(0) {}

	Message(
		const CString& id,
		const CString& content,
		const std::vector<FileItem>& files,
		const std::vector<ImageItem>& images,
		int isSend,
		const CTime& createdAt,
		int messageType)
		: id(id)
		, content(content)
		, files(files)
		, images(images)
		, isSend(isSend)
		, createdAt(createdAt)
		, messageType(messageType)
	{
	}

	static Message FromJson(const json& item) {
		Message msg;
		msg.id = item.contains("id") ? Utf8ToCString(item["id"].get<std::string>()) : CString(_T(""));
		msg.content = item.contains("Content") ? Utf8ToCString(item["Content"].get<std::string>()) : CString(_T(""));
		msg.isSend = item.contains("isSend") ? item["isSend"].get<int>() : 0;
		msg.messageType = item.contains("MessageType") ? item["MessageType"].get<int>() : 0;

		if (item.contains("Files") && item["Files"].is_array()) {
			for (const auto& fileJson : item["Files"]) {
				msg.files.push_back(FileItem::FromJson(fileJson));
			}
		}

		if (item.contains("Images") && item["Images"].is_array()) {
			for (const auto& imageJson : item["Images"]) {
				msg.images.push_back(ImageItem::FromJson(imageJson));
			}
		}

		if (item.contains("CreatedAt") && item["CreatedAt"].is_string()) {
			CString isoTime = Utf8ToCString(item["CreatedAt"].get<std::string>());
			if (isoTime.GetLength() >= 19) {
				int year, month, day, hour, minute, second;
				_stscanf_s(isoTime, _T("%d-%d-%dT%d:%d:%d"),
					&year, &month, &day, &hour, &minute, &second);
				msg.createdAt = CTime(year, month, day, hour, minute, second);
			}
		}

		return msg;
	}

	CString GetFormattedTime() const {
		return createdAt.Format(_T("%d/%m/%Y %H:%M"));
	}

	CString GetId() const { return id; }
	void SetId(const CString& value) { id = value; }

	CString GetContent() const { return content; }
	void SetContent(const CString& value) { content = value; }

	const std::vector<FileItem>& GetFiles() const { return files; }
	void SetFiles(const std::vector<FileItem>& value) { files = value; }

	const std::vector<ImageItem>& GetImages() const { return images; }
	void SetImages(const std::vector<ImageItem>& value) { images = value; }

	int GetIsSend() const { return isSend; }
	void SetIsSend(int value) { isSend = value; }

	CTime GetCreatedAt() const { return createdAt; }
	void SetCreatedAt(const CTime& value) { createdAt = value; }

	int GetMessageType() const { return messageType; }
	void SetMessageType(int value) { messageType = value; }

private:
	CString id;
	CString content;
	std::vector<FileItem> files;
	std::vector<ImageItem> images;
	int isSend;
	CTime createdAt;
	int messageType;

};

