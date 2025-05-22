#pragma once
#include "afxdialogex.h"


// Login dialog

class Login : public CDialogEx
{
	DECLARE_DYNAMIC(Login)

public:
	Login(CWnd* pParent = nullptr);   // standard constructor
	virtual ~Login();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedLogin();
    void LoginAccount(const CString& username, const CString& password, CString& errorMessage);
	afx_msg void OnStnClickedRegister();
	DECLARE_MESSAGE_MAP()

private:
    CString username;
    CString password;
    BOOL rememberMe;

    CStatic m_stTitle;
    CStatic m_stRegister;
    CStatic m_stUsername;
    CStatic m_stPassword;
    CStatic m_stError;
    CEdit m_editUsername;
    CEdit m_editPassword;
    CButton m_chkRemember;
    CButton m_btnLogin;

    CFont m_fontTitle;
    CFont m_fontText;
    CFont m_fontLink;
    CBrush m_brushTransparent;
    CBrush m_brushBackground;
};
