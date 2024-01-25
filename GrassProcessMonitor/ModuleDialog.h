#pragma once


// CModuleDialog 对话框
extern "C" DWORD g_dwProcessId;
class CModuleDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CModuleDialog)

public:
	CModuleDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CModuleDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MODULE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 模块列表
	CListCtrl m_lstModule;
	virtual BOOL OnInitDialog();
	// 填充模块列表
	void InitModuleList();
	// 右键卸载指定模块
	afx_msg void OnReleaseModule();
	// 模块列表中弹出右键菜单
	afx_msg void OnNMRClickListModule(NMHDR* pNMHDR, LRESULT* pResult);
};
