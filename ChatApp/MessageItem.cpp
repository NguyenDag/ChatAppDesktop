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
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_editSearch);
	DDX_Control(pDX, IDC_BTN_SEND, m_btnSend);
	DDX_Control(pDX, IDC_BTN_IMAGE, m_btnImage);
	DDX_Control(pDX, IDC_BTN_FILE, m_btnFile);
	DDX_Control(pDX, IDC_BTN_EMOJI, m_btnEmoji);
}


CString GetAbsolutePath(LPCTSTR relativePath)
{
	CString fullPath;
	GetModuleFileName(NULL, fullPath.GetBuffer(MAX_PATH), MAX_PATH);
	fullPath.ReleaseBuffer();
	int pos = fullPath.ReverseFind('\\');
	if (pos != -1) {
		fullPath = fullPath.Left(pos + 1) + relativePath;
	}
	return fullPath;
}

ULONG_PTR gdiplusToken;

BOOL MessageItem::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_friendName);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
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
	LoadMessages();

	//===========set Search bar=============
	m_fontText.CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));
	CRect rectSearch;
	m_editSearch.GetWindowRect(&rectSearch);// lấy tọa độ màn hình
	ScreenToClient(&rectSearch);// chuyển về tọa độ client

	m_editSearch.SetFont(&m_fontText);

	int left = 10;
	int top = clientRect.Height() - 50;
	int width = static_cast<int> (clientRect.Width() * 0.8);
	int height = 42;

	m_editSearch.MoveWindow(left, top, width, height);

	//===========load PNG len static icon===========
	int buttonSize = 42;
	int buttonSpacing = 5;
	int startX = clientRect.Width() - (4 * buttonSize + 3 * buttonSpacing + 10);
	int buttonY = clientRect.Height() - 50;

	// Move và resize các button
	m_btnSend.MoveWindow(startX, buttonY, buttonSize, buttonSize);
	m_btnEmoji.MoveWindow(startX + buttonSize + buttonSpacing, buttonY, buttonSize, buttonSize);
	m_btnImage.MoveWindow(startX + 2 * (buttonSize + buttonSpacing), buttonY, buttonSize, buttonSize);
	m_btnFile.MoveWindow(startX + 3 * (buttonSize + buttonSpacing), buttonY, buttonSize, buttonSize);

	setIconButton(m_btnSend, AfxGetApp()->LoadIcon(IDI_ICON_SEND));
	setIconButton(m_btnImage, AfxGetApp()->LoadIcon(IDI_ICON_IMAGE));
	setIconButton(m_btnFile, AfxGetApp()->LoadIcon(IDI_ICON_ATTACH));
	setIconButton(m_btnEmoji, AfxGetApp()->LoadIcon(IDI_ICON_EMOJI));
	return TRUE;
}

void MessageItem::OnDestroy()
{
	CDialogEx::OnDestroy();
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

void MessageItem::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	CBrush brush(RGB(240, 240, 240));
	dc.FillRect(&rect, &brush);
}

HBRUSH MessageItem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		return (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
		pDC->SetTextColor(RGB(0, 0, 0));
		return (HBRUSH)m_hbrBackground;
	}

	if (pWnd == &m_messageList)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
		return (HBRUSH)m_hbrBackground;
	}

	return hbr;
}

void MessageItem::LoadButtonImage(CImageButton& button, LPCTSTR imagePath)
{
	CString fullPath = GetAbsolutePath(imagePath);
	button.LoadImageFromFile(fullPath);
}

