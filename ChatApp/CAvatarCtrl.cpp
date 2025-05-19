#include "pch.h"
#include "CAvatarCtrl.h"
#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")

BEGIN_MESSAGE_MAP(CAvatarCtrl, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CAvatarCtrl::CAvatarCtrl() 
    : m_pImage(nullptr), m_gdiplusToken(0) 
{
}

CAvatarCtrl::~CAvatarCtrl() {
    if (m_pImage)
    {
        delete m_pImage;
        m_pImage = nullptr;
    }

    if (m_gdiplusToken != 0)
    {
        GdiplusShutdown(m_gdiplusToken);
    }
}

void CAvatarCtrl::PreSubclassWindow()
{
    if (m_gdiplusToken == 0)
    {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
    }
    CStatic::PreSubclassWindow();
}

void CAvatarCtrl::SetImagePath(const CString& path)
{
    m_strImagePath = path;

    if (m_pImage)
    {
        delete m_pImage;
        m_pImage = nullptr;
    }

    m_pImage = Image::FromFile(path);
    if (m_pImage == nullptr || m_pImage->GetLastStatus() != Ok)
    {
        AfxMessageBox(_T("Lỗi khi tải ảnh avatar."));
        delete m_pImage;
        m_pImage = nullptr;
    }

    Invalidate();
}

void CAvatarCtrl::OnPaint()
{
    CPaintDC dc(this);
    Graphics graphics(dc);

    CRect rect;
    GetClientRect(&rect);

    // Chống răng cưa
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // Vẽ nền trắng
    SolidBrush whiteBrush(Color(255, 255, 255));
    graphics.FillRectangle(&whiteBrush, rect.left, rect.top, rect.Width(), rect.Height());

    if (m_pImage && m_pImage->GetLastStatus() == Ok)
    {
        // Tạo hình tròn để clip
        GraphicsPath path;
        path.AddEllipse(rect.left, rect.top, rect.Width(), rect.Height());

        Region region(&path);
        graphics.SetClip(&region);

        // Vẽ ảnh vào vùng bo tròn
        graphics.DrawImage(m_pImage, rect.left, rect.top, rect.Width(), rect.Height());
    }
}