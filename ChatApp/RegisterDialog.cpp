// RegisterDialog.cpp : implementation file
//

#include "pch.h"
#include "ChatApp.h"
#include "afxdialogex.h"
#include "RegisterDialog.h"
#include "Login.h"


// RegisterDialog dialog

IMPLEMENT_DYNAMIC(RegisterDialog, CDialogEx)

RegisterDialog::RegisterDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGISTER_DIALOG, pParent)
	, name(_T(""))
	, username(_T(""))
	, password(_T(""))
	, confirmPassword(_T(""))
{

}

RegisterDialog::~RegisterDialog()
{
}

void RegisterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUT_NAME, name);
	DDX_Text(pDX, IDC_INPUT_USERNAME, username);
	DDX_Text(pDX, IDC_INPUT_PASSWORD, password);
	DDX_Text(pDX, IDC_INPUT_CONFIRMPASSWORD, confirmPassword);
	DDX_Control(pDX, IDC_BUTTON_REGISTER, m_btRegister);
	DDX_Control(pDX, IDC_STATIC_NAME, m_stName);
	DDX_Control(pDX, IDC_STATIC_USERNAME, m_stUsername);
	DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stPassword);
	DDX_Control(pDX, IDC_STATIC_CONFIRMPASSWORD, m_stConfirmPassword);
	DDX_Control(pDX, IDC_INPUT_NAME, m_edName);
	DDX_Control(pDX, IDC_INPUT_USERNAME, m_edUsername);
	DDX_Control(pDX, IDC_INPUT_PASSWORD, m_edPassword);
	DDX_Control(pDX, IDC_INPUT_CONFIRMPASSWORD, m_edConfirmPassword);
	DDX_Control(pDX, IDC_STATIC_ERROR, m_stError);
}

BOOL RegisterDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_brushBackground.CreateSolidBrush(RGB(255, 255, 255)); // màu nền mong muốn
	int dlgwidth = 600;
	int dlgheight = 500;

	int widthLable = 150;
	int heightLable = 23;
	int leftLable = 41;
	int leftEdit = 244;
	int widthEdit = 268;
	int heightEdit = 34;
	int topStart = 80;
	int spacing = 55;

	MoveWindow(0, 0, dlgwidth, dlgheight);
	CenterWindow();
	/*CRect rectName;
	m_stName.GetWindowRect(&rectName);*/

	m_fontText.CreateFont(
		20,                        // Chiều cao font (pixel)
		0,                         // Chiều rộng (0 = tự động)
		0,                         // Góc nghiêng
		0,                         // Góc nghiêng chữ
		FW_NORMAL,                // Độ đậm
		FALSE,                    // In nghiêng
		FALSE,                    // Gạch chân
		FALSE,                    // Gạch ngang
		ANSI_CHARSET,             // Charset
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		_T("Roboto"));            // Tên font
	m_stName.SetFont(&m_fontText);
	m_stUsername.SetFont(&m_fontText);
	m_stPassword.SetFont(&m_fontText);
	m_stConfirmPassword.SetFont(&m_fontText);

	int top = topStart;

	m_stName.MoveWindow(leftLable, top + 7, widthLable, heightLable);
	m_edName.MoveWindow(leftEdit, top, widthEdit, heightEdit);

	top += spacing;
	m_stUsername.MoveWindow(leftLable, top + 7, widthLable, heightLable);
	m_edUsername.MoveWindow(leftEdit, top, widthEdit, heightEdit);

	top += spacing;
	m_stPassword.MoveWindow(leftLable, top + 7, widthLable, heightLable);
	m_edPassword.MoveWindow(leftEdit, top, widthEdit, heightEdit);

	top += spacing;
	m_stConfirmPassword.MoveWindow(leftLable, top + 7, widthLable, heightLable);
	m_edConfirmPassword.MoveWindow(leftEdit, top, widthEdit, heightEdit);

	top += spacing;
	m_btRegister.MoveWindow(leftEdit, top + 30, 143, 37);

	CRect rectRegisterBtn;
	m_btRegister.GetWindowRect(&rectRegisterBtn);
	ScreenToClient(&rectRegisterBtn);

	top += spacing;

// Hiển thị lỗi ở giữa
	//lấy kích thước của ô static text
	CWnd* pStatic = GetDlgItem(IDC_STATIC_ERROR);
	if (pStatic)
	{
		CRect rectErrorStatic;
		pStatic->GetWindowRect(&rectErrorStatic);
		ScreenToClient(&rectErrorStatic);
		int newX = (dlgwidth - rectErrorStatic.Width()) / 2;
		pStatic->SetWindowPos(nullptr, newX, rectRegisterBtn.top + 60, rectErrorStatic.Width(), rectErrorStatic.Height(), SWP_NOZORDER | SWP_NOSIZE);
	}
	return TRUE;
}

void RegisterDialog::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
}

void RegisterDialog::SetColor(COLORREF bgColor, COLORREF textColor)
{
	m_bgColor = bgColor;
	m_textColor = textColor;
}

HBRUSH RegisterDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT id = pWnd->GetDlgCtrlID();

	if (nCtlColor == CTLCOLOR_DLG)
	{
		return (HBRUSH)m_brushBackground.GetSafeHandle(); // dùng brush của bạn
	}
	else if (id == IDC_STATIC_NAME)
	{
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
	else if (id == IDC_STATIC_CONFIRMPASSWORD)
	{
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	else if (id == IDC_STATIC_ERROR)
	{
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return m_brushTransparent;
	}
	//if (id == IDC_BUTTON_REGISTER)
	//{
	//	pDC->SetTextColor(RGB(0, 102, 255)); // Màu xanh dương
	//	pDC->SetBkMode(TRANSPARENT);
	//	return m_brushTransparent;
	//}
	
	return hbr;
}


BEGIN_MESSAGE_MAP(RegisterDialog, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_REGISTER, &RegisterDialog::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// RegisterDialog message handlers


void RegisterDialog::OnBnClickedButtonRegister()
{
	UpdateData(TRUE);
	CString errorMessage = _T("");
	m_stError.ShowWindow(SW_HIDE);

	if (name.Trim().IsEmpty() || username.Trim().IsEmpty() || password.IsEmpty() || confirmPassword.IsEmpty())
	{
		errorMessage = _T("Vui lòng điền đầy đủ thông tin!");
	}
	else if (username.Trim() == _T("user1"))
	{
		errorMessage = _T("Tài khoản đã tồn tại!");
	}
	else if (password != confirmPassword)
	{
		errorMessage = _T("Mật khẩu không khớp!");
	}

	if (!errorMessage.IsEmpty()) {
		m_stError.ShowWindow(SW_SHOW);
		m_stError.SetWindowTextW(errorMessage);
		return;
	}

	// Nếu đến đây thì đăng nhập thành công
	m_stError.SetWindowTextW(_T("")); // Xóa lỗi cũ (nếu có)
	AfxMessageBox(_T("Đăng nhập thành công"));

	EndDialog(IDOK);//close this dialog

	Login login;
	login.DoModal();
}
