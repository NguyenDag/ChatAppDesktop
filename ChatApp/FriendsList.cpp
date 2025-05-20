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
	{ _T("Lê Văn C"), _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png") }
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
}

BOOL FriendsList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Lấy kích thước màn hình
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

	// Tạo thư mục lưu nếu chưa có
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

	//set vị trí avatar 
	int avatarX = dlgWidth - 120;
	int avatarY = 30; //margin top
	int avatarWidth = 70;
	int avatarHeight = 70;
	
	// Di chuyển control đến vị trí mong muốn
	m_avatarCtrl.MoveWindow(avatarX, avatarY, avatarWidth, avatarHeight);

	//set fullname
	CRect rectFullName;
	m_stFullName.GetWindowRect(&rectFullName);// lấy tọa độ màn hình

	int widthFullName = rectFullName.Width();
	int heightFullName = rectFullName.Height();

	m_stFullName.MoveWindow(avatarX, avatarY + 90, widthFullName, heightFullName);

	m_fontText.CreateFont(25, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));
	m_stFullName.SetFont(&m_fontText);

	//set logo bkav
	m_fontTitle.CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));

	m_stTitle.SetFont(&m_fontTitle);

	//set chiều rộng cho input search
	CRect rectSearch;
	m_editSearch.GetWindowRect(&rectSearch);// lấy tọa độ màn hình
	ScreenToClient(&rectSearch);// chuyển về tọa độ client

	int width = static_cast<int>(screenWidth * 0.4);;
	int height = rectSearch.Height();
	int left =(dlgWidth - width) / 2;
	int top = static_cast<int>(dlgHeight * 0.15);

	m_editSearch.MoveWindow(left, top, width, height);

	//set vị trí list Friend
	CRect rectList;
	m_listFriend.GetWindowRect(&rectList);// lấy tọa độ màn hình

	int widthList = static_cast<int>(screenWidth * 0.6);
	int heightList = static_cast<int>(screenHeight * 0.45);
	int leftList = (dlgWidth - widthList) / 2;
	int topList = static_cast<int>(dlgHeight * 0.3);

	m_listFriend.MoveWindow(leftList, topList, widthList, heightList);

	//m_listFriend.ModifyStyle(0, 0x0020 | LVS_REPORT);
	//m_listFriend.InsertColumn(0, _T(""), LVCFMT_LEFT, 600);
	m_listFriend.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CImageList imageList;
	imageList.Create(50, 50, ILC_COLOR32, 0, 10);
	m_listFriend.SetImageList(&imageList, LVSIL_SMALL);

	for (size_t i = 0; i < friends.size(); ++i)
	{
		m_listFriend.SetData(localPath, friends[i].name);
	}

	return TRUE;
}

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
		pDC->SetTextColor(RGB(0, 102, 255)); // Màu xanh dương
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
	else if (id == IDC_STATIC_FULLNAME)
	{
		//pDC->SetTextColor(RGB(0, 102, 255)); // Màu xanh dương
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
	return hbr;
}


BEGIN_MESSAGE_MAP(FriendsList, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
END_MESSAGE_MAP()

