#include "pch.h"
#include "FriendItemCtrl.h"
#pragma comment(lib, "gdiplus.lib")

CFriendItemCtrl::CFriendItemCtrl()
{
}

CFriendItemCtrl::~CFriendItemCtrl()
{
}

BEGIN_MESSAGE_MAP(CFriendItemCtrl, CStatic)
END_MESSAGE_MAP()

void CFriendItemCtrl::SetData(const FriendInfo& f)
{
    FriendInfo info = f;
    CString url = _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png");
    CString localPath = _T("avatar\\avatar.png");

    if (url != 1)//test avatar != empty
    {
        Image* pImg = Image::FromFile(localPath);
        if (pImg && pImg->GetLastStatus() == Ok) {
            info.AvatarImage = pImg;
        }
        else {
            delete pImg;
        }
    }

    friends.push_back(info);
    int index = InsertItem(GetItemCount(), _T(""));
    SetItemText(index, 0, f.FullName);

    Invalidate();
}

void CFriendItemCtrl::ClearFriends()
{
    for (auto& f : friends)
        delete f.AvatarImage;
    friends.clear();
    DeleteAllItems();
}

const FriendInfo* CFriendItemCtrl::GetFriendInfoAt(int index) const
{
    if (index >= 0 && index < friends.size()) {
        return &friends[index];
    }
    return nullptr;
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



void CFriendItemCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    Graphics graphics(pDC->GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    int index = lpDrawItemStruct->itemID;
    if (index >= (int)friends.size()) return;

    CRect rect = lpDrawItemStruct->rcItem;
    const FriendInfo& f = friends[index];

    // Vẽ avatar
    int avatarSize = 40;
    int padding = 8;
    CRect avatarRect(rect.left + padding, rect.top + padding,
        rect.left + padding + avatarSize, rect.top + padding + avatarSize);

    if (f.AvatarImage)
        DrawCircleAvatar(graphics, f.AvatarImage, avatarRect);

    Gdiplus::Font font(L"Roboto", 14);
    SolidBrush textBrush(Color(0, 0, 0));
    CStringW nameW(f.FullName);
    graphics.DrawString(nameW, -1, &font,
        PointF((REAL)(avatarRect.right + 10), (REAL)(rect.top + padding)),
        &textBrush);
}
