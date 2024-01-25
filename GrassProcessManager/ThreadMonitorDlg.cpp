// ThreadMonitorDlg.cpp: 实现文件
// 用于显示线程信息的对话框

#include "pch.h"
#include "ProcessManager.h"
#include "ProcessManagerDlg.h"
#include "ThreadMonitorDlg.h"
#include "afxdialogex.h"
#include <TlHelp32.h>

// ThreadMonitorDlg 对话框

IMPLEMENT_DYNAMIC(ThreadMonitorDlg, CDialogEx)

ThreadMonitorDlg::ThreadMonitorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_THREAD, pParent)
{

}

ThreadMonitorDlg::~ThreadMonitorDlg()
{
}

void ThreadMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_THREAD, m_lstThread);
}

BOOL ThreadMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 列表风格和列表头行初始化
	m_lstThread.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_lstThread.InsertColumn(0, L"Thread ID", 0, 80);
	m_lstThread.InsertColumn(1, L"Parent Process ID", 0, 100);
	m_lstThread.InsertColumn(2, L"Thread Priority", 0, 0);
	// 填充列表
	InitThreadList();
	return TRUE;
}


void ThreadMonitorDlg::InitThreadList()
{
	// 根据进程ID获取到该进程的所有线程快照
	HANDLE hSnapThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	THREADENTRY32 thread32 = { sizeof(thread32) }; // 该结构体用于获取线程的各类标识信息
	CString csThreadID;
	CString csParentProcessID;
	CString csBasePri;
	DWORD dwIndex = 0;
	// 获取到第一个线程
	BOOL bRet = Thread32First(hSnapThread, &thread32);
	// 开始遍历
	while (bRet)
	{
		if (thread32.th32OwnerProcessID == g_dwProcessId)
		{
			// 获取/设置线程ID
			csThreadID.Format(L"%d", thread32.th32ThreadID);
			m_lstThread.InsertItem(dwIndex, csThreadID);
			// 获取/设置父进程ID
			csParentProcessID.Format(L"%d", thread32.th32OwnerProcessID);
			m_lstThread.SetItemText(dwIndex, 1, csParentProcessID);
			// 获取/设置线程优先级
			csBasePri.Format(L"%d", thread32.tpBasePri);
			m_lstThread.SetItemText(dwIndex, 2, csBasePri);
			dwIndex++;
		}
		bRet = Thread32Next(hSnapThread, &thread32);

	}
	CloseHandle(hSnapThread);

}


BEGIN_MESSAGE_MAP(ThreadMonitorDlg, CDialogEx)
	ON_COMMAND(ID_SUSPEND_THRE, &ThreadMonitorDlg::OnSuspendThread)
	ON_COMMAND(ID_RESUME_THRE, &ThreadMonitorDlg::OnResumeThre)
	ON_COMMAND(ID_KILL_THRE, &ThreadMonitorDlg::OnKillThread)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_THREAD, &ThreadMonitorDlg::OnRclickListThread)
END_MESSAGE_MAP()


// ThreadMonitorDlg 消息处理程序

// 挂起线程
void ThreadMonitorDlg::OnSuspendThread()
{
	// 根据选中的地方确定是哪个线程
	int iPos = (int)m_lstThread.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到线程ID(iPos为选中行, 0为该行的第一列(0,1,2))
	CString csThreadID = m_lstThread.GetItemText(iPos, 0);
	// 转换成int型
	int iThreadID = _ttoi(csThreadID);
	// 根据线程ID获取到线程句柄
	HANDLE hSuspendThread = OpenThread(THREAD_ALL_ACCESS, FALSE, iThreadID);
	if (INVALID_HANDLE_VALUE == hSuspendThread)
	{
		//AfxMessageBox(L"结束进程", MB_OK);
		return;
	}
	SuspendThread(hSuspendThread);
	CloseHandle(hSuspendThread);
}

// 恢复挂起的线程
void ThreadMonitorDlg::OnResumeThre()
{
	// 根据选中的地方确定是哪个线程
	int iPos = (int)m_lstThread.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到线程ID(iPos为选中行, 0为该行的第一列(0,1,2))
	CString csThreadID = m_lstThread.GetItemText(iPos, 0);
	// 转换成int型
	int iThreadID = _ttoi(csThreadID);
	// 根据线程ID获取到线程句柄
	HANDLE hResumeThread = OpenThread(THREAD_ALL_ACCESS, FALSE, iThreadID);
	if (INVALID_HANDLE_VALUE == hResumeThread)
	{
		//AfxMessageBox(L"结束进程", MB_OK);
		return;
	}
	// 恢复被挂起的线程
	ResumeThread(hResumeThread);
	CloseHandle(hResumeThread);
}

// 结束指定线程
void ThreadMonitorDlg::OnKillThread()
{
	// 根据选中的地方确定是哪个线程
	int iPos = (int)m_lstThread.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到线程ID(iPos为选中行, 0为该行的第一列(0,1,2))
	CString csThreadID = m_lstThread.GetItemText(iPos, 0);
	// 转换成int型
	int iThreadID = _ttoi(csThreadID);
	// 根据线程ID获取到线程句柄
	HANDLE hKillThread = OpenThread(THREAD_ALL_ACCESS, FALSE, iThreadID);
	if (INVALID_HANDLE_VALUE == hKillThread)
	{
		//AfxMessageBox(L"结束进程", MB_OK);
		return;
	}
	TerminateThread(hKillThread, 0);
	// 结束后需要重新获取到线程列表,并重新填充
	m_lstThread.DeleteAllItems();
	InitThreadList();
	CloseHandle(hKillThread);
}

// 弹出线程对话框的右键菜单
void ThreadMonitorDlg::OnRclickListThread(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint cPT;
	GetCursorPos(&cPT);
	HMENU hMenu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MENU_THREAD));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);

	TrackPopupMenu(hSubMenu, TPM_CENTERALIGN, cPT.x, cPT.y, 0, m_hWnd, NULL);
}
