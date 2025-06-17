#include "pch.h"
#include "CLoginManager.h"

const CString CLoginManager::CONFIG_FILENAME = _T("appconfig.dat");
const BYTE CLoginManager::ENCRYPTION_KEY[] = { 0x4B, 0x65, 0x79, 0x53, 0x65, 0x63, 0x72, 0x65, 0x74, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
const int CLoginManager::KEY_LENGTH = sizeof(ENCRYPTION_KEY);

CLoginManager::CLoginManager()
{
}

CLoginManager::~CLoginManager()
{
}

BOOL CLoginManager::SaveLoginInfo(const CString& username, const CString& password, BOOL rememberMe)
{
	if (!rememberMe)
	{
		return ClearLoginInfo();
	}

	CString configPath = GetConfigFilePath();

	try
	{
		// Mã hóa thông tin
		CString encryptedUsername = EncryptString(username);
		CString encryptedPassword = EncryptString(password);

		// Tạo nội dung file
		CString fileContent;
		fileContent.Format(_T("[LOGIN]\r\nUSER=%s\r\nPASS=%s\r\nSAVED=1\r\n"),
			encryptedUsername, encryptedPassword);

		// Ghi vào file
		HANDLE hFile = CreateFile(configPath, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		// Chuyển Unicode sang UTF-8 để ghi file
		int utf8Length = WideCharToMultiByte(CP_UTF8, 0, fileContent, -1, NULL, 0, NULL, NULL);
		char* utf8Buffer = new char[utf8Length];
		WideCharToMultiByte(CP_UTF8, 0, fileContent, -1, utf8Buffer, utf8Length, NULL, NULL);

		DWORD bytesWritten;
		BOOL result = WriteFile(hFile, utf8Buffer, utf8Length - 1, &bytesWritten, NULL);

		CloseHandle(hFile);
		delete[] utf8Buffer;

		return result;
	}
	catch (...)
	{
		return FALSE;
	}
}

BOOL CLoginManager::LoadLoginInfo(CString& username, CString& password)
{
	CString configPath = GetConfigFilePath();

	// Kiểm tra file có tồn tại không
	if (GetFileAttributes(configPath) == INVALID_FILE_ATTRIBUTES)
		return FALSE;

	try
	{
		HANDLE hFile = CreateFile(configPath, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		DWORD fileSize = GetFileSize(hFile, NULL);
		if (fileSize == 0 || fileSize == INVALID_FILE_SIZE)
		{
			CloseHandle(hFile);
			return FALSE;
		}

		// Đọc file
		char* buffer = new char[fileSize + 1];
		DWORD bytesRead;
		BOOL readResult = ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);
		buffer[fileSize] = 0;

		CloseHandle(hFile);

		if (!readResult)
		{
			delete[] buffer;
			return FALSE;
		}

		// Chuyển UTF-8 sang Unicode
		int unicodeLength = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
		TCHAR* unicodeBuffer = new TCHAR[unicodeLength];
		MultiByteToWideChar(CP_UTF8, 0, buffer, -1, unicodeBuffer, unicodeLength);

		CString fileContent = unicodeBuffer;
		delete[] buffer;
		delete[] unicodeBuffer;

		// Parse nội dung file
		CString encryptedUsername, encryptedPassword;
		int userPos = fileContent.Find(_T("USER="));
		int passPos = fileContent.Find(_T("PASS="));
		int savedPos = fileContent.Find(_T("SAVED=1"));

		if (userPos == -1 || passPos == -1 || savedPos == -1)
			return FALSE;

		// Lấy username
		int userStart = userPos + 5;
		int userEnd = fileContent.Find(_T("\r\n"), userStart);
		if (userEnd == -1) userEnd = fileContent.Find(_T("\n"), userStart);
		if (userEnd == -1) userEnd = fileContent.GetLength();
		encryptedUsername = fileContent.Mid(userStart, userEnd - userStart);

		// Lấy password
		int passStart = passPos + 5;
		int passEnd = fileContent.Find(_T("\r\n"), passStart);
		if (passEnd == -1) passEnd = fileContent.Find(_T("\n"), passStart);
		if (passEnd == -1) passEnd = fileContent.GetLength();
		encryptedPassword = fileContent.Mid(passStart, passEnd - passStart);

		// Giải mã
		username = DecryptString(encryptedUsername);
		password = DecryptString(encryptedPassword);

		return !username.IsEmpty();
	}
	catch (...)
	{
		return FALSE;
	}
}

BOOL CLoginManager::ClearLoginInfo()
{
	CString configPath = GetConfigFilePath();

	// Xóa file nếu tồn tại
	if (GetFileAttributes(configPath) != INVALID_FILE_ATTRIBUTES)
	{
		return DeleteFile(configPath);
	}

	return TRUE;
}

BOOL CLoginManager::HasSavedLogin()
{
	CString username, password;
	return LoadLoginInfo(username, password);
}

CString CLoginManager::GetConfigFilePath()
{
	TCHAR szPath[MAX_PATH];

	// Lấy đường dẫn thư mục AppData của user hiện tại
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		CString appDataPath = szPath;
		CString appFolder = appDataPath + _T("\\ChatApp");

		// Tạo thư mục nếu chưa tồn tại
		CreateDirectory(appFolder, NULL);

		CString fullPath = appFolder + _T("\\") + CONFIG_FILENAME;

		/*TRACE(_T("Config file path: %s\n"), fullPath);
		AfxMessageBox(_T("File config tại: ") + fullPath);*/

		return appFolder + _T("\\") + CONFIG_FILENAME;
	}

	// Fallback: lưu trong thư mục của ứng dụng
	TCHAR szModulePath[MAX_PATH];
	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	CString modulePath = szModulePath;
	int pos = modulePath.ReverseFind(_T('\\'));
	if (pos != -1)
	{
		modulePath = modulePath.Left(pos);
	}

	return modulePath + _T("\\") + CONFIG_FILENAME;
}

CString CLoginManager::EncryptString(const CString& plainText)
{
	if (plainText.IsEmpty())
		return _T("");

	// Chuyển CString sang byte array
	int length = plainText.GetLength() * sizeof(TCHAR);
	BYTE* data = new BYTE[length];
	memcpy(data, (LPCTSTR)plainText, length);

	// Mã hóa XOR với key
	XOREncrypt(data, length, ENCRYPTION_KEY, KEY_LENGTH);

	// Chuyển sang hex string để lưu trữ
	CString hexResult;
	for (int i = 0; i < length; i++)
	{
		CString hex;
		hex.Format(_T("%02X"), data[i]);
		hexResult += hex;
	}

	delete[] data;
	return hexResult;
}

CString CLoginManager::DecryptString(const CString& encryptedText)
{
	if (encryptedText.IsEmpty() || encryptedText.GetLength() % 2 != 0)
		return _T("");

	// Chuyển hex string về byte array
	int length = encryptedText.GetLength() / 2;
	BYTE* data = new BYTE[length];

	for (int i = 0; i < length; i++)
	{
		CString hex = encryptedText.Mid(i * 2, 2);
		data[i] = (BYTE)_tcstol(hex, NULL, 16);
	}

	// Giải mã XOR
	XOREncrypt(data, length, ENCRYPTION_KEY, KEY_LENGTH);

	// Chuyển về CString
	CString result;
	int charCount = length / sizeof(TCHAR);
	TCHAR* chars = (TCHAR*)data;

	for (int i = 0; i < charCount; i++)
	{
		if (chars[i] == 0) break; // Dừng ở null terminator
		result += chars[i];
	}

	delete[] data;
	return result;
}

CString CLoginManager::StringToHex(const CString& str)
{
	return CString();
}

CString CLoginManager::HexToString(const CString& hex)
{
	return CString();
}

void CLoginManager::XOREncrypt(BYTE* data, int length, const BYTE* key, int keyLength)
{
	for (int i = 0; i < length; i++)
	{
		data[i] ^= key[i % keyLength];
	}
}

CString CLoginManager::GenerateKey()
{
	return CString();
}
