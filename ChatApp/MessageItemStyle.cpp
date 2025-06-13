#include "pch.h"
#include "MessageItemStyle.h"
#include <gdiplus.h>
#include <curl/curl.h>
#include <iostream>
#include <winhttp.h>

#include <gdiplus.h>
#pragma comment(lib, "winhttp.lib")
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
	ON_WM_LBUTTONDOWN()
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

bool MessageItemStyle::HandleFileClick(CPoint point, int& fileIndex, int& imageIndex)
{
	for (size_t i = 0; i < m_downloadRects.size(); ++i)
	{
		if (m_downloadRects[i].PtInRect(point))
		{
			fileIndex = (int)i;
			return true;
		}
	}

	for (size_t i = 0; i < m_downloadImagesRects.size(); ++i)
	{
		if (m_downloadImagesRects[i].PtInRect(point))
		{
			imageIndex = (int)i;
			return true;
		}
	}
	return false;
}

void MessageItemStyle::DownloadFile(const FileItem& file)
{
	CFileDialog dlg(FALSE, NULL, file.fileName,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		CString savePath = dlg.GetPathName();


		if (DownloadFileFromServer(file, savePath))
		{
			MessageBox(_T("File đã được tải thành công!"), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(_T("Lỗi khi tải file!"), _T("Lỗi"), MB_OK | MB_ICONERROR);
		}
	}
}

void MessageItemStyle::DownloadImage(const ImageItem& image)
{
	CFileDialog dlg(FALSE, NULL, image.fileName,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		CString savePath = dlg.GetPathName();


		if (DownloadImageFromServer(image, savePath))
		{
			MessageBox(_T("Image đã được tải thành công!"), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(_T("Lỗi khi tải image!"), _T("Lỗi"), MB_OK | MB_ICONERROR);
		}
	}
}

bool MessageItemStyle::DownloadFileFromServer(const FileItem& fileItem, const CString& savePath)
{
	try
	{
		// Tạo URL download
		CString downloadUrl;
		downloadUrl.Format(_T("http://30.30.30.85:8888/api%s"), fileItem.url);

		// Sử dụng WinHTTP để tải file
		HINTERNET hSession = NULL;
		HINTERNET hConnect = NULL;
		HINTERNET hRequest = NULL;
		bool bResult = false;

		do
		{
			// Khởi tạo WinHTTP session
			hSession = WinHttpOpen(L"FileDownloader/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0);
			if (!hSession) break;

			// Kết nối đến server
			hConnect = WinHttpConnect(hSession, L"30.30.30.85", 8888, 0);
			if (!hConnect) break;

			// Tạo request path
			CString requestPath;
			requestPath.Format(_T("/api%s"), fileItem.url);

			// Tạo HTTP request
			hRequest = WinHttpOpenRequest(hConnect, L"GET", requestPath,
				NULL, WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				0);
			if (!hRequest) break;

			// Gửi request
			if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
				break;

			// Nhận response
			if (!WinHttpReceiveResponse(hRequest, NULL))
				break;

			// Kiểm tra status code
			DWORD statusCode = 0;
			DWORD statusCodeSize = sizeof(statusCode);
			if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, NULL))
				break;

			if (statusCode != 200)
			{
				CString errorMsg;
				errorMsg.Format(_T("Server trả về lỗi: HTTP %d"), statusCode);
				MessageBox(errorMsg, _T("Lỗi"), MB_OK | MB_ICONERROR);
				break;
			}

			// Tạo file để ghi
			HANDLE hFile = CreateFile(savePath, GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) break;

			// Đọc và ghi dữ liệu
			DWORD dwSize = 0;
			DWORD dwDownloaded = 0;
			DWORD dwWritten = 0;
			BYTE buffer[8192];

			do
			{
				// Kiểm tra có data để đọc không
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
					break;

				if (dwSize == 0)
					break; // Hết data

				// Đọc data
				DWORD dwToRead = min(dwSize, sizeof(buffer));
				if (!WinHttpReadData(hRequest, buffer, dwToRead, &dwDownloaded))
					break;

				// Ghi vào file
				if (!WriteFile(hFile, buffer, dwDownloaded, &dwWritten, NULL))
					break;

				if (dwWritten != dwDownloaded)
					break;

			} while (dwSize > 0);

			CloseHandle(hFile);
			bResult = true;

		} while (false);

		// Cleanup
		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);

		return bResult;
	}
	catch (...)
	{
		return false;
	}
}