bool MessageItem::SendMessageToFriend(const CString& token, const CString& friendID, const CString& content, const std::vector<FileItem>& files, CString& errorMessage)
{
	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;
	std::string response_str;
	long http_code = 0;
	curl_mime* mime = nullptr;
	struct curl_slist* headers = nullptr;
	nlohmann::json response;

	try {
		curl = curl_easy_init();
		if (!curl) {
			errorMessage = "Unable to initialize CURL";
			return false;
		}

		mime = curl_mime_init(curl);
		if (!mime) {
			errorMessage = "Unable to initialize curl_mime";
			curl_easy_cleanup(curl);
			return false;
		}

		std::string url = "http://30.30.30.85:8888/api/message/send-message";

		std::string authHeader = "Authorization: Bearer " + string(CT2A(token));
		headers = curl_slist_append(headers, authHeader.c_str());

		curl_mimepart* part = curl_mime_addpart(mime);
		curl_mime_name(part, "FriendID");
		curl_mime_data(part, CT2A(friendID), CURL_ZERO_TERMINATED);

		if (!content.IsEmpty()) {
			part = curl_mime_addpart(mime);
			curl_mime_name(part, "Content");
			curl_mime_data(part, CW2A(content, CP_UTF8), CURL_ZERO_TERMINATED);
		}

		for (const auto& file : files) {
			std::string filePath = CT2A(file.url);
			if (!filePath.empty()) {
				part = curl_mime_addpart(mime);
				curl_mime_name(part, "files");
				curl_mime_filedata(part, filePath.c_str());
			}
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_str);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			throw std::runtime_error(curl_easy_strerror(res));
		}

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (http_code != 200) {
			if (!response_str.empty()) {
				response = nlohmann::json::parse(response_str, nullptr, false);
				if (!response.is_discarded() && response.contains("message") && response["message"].is_string())
					throw std::runtime_error(response["message"].get<std::string>());
				else
					throw std::runtime_error("Error HTTP " + std::to_string(http_code));
			}
			else {
				throw std::runtime_error("Error HTTP " + std::to_string(http_code));
			}
		}


		response = nlohmann::json::parse(response_str, nullptr, false);
		if (response.is_discarded()) {
			throw std::runtime_error("Unable to analysis responses JSON");
		}

		int status = response.value("status", 0);
		if (status != 1) {
			std::string msg = response.value("message", "Fail to sent message!");
			throw std::runtime_error(msg);
		}

		curl_slist_free_all(headers);
		curl_mime_free(mime);
		curl_easy_cleanup(curl);

		return true;
	}
	catch (const std::exception& e) {
		errorMessage = e.what();

		if (headers) curl_slist_free_all(headers);
		if (mime) curl_mime_free(mime);
		if (curl) curl_easy_cleanup(curl);

		return false;
	}
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
		errorMessage = "Unable to initialize CURL";
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
		errorMessage = string("Connection error: ") + curl_easy_strerror(res);
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
			errorMessage = response.value("message", "Unknown error!");
		}
	}
	catch (const exception& e) {
		errorMessage = string("Analysis error JSON: ") + e.what();
	}

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	return false;
}

void MessageItem::LoadMessages()
{
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
			CString debugPoint;
			debugPoint.Format(_T("Index of files: %d\n"), msg.GetFiles());
			OutputDebugString(debugPoint);
		}
	}
	else {
		AfxMessageBox(CA2W(errorMessage.c_str()));
	}
}

void MessageItem::setIconButton(CMFCButton& _idc_button, HICON hicon)
{
	_idc_button.SetIcon(hicon);
	_idc_button.SizeToContent();
	_idc_button.m_bDrawFocus = FALSE;
	_idc_button.m_bTransparent = TRUE;
	_idc_button.m_nFlatStyle = CMFCButton::BUTTONSTYLE_NOBORDERS;
	_idc_button.SetFaceColor(RGB(240, 240, 240), TRUE);
	_idc_button.SetTextColor(RGB(100, 100, 100));
	_idc_button.SetWindowPos(nullptr, 0, 0, 40, 40, SWP_NOMOVE | SWP_NOZORDER);
}

BEGIN_MESSAGE_MAP(MessageItem, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_SEND, &MessageItem::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_IMAGE, &MessageItem::OnBnClickedBtnImage)
	ON_BN_CLICKED(IDC_BTN_FILE, &MessageItem::OnBnClickedBtnFile)
	ON_BN_CLICKED(IDC_BTN_EMOJI, &MessageItem::OnBnClickedBtnEmoji)
END_MESSAGE_MAP()

