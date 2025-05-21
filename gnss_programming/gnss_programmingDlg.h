
// gnss_programmingDlg.h: 头文件
//

#pragma once



// CgnssprogrammingDlg 对话框
class CgnssprogrammingDlg : public CDialogEx
{
// 构造
public:
	CgnssprogrammingDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GNSS_PROGRAMMING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP();

public:
	afx_msg void OnBnClickedtimeconfirm();
	afx_msg void OnCbnSelchangeComboorigin();
	CComboBox combo_origin;

	afx_msg void OnCbnSelchangeCombo2target();
	CEdit time_input;
	CEdit time_output;
	CComboBox combo_target;
	CEdit space_origin_input;
	CEdit space_output;
	CComboBox space_tyep_target;
	CComboBox space_type_origin;
	afx_msg void OnCbnSelchangecombospaceorigin();
	afx_msg void OnCbnSelchangecombospacetarget();
	afx_msg void OnBnClickedconfirm();
	afx_msg void OnBnClickedNfilebutton();
	afx_msg void OnBnClickedOfilebutton();
};
