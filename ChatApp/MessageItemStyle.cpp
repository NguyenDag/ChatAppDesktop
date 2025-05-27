#include "pch.h"
#include "MessageItemStyle.h"
#include <gdiplus.h>
#include <curl/curl.h>

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

IMPLEMENT_DYNAMIC(MessageItemStyle, CWnd)

MessageItemStyle::MessageItemStyle()
    : m_nScrollPos(0)
    , m_nScrollMax(0)
    , m_nMessageHeight(60)
    , m_nVisibleMessages(0)
{
    InitializeBrushes();
}

MessageItemStyle::~MessageItemStyle()
{
}

BEGIN_MESSAGE_MAP(MessageItemStyle, CWnd)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

BOOL MessageItemStyle::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    // Register window class
    CString strWndClass = AfxRegisterWndClass(
        CS_VREDRAW | CS_HREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        NULL);

    BOOL bResult = CWnd::Create(strWndClass, _T(""),
        dwStyle | WS_VSCROLL, rect, pParentWnd, nID);

    if (bResult)
    {
        InitializeFonts();
        UpdateScrollInfo();
    }

    return bResult;
}

void MessageItemStyle::AddMessage(const Message& message)
{
    m_messages.push_back(message);
    CDC* pDC = GetDC();
    CSize msgSize = CalculateMessageSize(pDC, message);
    ReleaseDC(pDC);

    m_messageHeights.push_back(msgSize.cy);
    UpdateScrollInfo();
    ScrollToBottom();
    Invalidate();
}

void MessageItemStyle::AddMessage(const CString& id, const CString& content,
    const std::vector<CString>& files, const std::vector<CString>& images,
    int isSend, const CTime& createdAt, int messageType)
{
    Message msg(id, content, files, images, isSend, createdAt, messageType);
    AddMessage(msg);
}

void MessageItemStyle::SetMessages(const std::vector<Message>& messages)
{
    m_messages = messages;
    m_messageHeights.clear();

    CDC* pDC = GetDC();
    for (const auto& msg : m_messages)
    {
        m_messageHeights.push_back(CalculateMessageSize(pDC, msg).cy);
    }
    ReleaseDC(pDC);

    UpdateScrollInfo();
    ScrollToBottom();
    Invalidate();
}

void MessageItemStyle::ClearMessages()
{
    m_messages.clear();
    m_nScrollPos = 0;
    UpdateScrollInfo();
    Invalidate();
}

void MessageItemStyle::ScrollToBottom()
{
    if (m_nScrollMax > 0)
    {
        m_nScrollPos = m_nScrollMax;
        SetScrollPos(SB_VERT, m_nScrollPos);
        Invalidate();
    }
}

void MessageItemStyle::OnPaint()
{
    CPaintDC dc(this);
    CRect clientRect;
    GetClientRect(&clientRect);

    dc.FillRect(&clientRect, &m_brushBg);

    if (m_messages.empty()) return;

    int yOffset = -m_nScrollPos;

    for (size_t i = 0; i < m_messages.size(); ++i)
    {
        int msgHeight = m_messageHeights[i];

        if (yOffset + msgHeight < 0)
        {
            yOffset += msgHeight;
            continue; 
        }

        if (yOffset > clientRect.Height())
            break; 

        CRect msgRect(0, yOffset, clientRect.Width(), yOffset + msgHeight);
        DrawMessage(&dc, m_messages[i], msgRect, (int)i);
        yOffset += msgHeight;
    }
}


void MessageItemStyle::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    m_nVisibleMessages = cy / m_nMessageHeight;
    UpdateScrollInfo();
    Invalidate();
}

BOOL MessageItemStyle::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // Prevent flicker
}

