
// ProcessMonitorDlg.h: 头文件
//

#pragma once
#include "ThreadMonitorDlg.h"
#include "ModuleDialog.h"
#include "SoftDlg.h"
#include "WindowMonitor.h"

// CProcessMonitorDlg 对话框
class CProcessMonitorDlg : public CDialogEx
{
// 构造
public:
	CProcessMonitorDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESSMONITOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	// 用于保存对话框窗口的大小,方便之后调整大小(重绘窗口)
	CRect m_cDlgRect;

	// 存储要注入的DLL文件的路径
	LPVOID m_lpAddr;
	// 确定要注入的DLL的路径
	const TCHAR* m_szPath = L"E:\\Code\\lib\\Dll1\\Debug\\Dll1.dll";

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 重绘,更新控件的大小和位置
	void RePaint(UINT uID, int iLastWidth, int iNowWidth, int iLastHeight, int iNowHeight);
	// 进程列表
	CListCtrl m_lstProcess;
	// 初始化列表控件并显示
	void InitProcessList();
	// 查看该进程的线程
	afx_msg void OnCheckThread();
	// 弹出右键菜单
	afx_msg void OnRclickListProcess(NMHDR* pNMHDR, LRESULT* pResult);
	// 杀死进程
	afx_msg void OnKillProcess();
	// 刷新进程列表
	afx_msg void OnFlushProcess();
	// 打开模块列表
	afx_msg void OnOpenModule();
	// 通过远程创建线程的方式注入到进程中
	afx_msg void OnInjectThread();
	void OnInjectThread(DWORD dwPID, TCHAR* szPath);
	// 卸载注入的线程
	afx_msg void OnReleaseThread();
	// 调整窗口大小时进行重绘
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// 弹出已安装软件列表
	afx_msg void OnBnClickedButtonAllSoft();
	// 弹出窗口列表
	afx_msg void OnBnClickedButtonWindow();
};
