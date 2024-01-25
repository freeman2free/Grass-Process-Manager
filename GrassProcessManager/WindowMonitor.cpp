// WindowMonitor.cpp: 实现文件
//

#include "pch.h"
#include "ProcessManager.h"
#include "WindowMonitor.h"
#include "afxdialogex.h"
#include <Psapi.h>


// CWindowMonitor 对话框

IMPLEMENT_DYNAMIC(CWindowMonitor, CDialogEx)

CWindowMonitor::CWindowMonitor(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_WINDOW, pParent)
{

}

CWindowMonitor::~CWindowMonitor()
{
}

void CWindowMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_WINDOW, m_lstWindow);
}


BEGIN_MESSAGE_MAP(CWindowMonitor, CDialogEx)
	ON_COMMAND(ID_MENUWINDOW_SHOWWINDOW, &CWindowMonitor::OnMenuwindowShowwindow)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_WINDOW, &CWindowMonitor::OnNMRClickListWindow)
	ON_COMMAND(ID_MENUWINDOW_HIDEWINDOW, &CWindowMonitor::OnMenuwindowHidewindow)
	ON_COMMAND(ID_MENUWINDOW_MAXIMIZE, &CWindowMonitor::OnMenuwindowMaximize)
	ON_COMMAND(ID_MENUWINDOW_MINIMIZE, &CWindowMonitor::OnMenuwindowMinimize)
	ON_COMMAND(ID_MENUWINDOW_KILLPROCESS, &CWindowMonitor::OnMenuwindowKillprocess)
	ON_COMMAND(ID_MENUWINDOW_REFRESH, &CWindowMonitor::OnMenuwindowRefresh)
END_MESSAGE_MAP()


// CWindowMonitor 消息处理程序


BOOL CWindowMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lstWindow.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT); // 设置列表风格
	// 设置该列表每一列的标题
	m_lstWindow.InsertColumn(0, L"Window Name", 0, 300);
	m_lstWindow.InsertColumn(1, L"Running State", 0, 60);
	m_lstWindow.InsertColumn(2, L"Process ID", 0, 50);
	m_lstWindow.InsertColumn(3, L"Window Handle", 0, 80);
	m_lstWindow.InsertColumn(4, L"Path", 0, 600);

	m_lstImage.Create(18, 18, ILC_MASK | ILC_COLOR32, 0, 1);

	m_lstWindow.SetImageList(&m_lstImage, LVSIL_SMALL);

	InitWindowList();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

VOID CWindowMonitor::InitWindowList()
{
	// 使用函数指针动态加载检测窗口是否响应的函数
	typedef int(_stdcall* FuncIsHungAppWindow)(HWND hWnd);
	FuncIsHungAppWindow pIsHung = (FuncIsHungAppWindow)GetProcAddress(LoadLibrary(L"user32.dll"), "IsHungAppWindow");
	// 先清空原先的内容(防止重新初始化时有错误)
	m_lstWindow.DeleteAllItems();
	CWnd* pWnd = AfxGetMainWnd()->GetWindow(GW_HWNDFIRST);

	CString csCaption;
	DWORD dwIndex = 0;

	while (m_lstImage.Remove(0));

	while (pWnd)
	{
		// 如果该窗口可见且没有所有者
		if ((pWnd->IsWindowVisible()) && (!pWnd->GetOwner()))
		{
			// 获取/设置窗口标题
			pWnd->GetWindowText(csCaption);
			// 如果获取到没有标题的窗口那么就不需要列出来
			if (csCaption.IsEmpty())
			{
				pWnd = pWnd->GetWindow(GW_HWNDNEXT);
				continue;
			}
			csCaption.TrimLeft(); // 对窗口标题文本进行一些处理
			csCaption.TrimRight();
			m_lstWindow.InsertItem(dwIndex, csCaption, dwIndex); // 将标题设置到列表控件上


			// 标题栏不为空
			if (!csCaption.IsEmpty())
			{
				/*获取并处理数据*/
				// 获取进程ID
				TCHAR szProcessID[MAX_PATH] = { 0 }; // 进程ID
				DWORD dwWindowPeocessID = 0;
				GetWindowThreadProcessId(pWnd->m_hWnd, &dwWindowPeocessID);
				wsprintf(szProcessID, L"%d", dwWindowPeocessID);
				// 获取窗口句柄
				TCHAR szHWnd[MAX_PATH] = { 0 }; // 窗口句柄
				wsprintf(szHWnd, L"%d", (DWORD)pWnd->m_hWnd);
				// 获取窗口路径
				TCHAR szPath[MAX_PATH] = { 0 }; // 窗口路径
				HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwWindowPeocessID);
				GetModuleFileNameEx(hProc, NULL, szPath, MAX_PATH); // 检索包含指定模块的文件的完整路径并返回该路径
				/*设置数据到列表控件上*/
				// 将运行状态设置在列表控件上
				if (pIsHung(pWnd->m_hWnd)) // 判断窗口是否未响应
				{
					m_lstWindow.SetItemText(dwIndex, 1, L"Non-Response");
				}
				else
				{
					m_lstWindow.SetItemText(dwIndex, 1, L"Running");
				}
				// 将进程ID/窗口句柄/窗口路径设置在列表控件上
				m_lstWindow.SetItemText(dwIndex, 2, szProcessID);
				m_lstWindow.SetItemText(dwIndex, 3, szHWnd);
				m_lstWindow.SetItemText(dwIndex, 4, szPath);
				InsertImage(szPath);
				// 行数每次+1
				dwIndex++;
			}
		}
		pWnd = pWnd->GetWindow(GW_HWNDNEXT);
	}
	return;
}

