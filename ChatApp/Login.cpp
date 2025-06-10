#include "pch.h"
#include "ChatApp.h"
#include "afxdialogex.h"
#include "Login.h"
#include "FriendsList.h"
#include "RegisterDialog.h"
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "Globals.h"
#include "MessageItem.h"

using json = nlohmann::json;
using namespace std;

IMPLEMENT_DYNAMIC(Login, CDialogEx)

Login::Login(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent)
	, username(_T(""))
	, password(_T(""))
	, rememberMe(FALSE)
{

}

Login::~Login()
{
}

void Login::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUT_USERNAME, username);
	DDX_Text(pDX, IDC_INPUT_PASSWORD, password);
	DDX_Check(pDX, IDC_CHECK_REMEMBER, rememberMe);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_stTitle);
	DDX_Control(pDX, IDC_STATIC_REGISTER, m_stRegister);
	DDX_Control(pDX, IDC_INPUT_USERNAME, m_editUsername);
	DDX_Control(pDX, IDC_INPUT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_CHECK_REMEMBER, m_chkRemember);
	DDX_Control(pDX, IDC_BUTTON_LOGIN, m_btnLogin);
	DDX_Control(pDX, IDC_STATIC_USERNAME, m_stUsername);
	DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stPassword);
	DDX_Control(pDX, IDC_STATIC_ERROR, m_stError);
}


BEGIN_MESSAGE_MAP(Login, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &Login::OnBnClickedLogin)
	ON_STN_CLICKED(IDC_STATIC_REGISTER, &Login::OnStnClickedRegister)
END_MESSAGE_MAP()


// Login message handlers
BOOL Login::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_brushBackground.CreateSolidBrush(RGB(255, 255, 255));

	CRect rect;
	GetWindowRect(&rect);

	int dlgwidth = 490;
	int dlgheight = 490;

	MoveWindow(0, 0, dlgwidth, dlgheight);
	CenterWindow();

	//set chiều rộng cho input username
	CRect rectUsername;
	m_editUsername.GetWindowRect(&rectUsername);// lấy tọa độ màn hình
	ScreenToClient(&rectUsername);// chuyển về tọa độ client

	int left = rectUsername.left; 
	int top = rectUsername.top;
	int width = 270;
	int height = rectUsername.Height();

	m_editUsername.MoveWindow(left, top, width, height);

	//set chiều rộng cho input password
	CRect rectPassword;
	m_editPassword.GetWindowRect(&rectPassword);// lấy tọa độ màn hình
	ScreenToClient(&rectPassword);// chuyển về tọa độ client

	int leftpass = rectPassword.left; 
	int toppass = rectPassword.top;

	m_editPassword.MoveWindow(leftpass, toppass, width, height);

	// Font tiêu đề
	m_fontTitle.CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));

	m_stTitle.SetFont(&m_fontTitle);

	// Font text & nút
	m_fontText.CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));
	m_btnLogin.SetFont(&m_fontText);
	m_editUsername.SetFont(&m_fontText);
	m_editPassword.SetFont(&m_fontText);
	m_chkRemember.SetFont(&m_fontText);
	m_stUsername.SetFont(&m_fontText);
	m_stPassword.SetFont(&m_fontText);

	// Font link đăng ký
	m_fontLink.CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Roboto"));
	m_stRegister.SetFont(&m_fontLink);

	CRect rectLoginBtn;
	m_btnLogin.GetWindowRect(&rectLoginBtn);
	ScreenToClient(&rectLoginBtn);

	// Hiển thị lỗi ở giữa
		//lấy kích thước của ô static text
	CWnd* pStatic = GetDlgItem(IDC_STATIC_ERROR);
	if (pStatic)
	{
		CRect rectErrorStatic;
		pStatic->GetWindowRect(&rectErrorStatic);
		ScreenToClient(&rectErrorStatic);
		int newX = (dlgwidth - rectErrorStatic.Width()) / 2;
		//m_stError.MoveWindow(newX, rectLoginBtn.top + 40, rectErrorStatic.Width(), rectErrorStatic.Height());
		pStatic->SetWindowPos(nullptr, newX, rectLoginBtn.top + 100, rectErrorStatic.Width(), rectErrorStatic.Height(), SWP_NOZORDER | SWP_NOSIZE);
	}
	//m_stError.CenterWindow();

	// Brush nền trong suốt
	m_brushTransparent.CreateStockObject(NULL_BRUSH);

	return TRUE;
}

