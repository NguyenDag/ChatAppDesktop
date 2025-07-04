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
    void AddMessage(const CString& id, const CString& content, const std::vector<FileItem>& files = {},
        const std::vector<ImageItem>& images = {}, int isSend = 1,
        const CTime& createdAt = CTime::GetCurrentTime(), int messageType = 0);
    void SetMessages(const std::vector<Message>& messages);
    void ClearMessages();
    void ScrollToBottom();
    // Get messages
    const std::vector<Message>& GetMessages() const { return m_messages; }
    size_t GetMessageCount() const { return m_messages.size(); }

    bool HandleFileClick(CPoint point, int& fileIndex, int& imageIndex);
    void DownloadFile(const FileItem& file);
    void DownloadImage(const ImageItem& image);

    bool DownloadFileFromServer(const FileItem& fileItem, const CString& savePath);
    bool DownloadImageFromServer(const ImageItem& fileItem, const CString& savePath);

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

private:
    vector<Message> m_messages;
    vector<int> m_messageHeights;
    vector<CRect> m_downloadRects;
    vector<CRect> m_downloadImagesRects;

    vector<FileItem> m_currentFiles;
    vector<ImageItem> m_currentImages;
    
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
    static const int BUBBLE_PADDING = 10;
    static const int AVATAR_SIZE = 40;
    static const int AVATAR_MARGIN = 10;
    static const int TIME_HEIGHT = 22;
    static const int FILE_ITEM_HEIGHT = 30;
    static const int IMAGE_PREVIEW_HEIGHT = 200 / 3 * 4;
    static const int FILE_ITEM_PADDING = 10;
    static const int MAX_FILE_ITEM_WIDTH = 200;

    // Private methods
    void DrawMessage(CDC* pDC, const Message& msg, CRect& rect, int index);
    void DrawOutgoingMessage(CDC* pDC, const Message& msg, CRect& rect);
    void DrawIncomingMessage(CDC* pDC, const Message& msg, CRect& rect);
    void DrawAvatar(CDC* pDC, CRect& rect, bool isOutgoing);
    void DrawMessageBubble(CDC* pDC, CRect& rect, bool isOutgoing);
    void DrawMessageContent(CDC* pDC, const Message& msg, CRect& rect);
    void DrawFiles(CDC* pDC, const std::vector<FileItem>& files, CRect& rect);
    void DrawImages(CDC* pDC, const std::vector<ImageItem>& images, CRect& rect);
    void DrawCheckmarks(CDC* pDC, CRect& rect);
    void DrawTimeStamp(CDC* pDC, const Message& msg, CRect& rect, bool isOutgoing);

    CSize CalculateMessageSize(CDC* pDC, const Message& msg);
    CSize CalculateTextSize(CDC* pDC, const CString& text, int maxWidth);
    int CalculateFilesHeight(const std::vector<FileItem>& files);
    int CalculateImagesHeight(const std::vector<ImageItem>& images);

    CString GetFileIcon(const CString& extension);

    void UpdateScrollInfo();
    void InitializeFonts();
    void InitializeBrushes();

    // Helper methods for message types
    bool IsOutgoingMessage(const Message& msg) const { return msg.GetMessageType() == 1; }
    bool IsIncomingMessage(const Message& msg) const { return msg.GetMessageType() == 0; }
    bool HasFiles(const Message& msg) const { return !msg.GetFiles().empty(); }
    bool HasImages(const Message& msg) const { return !msg.GetImages().empty(); }

    bool DownloadImageIfNotExists(const ImageItem image, CString& localPathOut);
};