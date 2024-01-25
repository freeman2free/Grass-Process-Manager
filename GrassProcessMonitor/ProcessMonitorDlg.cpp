
// ProcessMonitorDlg.cpp: 实现文件
// MFC实现一个进程管理器(进程List)
/*功能目前有:
* 1.显示进程信息
* 2.右键菜单:显示指定进程的线程信息
* 3.右键菜单:显示指定进程的模块信息
* 4.右键菜单:使用远程创建线程的方式进行注入
* 5.模块界面中可卸载指定模块(未完善)
*/
#include "pch.h"
#include "framework.h"
#include "ProcessMonitor.h"
#include "ProcessMonitorDlg.h"
#include "afxdialogex.h"
#include <TlHelp32.h>
#include <Psapi.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 保存进程ID以便进程调用
DWORD g_dwProcessId = 0;
// CProcessMonitorDlg 对话框



CProcessMonitorDlg::CProcessMonitorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROCESSMONITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProcessMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_lstProcess);
}

BEGIN_MESSAGE_MAP(CProcessMonitorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_CHECK_THREAD, &CProcessMonitorDlg::OnCheckThread)
	ON_COMMAND(ID_KILL_PROCESS, &CProcessMonitorDlg::OnKillProcess)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CProcessMonitorDlg::OnRclickListProcess)
	ON_COMMAND(ID_FLUSH_PROCESS, &CProcessMonitorDlg::OnFlushProcess)
	ON_COMMAND(ID_OPEN_MODULE, &CProcessMonitorDlg::OnOpenModule)
	ON_COMMAND(ID_INJECT_THREAD, &CProcessMonitorDlg::OnInjectThread)
	ON_COMMAND(ID_RELEASE_THRE, &CProcessMonitorDlg::OnReleaseThread)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_ALLSOFT, &CProcessMonitorDlg::OnBnClickedButtonAllSoft)
	ON_BN_CLICKED(IDC_BUTTON_WINDOW, &CProcessMonitorDlg::OnBnClickedButtonWindow)
END_MESSAGE_MAP()


// CProcessMonitorDlg 消息处理程序

BOOL CProcessMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 初始化
	GetClientRect(&m_cDlgRect);
	m_cDlgRect.SetRect(m_cDlgRect.left, m_cDlgRect.top, m_cDlgRect.right, m_cDlgRect.bottom);
	// 初始化列表控件
		// 列表风格和列表头初始化
	m_lstProcess.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_lstProcess.InsertColumn(0, L"Process Name", 0, 130);
	m_lstProcess.InsertColumn(1, L"Parent Process", 0, 50);
	m_lstProcess.InsertColumn(2, L"Process ID", 0, 50);
	m_lstProcess.InsertColumn(3, L"Number of threads", 0, 50);
	m_lstProcess.InsertColumn(4, L"Priority", 0, 50);
	m_lstProcess.InsertColumn(5, L"Path", 0, 300);
	m_lstProcess.InsertColumn(6, L"File creation time", 0, 220);
		// 
	InitProcessList();

	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CProcessMonitorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CProcessMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CProcessMonitorDlg::RePaint(UINT uID, int iLastWidth, int iNowWidth, int iLastHeight, int iNowHeight)
{
	CRect rect;
	CWnd* wnd = NULL;
	wnd = GetDlgItem(uID);
	if (NULL == wnd)
	{
		return;
	}
	wnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.left = (long)((double)rect.left / (double)iLastWidth * (double)iNowWidth);
	rect.right = (long)((double)rect.right / (double)iLastWidth * (double)iNowWidth);
	rect.top = (long)((double)rect.top / (double)iLastHeight * (double)iNowHeight);
	rect.bottom = (long)((double)rect.bottom / (double)iLastHeight * (double)iNowHeight);
	wnd->MoveWindow(&rect);
}

