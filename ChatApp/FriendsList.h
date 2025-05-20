#pragma once
#include "afxdialogex.h"
#include <gdiplus.h>
#include "FriendItemCtrl.h"
#include <vector>
using namespace Gdiplus;
using namespace std;


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
	virtual void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy(); // để shutdown GDI+

	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBackground;
	CStatic m_avatarCtrl;
	Image* m_avatarImage;
	CFriendItemCtrl m_listFriend;

	// GDI+ khởi tạo
	GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;
	CStatic m_stTitle;
	CStatic m_stFullName;
	CEdit m_editSearch;

	CFont m_fontTitle;
	CFont m_fontText;
	CImageList m_ImageList;
};
