#pragma once
#include <afxwin.h>
#include <gdiplus.h>
using namespace Gdiplus;
class CImageButton : public CButton
{
public:
    CImageButton() : m_pBitmap(nullptr) {}
    virtual ~CImageButton() { if (m_pBitmap) delete m_pBitmap; }

    BOOL LoadImageFromFile(LPCTSTR filePath)
    {
        if (m_pBitmap)
        {
            delete m_pBitmap;
            m_pBitmap = nullptr;
        }
        m_pBitmap = Gdiplus::Bitmap::FromFile(filePath);
        return (m_pBitmap && m_pBitmap->GetLastStatus() == Gdiplus::Ok);
    }

protected:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override
    {
        CDC dc;
        dc.Attach(lpDrawItemStruct->hDC);

        CRect rect = lpDrawItemStruct->rcItem;

        // Vẽ nền button
        COLORREF bgColor = (lpDrawItemStruct->itemState & ODS_SELECTED) ? RGB(200, 200, 200) : RGB(240, 240, 240);
        dc.FillSolidRect(rect, bgColor);

        // Vẽ viền button
        dc.DrawEdge(rect, EDGE_RAISED, BF_RECT);

        if (m_pBitmap)
        {
            // Vẽ ảnh PNG có alpha bằng GDI+
            Graphics graphics(dc.GetSafeHdc());

            UINT width = m_pBitmap->GetWidth();
            UINT height = m_pBitmap->GetHeight();

            // Tính vị trí để vẽ ở giữa button
            int x = rect.left + (rect.Width() - width) / 2;
            int y = rect.top + (rect.Height() - height) / 2;

            graphics.DrawImage(m_pBitmap, x, y, width, height);
        }

        dc.Detach();
    }

private:
    Gdiplus::Bitmap* m_pBitmap;
};