void MessageItemStyle::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int nPrevPos = m_nScrollPos;

    switch (nSBCode)
    {
    case SB_TOP:
        m_nScrollPos = 0;
        break;
    case SB_BOTTOM:
        m_nScrollPos = m_nScrollMax;
        break;
    case SB_LINEUP:
        m_nScrollPos = max(0, m_nScrollPos - m_nMessageHeight);
        break;
    case SB_LINEDOWN:
        m_nScrollPos = min(m_nScrollMax, m_nScrollPos + m_nMessageHeight);
        break;
    case SB_PAGEUP:
        m_nScrollPos = max(0, m_nScrollPos - (m_nVisibleMessages * m_nMessageHeight));
        break;
    case SB_PAGEDOWN:
        m_nScrollPos = min(m_nScrollMax, m_nScrollPos + (m_nVisibleMessages * m_nMessageHeight));
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        m_nScrollPos = nPos;
        break;
    }

    if (m_nScrollPos != nPrevPos)
    {
        SetScrollPos(SB_VERT, m_nScrollPos);
        Invalidate();
    }

    CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL MessageItemStyle::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int nScrollLines = 3; // Number of lines to scroll
    int nDelta = -zDelta / WHEEL_DELTA * nScrollLines * (m_nMessageHeight / 3);

    int nNewPos = max(0, min(m_nScrollMax, m_nScrollPos + nDelta));

    if (nNewPos != m_nScrollPos)
    {
        m_nScrollPos = nNewPos;
        SetScrollPos(SB_VERT, m_nScrollPos);
        Invalidate();
    }

    return TRUE;
}

void MessageItemStyle::DrawMessage(CDC* pDC, const Message& msg, CRect& rect, int index)
{
    if (IsOutgoingMessage(msg))
    {
        DrawOutgoingMessage(pDC, msg, rect);
    }
    else
    {
        DrawIncomingMessage(pDC, msg, rect);
    }
}

void MessageItemStyle::DrawOutgoingMessage(CDC* pDC, const Message& msg, CRect& rect)
{
    // Calculate content size
    CSize contentSize = CalculateTextSize(pDC, msg.GetContent(), rect.Width() * 2 / 3);

    // Add space for files and images
    int filesHeight = CalculateFilesHeight(msg.GetFiles());
    int imagesHeight = CalculateImagesHeight(msg.GetImages());

    // Calculate bubble size
    int bubbleWidth = min(contentSize.cx + (BUBBLE_PADDING * 2), rect.Width() * 2 / 3);
    int bubbleHeight = contentSize.cy + filesHeight + imagesHeight + (BUBBLE_PADDING * 2);

    // Position bubble on the right
    CRect bubbleRect;
    bubbleRect.right = rect.right - MESSAGE_PADDING;
    bubbleRect.left = bubbleRect.right - bubbleWidth;
    bubbleRect.top = rect.top + MESSAGE_PADDING;
    bubbleRect.bottom = bubbleRect.top + bubbleHeight;

    // Draw bubble
    DrawMessageBubble(pDC, bubbleRect, true);

    // Draw content
    DrawMessageContent(pDC, msg, bubbleRect);

    // Draw timestamp
    CRect timeRect = rect;
    timeRect.top = bubbleRect.bottom + 2;
    timeRect.right = bubbleRect.right;
    timeRect.left = bubbleRect.right - 100;
    DrawTimeStamp(pDC, msg, timeRect, true);

    // Draw checkmarks
    CRect checkRect;
    checkRect.left = bubbleRect.right + 5;
    checkRect.right = checkRect.left + 20;
    checkRect.top = bubbleRect.bottom - 15;
    checkRect.bottom = bubbleRect.bottom;
    DrawCheckmarks(pDC, checkRect);
}

