#pragma once
#include "afxdialogex.h"
#include "CAvatarCtrl.h"
#include <gdiplus.h>
using namespace Gdiplus;


// FriendsList dialog

class FriendsList : public CDialogEx
{
	DECLARE_DYNAMIC(FriendsList)

public:
	FriendsList(CWnd* pParent = nullptr);   // standard constructor
	virtual ~FriendsList();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRIENDSLIST_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy(); // để shutdown GDI+

	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBackground;
	CAvatarCtrl m_avatarCtrl;

	// GDI+ khởi tạo
	GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;
	CStatic m_stTitle;
	CStatic m_stFullName;
	CEdit m_editSearch;

	CFont m_fontTitle;
	CFont m_fontText;
	CListCtrl m_listFriend;
};
