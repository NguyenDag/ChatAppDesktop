#pragma once
class CLoginManager
{
public:
    CLoginManager();
    ~CLoginManager();

    // Lưu thông tin đăng nhập
    BOOL SaveLoginInfo(const CString& username, const CString& password, BOOL rememberMe = TRUE);

    // Tải thông tin đăng nhập đã lưu
    BOOL LoadLoginInfo(CString& username, CString& password);

    // Xóa thông tin đăng nhập đã lưu
    BOOL ClearLoginInfo();

    // Kiểm tra có thông tin đăng nhập đã lưu không
    BOOL HasSavedLogin();

private:
    CString GetConfigFilePath();
    CString EncryptString(const CString& plainText);
    CString DecryptString(const CString& encryptedText);
    CString StringToHex(const CString& str);
    CString HexToString(const CString& hex);
    void XOREncrypt(BYTE* data, int length, const BYTE* key, int keyLength);
    CString GenerateKey();

    static const CString CONFIG_FILENAME;
    static const BYTE ENCRYPTION_KEY[];
    static const int KEY_LENGTH;
};