void CProcessMonitorDlg::InitProcessList()
{
	DWORD dwIndex = 0;
	HANDLE hSnapProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL); // 创建所有进程的快照对象
	if (hSnapProcess == INVALID_HANDLE_VALUE)
	{
		printf("Create process snap Fail: %d\r\n", GetLastError());
		return;
	}
	PROCESSENTRY32 pe32Pro = { sizeof(PROCESSENTRY32) };
	BOOL bRet = Process32First(hSnapProcess, &pe32Pro);
	CString csParentProcess;
	CString csPID;
	CString csThreadCnt;
	CString csPriClassBase;
	CString csCreationTime;
	TCHAR szFilePath[MAX_PATH];
	// 遍历所有进程
	while (bRet)
	{
		// 获取/设置进程名
		m_lstProcess.InsertItem(dwIndex, pe32Pro.szExeFile);
		// 获取/设置"父进程"
		csParentProcess.Format(L"%d", pe32Pro.th32ParentProcessID);
		m_lstProcess.SetItemText(dwIndex, 1, csParentProcess);

		// 获取/设置"进程ID"
		csPID.Format(L"%d", pe32Pro.th32ProcessID);
		m_lstProcess.SetItemText(dwIndex, 2, csPID);

		// 获取/设置线程数
		csThreadCnt.Format(L"%d", pe32Pro.cntThreads);
		m_lstProcess.SetItemText(dwIndex, 3, csThreadCnt);

		// 获取/设置优先级
		csPriClassBase.Format(L"%d", pe32Pro.pcPriClassBase);
		m_lstProcess.SetItemText(dwIndex, 4, csPriClassBase);

		// 获取/设置进程路径
		HANDLE hCurProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32Pro.th32ProcessID);
		
		DWORD dwRet = GetModuleFileNameEx(hCurProcess, NULL, szFilePath, MAX_PATH);
		if (dwRet == 0)
		{
			m_lstProcess.SetItemText(dwIndex, 5, L"System Process");
		}
		else
		{
			m_lstProcess.SetItemText(dwIndex, 5, szFilePath);
		}
		// 获取/设置文件创建时间
		WIN32_FIND_DATA tFileInfo;
		SYSTEMTIME tCreateTime;
		FindFirstFile(szFilePath, &tFileInfo);
		FileTimeToSystemTime(&tFileInfo.ftCreationTime, &tCreateTime);
		csCreationTime.Format(L"%d-%d-%d", tCreateTime.wYear, tCreateTime.wMonth, tCreateTime.wDay);
		m_lstProcess.SetItemText(dwIndex, 6, csCreationTime);

		CloseHandle(hCurProcess);
		bRet = Process32Next(hSnapProcess, &pe32Pro);
		dwIndex++;
	}
	CloseHandle(hSnapProcess);
}


// 点击后查看线程(弹出线程对话框)
void CProcessMonitorDlg::OnCheckThread()
{
	// 想查看一个进程的线程需要获取该线程的句柄
	// 根据选中的地方确定是哪个进程
	int iPos = (int)m_lstProcess.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到进程ID(iPos为选中行, 2为该行的第三列(0,1,2))
	CString csProcessID = m_lstProcess.GetItemText(iPos, 2);
	// 转换成int型
	g_dwProcessId = _ttoi(csProcessID);

	ThreadMonitorDlg cThreadDlg;

	cThreadDlg.DoModal();
}

// 右键菜单弹出
void CProcessMonitorDlg::OnRclickListProcess(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint cPT;
	GetCursorPos(&cPT);
	HMENU hMenu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MENU_PROCESS));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);

	TrackPopupMenu(hSubMenu, TPM_CENTERALIGN, cPT.x, cPT.y, 0, m_hWnd, NULL);
	*pResult = 0;
}

// 结束进程
void CProcessMonitorDlg::OnKillProcess()
{
	// 根据选中的地方确定是哪个进程
	int iPos = (int)m_lstProcess.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到进程ID(iPos为选中行, 2为该行的第三列(0,1,2))
	CString csProcessID = m_lstProcess.GetItemText(iPos,2);
	// 转换成int型
	int iProcessID = _ttoi(csProcessID);
	// 根据进程ID获取到进程句柄
	HANDLE hKillProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);
	if (INVALID_HANDLE_VALUE == hKillProcess)
	{
		//AfxMessageBox(L"结束进程", MB_OK);
		return;
	}
	// 结束该进程
	TerminateProcess(hKillProcess, 0);
	// 结束进程后需要重新获取进程列表(已经出现了变化),并重新填充
	OnFlushProcess();
}

// 刷新列表
void CProcessMonitorDlg::OnFlushProcess()
{
	m_lstProcess.DeleteAllItems();
	InitProcessList();
}

// 打开指定进程的模块列表
void CProcessMonitorDlg::OnOpenModule()
{

	// 想查看一个进程的线程需要获取该线程的句柄
	// 根据选中的地方确定所选择的是哪个进程
	int iPos = (int)m_lstProcess.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到进程ID(iPos为选中行, 2为该行的第三列(0,1,2))
	CString csProcessID = m_lstProcess.GetItemText(iPos, 2);
	// 转换成int型
	g_dwProcessId = _ttoi(csProcessID);

	CModuleDialog cModuleDlg;
	cModuleDlg.DoModal();
}


void CProcessMonitorDlg::OnInjectThread()
{
	// 根据选中的地方确定是哪个进程
	int iPos = (int)m_lstProcess.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到进程的ID(iPos为选中行, 2为该行的第三列(0,1,2))
	CString csProcessID = m_lstProcess.GetItemText(iPos, 2);
	// 转换成int型
	DWORD dwProcessID = _ttoi(csProcessID);

	OnInjectThread(dwProcessID, (TCHAR*)m_szPath);
}