void MessageItem::OnBnClickedBtnSend()
{
	CString messageText;
	m_editSearch.GetWindowTextW(messageText);

	if (messageText.IsEmpty()) {
		return;
	}

	CString token = CA2T(g_accessToken.c_str());
	CString friendID = m_friendId;
	CString errorMessage;
	std::vector<FileItem> files;

	if (SendMessageToFriend(token, friendID, messageText, files, errorMessage)) {
		m_messageList.ClearMessages();
		LoadMessages();
		m_editSearch.SetWindowText(_T(""));
	}
}

void MessageItem::OnBnClickedBtnImage()
{
	CString token = CA2T(g_accessToken.c_str());
	CString friendID = m_friendId;
	CString errorMessage;
	CString content;
	std::vector<FileItem> selectedFiles;
	CString filter = _T("Image Files (*.bmp; *.jpg; *.jpeg; *.png; *.gif; *.tiff)|*.bmp;*.jpg;*.jpeg;*.png;*.gif;*.tiff|")
		_T("Bitmap Files (*.bmp)|*.bmp|")
		_T("JPEG Files (*.jpg;*.jpeg)|*.jpg;*.jpeg|")
		_T("PNG Files (*.png)|*.png|")
		_T("GIF Files (*.gif)|*.gif|")
		_T("TIFF Files (*.tiff)|*.tiff|")
		_T("All Files (*.*)|*.*||");

	CFileDialog openDlg(TRUE, _T("txt"), NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, filter, this);

	if (openDlg.DoModal() == IDOK) {
		POSITION pos = openDlg.GetStartPosition();
		while (pos != NULL) {
			CString cstrPath = openDlg.GetNextPathName(pos);

			FileItem item;
			item.url = cstrPath;
			int index = cstrPath.ReverseFind(_T('\\'));
			if (index != -1)
				item.fileName = cstrPath.Mid(index + 1);
			else
				item.fileName = cstrPath;

			selectedFiles.push_back(item);
		}

		if (!selectedFiles.empty()) {
			if (SendMessageToFriend(token, friendID, content, selectedFiles, errorMessage)) {
				m_messageList.ClearMessages();
				LoadMessages();
			}
		}
	}
}

void MessageItem::OnBnClickedBtnFile()
{
	CString token = CA2T(g_accessToken.c_str());
	CString friendID = m_friendId;
	CString errorMessage;
	CString content;
	std::vector<FileItem> selectedFiles;

	CFileDialog openDlg(TRUE, _T("txt"), NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
		_T("Text Files (*.txt)|*.txt|Data Files (*.dat)|*.dat|All Files (*.*)|*.*||"),
		this);

	if (openDlg.DoModal() == IDOK) {
		POSITION pos = openDlg.GetStartPosition();
		while (pos != NULL) {
			CString cstrPath = openDlg.GetNextPathName(pos);

			FileItem item;
			item.url = cstrPath;
			int index = cstrPath.ReverseFind(_T('\\'));
			if (index != -1)
				item.fileName = cstrPath.Mid(index + 1);
			else
				item.fileName = cstrPath;

			selectedFiles.push_back(item);
		}

		if (!selectedFiles.empty()) {
			if (SendMessageToFriend(token, friendID, content, selectedFiles, errorMessage)) {
				m_messageList.ClearMessages();
				LoadMessages();
			}
		}
	}
}

void MessageItem::OnBnClickedBtnEmoji()
{
	// TODO: Add your control notification handler code here
}

void DrawDownloadIcon(CDC* pDC, CRect rect)
{
	CPen pen(PS_SOLID, 2, RGB(0, 150, 0));
	CPen* oldPen = pDC->SelectObject(&pen);

	CPoint center = rect.CenterPoint();
	int size = min(rect.Width(), rect.Height()) / 3;

	pDC->MoveTo(center.x, center.y - size);
	pDC->LineTo(center.x, center.y + size);

	pDC->MoveTo(center.x - size / 2, center.y + size / 2);
	pDC->LineTo(center.x, center.y + size);
	pDC->LineTo(center.x + size / 2, center.y + size / 2);

	pDC->SelectObject(oldPen);
}