VOID CWindowMonitor::InsertImage(CString lPNamePath)
{
	// 获取图标
	HICON hSmallIcon = ::ExtractIcon(NULL, lPNamePath, 0);
	// 没有获取到的话
	if (NULL == hSmallIcon)
	{
		m_lstImage.Add(LoadIcon(NULL, IDI_APPLICATION)); // 就加载默认的
	}
	else
	{
		m_lstImage.Add(hSmallIcon);
	}
}


void CWindowMonitor::OnMenuwindowShowwindow()
{
	// 根据选中的地方确定是哪一行(哪个窗口)
	int iPos = (int)m_lstWindow.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到窗口句柄(iPos为选中行, 3为该行的第四列(0,1,2,3))
	CString csWnd = m_lstWindow.GetItemText(iPos, 3);

	int iWnd = _ttoi(csWnd);
	// ShowWindowAsync该函数可以指定窗口及其显示模式
	ShowWindowAsync((HWND)iWnd, SW_SHOW);
}


void CWindowMonitor::OnNMRClickListWindow(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	POINT pt = { 0 };
	GetCursorPos(&pt); // 获取点击位置,稍后弹出时需要用到
	// 获取主菜单IDC_LIST_PROCESS
	HMENU hMenu = LoadMenu((HINSTANCE)GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDR_MENU_WINDOW));
	// 获取子菜单
	HMENU hSubMenu = GetSubMenu(hMenu, NULL);
	// 弹出菜单
	TrackPopupMenu(hSubMenu, TPM_CENTERALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	*pResult = 0;
}


void CWindowMonitor::OnMenuwindowHidewindow()
{
	// 根据选中的地方确定是哪一行(哪个窗口)
	int iPos = (int)m_lstWindow.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到窗口句柄(iPos为选中行, 3为该行的第四列(0,1,2,3))
	CString csWnd = m_lstWindow.GetItemText(iPos, 3);

	int iWnd = _ttoi(csWnd);
	// ShowWindowAsync该函数可以指定窗口及其显示模式
	ShowWindowAsync((HWND)iWnd, SW_HIDE);
}


void CWindowMonitor::OnMenuwindowMaximize()
{
	// 根据选中的地方确定是哪一行(哪个窗口)
	int iPos = (int)m_lstWindow.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到窗口句柄(iPos为选中行, 3为该行的第四列(0,1,2,3))
	CString csWnd = m_lstWindow.GetItemText(iPos, 3);

	int iWnd = _ttoi(csWnd);
	// ShowWindowAsync该函数可以指定窗口及其显示模式
	ShowWindowAsync((HWND)iWnd, SW_MAXIMIZE);
}


void CWindowMonitor::OnMenuwindowMinimize()
{
	// 根据选中的地方确定是哪一行(哪个窗口)
	int iPos = (int)m_lstWindow.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到窗口句柄(iPos为选中行, 3为该行的第四列(0,1,2,3))
	CString csWnd = m_lstWindow.GetItemText(iPos, 3);

	int iWnd = _ttoi(csWnd);
	// ShowWindowAsync该函数可以指定窗口及其显示模式
	ShowWindowAsync((HWND)iWnd, SW_MINIMIZE);
}


void CWindowMonitor::OnMenuwindowKillprocess()
{
	// 根据选中的地方确定是哪一行(哪个窗口)
	int iPos = (int)m_lstWindow.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到进程ID(iPos为选中行, 2为该行的第三列(0,1,2))
	CString csProcessID = m_lstWindow.GetItemText(iPos, 2);
	// 转换成int型的进程ID
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
	CloseHandle(hKillProcess);
	// 结束进程后需要重新获取进程列表(已经出现了变化),并重新填充
	InitWindowList();
}


void CWindowMonitor::OnMenuwindowRefresh()
{
	m_lstWindow.DeleteAllItems();
	InitWindowList();
}
