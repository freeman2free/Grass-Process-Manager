#pragma once

extern "C" DWORD g_dwProcessId;
// ThreadMonitorDlg 对话框

class ThreadMonitorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ThreadMonitorDlg)

public:
	ThreadMonitorDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ThreadMonitorDlg();

	// 用于保存对话框窗口的大小,方便之后调整大小(重绘窗口)
	CRect m_cDlgRect;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_THREAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	// 线程列表控件
	CListCtrl m_lstThread;

	void InitThreadList();
	// 挂起线程
	afx_msg void OnSuspendThread();
	// 恢复线程
	afx_msg void OnResumeThre();
	// 结束线程
	afx_msg void OnKillThread();
	// 弹出线程的右键菜单
	afx_msg void OnRclickListThread(NMHDR* pNMHDR, LRESULT* pResult);
};
