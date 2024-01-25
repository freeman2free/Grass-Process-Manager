// SoftDlg.cpp: 实现文件
//

#include "pch.h"
#include "ProcessMonitor.h"
#include "SoftDlg.h"
#include "afxdialogex.h"


// CSoftDlg 对话框

IMPLEMENT_DYNAMIC(CSoftDlg, CDialogEx)

CSoftDlg::CSoftDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ALLSOFT, pParent)
{

}

CSoftDlg::~CSoftDlg()
{
}

void CSoftDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SOFTINFO, m_lstSoft);
}


BEGIN_MESSAGE_MAP(CSoftDlg, CDialogEx)
  ON_BN_CLICKED(IDC_BUTTON_UNISTALL, &CSoftDlg::OnBnClickedButtonUnistall)
END_MESSAGE_MAP()


// CSoftDlg 消息处理程序


void CSoftDlg::InitSoftList()
{
  HKEY hKeyRet;
  HKEY hQueryKey; // 要查询的键
  DWORD dwIndex = 0; // 遍历注册表项的下标(要检索的子项的下标)
  DWORD dwIndexList = 0;
  TCHAR szSubKeyName[MAX_PATH] = { 0 }; // 返回的遍历到的子项的名称
  DWORD dwSubKeyNameSize = 255;           // 返回的遍历到的子项名称的长度
  DWORD dwQueryDataSize = 255;           // 存储查询到的数据的缓冲区的长度
  DWORD dwType = REG_SZ;               // 要查询的注册表项的数据类型
  // 打开指定的注册表项(HKEY_LOCAL_MACHINE主键下的szKey子项)
  LSTATUS lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_csKey.GetString(), NULL, KEY_ALL_ACCESS, &hKeyRet);
  if (lResult != ERROR_SUCCESS)
  {
    if (lResult == ERROR_FILE_NOT_FOUND)
    {
      printf("Key not found.\n");
      return;
    }
    else
    {
      printf("Error opening key.\n");
      return;
    }
  }

  TCHAR szName[255] = { 0 };           // 名称
  TCHAR szVersion[255] = { 0 };         // 版本
  TCHAR szInstallDate[255] = { 0 };     // 安装时间
  TCHAR szPublisher[255] = { 0 };       // 发行商
  TCHAR szInstallPath[255] = { 0 };     // 安装路径
  TCHAR szUninstall[255] = { 0 };       // 卸载路径
  // 开始遍历,逐个遍历该子项下的键
  while (SHEnumKeyEx(hKeyRet, dwIndex, szSubKeyName, &dwSubKeyNameSize) != ERROR_NO_MORE_ITEMS)
  {
    // 拼接好要查询的键的完整路径
    CString csTemp(m_csKey);
    csTemp += "\\";
    csTemp += szSubKeyName;
    // 先查询一下键是否有值
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, csTemp.GetString(), NULL, KEY_ALL_ACCESS, &hQueryKey) == ERROR_SUCCESS)
    {
      // 查询指定的信息
      RegQueryValueEx(hQueryKey, L"DisplayName", NULL, &dwType, (LPBYTE)szName, &dwQueryDataSize);

      dwQueryDataSize = 255;
      RegQueryValueEx(hQueryKey, L"DisplayVersion", NULL, &dwType, (LPBYTE)szVersion, &dwQueryDataSize);

      dwQueryDataSize = 255;
      RegQueryValueEx(hQueryKey, L"InstallDate", NULL, &dwType, (LPBYTE)szInstallDate, &dwQueryDataSize);

      dwQueryDataSize = 255;
      RegQueryValueEx(hQueryKey, L"Publisher", NULL, &dwType, (LPBYTE)szPublisher, &dwQueryDataSize);

      dwQueryDataSize = 255;
      RegQueryValueEx(hQueryKey, L"InstallLocation", NULL, &dwType, (LPBYTE)szInstallPath, &dwQueryDataSize);

      dwQueryDataSize = 255;
      RegQueryValueEx(hQueryKey, L"UninstallString", NULL, &dwType, (LPBYTE)szUninstall, &dwQueryDataSize);
    }
    if (255 != dwQueryDataSize)
    {
      m_lstSoft.InsertItem(dwIndexList, szName);
      m_lstSoft.SetItemText(dwIndexList, 1, szVersion);
      m_lstSoft.SetItemText(dwIndexList, 2, szInstallDate);
      m_lstSoft.SetItemText(dwIndexList, 3, szPublisher);
      m_lstSoft.SetItemText(dwIndexList, 4, szInstallPath);
      m_lstSoft.SetItemText(dwIndexList, 5, szUninstall);
      dwIndexList++;
    }


    // 关闭本次查询的注册表项(键)
    RegCloseKey(hQueryKey);
    // 重置一下
    dwSubKeyNameSize = 255;
    dwQueryDataSize = 255;
    // 下标++
    dwIndex++;
  }
}

BOOL CSoftDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lstSoft.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_lstSoft.InsertColumn(0, L"Name", 0, 80);
	m_lstSoft.InsertColumn(1, L"Version", 0, 80);
	m_lstSoft.InsertColumn(2, L"Install Date", 0, 80);
	m_lstSoft.InsertColumn(3, L"Publisher", 0, 80);
	m_lstSoft.InsertColumn(4, L"Position", 0, 100);
	m_lstSoft.InsertColumn(5, L"Uninstall Position", 0, 100);
  m_csKey = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
	InitSoftList();

	return TRUE;  // return TRUE unless you set the focus to a control
								// 异常: OCX 属性页应返回 FALSE
}

// 卸载程序
void CSoftDlg::OnBnClickedButtonUnistall()
{
  // 根据选中的地方确定是哪个软件
  int iPos = (int)m_lstSoft.GetFirstSelectedItemPosition();
  iPos -= 1;
  CString csUnistallPath = m_lstSoft.GetItemText(iPos, 5);
  // 执行卸载程序
  PROCESS_INFORMATION procInfo;
  STARTUPINFO startInfo = { sizeof(STARTUPINFO) };
  BOOL bRet = CreateProcess(NULL, (LPWSTR)csUnistallPath.GetString(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startInfo, &procInfo);
  if(!bRet)
  {
    return;
  }
  WaitForSingleObject(procInfo.hProcess, INFINITE);
  CloseHandle(procInfo.hThread);
  CloseHandle(procInfo.hProcess);
}
