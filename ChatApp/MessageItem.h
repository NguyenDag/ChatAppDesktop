#pragma once
#include "afxdialogex.h"
#include "MessageItemStyle.h"


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

	DECLARE_MESSAGE_MAP()
private:
	//MessageItemStyle m_listChat;
	MessageItemStyle m_messageList;
	CString m_friendId;
	CString m_friendName;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	bool GetMessages(const string& token, vector<Message>& message, string& errorMessage, const string& friendId);
	//afx_msg void OnBnClickedSend(); // Button gửi
private:
	CEdit m_editSearch;
};
