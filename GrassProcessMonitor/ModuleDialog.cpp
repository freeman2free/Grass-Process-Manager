// ModuleDialog.cpp: 实现文件
// 用于显示模块信息的对话框

#include "pch.h"
#include "ProcessMonitor.h"
#include "ProcessMonitorDlg.h"
#include "ModuleDialog.h"
#include "afxdialogex.h"
#include <TlHelp32.h>

// CModuleDialog 对话框

IMPLEMENT_DYNAMIC(CModuleDialog, CDialogEx)

CModuleDialog::CModuleDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MODULE, pParent)
{

}

CModuleDialog::~CModuleDialog()
{
}

void CModuleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MODULE, m_lstModule);
}


BEGIN_MESSAGE_MAP(CModuleDialog, CDialogEx)
	ON_COMMAND(ID_RELEASE_MODULE, &CModuleDialog::OnReleaseModule)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_MODULE, &CModuleDialog::OnNMRClickListModule)
END_MESSAGE_MAP()


// CModuleDialog 消息处理程序


BOOL CModuleDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 列表风格和列表头行初始化
	m_lstModule.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_lstModule.InsertColumn(0, L"Module Name", 0, 150);
	m_lstModule.InsertColumn(1, L"Process ID", 0, 80);
	m_lstModule.InsertColumn(2, L"Module Base", 0, 80);
	m_lstModule.InsertColumn(3, L"Module Size(Byte)", 0, 100);
	m_lstModule.InsertColumn(4, L"Module Path", 0, 200);

	InitModuleList();
	return TRUE;  // return TRUE unless you set the focus to a control
								// 异常: OCX 属性页应返回 FALSE
}

void CModuleDialog::InitModuleList()
{
	// 根据进程ID获取到该进程的所有加载的模块的快照
	HANDLE hSnapModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32, g_dwProcessId);
	MODULEENTRY32 module32 = { sizeof(MODULEENTRY32) }; // 该结构体用于获取模块的各类标识信息
	CString csModuleName;
	CString csParentProcessID;
	CString csBaseAddr;
	CString csModuleSize;
	CString csModulePath;
	DWORD dwIndex = 0;
	// 获取到第一个模块
	BOOL bRet = Module32First(hSnapModule, &module32);
	// 开始遍历所有模块
	while (bRet)
	{
		// 在列表上填充获取的数据
		if (module32.th32ProcessID == g_dwProcessId)
		{
			// 获取/设置模块名
			csModuleName.Format(L"%s", module32.szModule);
			m_lstModule.InsertItem(dwIndex, csModuleName);
			// 获取/设置所属进程ID
			csParentProcessID.Format(L"%d", module32.th32ProcessID);
			m_lstModule.SetItemText(dwIndex, 1, csParentProcessID);
			// 获取/设置模块基址
			csBaseAddr.Format(L"0x%p", module32.modBaseAddr);
			m_lstModule.SetItemText(dwIndex, 2, csBaseAddr);
			// 获取/设置模块大小
			csModuleSize.Format(L"%d", module32.modBaseSize);
			m_lstModule.SetItemText(dwIndex, 3, csModuleSize);
			// 获取/设置模块路径
			csModulePath.Format(L"%s", module32.szExePath);
			m_lstModule.SetItemText(dwIndex, 4, csModulePath);
			dwIndex++;
		}
		bRet = Module32Next(hSnapModule, &module32);
	}
	CloseHandle(hSnapModule);
}

// 卸载指定模块
void CModuleDialog::OnReleaseModule()
{
	// 根据选中的地方确定是哪个模块
	int iPos = (int)m_lstModule.GetFirstSelectedItemPosition();
	iPos -= 1;
	// 获取到模块的基址(iPos为选中行, 0为该行的第一列(0,1,2))
	CString csBaseAddr = m_lstModule.GetItemText(iPos, 2);
	// 获取到模块的路径
	CString csModulePath = m_lstModule.GetItemText(iPos, 4);
	// 基址转换成int型
	DWORD iBaseAddr = _ttoi(csBaseAddr);

	HMODULE hDll = GetModuleHandle(csModulePath);
	if (hDll == INVALID_HANDLE_VALUE || hDll == NULL)
	{
		return;
	}
	BOOL bRet = FreeLibrary(hDll);
	if (bRet == FALSE)
	{
		CloseHandle(hDll);
		return;
	}

	// 删除完毕后刷新一下界面
	m_lstModule.DeleteAllItems();
	InitModuleList();
}

// 弹出右键菜单
void CModuleDialog::OnNMRClickListModule(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint cPT;
	GetCursorPos(&cPT);
	HMENU hMenu = LoadMenu(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MENU_MODULE));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);

	TrackPopupMenu(hSubMenu, TPM_CENTERALIGN, cPT.x, cPT.y, 0, m_hWnd, NULL);
	*pResult = 0;
}
