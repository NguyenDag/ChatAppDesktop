#pragma once
#include "afxdialogex.h"
#include <functional>


// EmojiDialog dialog

class EmojiDialog : public CDialogEx
{
	DECLARE_DYNAMIC(EmojiDialog)

public:
	EmojiDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~EmojiDialog();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EMOJI_DIALOG };
#endif

	CString GetSelectedEmoji() const { return m_selectedEmoji; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CArray<CString> m_emojiArray;
	CString m_selectedEmoji;

	CStatic m_emojiGrid;

	void DrawEmojiGrid();
	void CreateEmojiButtons();

public:
	CRect m_emojiPosition;
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEmojiButtonClicked(UINT nID);
	std::function<void(const CString&)> OnEmojiSelected;
};
