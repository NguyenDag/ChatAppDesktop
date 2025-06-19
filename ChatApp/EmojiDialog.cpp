// EmojiDialog.cpp : implementation file
//

#include "pch.h"
#include "ChatApp.h"
#include "afxdialogex.h"
#include "EmojiDialog.h"


// EmojiDialog dialog

IMPLEMENT_DYNAMIC(EmojiDialog, CDialogEx)

EmojiDialog::EmojiDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EMOJI_DIALOG, pParent)
{
	m_emojiArray.Add(_T("😀")); m_emojiArray.Add(_T("😁"));
	m_emojiArray.Add(_T("😂")); m_emojiArray.Add(_T("🤣"));
	m_emojiArray.Add(_T("😃")); m_emojiArray.Add(_T("😄"));
	m_emojiArray.Add(_T("😅")); m_emojiArray.Add(_T("😆"));
	m_emojiArray.Add(_T("😉")); m_emojiArray.Add(_T("😊"));
	m_emojiArray.Add(_T("😋")); m_emojiArray.Add(_T("😎"));
	m_emojiArray.Add(_T("😍")); m_emojiArray.Add(_T("😘"));
	m_emojiArray.Add(_T("🥰")); m_emojiArray.Add(_T("😗"));
	m_emojiArray.Add(_T("😙")); m_emojiArray.Add(_T("😚"));
	m_emojiArray.Add(_T("🙂")); m_emojiArray.Add(_T("🤗"));
	m_emojiArray.Add(_T("🤩")); m_emojiArray.Add(_T("🤔"));
	m_emojiArray.Add(_T("🤨")); m_emojiArray.Add(_T("😐"));
	m_emojiArray.Add(_T("😑")); m_emojiArray.Add(_T("😶"));
	m_emojiArray.Add(_T("🙄")); m_emojiArray.Add(_T("😏"));
	m_emojiArray.Add(_T("😣")); m_emojiArray.Add(_T("😥"));
	m_emojiArray.Add(_T("😮")); m_emojiArray.Add(_T("🤐"));
	m_emojiArray.Add(_T("😯")); m_emojiArray.Add(_T("😪"));
	m_emojiArray.Add(_T("😫")); m_emojiArray.Add(_T("🥱"));
	m_emojiArray.Add(_T("😴")); m_emojiArray.Add(_T("😌"));
	m_emojiArray.Add(_T("😛")); m_emojiArray.Add(_T("😜"));
	m_emojiArray.Add(_T("😝")); m_emojiArray.Add(_T("🤤"));
	m_emojiArray.Add(_T("😒")); m_emojiArray.Add(_T("😓"));
	m_emojiArray.Add(_T("😔")); m_emojiArray.Add(_T("😕"));
	m_emojiArray.Add(_T("🙃")); m_emojiArray.Add(_T("🤑"));
	m_emojiArray.Add(_T("😲")); m_emojiArray.Add(_T("😵"));
	m_emojiArray.Add(_T("😷")); m_emojiArray.Add(_T("🤒"));
	m_emojiArray.Add(_T("🤕")); m_emojiArray.Add(_T("🤢"));
	m_emojiArray.Add(_T("🤮")); m_emojiArray.Add(_T("🤧"));
	m_emojiArray.Add(_T("🥵")); m_emojiArray.Add(_T("🥶"));
	m_emojiArray.Add(_T("😈")); m_emojiArray.Add(_T("👿"));
	m_emojiArray.Add(_T("😠")); m_emojiArray.Add(_T("😡"));
	m_emojiArray.Add(_T("🤬")); m_emojiArray.Add(_T("😤"));
	m_emojiArray.Add(_T("😭")); m_emojiArray.Add(_T("😢"));
	m_emojiArray.Add(_T("🥺")); m_emojiArray.Add(_T("😨"));
	m_emojiArray.Add(_T("😰")); m_emojiArray.Add(_T("😱"));
}

EmojiDialog::~EmojiDialog()
{
}

void EmojiDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(EmojiDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND_RANGE(ID_EMOJI_START, ID_EMOJI_END, &EmojiDialog::OnEmojiButtonClicked)
END_MESSAGE_MAP()

BOOL EmojiDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect dlgRect;
	GetWindowRect(&dlgRect);

	int x = m_emojiPosition.left;
	int y = m_emojiPosition.top - dlgRect.Height() - 5;

	CRect screenRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
	if (x + dlgRect.Width() > screenRect.right)
		x = screenRect.right - dlgRect.Width();
	if (y < screenRect.top)
		y = m_emojiPosition.bottom + 5;

	SetWindowPos(NULL, 0, 0, 370, 300, SWP_NOMOVE | SWP_NOZORDER);

	CreateEmojiButtons();

	return TRUE;
}

void EmojiDialog::DrawEmojiGrid()
{
}

void EmojiDialog::CreateEmojiButtons()
{
	const int BUTTON_SIZE = 30;
	const int BUTTONS_PER_ROW = 10;
	const int MARGIN = 5;

	for (int i = 0; i < m_emojiArray.GetSize(); i++)
	{
		int row = i / BUTTONS_PER_ROW;
		int col = i % BUTTONS_PER_ROW;

		int x = MARGIN + col * (BUTTON_SIZE + MARGIN);
		int y = MARGIN + row * (BUTTON_SIZE + MARGIN);

		CButton* pButton = new CButton();
		pButton->Create(m_emojiArray[i], WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			CRect(x, y, x + BUTTON_SIZE, y + BUTTON_SIZE),
			this, ID_EMOJI_START + i);

		// Thiết lập font cho emoji
		CFont* pFont = new CFont();
		pFont->CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, _T("Segoe UI Emoji"));
		pButton->SetFont(pFont);
	}
}

void EmojiDialog::OnPaint()
{
	CPaintDC dc(this);
}

void EmojiDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonDown(nFlags, point);
}

void EmojiDialog::OnEmojiButtonClicked(UINT nID)
{
	int index = nID - ID_EMOJI_START;
	if (index >= 0 && index < m_emojiArray.GetSize())
	{
		/*m_selectedEmoji = m_emojiArray[index];
		EndDialog(IDOK);*/
		CString emoji = m_emojiArray[index];
		if (OnEmojiSelected)
		{
			OnEmojiSelected(emoji); // Gọi callback
		}
	}
}
