#include "pch.h"
#include "ChatApp.h"
#include "afxdialogex.h"
#include "FriendsList.h"
#include <urlmon.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>
#include "Globals.h"
#include "Util.h"
#include "MessageItem.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "gdiplus.lib")

using json = nlohmann::json;

IMPLEMENT_DYNAMIC(FriendsList, CDialogEx)

FriendsList::FriendsList(CWnd* pParent)
	: CDialogEx(IDD_FRIENDSLIST_DIALOG, pParent), m_gdiplusToken(0)
{
}

FriendsList::~FriendsList()
{
}

void FriendsList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AVATAR, m_avatarCtrl);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_stTitle);
	DDX_Control(pDX, IDC_INPUT_SEARCH, m_editSearch);
	DDX_Control(pDX, IDC_STATIC_FULLNAME, m_stFullName);
	DDX_Control(pDX, IDC_LIST_FRIEND, m_listFriend);
	DDX_Control(pDX, IDC_STATIC_NAME_LIST_FRIEND, m_stNameListFriend);
}

BOOL FriendsList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//========get size of screen==========
	CRect screenRect;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

	int screenWidth = screenRect.Width();
	int screenHeight = screenRect.Height();

	double widthRatio = 0.7;
	double heightRatio = 0.8;

	int dlgWidth = static_cast<int>(screenWidth * widthRatio);
	int dlgHeight = static_cast<int>(screenHeight * heightRatio);

	int dlgLeft = screenRect.left + (screenWidth - dlgWidth) / 2;
	int dlgTop = screenRect.top + (screenHeight - dlgHeight) / 2;

	MoveWindow(dlgLeft, dlgTop, dlgWidth, dlgHeight);

	// Khởi tạo GDI+
	GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, nullptr);

	m_brushBackground.CreateSolidBrush(RGB(255, 255, 255));

	//============set folder if not exists========
	CreateDirectory(_T("avatar"), NULL);

	CString url = _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png");
	CString localPath = _T("avatar\\avatar.png");

	HRESULT hr = URLDownloadToFile(NULL, url, localPath, 0, NULL);
	if (SUCCEEDED(hr))
	{
		m_avatarImage = Image::FromFile(localPath);
	}
	else
	{
		AfxMessageBox(_T("Tải ảnh avatar thất bại!"));
	}

	//=============set vị trí avatar========== 
	int avatarX = dlgWidth - 120;
	int avatarY = 30; //margin top
	int avatarWidth = 70;
	int avatarHeight = 70;

	m_avatarCtrl.MoveWindow(avatarX, avatarY, avatarWidth, avatarHeight);

	//==========set name for account=========
	CRect rectFullName;
	m_stFullName.GetWindowRect(&rectFullName);

	int widthFullName = rectFullName.Width();
	int heightFullName = rectFullName.Height();

	m_stFullName.MoveWindow(avatarX, avatarY + 90, widthFullName, heightFullName);

	m_fontText.CreateFont(25, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));
	m_stFullName.SetFont(&m_fontText);
	m_stNameListFriend.SetFont(&m_fontText);

	//set logo bkav font
	m_fontTitle.CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));

	m_stTitle.SetFont(&m_fontTitle);

	//==========set for search input=========
	CRect rectSearch;
	m_editSearch.GetWindowRect(&rectSearch);
	ScreenToClient(&rectSearch);

	int width = static_cast<int>(screenWidth * 0.4);;
	int height = rectSearch.Height();
	int left = (dlgWidth - width) / 2;
	int top = static_cast<int>(dlgHeight * 0.15);

	m_editSearch.MoveWindow(left, top, width, height);

	//==========set for list control=============
	CRect rectList;
	m_listFriend.GetWindowRect(&rectList);

	int widthList = static_cast<int>(screenWidth * 0.6);
	int heightList = static_cast<int>(screenHeight * 0.45);
	int leftList = (dlgWidth - widthList) / 2;
	int topList = static_cast<int>(dlgHeight * 0.3);

	m_listFriend.MoveWindow(leftList, topList, widthList, heightList);

	m_listFriend.ModifyStyle(LVS_SORTASCENDING | LVS_SORTDESCENDING, LVS_OWNERDRAWFIXED | LVS_REPORT);
	m_listFriend.InsertColumn(0, _T(""), LVCFMT_LEFT, 600);
	m_listFriend.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CImageList imageList;
	imageList.Create(50, 50, ILC_COLOR32, 0, 10);
	m_listFriend.SetImageList(&imageList, LVSIL_SMALL);