bool MessageItemStyle::DownloadImageFromServer(const ImageItem& imageItem, const CString& savePath)
{
	try
	{
		// Tạo URL download
		CString downloadUrl;
		downloadUrl.Format(_T("http://30.30.30.85:8888/api%s"), imageItem.url);

		// Sử dụng WinHTTP để tải file
		HINTERNET hSession = NULL;
		HINTERNET hConnect = NULL;
		HINTERNET hRequest = NULL;
		bool bResult = false;

		do
		{
			// Khởi tạo WinHTTP session
			hSession = WinHttpOpen(L"FileDownloader/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0);
			if (!hSession) break;

			// Kết nối đến server
			hConnect = WinHttpConnect(hSession, L"30.30.30.85", 8888, 0);
			if (!hConnect) break;

			// Tạo request path
			CString requestPath;
			requestPath.Format(_T("/api%s"), imageItem.url);

			// Tạo HTTP request
			hRequest = WinHttpOpenRequest(hConnect, L"GET", requestPath,
				NULL, WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				0);
			if (!hRequest) break;

			// Gửi request
			if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
				break;

			// Nhận response
			if (!WinHttpReceiveResponse(hRequest, NULL))
				break;

			// Kiểm tra status code
			DWORD statusCode = 0;
			DWORD statusCodeSize = sizeof(statusCode);
			if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, NULL))
				break;

			if (statusCode != 200)
			{
				CString errorMsg;
				errorMsg.Format(_T("Server trả về lỗi: HTTP %d"), statusCode);
				MessageBox(errorMsg, _T("Lỗi"), MB_OK | MB_ICONERROR);
				break;
			}

			// Tạo file để ghi
			HANDLE hFile = CreateFile(savePath, GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) break;

			// Đọc và ghi dữ liệu
			DWORD dwSize = 0;
			DWORD dwDownloaded = 0;
			DWORD dwWritten = 0;
			BYTE buffer[8192];

			do
			{
				// Kiểm tra có data để đọc không
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
					break;

				if (dwSize == 0)
					break; // Hết data

				// Đọc data
				DWORD dwToRead = min(dwSize, sizeof(buffer));
				if (!WinHttpReadData(hRequest, buffer, dwToRead, &dwDownloaded))
					break;

				// Ghi vào file
				if (!WriteFile(hFile, buffer, dwDownloaded, &dwWritten, NULL))
					break;

				if (dwWritten != dwDownloaded)
					break;

			} while (dwSize > 0);

			CloseHandle(hFile);
			bResult = true;

		} while (false);

		// Cleanup
		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);

		return bResult;
	}
	catch (...)
	{
		return false;
	}
}

void MessageItemStyle::OnPaint()
{
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);

	dc.FillRect(&clientRect, &m_brushBg);

	if (m_messages.empty()) return;
	m_downloadRects.clear();
	m_downloadImagesRects.clear();
	OutputDebugString(_T("DUBUG: tao delete here!\n"));

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

