#pragma once
#include "afxdialogex.h"


// RegisterDialog dialog

class RegisterDialog : public CDialogEx
{
	DECLARE_DYNAMIC(RegisterDialog)

public:
	RegisterDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~RegisterDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REGISTER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void SetColor(COLORREF bgColor, COLORREF textColor);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonRegister();

	void RegisterAccount(const CString& fullName, const CString& username, const CString& password, CString& errorMessage);

	DECLARE_MESSAGE_MAP()
private:
	CString name;
	CString username;
	CString password;
	CString confirmPassword;
	CButton m_btRegister;
	CButton m_btClose;

	CFont m_fontTitle;
	CFont m_fontText;
	CStatic m_titleBar;
	CStatic m_stName;
	CStatic m_stUsername;
	CStatic m_stPassword;
	CStatic m_stConfirmPassword;
	CStatic m_stError;
	CEdit m_edName;
	CEdit m_edUsername;
	CEdit m_edPassword;
	CEdit m_edConfirmPassword;
	CBrush m_brushTransparent;
	CBrush m_brushBackground;

	COLORREF m_bgColor = RGB(0, 122, 255);
	COLORREF m_textColor = RGB(255, 255, 255);
};
