#pragma once


// CWindowMonitor 对话框

class CWindowMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CWindowMonitor)

public:
	CWindowMonitor(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWindowMonitor();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WINDOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	// 加载窗口列表
	VOID InitWindowList();
	// 加载窗口的图标
	VOID InsertImage(CString lPNamePath);
	// 弹出窗口右键菜单
	afx_msg void OnNMRClickListWindow(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示窗口
	afx_msg void OnMenuwindowShowwindow();
	// 隐藏窗口
	afx_msg void OnMenuwindowHidewindow();
	// 最大化窗口
	afx_msg void OnMenuwindowMaximize();
	// 最小化窗口
	afx_msg void OnMenuwindowMinimize();
	// 结束指定窗口进程
	afx_msg void OnMenuwindowKillprocess();
	// 刷新已有窗口列表
	afx_msg void OnMenuwindowRefresh();
public:
	// 窗口列表
	CListCtrl m_lstWindow;
	// 窗口图标
	CImageList m_lstImage;


};
