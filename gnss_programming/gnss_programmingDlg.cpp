
// gnss_programmingDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "gnss_programming.h"
#include "gnss_programmingDlg.h"
#include "afxdialogex.h"
#include "spacetime_transform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CgnssprogrammingDlg 对话框



CgnssprogrammingDlg::CgnssprogrammingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GNSS_PROGRAMMING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgnssprogrammingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_origin, combo_origin);
	DDX_Control(pDX, IDC_timeOutput, time_input);
	DDX_Control(pDX, IDC_timeInput, time_output);
	DDX_Control(pDX, IDC_COMBO2_target, combo_target);
	DDX_Control(pDX, IDC_EDIT2, space_origin_input);
	DDX_Control(pDX, IDC_EDIT1, space_output);
	DDX_Control(pDX, IDC_combo_space_target, space_tyep_target);
	DDX_Control(pDX, IDC_combo_space_origin, space_type_origin);
}

BEGIN_MESSAGE_MAP(CgnssprogrammingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(timeConfirm, &CgnssprogrammingDlg::OnBnClickedtimeconfirm)
	ON_CBN_SELCHANGE(IDC_COMBO_origin, &CgnssprogrammingDlg::OnCbnSelchangeComboorigin)
	ON_CBN_SELCHANGE(IDC_COMBO2_target, &CgnssprogrammingDlg::OnCbnSelchangeCombo2target)
	ON_CBN_SELCHANGE(IDC_combo_space_origin, &CgnssprogrammingDlg::OnCbnSelchangecombospaceorigin)
	ON_CBN_SELCHANGE(IDC_combo_space_target, &CgnssprogrammingDlg::OnCbnSelchangecombospacetarget)
	ON_BN_CLICKED(space_confirm, &CgnssprogrammingDlg::OnBnClickedconfirm)
END_MESSAGE_MAP()


// CgnssprogrammingDlg 消息处理程序

BOOL CgnssprogrammingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CgnssprogrammingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CgnssprogrammingDlg::OnPaint()
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
HCURSOR CgnssprogrammingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}








void CgnssprogrammingDlg::OnBnClickedtimeconfirm() {
	// TODO: 在此添加控件通知处理程序代码
	// 解析输入
	CString strInput;
	time_input.GetWindowText(strInput);
	std::string inputStr = CT2A(strInput.GetString());
	timeTransformer::instance().parserInput(inputStr);

	// 时间转换
	timeTransformer::instance().run();

	// 输出
	CString strOutput = CString(timeTransformer::instance().getTarget().c_str());
	time_output.SetWindowText(strOutput);
}


void CgnssprogrammingDlg::OnCbnSelchangeComboorigin()
{
	// TODO
	int origin = combo_origin.GetCurSel();
	timeTransformer::instance().setOrigin((timeTransformer::TimeType)origin);

}

void CgnssprogrammingDlg::OnCbnSelchangeCombo2target()
{
	// TODO: 在此添加控件通知处理程序代码
	int origin = combo_target.GetCurSel();
	timeTransformer::instance().setTarget((timeTransformer::TimeType)origin);
}

void CgnssprogrammingDlg::OnCbnSelchangecombospaceorigin()
{
	// TODO: 在此添加控件通知处理程序代码
	int origin = space_type_origin.GetCurSel();
	spaceTransformer::instance().setOrigin((spaceTransformer::spaceType)origin);
}

void CgnssprogrammingDlg::OnCbnSelchangecombospacetarget()
{
	// TODO: 在此添加控件通知处理程序代码
	int target = space_tyep_target.GetCurSel();
	spaceTransformer::instance().setTarget((spaceTransformer::spaceType)target);
}

void CgnssprogrammingDlg::OnBnClickedconfirm()
{
	// TODO: 在此添加控件通知处理程序代码
	// 解析输入
	CString strInput;
	space_origin_input.GetWindowText(strInput);
	std::string inputStr = CT2A(strInput.GetString());
	spaceTransformer::instance().parserInput(inputStr);
	// 空间转换
	spaceTransformer::instance().run();
	// 输出
	CString strOutput = CString(spaceTransformer::instance().getTarget().c_str());
	space_output.SetWindowText(strOutput);
}
