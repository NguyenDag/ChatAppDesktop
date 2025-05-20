#include "pch.h"
#include "FriendItemCtrl.h"
#pragma comment(lib, "gdiplus.lib")

BEGIN_MESSAGE_MAP(CFriendItemCtrl, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CFriendItemCtrl::CFriendItemCtrl()
{
}

CFriendItemCtrl::~CFriendItemCtrl()
{
}

void CFriendItemCtrl::DrawCircleAvatar(Graphics& graphics, Image* pImage, const CRect& rect)
{
    if (!pImage) return;

    GraphicsPath path;
    path.AddEllipse(rect.left, rect.top, rect.Width(), rect.Height());
    Region region(&path);
    graphics.SetClip(&region);

    graphics.DrawImage(pImage, rect.left, rect.top, rect.Width(), rect.Height());
    graphics.ResetClip();
}

void CFriendItemCtrl::SetData(const CString& imagePath, const CString& name)
{
    Image* pImg = Image::FromFile(imagePath);
    if (!pImg || pImg->GetLastStatus() != Ok)
    {
        if (pImg) delete pImg;
        pImg = nullptr;
    }

    m_name.push_back(name);
    m_pImage.push_back(Image::FromFile(imagePath));
    int index = InsertItem(GetItemCount(), _T(""));
    SetItemText(index, 0, name);

	Invalidate();
}

void CFriendItemCtrl::ClearFriends()
{

}

void CFriendItemCtrl::OnPaint()
{
    CPaintDC dc(this);
    Graphics graphics(dc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    CRect rect;
    GetClientRect(&rect);

    int avatarSize = 50;
    int padding = 10;
    // Vẽ nền trắng

    SolidBrush whiteBrush(Color(255, 255, 255));
    graphics.FillRectangle(&whiteBrush, rect.left, rect.top, rect.Width(), rect.Height());

    int textOffset = 10;
    int y = padding;

    Gdiplus::Font font(L"Segoe UI", 14, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(Color(0, 0, 0));

    for (size_t i = 0; i < m_pImage.size(); ++i)
    {
        CRect avatarRect(padding, y, padding + avatarSize, y + avatarSize);
        DrawCircleAvatar(graphics, m_pImage[i], avatarRect);

        // Vẽ tên bên phải avatar
        CStringW wideName(m_name[i]);
        graphics.DrawString(wideName, -1, &font,
            PointF((REAL)(avatarRect.right + textOffset), (REAL)(y + avatarSize / 4)),
            &textBrush);

        y += avatarSize + padding;
    }
}