HBRUSH Login::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT id = pWnd->GetDlgCtrlID();
	if (nCtlColor == CTLCOLOR_DLG)
	{
		return (HBRUSH)m_brushBackground.GetSafeHandle(); // dùng brush của bạn
	}else if (id == IDC_STATIC_TITLE)
	{
		pDC->SetTextColor(RGB(0, 102, 255)); // Màu xanh dương
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	else if (id == IDC_STATIC_REGISTER)
	{
		pDC->SetTextColor(RGB(0, 122, 255));
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	else if (id == IDC_STATIC_ERROR)
	{
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	else if (id == IDC_STATIC_USERNAME)
	{
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	else if (id == IDC_STATIC_PASSWORD)
	{
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	if (nCtlColor == CTLCOLOR_BTN)
	{
		if (pWnd->GetDlgCtrlID() == IDC_CHECK_REMEMBER)
		{
			pDC->SetBkMode(TRANSPARENT);
			return (HBRUSH)m_brushBackground.GetSafeHandle();
		}
	}

	return hbr;
}

void Login::OnBnClickedLogin()
{
	UpdateData(TRUE);
	CString errorMessage = _T("");
	m_stError.ShowWindow(SW_HIDE);
	//m_stError.SetWindowTextW(_T(""));

	if (username.IsEmpty())
	{
		errorMessage = _T("Tên đăng nhập không được để trống");
	}
	else if (password.IsEmpty())
	{
		errorMessage = _T("Mật khẩu không được để trống");
	}
	else
		LoginAccount(username, password, errorMessage);

	if (!errorMessage.IsEmpty()) {
		m_stError.ShowWindow(SW_SHOW);
		m_stError.SetWindowTextW(errorMessage);
		return;
	}

	m_stError.SetWindowTextW(_T("")); 
	AfxMessageBox(_T("Đăng nhập thành công"));

	//ShowWindow(SW_HIDE); // ẩn trang login
	//EndDialog;
	FriendsList friendsList;
	friendsList.DoModal();
	return;
}

// Callback để nhận dữ liệu từ libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
	size_t totalSize = size * nmemb;
	userp->append((char*)contents, totalSize);
	return totalSize;
}

void Login::LoginAccount(const CString& username, const CString& password, CString& errorMessage)
{
	CURL* curl;
	CURLcode res;
	string readBuffer;

	// JSON body
	string jsonBody = "{\"Username\":\"" + string(CT2A(username)) +
		"\",\"Password\":\"" + string(CT2A(password)) + "\"}";

	// Init libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl) {
		// URL & POST
		curl_easy_setopt(curl, CURLOPT_URL, "http://30.30.30.85:8888/api/auth/login");
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());

		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			errorMessage = _T("Lỗi kết nối: ") + CString(curl_easy_strerror(res));
		}
		else {
			try {
				json response = json::parse(readBuffer);
				if (response.contains("status") && response["status"] == 1 &&
					response.contains("data") && response["data"].contains("token")) {

					g_accessToken = response["data"]["token"].get<string>();

					fullName = CString(response["data"]["FullName"].get<string>().c_str());
					//avatar = CString(response["data"]["Avatar"].get<string>().c_str());
					errorMessage = _T(""); 
				}
				else {
					string msg = response.value("message", "Đăng nhập thất bại");
					errorMessage = CString(msg.c_str());
				}
			}
			catch (const exception& ex) {
				errorMessage = _T("Lỗi phân tích JSON: ") + CString(ex.what());
			}
		}
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}
	else {
		errorMessage = _T("Không thể khởi tạo libcurl!");
	}

	curl_global_cleanup();
}

void Login::OnStnClickedRegister()
{
	//ShowWindow(SW_HIDE);
	RegisterDialog regis;
	regis.DoModal();
	return;
}
