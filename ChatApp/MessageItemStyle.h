#pragma once
#include <vector>
#include "Message.h"
using namespace std;

class MessageItemStyle : public CWnd
{
    DECLARE_DYNAMIC(MessageItemStyle)

public:
    MessageItemStyle();
    virtual ~MessageItemStyle();

    // Updated methods to work with Message class
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
    void AddMessage(const Message& message);
    void AddMessage(const CString& id, const CString& content, const std::vector<CString>& files = {},
        const std::vector<CString>& images = {}, int isSend = 1,
        const CTime& createdAt = CTime::GetCurrentTime(), int messageType = 0);
    void SetMessages(const std::vector<Message>& messages);
    void ClearMessages();
    void ScrollToBottom();
    // Get messages
    const std::vector<Message>& GetMessages() const { return m_messages; }
    size_t GetMessageCount() const { return m_messages.size(); }

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
    std::vector<Message> m_messages;
    std::vector<int> m_messageHeights;
    CFont m_fontMessage;
    CFont m_fontTime;
    CBrush m_brushBg;
    CBrush m_brushOutgoing;
    CBrush m_brushIncoming;
    CBrush m_brushAvatar;

    int m_nScrollPos;
    int m_nScrollMax;
    int m_nMessageHeight;
    int m_nVisibleMessages;

    // Constants
    static const int MESSAGE_PADDING = 15;
    static const int BUBBLE_PADDING = 12;
    static const int AVATAR_SIZE = 40;
    static const int AVATAR_MARGIN = 10;
    static const int TIME_HEIGHT = 22;
    static const int FILE_ITEM_HEIGHT = 25;
    static const int IMAGE_PREVIEW_HEIGHT = 100;

    // Private methods
    void DrawMessage(CDC* pDC, const Message& msg, CRect& rect, int index);
    void DrawOutgoingMessage(CDC* pDC, const Message& msg, CRect& rect);
    void DrawIncomingMessage(CDC* pDC, const Message& msg, CRect& rect);
    void DrawAvatar(CDC* pDC, CRect& rect, bool isOutgoing);
    void DrawMessageBubble(CDC* pDC, CRect& rect, bool isOutgoing);
    void DrawMessageContent(CDC* pDC, const Message& msg, CRect& rect);
    void DrawFiles(CDC* pDC, const std::vector<CString>& files, CRect& rect);
    void DrawImages(CDC* pDC, const std::vector<CString>& images, CRect& rect);
    void DrawCheckmarks(CDC* pDC, CRect& rect);
    void DrawTimeStamp(CDC* pDC, const Message& msg, CRect& rect, bool isOutgoing);

    CSize CalculateMessageSize(CDC* pDC, const Message& msg);
    CSize CalculateTextSize(CDC* pDC, const CString& text, int maxWidth);
    int CalculateFilesHeight(const std::vector<CString>& files);
    int CalculateImagesHeight(const std::vector<CString>& images);

    void UpdateScrollInfo();
    void InitializeFonts();
    void InitializeBrushes();

    // Helper methods for message types
    bool IsOutgoingMessage(const Message& msg) const { return msg.GetMessageType() == 1; }
    bool IsIncomingMessage(const Message& msg) const { return msg.GetMessageType() == 0; }
    bool HasFiles(const Message& msg) const { return !msg.GetFiles().empty(); }
    bool HasImages(const Message& msg) const { return !msg.GetImages().empty(); }
};