void MessageItemStyle::DrawIncomingMessage(CDC* pDC, const Message& msg, CRect& rect)
{
    int leftMargin = MESSAGE_PADDING;

    // Draw avatar for incoming messages
    CRect avatarRect;
    avatarRect.left = MESSAGE_PADDING;
    avatarRect.right = avatarRect.left + AVATAR_SIZE;
    avatarRect.top = rect.top + MESSAGE_PADDING;
    avatarRect.bottom = avatarRect.top + AVATAR_SIZE;

    DrawAvatar(pDC, avatarRect, false);
    leftMargin = avatarRect.right + AVATAR_MARGIN;

    // Calculate content size
    CSize contentSize = CalculateTextSize(pDC, msg.GetContent(), rect.Width() * 2 / 3);

    // Add space for files and images
    int filesHeight = CalculateFilesHeight(msg.GetFiles());
    int imagesHeight = CalculateImagesHeight(msg.GetImages());

    // Calculate bubble size
    int bubbleWidth = min(contentSize.cx + (BUBBLE_PADDING * 2), rect.Width() * 2 / 3);
    int bubbleHeight = contentSize.cy + filesHeight + imagesHeight + (BUBBLE_PADDING * 2);

    // Position bubble on the left
    CRect bubbleRect;
    bubbleRect.left = leftMargin;
    bubbleRect.right = bubbleRect.left + bubbleWidth;
    bubbleRect.top = rect.top + MESSAGE_PADDING;
    bubbleRect.bottom = bubbleRect.top + bubbleHeight;

    // Draw bubble
    DrawMessageBubble(pDC, bubbleRect, false);

    // Draw content
    DrawMessageContent(pDC, msg, bubbleRect);

    // Draw timestamp
    CRect timeRect = rect;
    timeRect.top = bubbleRect.bottom + 2;
    timeRect.left = bubbleRect.left;
    timeRect.right = bubbleRect.left + 100;
    DrawTimeStamp(pDC, msg, timeRect, false);
}