//========handling get list friends by token========
	vector<FriendInfo> friends;
	string error;

	if (GetFriendList(g_accessToken, friends, error)) {
		for (const auto& friendInfo : friends) {
			m_listFriend.SetData(friendInfo);
		}
	}
	else {
		MessageBoxA(nullptr, error.c_str(), "Lỗi", MB_ICONERROR);
	}
	//==========set title for list control========
	CRect rectNameList;
	m_stNameListFriend.GetWindowRect(&rectNameList);

	int topTitleList = static_cast<int>(dlgHeight * 0.25);

	m_stNameListFriend.MoveWindow(leftList + 10, topTitleList, rectNameList.Width(), rectNameList.Height());
	return TRUE;
}

//=========Paint for avatar==========
void FriendsList::OnPaint()
{
	CPaintDC dc(this);
	CDialogEx::OnPaint();
	if (m_avatarImage)
	{
		CRect rect;
		m_avatarCtrl.GetWindowRect(&rect);
		ScreenToClient(&rect);

		int size = min(rect.Width(), rect.Height());

		Graphics graphics(dc);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		path.AddEllipse(rect.left, rect.top, size, size);
		Region region(&path);
		graphics.SetClip(&path);

		graphics.DrawImage(m_avatarImage, rect.left, rect.top, size, size);
	}
}

void FriendsList::OnDestroy()
{
	CDialogEx::OnDestroy();

	// Giải phóng GDI+
	GdiplusShutdown(m_gdiplusToken);
}

void FriendsList::OnNMClickListFriend(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pNMItemActivate->iItem;

	if (nItem >= 0) {
		const FriendInfo* pFriend = m_listFriend.GetFriendInfoAt(nItem);
		if (pFriend) {
			CString friendId = pFriend->FriendID;
			CString fullname = pFriend->FullName;

			// Mở dialog chat
			MessageItem* pChatDlg = new MessageItem(friendId, fullname);
			pChatDlg->Create(IDD_CHAT_DIALOG, this);
			pChatDlg->ShowWindow(SW_SHOW);
		}
	}

	*pResult = 0;
}

bool FriendsList::GetFriendList(const string& token, vector<FriendInfo>& friends, string& errorMessage)
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

	curl_easy_setopt(curl, CURLOPT_URL, "http://30.30.30.85:8888/api/message/list-friend");
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
				FriendInfo f;

				f.FullName = Utf8ToCString(item.value("FullName", ""));
				f.FriendID = Utf8ToCString(item.value("FriendID", ""));
				f.Username = Utf8ToCString(item.value("Username", ""));
				f.Avatar = Utf8ToCString(item.value("Avatar", ""));
				f.isOnline = item.value("isOnline", false);
				f.Content = Utf8ToCString(item.value("Content", ""));
				f.isSend = item.value("isSend", 0);

				friends.push_back(f);
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

HBRUSH FriendsList::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT id = pWnd->GetDlgCtrlID();
	if (nCtlColor == CTLCOLOR_DLG)
	{
		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
	else if (id == IDC_STATIC_TITLE)
	{
		pDC->SetTextColor(RGB(0, 102, 255)); // Blue
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
	else if (id == IDC_STATIC_FULLNAME)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
	else if (id == IDC_STATIC_NAME_LIST_FRIEND)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
	return hbr;
}


BEGIN_MESSAGE_MAP(FriendsList, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_NOTIFY(NM_CLICK, IDC_LIST_FRIEND, &FriendsList::OnNMClickListFriend)
END_MESSAGE_MAP()

