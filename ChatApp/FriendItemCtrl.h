#pragma once
#include <afxwin.h>     // Cho CStatic, CString
#include <gdiplus.h> 
#include <vector>
#include <string>

using namespace Gdiplus;
using namespace std;

struct FriendInfo {
    CString FriendID;
    CString FullName;
    CString Username;
    CString Avatar;
    Image* AvatarImage;
    bool isOnline;
    CString Content;
    bool isSend;
};

class CFriendItemCtrl : public CListCtrl
{
public:
    CFriendItemCtrl();
    virtual ~CFriendItemCtrl();

    static void DrawCircleAvatar(Graphics& graphics, Image* pImage, const CRect& rect);
    void SetData(const FriendInfo& f);
    void ClearFriends();

protected:
    afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct); // Owner draw
    DECLARE_MESSAGE_MAP()

private:
    vector <FriendInfo> friends;
    CFont m_font;
};