void MessageItemStyle::DrawAvatar(CDC* pDC, CRect& rect, bool isOutgoing)
{
    pDC->SelectObject(&m_brushAvatar);
    pDC->Ellipse(&rect);

    // Draw simple face
    pDC->SetTextColor(RGB(255, 255, 255));
    pDC->SetBkMode(TRANSPARENT);
    CRect faceRect = rect;
    faceRect.DeflateRect(10, 15);
    pDC->DrawText(_T("👤"), &faceRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void MessageItemStyle::DrawMessageBubble(CDC* pDC, CRect& rect, bool isOutgoing)
{
    if (isOutgoing)
    {
        pDC->SelectObject(&m_brushOutgoing);
    }
    else
    {
        pDC->SelectObject(&m_brushIncoming);
    }

    // Draw rounded rectangle
    pDC->RoundRect(&rect, CPoint(15, 15));
}

void MessageItemStyle::DrawMessageContent(CDC* pDC, const Message& msg, CRect& rect)
{
    CRect contentRect = rect;
    contentRect.DeflateRect(BUBBLE_PADDING, BUBBLE_PADDING);

    // Set text color based on message type
    if (IsOutgoingMessage(msg))
    {
        pDC->SetTextColor(RGB(255, 255, 255));
    }
    else
    {
        pDC->SetTextColor(RGB(0, 0, 0));
    }

    pDC->SetBkMode(TRANSPARENT);
    pDC->SelectObject(&m_fontMessage);

    // Draw text content
    if (!msg.GetContent().IsEmpty())
    {
        CSize textSize = CalculateTextSize(pDC, msg.GetContent(), contentRect.Width());
        CRect textRect = contentRect;
        textRect.bottom = textRect.top + textSize.cy;
        pDC->DrawText(msg.GetContent(), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
        contentRect.top = textRect.bottom + 5;
    }

    // Draw files
    if (HasFiles(msg))
    {
        DrawFiles(pDC, msg.GetFiles(), contentRect);
        contentRect.top += CalculateFilesHeight(msg.GetFiles()) + 5;
    }

    // Draw images
    if (HasImages(msg))
    {
        DrawImages(pDC, msg.GetImages(), contentRect);
    }
}

void MessageItemStyle::DrawFiles(CDC* pDC, const std::vector<CString>& files, CRect& rect)
{
    pDC->SetTextColor(RGB(0, 100, 200));
    CRect fileRect = rect;
    fileRect.bottom = fileRect.top + FILE_ITEM_HEIGHT;

    for (const auto& file : files)
    {
        pDC->DrawText(_T("📎 ") + file, &fileRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        fileRect.OffsetRect(0, FILE_ITEM_HEIGHT);
    }
}

void MessageItemStyle::DrawImages(CDC* pDC, const std::vector<CString>& images, CRect& rect)
{
    CRect imageRect = rect;
    imageRect.bottom = imageRect.top + IMAGE_PREVIEW_HEIGHT;

    for (const auto& image : images)
    {
        // Draw placeholder for image
        pDC->Rectangle(&imageRect);
        pDC->SetTextColor(RGB(100, 100, 100));
        pDC->DrawText(_T("🖼️ ") + image, &imageRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        imageRect.OffsetRect(0, IMAGE_PREVIEW_HEIGHT + 5);
    }
}

void MessageItemStyle::DrawCheckmarks(CDC* pDC, CRect& rect)
{
    pDC->SetTextColor(RGB(100, 200, 100));
    pDC->SetBkMode(TRANSPARENT);
    pDC->DrawText(_T("✓✓"), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void MessageItemStyle::DrawTimeStamp(CDC* pDC, const Message& msg, CRect& rect, bool isOutgoing)
{
    pDC->SelectObject(&m_fontTime);
    pDC->SetTextColor(RGB(150, 150, 150));
    pDC->SetBkMode(TRANSPARENT);

    CString timeStr = msg.GetFormattedTime();
    DWORD alignment = isOutgoing ? DT_RIGHT : DT_LEFT;
    pDC->DrawText(timeStr, &rect, alignment | DT_VCENTER | DT_SINGLELINE);
}

CSize MessageItemStyle::CalculateMessageSize(CDC* pDC, const Message& msg)
{
    CSize contentSize = CalculateTextSize(pDC, msg.GetContent(), 200);
    int filesHeight = CalculateFilesHeight(msg.GetFiles());
    int imagesHeight = CalculateImagesHeight(msg.GetImages());

    int width = contentSize.cx + (BUBBLE_PADDING * 2) + MESSAGE_PADDING;
    int height = max(m_nMessageHeight, contentSize.cy + filesHeight + imagesHeight + (BUBBLE_PADDING * 2) + TIME_HEIGHT + MESSAGE_PADDING);

    return CSize(width, height);
}

CSize MessageItemStyle::CalculateTextSize(CDC* pDC, const CString& text, int maxWidth)
{
    if (text.IsEmpty())
        return CSize(0, 0);

    pDC->SelectObject(&m_fontMessage);
    CRect calcRect(0, 0, maxWidth, 0);
    pDC->DrawText(text, &calcRect, DT_CALCRECT | DT_WORDBREAK);
    return CSize(calcRect.Width(), calcRect.Height());
}

int MessageItemStyle::CalculateFilesHeight(const std::vector<CString>& files)
{
    return (int)files.size() * FILE_ITEM_HEIGHT;
}

int MessageItemStyle::CalculateImagesHeight(const std::vector<CString>& images)
{
    return (int)images.size() * (IMAGE_PREVIEW_HEIGHT + 5);
}

void MessageItemStyle::UpdateScrollInfo()
{
    CRect clientRect;
    GetClientRect(&clientRect);

    // Clear chiều cao cũ
    m_messageHeights.clear();

    // Tạo CDC để đo text
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&m_fontMessage);

    int totalHeight = 0;
    for (const auto& msg : m_messages)
    {
        int textHeight = CalculateTextSize(&dc, msg.GetContent(), clientRect.Width() * 2 / 3).cy;
        int filesHeight = CalculateFilesHeight(msg.GetFiles());
        int imagesHeight = CalculateImagesHeight(msg.GetImages());
        int msgHeight = textHeight + filesHeight + imagesHeight + BUBBLE_PADDING + TIME_HEIGHT + MESSAGE_PADDING;

        msgHeight = max(msgHeight, 40); // đảm bảo tối thiểu
        m_messageHeights.push_back(msgHeight);
        totalHeight += msgHeight;
    }

    m_nScrollMax = max(0, totalHeight - clientRect.Height());

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = totalHeight;
    si.nPage = clientRect.Height();
    si.nPos = m_nScrollPos;

    SetScrollInfo(SB_VERT, &si);
    dc.SelectObject(pOldFont);
}


void MessageItemStyle::InitializeFonts()
{
    m_fontMessage.CreateFont(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_fontTime.CreateFont(
        11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
}

void MessageItemStyle::InitializeBrushes()
{
    m_brushBg.CreateSolidBrush(RGB(245, 245, 245));
    m_brushOutgoing.CreateSolidBrush(RGB(74, 144, 226));  // Blue
    m_brushIncoming.CreateSolidBrush(RGB(230, 230, 230)); // Light gray
    m_brushAvatar.CreateSolidBrush(RGB(150, 150, 150));   // Gray for avatar
}