// 注入到指定进程中(根据进程的ID找到进程之后注入)
void CProcessMonitorDlg::OnInjectThread(DWORD dwPID, TCHAR* szPath)
{
	// 获取到进程的句柄
	HANDLE hInjectProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (INVALID_HANDLE_VALUE == hInjectProcess)
	{
		return;
	}

	// 准备注入
	// 在目标进程体内申请一段空间,用于存储要加载的DLL的路径
	m_lpAddr = VirtualAllocEx(hInjectProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	SIZE_T dwWriteSize = 0;
	DWORD dwRetID = 0;
	// 判断一下分配内存的保护属性
	MEMORY_BASIC_INFORMATION memInfo = { 0 };
	DWORD dwOldProtect = 0;
	SIZE_T sLength = VirtualQueryEx(hInjectProcess, m_lpAddr, &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
	if (sLength == 0)
	{
		return;
	}
	if (memInfo.AllocationProtect != PAGE_EXECUTE_READWRITE)
	{
		VirtualProtectEx(hInjectProcess, m_lpAddr, MAX_PATH, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	}

	// 将要加载的DLL的路径写入开辟好的空间中
	BOOL bRet = WriteProcessMemory(hInjectProcess, m_lpAddr, m_szPath, lstrlen(m_szPath), &dwWriteSize);
	if (!bRet)
	{
		CloseHandle(hInjectProcess);
		return;
	}
	// 把要加载的DLL通过创建远程线程的方式注入到指定进程中
	// 在目标进程内部创建一个线程:该线程调用LoadLibrary函数,将上面写好的DLL加载路径作为其参数传入
	HANDLE hInjectThread = CreateRemoteThread(hInjectProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, m_lpAddr, NULL, &dwRetID);
	if (INVALID_HANDLE_VALUE == hInjectThread)
	{
		CloseHandle(hInjectProcess);
		return;
	}
	if (NULL == hInjectThread)
	{
		CloseHandle(hInjectProcess);
		return;
	}
	// 等待该线程执行完毕
	WaitForSingleObject(hInjectThread, -1);
	// 再将旧的保护属性修改回来
	VirtualProtectEx(hInjectProcess, m_lpAddr, MAX_PATH, dwOldProtect, &dwOldProtect);

	CloseHandle(hInjectProcess);
	CloseHandle(hInjectThread);
}

// 卸载注入的线程
void CProcessMonitorDlg::OnReleaseThread()
{
	int iPos = (int)m_lstProcess.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到进程的ID(iPos为选中行, 2为该行的第三列(0,1,2))
	CString csProcessID = m_lstProcess.GetItemText(iPos, 2);
	// 转换成int型
	DWORD dwProcessID = _ttoi(csProcessID);
	// 通过进程ID获取到被注入的进程的句柄
	HANDLE hInjectProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
	// 释放掉刚才在目标进程中申请好的空间
	BOOL bRet = VirtualFreeEx(hInjectProcess, m_lpAddr, 0, MEM_RELEASE);
	if (bRet == FALSE)
	{
		CloseHandle(hInjectProcess);
		return;
	}
	CloseHandle(hInjectProcess);
}

// 使列表尺寸随着窗口尺寸一起变化
void CProcessMonitorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (0 == m_cDlgRect.left && 0 == m_cDlgRect.right && 0 == m_cDlgRect.bottom && 0 == m_cDlgRect.top)
	{
		return;
	}
	else
	{
		if (0 == cx && 0 == cy)
		{
			return;
		}
		// 如果对话框窗口尺寸出现变化进行重绘
		CRect rectChangeSize;
		GetClientRect(&rectChangeSize);
		//重绘函数，用以更新对话框上的列表控件的位置和大小
		RePaint(IDC_LIST_PROCESS, m_cDlgRect.Width(), rectChangeSize.Width(), m_cDlgRect.Height(), rectChangeSize.Height());
	}
	// save size of dialog
	GetClientRect(&m_cDlgRect);
	Invalidate(); // 制造无效区域进行重绘
}

// 查看当前系统所有已安装软件信息()
void CProcessMonitorDlg::OnBnClickedButtonAllSoft()
{
	// TODO: 在此添加控件通知处理程序代码
	CSoftDlg cSoftDlg;
	cSoftDlg.DoModal();
}


void CProcessMonitorDlg::OnBnClickedButtonWindow()
{
	// TODO: 在此添加控件通知处理程序代码
	CWindowMonitor cWindowDlg;
	cWindowDlg.DoModal();
	// 结束后刷新一下进程列表,因为有可能出现已经结束的进程
	OnFlushProcess();
}
