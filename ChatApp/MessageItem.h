#pragma once
#include "afxdialogex.h"
#include "MessageItemStyle.h"
#include "CImageButton.h"

// MessageItem dialog

class MessageItem : public CDialogEx
{
	DECLARE_DYNAMIC(MessageItem)

public:
	MessageItem(const CString& friendId, const CString& fullname, CWnd* pParent = nullptr);   // standard constructor
	//void CreateSampleMessages(vector<Message>* messages);
	virtual ~MessageItem();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void LoadButtonImage(CImageButton& button, LPCTSTR imagePath);
	bool SendMessageToFriend(const string& token, const string& friendID, const string& content, const vector<FileItem>& files, string& errorMessage);
	DECLARE_MESSAGE_MAP()
private:
	MessageItemStyle m_messageList;
	CString m_friendId;
	CString m_friendName;

	CEdit m_editSearch;
	CBrush m_hbrBackground;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	bool GetMessages(const string& token, vector<Message>& message, string& errorMessage, const string& friendId);
	void LoadMessages();
	void setIconButton(CMFCButton& _idc_button, HICON hicon);
private:
	CMFCButton m_btnSend;
	CMFCButton m_btnImage;
	CMFCButton m_btnFile;
	CMFCButton m_btnEmoji;
public:
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedBtnImage();
	afx_msg void OnBnClickedBtnFile();
	afx_msg void OnBnClickedBtnEmoji();
};
