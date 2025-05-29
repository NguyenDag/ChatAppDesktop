// MessageItem.cpp : implementation file
//

#include "pch.h"
#include "ChatApp.h"
#include "afxdialogex.h"
#include "MessageItem.h"
#include <vector>
#include "Message.h"
#include <curl/curl.h>
#include "Globals.h"

using namespace std;


// MessageItem dialog

IMPLEMENT_DYNAMIC(MessageItem, CDialogEx)

MessageItem::MessageItem(const CString& friendId, const CString& fullname, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{
	m_friendId = friendId;
	m_friendName = fullname;
}

MessageItem::~MessageItem()
{
}

void MessageItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_LIST_CHAT, m_listChat);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_editSearch);
}

BOOL MessageItem::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_friendName);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Lưu token lại để gọi GdiplusShutdown(gdiplusToken) khi thoát ứng dụng

	//========get size of screen==========
	CRect screenRect;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

	int screenWidth = screenRect.Width();
	int screenHeight = screenRect.Height();

	double widthRatio = 0.6;
	double heightRatio = 0.6;

	int dlgWidth = static_cast<int>(screenWidth * widthRatio);
	int dlgHeight = static_cast<int>(screenHeight * heightRatio);

	int dlgLeft = screenRect.left + (screenWidth - dlgWidth) / 2;
	int dlgTop = screenRect.top + (screenHeight - dlgHeight) / 2;

	MoveWindow(dlgLeft, dlgTop, dlgWidth, dlgHeight);
//========set for list control===========
	CRect clientRect;
	GetClientRect(&clientRect);

	// Vùng hiển thị message list
	CRect msgRect(10, 10, clientRect.Width() - 10, clientRect.Height() - 60);
	m_messageList.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL, msgRect, this, 1001);
	std::vector<Message> messages;
	std::string errorMessage;
	std::string friendIdStr = CT2A(m_friendId);

	if (GetMessages(g_accessToken, messages, errorMessage, friendIdStr)) {
		for (const auto& msg : messages) {
			m_messageList.AddMessage(
				msg.GetId(),
				msg.GetContent(),
				msg.GetFiles(),
				msg.GetImages(),
				msg.GetIsSend(),
				msg.GetCreatedAt(),
				msg.GetMessageType()
			);
		}
	}
	else {
		AfxMessageBox(CA2W(errorMessage.c_str()));
	}
//===========set Search bar=============
	CRect rectSearch;
	m_editSearch.GetWindowRect(&rectSearch);// lấy tọa độ màn hình
	ScreenToClient(&rectSearch);// chuyển về tọa độ client

	int left = rectSearch.left;
	int top = rectSearch.top;
	int width = 270;
	int height = rectSearch.Height();

	m_editSearch.MoveWindow(left, top, width, height);
	return TRUE;
}

void MessageItem::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_messageList.GetSafeHwnd())
	{
		m_messageList.MoveWindow(10, 10, cx - 20, cy - 60);
	}
}

bool MessageItem::GetMessages(const string& token, vector<Message>& message, string& errorMessage, const string& friendId)
{
	CURL* curl;
	CURLcode res;
	string readBuffer;

	curl = curl_easy_init();
	if (!curl) {
		errorMessage = "Không thể khởi tạo CURL";
		return false;
	}

	struct curl_slist* headers = nullptr;
	string authHeader = "Authorization: Bearer " + token;
	headers = curl_slist_append(headers, authHeader.c_str());

	std::string url = "http://30.30.30.85:8888/api/message/get-message?FriendID=" + friendId;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		errorMessage = string("Lỗi kết nối: ") + curl_easy_strerror(res);
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
		return false;
	}

	try {
		json response = json::parse(readBuffer);
		if (response["status"] == 1) {
			for (const auto& item : response["data"]) {
				Message msg = Message::FromJson(item);
				message.push_back(msg);
			}

			errorMessage.clear();
			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);
			return true;
		}
		else {
			errorMessage = response.value("message", "Lỗi không xác định");
		}
	}
	catch (const exception& e) {
		errorMessage = string("Lỗi phân tích JSON: ") + e.what();
	}

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	return false;
}

BEGIN_MESSAGE_MAP(MessageItem, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()
