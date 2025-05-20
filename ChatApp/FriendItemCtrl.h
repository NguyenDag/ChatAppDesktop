#pragma once
#include <afxwin.h>     // Cho CStatic, CString
#include <gdiplus.h> 
#include <vector>
using namespace Gdiplus;
using namespace std;

struct FriendItem
{
    CString name;
    Image* pImage;

    FriendItem(const CString& n, Image* img) : name(n), pImage(img) {}
};

class CFriendItemCtrl : public CListCtrl
{
public:
    CFriendItemCtrl();
    virtual ~CFriendItemCtrl();

    static void DrawCircleAvatar(Graphics& graphics, Image* pImage, const CRect& rect);
    void SetData(const CString& imagePath, const CString& name);
    void ClearFriends();

protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()

private:
    vector <Image*> m_pImage;
    vector <CString> m_name;
    CFont m_font;
};

