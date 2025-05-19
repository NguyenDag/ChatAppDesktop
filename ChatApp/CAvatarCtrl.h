#pragma once

#include <afxwin.h>     // Cho CStatic, CString
#include <gdiplus.h>    // GDI+ cho xử lý ảnh
using namespace Gdiplus;
class CAvatarCtrl : public CStatic
{
public:
	CAvatarCtrl();
	virtual ~CAvatarCtrl();
	void SetImagePath(const CString& path);

protected:
	virtual void PreSubclassWindow();
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
	CString m_strImagePath;
	CBitmap m_bitmap;
	Image* m_pImage;
	ULONG_PTR m_gdiplusToken;
};