void MessageItemStyle::OnLButtonDown(UINT nFlags, CPoint point)
{
	CString debugPoint;
	debugPoint.Format(_T("Chuột đã click ở đâyyyy: x = %d, y = %d"), point.x, point.y);
	OutputDebugString(debugPoint);

	int fileIndex = -1;
	int imageIndex = -1;
	if (HandleFileClick(point, fileIndex, imageIndex))
	{
		CString debugClick;
		debugClick.Format(_T("Nút download đã click ở vị trí: Index File: %d; Index Image: %d\n"), fileIndex, imageIndex);
		OutputDebugString(debugClick);
		//MessageBox(_T("Trúng nút download!"), _T("Test 2"));

		if (fileIndex >= 0 && fileIndex < m_currentFiles.size())
		{
			DownloadFile(m_currentFiles[fileIndex]);
		}

		if (imageIndex >= 0 && imageIndex < m_currentImages.size())
		{
			DownloadImage(m_currentImages[imageIndex]);
		}


	}
	CWnd::OnLButtonDown(nFlags, point);
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
	CSize contentSize = CSize(0, 0);
	bool hasContent = !msg.GetContent().IsEmpty();
	if (hasContent)
		contentSize = CalculateTextSize(pDC, msg.GetContent(), rect.Width() * 2 / 3);

	int filesHeight = CalculateFilesHeight(msg.GetFiles());
	int imagesHeight = CalculateImagesHeight(msg.GetImages());

	int bubbleWidth = 0;
	int bubbleHeight = 0;
	CRect bubbleRect;

	if (hasContent)
	{
		// Nếu có content thì vẽ khung bubble
		bubbleWidth = min(contentSize.cx + (BUBBLE_PADDING * 2), rect.Width() * 2 / 3);
		bubbleHeight = contentSize.cy + (BUBBLE_PADDING * 2);

		bubbleRect.right = rect.right - MESSAGE_PADDING;
		bubbleRect.left = bubbleRect.right - bubbleWidth - MESSAGE_PADDING;
		bubbleRect.top = rect.top;
		bubbleRect.bottom = bubbleRect.top + bubbleHeight;

		DrawMessageBubble(pDC, bubbleRect, true);

		DrawMessageContent(pDC, msg, bubbleRect);
	}
	else
	{
		bubbleRect.left = rect.right - MESSAGE_PADDING - 200;
		bubbleRect.right = rect.right - MESSAGE_PADDING;
		bubbleRect.top = rect.top + MESSAGE_PADDING;
		bubbleHeight = filesHeight + imagesHeight + (BUBBLE_PADDING);
		bubbleRect.bottom = bubbleRect.top + bubbleHeight;

		DrawMessageContent(pDC, msg, bubbleRect);
	}

	// Draw timestamp
	/*CRect timeRect = rect;
	timeRect.top = bubbleRect.bottom + 2;
	timeRect.right = bubbleRect.right;
	timeRect.left = bubbleRect.right - 100;
	DrawTimeStamp(pDC, msg, timeRect, true);*/

	// Vẽ checkmark
	if (!msg.GetIsSend()) {
		CRect checkRect;
		checkRect.left = bubbleRect.right + 5;
		checkRect.right = checkRect.left + 20;
		checkRect.top = bubbleRect.bottom - 15;
		checkRect.bottom = bubbleRect.bottom;
		DrawCheckmarks(pDC, checkRect);
	}
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

	// Content size mặc định là 0
	CSize contentSize(0, 0);
	if (!msg.GetContent().IsEmpty())
		contentSize = CalculateTextSize(pDC, msg.GetContent(), rect.Width() * 2 / 3);

	int filesHeight = CalculateFilesHeight(msg.GetFiles());
	int imagesHeight = CalculateImagesHeight(msg.GetImages());

	int totalHeight = contentSize.cy + filesHeight + imagesHeight + (BUBBLE_PADDING * 2);

	// Bắt đầu vẽ từ đây
	CRect contentRect;
	contentRect.left = leftMargin;
	contentRect.top = rect.top + MESSAGE_PADDING;

	if (!msg.GetContent().IsEmpty())
	{
		// Chỉ khi có content thì mới vẽ bubble
		int bubbleWidth = min(contentSize.cx + (BUBBLE_PADDING * 2), rect.Width() * 2 / 3);
		contentRect.right = contentRect.left + bubbleWidth;
		contentRect.bottom = contentRect.top + totalHeight;

		DrawMessageBubble(pDC, contentRect, false);
		DrawMessageContent(pDC, msg, contentRect);
	}
	else
	{
		// Không có content, vẽ ảnh hoặc file không có bubble
		int width = 200; // cố định chiều rộng
		contentRect.right = contentRect.left + width;
		contentRect.bottom = contentRect.top + totalHeight;

		DrawMessageContent(pDC, msg, contentRect);
	}

	// Vẽ thời gian
	/*CRect timeRect;
	timeRect.top = contentRect.bottom + 2;
	timeRect.left = contentRect.left;
	timeRect.right = contentRect.left + 100;
	DrawTimeStamp(pDC, msg, timeRect, false);*/
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
	if (!msg.GetContent().IsEmpty())
		contentRect.DeflateRect(BUBBLE_PADDING, BUBBLE_PADDING);
	/*if (msg.GetContent().IsEmpty() && !HasImages(msg) && HasFiles(msg))
	{
		contentRect.right = contentRect.left + 200;
	}*/
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

	// Draw text content
	if (!msg.GetContent().IsEmpty())
	{
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
	fileRect.right = fileRect.left + rect.Width();
	fileRect.bottom = fileRect.top + FILE_ITEM_HEIGHT;
	
	const int DOWNLOAD_ICON_WIDTH = 30;
	const int DOWNLOAD_ICON_MARGIN = 5;

	for (size_t i = 0; i < files.size(); ++i)
	{
		const auto& file = files[i];

		// Vẽ background
		pDC->FillSolidRect(&fileRect, RGB(240, 240, 250));

		// Tạo rect cho text (trừ phần icon download)
		CRect textRect = fileRect;
		textRect.left += 10;
		textRect.right -= (DOWNLOAD_ICON_WIDTH + DOWNLOAD_ICON_MARGIN * 2);

		// Vẽ text file
		pDC->DrawText(_T("📄 ") + file.fileName, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

		// Tạo rect cho icon download
		CRect downloadRect;
		downloadRect.left = textRect.right + DOWNLOAD_ICON_MARGIN;
		downloadRect.right = downloadRect.left + DOWNLOAD_ICON_WIDTH;
		downloadRect.top = fileRect.top + 2;
		downloadRect.bottom = fileRect.bottom - 2;

		// Lưu vị trí download button
		m_downloadRects.push_back(downloadRect);
		m_currentFiles.push_back(files[i]);

		// Vẽ icon download (sử dụng ký tự Unicode hoặc vẽ custom)
		pDC->SetTextColor(RGB(0, 150, 0));
		pDC->DrawText(_T("⇓"), &downloadRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		// Vẽ border cho download button
		pDC->Draw3dRect(&downloadRect, RGB(150, 150, 150), RGB(150, 150, 150));

		// Reset màu text
		pDC->SetTextColor(RGB(0, 100, 200));

		// Vẽ border cho toàn bộ file item
		pDC->Draw3dRect(&fileRect, RGB(200, 200, 200), RGB(200, 200, 200));

		fileRect.OffsetRect(0, FILE_ITEM_HEIGHT + 10);

	}
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

		int buttonSize = 24;
		CRect buttonRect(
			imageRect.right - buttonSize - 5,
			imageRect.top + 5,
			imageRect.right - 5,
			imageRect.top + 5 + buttonSize
		);

		// Lưu lại vùng button để xử lý click
		m_downloadImagesRects.push_back(buttonRect);
		m_currentImages.push_back(image);

		// Vẽ nút
		pDC->FillSolidRect(&buttonRect, RGB(230, 230, 250));
		pDC->Draw3dRect(&buttonRect, RGB(100, 100, 100), RGB(100, 100, 100));
		pDC->DrawText(_T("⬇"), &buttonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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
		22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
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
	TCHAR tempPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, tempPath);

	CString filePath;
	filePath.Format(_T("%s\\MyAppCache\\%s"), tempPath, image.fileName);

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
