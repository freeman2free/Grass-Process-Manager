#pragma once


// CSoftDlg 对话框

class CSoftDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSoftDlg)

public:
	CSoftDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSoftDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ALLSOFT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 软件信息列表控件
	CListCtrl m_lstSoft;
	// 子键
	CString m_csKey;
	void InitSoftList();

	virtual BOOL OnInitDialog();
	// 执行卸载
	afx_msg void OnBnClickedButtonUnistall();
};
