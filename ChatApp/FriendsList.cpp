#include "pch.h"
#include "ChatApp.h"
#include "afxdialogex.h"
#include "FriendsList.h"
#include <urlmon.h>
#include <vector>

struct Friend {
	CString name;
	CString avatarPath;
};

vector<Friend> friends = {
	{ _T("Nguyễn Văn A"), _T("https://th.bing.com/th?id=OIF.StUEcUP%2bfiJoT%2bceDkb47A&rs=1&pid=ImgDetMain") },
	{ _T("Trần Thị B"), _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png") },
	{ _T("Lê Văn C"), _T("https://th.bing.com/th/id/OIP.7gtJht5peBdvIbqUptBqsgHaH7?cb=iwp2&rs=1&pid=ImgDetMain") },
	{ _T("Phạm Minh D"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/11.1.jpg?alt=media&token=b19c9101-aa13-47d3-91d3-763b2f3032dc") },
	{ _T("Hoàng Thị E"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/10.1.jpg?alt=media&token=6d0b3ce9-bbea-4a41-9af2-14559e1b0f9c") },
	{ _T("Đỗ Văn F"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/12.1.jpg?alt=media&token=7a73d56d-abbc-43f3-a229-dd9adfbd280f") },
	{ _T("Ngô Thị G"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/13.1.jpg?alt=media&token=0e0ad1e2-ca9c-4e90-b73e-894593a96ac8") },
	{ _T("Bùi Văn H"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/14.1.jpg?alt=media&token=ac23da06-256f-4d57-8b9b-80040145c142") },
	{ _T("Vũ Thị I"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/15.1.jpg?alt=media&token=d4606aa6-dc7a-4a96-8b65-db408d9d3d6e") },
	{ _T("Lý Văn K"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/16.1.jpg?alt=media&token=43621ce1-5045-42e8-bbef-e165500d8f7c") },
	{ _T("Trần Thị B"), _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png") },
	{ _T("Lê Văn C"), _T("https://th.bing.com/th/id/OIP.7gtJht5peBdvIbqUptBqsgHaH7?cb=iwp2&rs=1&pid=ImgDetMain") },
	{ _T("Phạm Minh D"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/11.1.jpg?alt=media&token=b19c9101-aa13-47d3-91d3-763b2f3032dc") },
	{ _T("Hoàng Thị E"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/10.1.jpg?alt=media&token=6d0b3ce9-bbea-4a41-9af2-14559e1b0f9c") },
	{ _T("Đỗ Văn F"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/12.1.jpg?alt=media&token=7a73d56d-abbc-43f3-a229-dd9adfbd280f") },
	{ _T("Ngô Thị G"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/13.1.jpg?alt=media&token=0e0ad1e2-ca9c-4e90-b73e-894593a96ac8") },
	{ _T("Bùi Văn H"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/14.1.jpg?alt=media&token=ac23da06-256f-4d57-8b9b-80040145c142") },
	{ _T("Vũ Thị I"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/15.1.jpg?alt=media&token=d4606aa6-dc7a-4a96-8b65-db408d9d3d6e") },
	{ _T("Lý Văn K"), _T("https://firebasestorage.googleapis.com/v0/b/nguyen-dang.appspot.com/o/16.1.jpg?alt=media&token=43621ce1-5045-42e8-bbef-e165500d8f7c") }
};

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "gdiplus.lib")
// FriendsList dialog

IMPLEMENT_DYNAMIC(FriendsList, CDialogEx)

FriendsList::FriendsList(CWnd* pParent /*=nullptr*/)
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

	m_listFriend.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	m_listFriend.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CImageList imageList;
	imageList.Create(50, 50, ILC_COLOR32, 0, 10);
	m_listFriend.SetImageList(&imageList, LVSIL_SMALL);

	for (size_t i = 0; i < friends.size(); ++i)
	{
		CString fileName;
		fileName.Format(_T("avatar\\friend_%d.png"), (int)i);

		// Tải ảnh từ URL về file local
		HRESULT hr = URLDownloadToFile(NULL, friends[i].avatarPath, fileName, 0, NULL);
		m_listFriend.SetData(fileName, friends[i].name);
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
END_MESSAGE_MAP()

