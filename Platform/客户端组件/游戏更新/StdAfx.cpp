// stdafx.cpp : source file that includes just the standard includes
//	GameUpdate.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "GameUpdate.h"
#include "RMessageBox.h"

// 记录日志级别
int g_iLogLevel;

// 显示日志的文本框
//extern CRichEditCtrl *g_pLogViewer;

// 日志格式[时间][模块ID][日志级别][日志内容]
#define LOG_FORMAT "[%s][%d][%d][%s]\r\n"
// 日志缓冲区大小
#define LOG_BUFFER_SIZE 1024
// 写日志
// iModuleID：模块ID
// iLevel：日志级别
// szFormat：日志格式
// ...：日志内容
CCriticalSpection criticalLog; 
void LOG(int iModuleID, int iLevel, const char* sFormat, ...)
{
	if (FALSE == g_IsWriteLog)
		return;

	OutputDebugString("GameUpdateDlg.cpp,进入关键代码段, LOG,Guard(&criticalLog);");//Test
	CThreadGuard Guard(&criticalLog);
//#ifdef _DEBUG
	// 只记录级别在指定日志级别以上的日志
	if (iLevel < g_iLogLevel)
	{
		return;
	}

	char *pBuffer1 = new char[LOG_BUFFER_SIZE];
	char *pBuffer2 = new char[LOG_BUFFER_SIZE];
	memset(pBuffer1, 0, LOG_BUFFER_SIZE);
	memset(pBuffer2, 0, LOG_BUFFER_SIZE);

	va_list args;
	va_start(args, sFormat);
	_vsnprintf(pBuffer1, LOG_BUFFER_SIZE, sFormat, args);
	va_end(args);

	CString sNow = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
	if (0 < strlen(pBuffer1))
	{
		_snprintf(pBuffer2, LOG_BUFFER_SIZE, LOG_FORMAT, sNow.GetBuffer(0), iModuleID, iLevel, pBuffer1);
	}
	else
	{
		memset(pBuffer2, 0, LOG_BUFFER_SIZE);
		_snprintf(pBuffer2, LOG_BUFFER_SIZE, TEXT("\r\n"));
	}

	// 输出到文本框
//	g_pLogViewer->SetSel(-1, 0);
//	g_pLogViewer->ReplaceSel(pBuffer2 + CString("\n"));

	// 写入文件
	CFile file;
	CreateDirectory(GetAppDirectory() + "LOG", NULL);
	file.Open(GetAppDirectory() + "LOG\\GameUpdate.log"
		, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
	file.SeekToEnd();
	file.Write(pBuffer2, (UINT)strlen(pBuffer2));
	file.Close();

	// 释放临时申请的内存
	delete []pBuffer1;
	delete []pBuffer2;
	pBuffer1 = NULL;
	pBuffer2 = NULL;

	OutputDebugString("GameUpdateDlg.cpp,离开关键代码段, LOG,Guard.leave();");//Test
//#endif
}

// 取得程序运行的目录
// bEndWithBackSlash：是否以反斜线结尾
CString GetAppDirectory(BOOL bEndWithBackSlash)
{
	char acBuffer[MAX_PATH];

	memset((void *)acBuffer, 0, MAX_PATH);
	GetModuleFileName(AfxGetInstanceHandle(), acBuffer, sizeof(acBuffer));

	char *p = strrchr(acBuffer, '\\');
	if (p != NULL)
	{
		if (bEndWithBackSlash)
		{
			*(p + 1) = '\0';
		}
		else
		{
			*p = '\0';
		}
	}
	else
	{
		p = strrchr(acBuffer, ':');
		if (p != NULL)
		{
			if (bEndWithBackSlash)
			{
				*(p + 1) = '\\';
				*(p + 2) = '\0';
			}
			else
			{
				*p = '\\';
				*(p + 1) = '\0';
			}
		}
	}

	return CString(acBuffer);
}

// 从文件全路径取得文件名
CString GetFilename(const char* sFilePath)
{
	if (sFilePath == NULL)
	{
		return CString("");
	}

	const char *pBackSlash = strrchr(sFilePath, '\\');
	if (pBackSlash == NULL)
	{
		return CString(sFilePath);
	}
	else
	{
		return CString(++pBackSlash);
	}
}

CString GetFilename(const CString &sFilePath)
{
	int iPos = sFilePath.ReverseFind('\\');
	if (iPos == -1)
	{
		return sFilePath;
	}
	else
	{
		return sFilePath.Mid(iPos + 1);
	}
}

// 取得文件扩展名（不含'.'字符）
CString GetFileExtendName(const char* sFilename)
{
	if (sFilename == NULL)
	{
		return CString("");
	}

	const char *pExtendName = strrchr(sFilename, '.');
	if (pExtendName == NULL)
	{
		return CString("");
	}
	else
	{
		return CString(++pExtendName);
	}
}

// 取得文件扩展名（不含'.'字符）
CString GetFileExtendName(const CString& sFilename)
{
	int iPos = sFilename.ReverseFind('.');
	if (iPos == -1)
	{
		return CString("");
	}
	else
	{
		return sFilename.Mid(iPos + 1);
	}
}

// 取得文件大小
size_t GetFileSize(const char* sFilename)
{
	CFileFind FileFinder;

	if (sFilename == NULL)
	{
		return 0;
	}

	if (FileFinder.FindFile(sFilename))
	{
		FileFinder.FindNextFile();
		return (size_t)FileFinder.GetLength();
	}
	else
	{
		return 0;
	}
}

// 取得文件大小
size_t GetFileSize(const CString& sFilename)
{
	CFileFind FileFinder;

	if (FileFinder.FindFile(sFilename))
	{
		FileFinder.FindNextFile();
		return (size_t)FileFinder.GetLength();
	}
	else
	{
		return 0;
	}
}

// 显示消息框
int RMessageBox(CString& sText, UINT nType, UINT nIDHelp)
{
	return RMessageBox(sText.GetBuffer(0), nType, nIDHelp);
}

// 显示消息框
int RMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	ASSERT(lpszText);

	CRMessageBox mb;

	mb.m_sTitle = AfxGetApp()->m_pszAppName;
	mb.m_iType = nType;
	mb.m_sPromptMessage = lpszText;
	mb.DoModal();
	return mb.m_iID;
}

