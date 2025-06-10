#include "pch.h"
#include "MessageItemStyle.h"
#include <gdiplus.h>
#include <curl/curl.h>
#include <iostream>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

#include <shlobj.h>  // SHGetFolderPath
#include <atlstr.h>  // CString

using namespace Gdiplus;

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

Gdiplus::GraphicsPath* CreateRoundRectPath(Gdiplus::Rect rect, int radius)
{
	auto path = new Gdiplus::GraphicsPath();

	int diameter = radius * 2;
	if (diameter <= 0) {
		path->AddRectangle(rect);
		return path;
	}
	if (diameter > rect.Width) diameter = rect.Width;
	if (diameter > rect.Height) diameter = rect.Height;

	Gdiplus::Rect arcRectTL(rect.X, rect.Y, diameter, diameter);
	Gdiplus::Rect arcRectTR(rect.GetRight() - diameter, rect.Y, diameter, diameter);
	Gdiplus::Rect arcRectBR(rect.GetRight() - diameter, rect.GetBottom() - diameter, diameter, diameter);
	Gdiplus::Rect arcRectBL(rect.X, rect.GetBottom() - diameter, diameter, diameter);

	path->AddArc(arcRectTL, 180, 90);
	path->AddArc(arcRectTR, 270, 90);
	path->AddArc(arcRectBR, 0, 90);
	path->AddArc(arcRectBL, 90, 90);

	path->CloseFigure();
	return path;
}

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
	const std::vector<FileItem>& files, const std::vector<ImageItem>& images,
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
	/*CRect timeRect = rect;
	timeRect.top = bubbleRect.bottom + 2;
	timeRect.right = bubbleRect.right;
	timeRect.left = bubbleRect.right - 100;
	DrawTimeStamp(pDC, msg, timeRect, true);*/

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
	if (msg.GetContent().IsEmpty() && !HasImages(msg) && HasFiles(msg))
	{
		contentRect.right = contentRect.left + 200;
	}
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

	CSize textSize = CalculateMessageSize(pDC, msg);
	//CSize textSize

	// Draw text content
	if (!msg.GetContent().IsEmpty())
	{
		//CSize textSize = CalculateTextSize(pDC, msg.GetContent(), contentRect.Width());
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

void MessageItemStyle::DrawFiles(CDC* pDC, const std::vector<FileItem>& files, CRect& rect)
{
	pDC->SetTextColor(RGB(0, 100, 200));
	CPoint origin = rect.TopLeft();
	CRect fileRect = rect;
	//fileRect.right = fileRect.left + min(MAX_FILE_ITEM_WIDTH, rect.Width() - FILE_ITEM_PADDING * 2);
	fileRect.right = fileRect.left + rect.Width();
	fileRect.bottom = fileRect.top + FILE_ITEM_HEIGHT;

	for (const auto& file : files)
	{
		pDC->FillSolidRect(&fileRect, RGB(240, 240, 250));
		pDC->DrawText(_T("    📎 ") + file.fileName, &fileRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pDC->Draw3dRect(&fileRect, RGB(200, 200, 200), RGB(200, 200, 200));

		fileRect.OffsetRect(0, FILE_ITEM_HEIGHT);
	}
	origin.y += FILE_ITEM_HEIGHT + 20;
}

void MessageItemStyle::DrawImages(CDC* pDC, const std::vector<ImageItem>& images, CRect& rect)
{
	Graphics graphics(pDC->GetSafeHdc());
	CRect imageRect = rect;
	imageRect.bottom = imageRect.top + IMAGE_PREVIEW_HEIGHT;

	for (const auto& image : images)
	{
		CString localPath;
		if (DownloadImageIfNotExists(image, localPath))
		{
			Gdiplus::Bitmap bitmap((LPCWSTR)localPath);
			if (bitmap.GetLastStatus() == Gdiplus::Ok)
			{
				Gdiplus::Rect gdipRect(imageRect.left, imageRect.top,
					imageRect.Width(), imageRect.Height());

				if (auto path = CreateRoundRectPath(gdipRect, 5))
				{
					graphics.SetClip(path);
					graphics.DrawImage(&bitmap, gdipRect);
					graphics.ResetClip();
					delete path;
				}
				else
				{
					graphics.DrawImage(&bitmap, gdipRect); // fallback
				}
			}
			else
			{
				// Trường hợp bitmap không hợp lệ
				pDC->Rectangle(&imageRect);
				pDC->SetTextColor(RGB(100, 100, 100));
				pDC->DrawText(_T("🖼️ Failed: ") + image.fileName, &imageRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
		}
		else
		{
			// Không tải được ảnh
			pDC->Rectangle(&imageRect);
			pDC->SetTextColor(RGB(100, 100, 100));
			pDC->DrawText(_T("❌ ") + image.fileName, &imageRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

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
	int width;
	if (!msg.GetContent().IsEmpty())
		width = contentSize.cx + (BUBBLE_PADDING * 2) + MESSAGE_PADDING;
	else if (HasFiles(msg) && !HasImages(msg))
		width = 250 + (BUBBLE_PADDING * 2) + MESSAGE_PADDING;
	else 
		width = (BUBBLE_PADDING * 2) + MESSAGE_PADDING;
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

int MessageItemStyle::CalculateFilesHeight(const std::vector<FileItem>& files)
{
	return (int)files.size() * FILE_ITEM_HEIGHT;
}

int MessageItemStyle::CalculateImagesHeight(const std::vector<ImageItem>& images)
{
	return (int)images.size() * (IMAGE_PREVIEW_HEIGHT + 5);
}

CString MessageItemStyle::GetFileIcon(const CString& extension)
{
	CString ext = extension;
	ext.MakeLower();
	if (ext == _T("pdf")) return _T("📄");
	if (ext == _T("doc") || ext == _T("docx")) return _T("📝");
	if (ext == _T("xls") || ext == _T("xlsx")) return _T("📊");
	if (ext == _T("ppt") || ext == _T("pptx")) return _T("📈");
	if (ext == _T("zip") || ext == _T("rar") || ext == _T("7z")) return _T("🗜️");
	if (ext == _T("txt")) return _T("📋");
	if (ext == _T("jpg") || ext == _T("jpeg") || ext == _T("png") || ext == _T("gif")) return _T("🖼️");
	if (ext == _T("mp4") || ext == _T("avi") || ext == _T("mov")) return _T("🎬");
	if (ext == _T("mp3") || ext == _T("wav") || ext == _T("flac")) return _T("🎵");

	return _T("📎");
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

bool MessageItemStyle::DownloadImageIfNotExists(const ImageItem image, CString& localPathOut)
{
	// Lấy đường dẫn thư mục tạm
	TCHAR tempPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, tempPath);

	CString filePath;
	filePath.Format(_T("%s\\MyAppCache\\%s"), tempPath, image.fileName);

	// Kiểm tra nếu file đã tồn tại
	if (PathFileExists(filePath))
	{
		localPathOut = filePath;
		return true;
	}

	// Tạo thư mục nếu chưa có
	CreateDirectory(CString(tempPath) + _T("\\MyAppCache"), nullptr);

	CString fullUrl = _T("http://30.30.30.85:8888/api") + image.url;
	// Tải file từ URL
	HRESULT hr = URLDownloadToFile(nullptr, fullUrl, filePath, 0, nullptr);
	if (SUCCEEDED(hr))
	{
		localPathOut = filePath;
		return true;
	}

	return false;
}
