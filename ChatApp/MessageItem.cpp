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

MessageItem::MessageItem(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{

}

void MessageItem::CreateSampleMessages(vector<Message>* messages)
{
	for (int i = 0; i < 50; ++i)
	{
		CString id;
		id.Format(_T("msg_%d"), i + 1);

		CString content;
		content.Format(_T("Đây là tin nhắn mẫu số Đây là tin nhắn mẫu số %d"), i + 1);

		std::vector<CString> files;
		if (i == 3) files.push_back(_T("example.pdf"));
		if (i == 7) files.push_back(_T("document.docx"));

		std::vector<CString> images;
		if (i == 2 || i == 6) images.push_back(_T("image_sample.jpg"));

		int isSend = (i % 2 == 0) ? 1 : 0;
		CTime createdAt = CTime::GetCurrentTime() - CTimeSpan(0, 0, i * 2, 0);
		int messageType = 0;

		messages->emplace_back(id, content, files, images, isSend, createdAt, messageType);
	}
}

MessageItem::~MessageItem()
{
}

void MessageItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_LIST_CHAT, m_listChat);
}

BOOL MessageItem::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	if (GetMessages(g_accessToken, messages, errorMessage)) {
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
	// Gửi một vài tin nhắn test
	/*m_messageList.AddMessage(_T("1"), _T("Xin chào!"), {}, {}, 0, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không. Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không"), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("1"), _T("The dialog includes working send functionality - you can type messages and they'll appear as blue bubbles on the right side. The emoji, attachment, and image buttons show placeholder dialogs that you can extend with actual functionality. The dialog includes working send functionality - you can type messages and they'll appear as blue bubbles on the right side. The emoji, attachment, and image buttons show placeholder dialogs that you can extend with actual functionality."), {}, {}, 0, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 0, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("1"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("1"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("1"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 0, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 0, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("1"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 1, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 0, CTime::GetCurrentTime(), 0);
	m_messageList.AddMessage(_T("2"), _T("Chào bạn! Đây là một tin nhắn dài dài dài để test xem word wrap hoạt động không."), {}, {}, 0, CTime::GetCurrentTime(), 0);*/

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

bool MessageItem::GetMessages(const string& token, vector<Message>& message, string& errorMessage)
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

	curl_easy_setopt(curl, CURLOPT_URL, "http://30.30.30.85:8888/api/message/get-message?FriendID=682d7fdbce77700d5a735b11");
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


// MessageItem message handlers