// 替换路径中的特殊字符串为实际路径
CString ResolvePath(const char* sPath)
{
	if (sPath == NULL)
	{
		ASSERT(FALSE);
		return CString("");
	}

	char acBuffer[MAX_PATH];
	CString sDestFilename = sPath;

	sDestFilename.Replace("<INSTDIR>", GetAppDirectory().GetBuffer(0));

	memset(acBuffer, 0, MAX_PATH);
	GetWindowsDirectory(acBuffer, MAX_PATH);
	sDestFilename.Replace("<WINDIR>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	GetSystemDirectory(acBuffer, MAX_PATH);
	sDestFilename.Replace("<SYSDIR>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, 0x0014, FALSE); // CSIDL_FONTS (0x0014)
	sDestFilename.Replace("<FONTS>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, 0x0026, FALSE); // CSIDL_PROGRAM_FILES (0x0026)
	sDestFilename.Replace("<PROGRAMFILES>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, 0x002b, FALSE); // CSIDL_PROGRAM_FILES_COMMON (0x002b)
	sDestFilename.Replace("<COMMONFILES>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, 0x0028, FALSE); // CSIDL_PROFILE (0x0028)
	sDestFilename.Replace("<PROFILE>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_PERSONAL, FALSE);
	sDestFilename.Replace("<DOCUMENTS>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_DESKTOPDIRECTORY, FALSE);
	sDestFilename.Replace("<DESKTOP>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_STARTMENU, FALSE);
	sDestFilename.Replace("<STARTMENU>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_PROGRAMS, FALSE);
	sDestFilename.Replace("<SMPROGRAMS>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_STARTUP, FALSE);
	sDestFilename.Replace("<SMSTARTUP>", acBuffer);

//	sDestFilename.Replace("<QUICKLAUNCH>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_SENDTO, FALSE);
	sDestFilename.Replace("<SENDTO>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, 0x0006, FALSE); // CSIDL_FAVORITES (0x0006)
	sDestFilename.Replace("<FAVORITES>", acBuffer);

	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, CSIDL_APPDATA, FALSE);
	sDestFilename.Replace("<APPDATA>", acBuffer);
	
	memset(acBuffer, 0, MAX_PATH);
	SHGetSpecialFolderPath(NULL, acBuffer, 0x001c, FALSE); // CSIDL_LOCAL_APPDATA (0x001c)
	sDestFilename.Replace("<LOCAL_APPDATA>", acBuffer);

//	sDestFilename.Replace("<TEMP>", acBuffer);

	return sDestFilename;
}

string& replace_all(string& str, const string& old_value, const string& new_value)
{
	while (true)
	{
		string::size_type pos(0);
		if( string::npos != (pos=str.find(old_value)) )
			str.replace(pos,old_value.length(),new_value);
		else
			break;
	}

	return str;
}