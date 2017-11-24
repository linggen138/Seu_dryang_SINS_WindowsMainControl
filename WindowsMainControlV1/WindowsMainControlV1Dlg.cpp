


// WindowsMainControlV1Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WindowsMainControlV1.h"
#include "WindowsMainControlV1Dlg.h"
#include "afxdialogex.h"
#include "BoardCard.h"
#include "DataConvert.h"
#include "CoarseAlign.h"
#include "FineAlign.h"
#include "Navigation.h"
#include "Calibration.h"
#include "ReadSimuDlg.h"
#include "SimuDataDlg.h"
#include <WinSock2.h>
#include <MMSystem.h>
#include "afxwin.h"
#pragma comment(lib,"WS2_32.lib")  
#pragma comment(lib, "winmm.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region VarDef 
//*******�ǶԻ������������*******//
bool blrxreset;//�忨ͨ����λ����
//�ļ��������
FILE* fid_FOSN;
FILE* fid_PHINS;
FILE* fid_gps;
FILE* fid_zt;
FILE* fid_Cal;
bool is_SingleFile;//Ϊ1��ʾ���ļ����棬2��ʾ���ļ�����
bool saveStart;
CWinThread* phins_Thread;
//ˢ��������
int refresh_time;
bool is_UpdateData;//ˢ�¹رպͿ���
//��ʱ����
int save_time;//���涨ʱ����
int m_save_time;//��ʾ��ʱ����������ʱ
int temp_cnt_s;//���ڼ�ȥ�㵹��ʱ�Ѿ���ȥ��ϵͳʱ��
bool is_timeSave;

//��ʼλ��,��ʾ�ã���λΪ��
double initial_latitude, initial_longitude, initial_height;
//ģʽѡ��
int CoarseModeNum;
int FineModeNum;
int NaviModeNum;
int TestModeNum;
int PureINSModeNum;
bool isStartCalOk;
//��־�ж�
bool is_cardReceive;
bool is_startCal;
bool is_cardReset;//�忨�ؿ�
//bool is_InitNaviVar;//����������ʼ����ɱ�־
bool is_data_used;//���������Ƿ��Ѿ�������ʹ��
bool is_start_phins;
bool mquit;//�����˳��������߳�ѭ���˳�����

//FOSN����
BYTE bufFOSN[70];
//gps����
BYTE BufGPS[180];
////ת̨���ݽ���	
BYTE BufZT[50] = { 0 };
int m_cnt_err;//֡��

//��������
int m_cnt_s;
//PHINS����
char m_Recv_PHINS_Buff[1024];

//��������ģʽ
READSIMULATION RS_para;
//��ý�嶨ʱ��
void CALLBACK TimeDalay(UINT uID, UINT uMsg, DWORD dwUsers, DWORD dw1, DWORD dw2);
UINT TimerRes = 5;
UINT TimerID;
bool isCreateTimer = false;//������ʱ���¼�
int m_PRecNum;//��¼��¼ȡ��������
int datanavinum = 0;
bool temp_test = true;
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
#pragma endregion VarDef 
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CWindowsMainControlV1Dlg �Ի���
CWindowsMainControlV1Dlg::CWindowsMainControlV1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWindowsMainControlV1Dlg::IDD, pParent)
	, edit_data_f(100)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	initial_latitude = 32.05731;
	initial_longitude = 118.786355;
	initial_height = 31;
	Init_CardVar();

	is_SingleFile = 1;

	CoarseModeNum = 0;
	FineModeNum = 0;
	NaviModeNum = 0;
	PureINSModeNum = 0;
	TestModeNum = 0;

	sysc.coarse_time = 80;
	sysc.fine_level = 120;
	sysc.fine_azimuth = 400;
	sysc.algn_time = 600;

	refresh_time = 1000;
	is_UpdateData = true;
	is_cardReset = 1;
	mquit = 0;
	init_var();
}
void CWindowsMainControlV1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CoarseAlignMode, m_CoarseAignMode);
	DDX_Control(pDX, IDC_COMBO_FineAlignMode, m_FineAignMode);
	DDX_Control(pDX, IDC_COMBO_NavigationMode, m_NaviMode);
	DDX_Control(pDX, IDC_COMBO_HeightMode, m_HeightMode);
	DDX_Text(pDX, IDC_COARSE_TIME, sysc.coarse_time);
	DDX_Text(pDX, IDC_FINE_LEVEL, sysc.fine_level);
	DDX_Text(pDX, IDC_FINE_AZIMUTH, sysc.fine_azimuth);
	DDX_Text(pDX, IDC_FINE_TIME, sysc.algn_time);
	DDX_Text(pDX, IDC_FOSN_UTC1, fosn.s);
	DDX_Text(pDX, IDC_FOSN_UTC2, fosn.ms);
	DDX_Text(pDX, IDC_FOSN_GRYO_X, IMUout.gyro_b[0]);
	DDX_Text(pDX, IDC_FOSN_GRYO_Y, IMUout.gyro_b[1]);
	DDX_Text(pDX, IDC_FOSN_GRYO_Z, IMUout.gyro_b[2]);
	DDX_Text(pDX, IDC_FOSN_ACCE_X, IMUout.acce_b[0]);
	DDX_Text(pDX, IDC_FOSN_ACCE_Y, IMUout.acce_b[1]);
	DDX_Text(pDX, IDC_FOSN_ACCE_Z, IMUout.acce_b[2]);
	DDX_Text(pDX, IDC_FOSN_Latitude, fosn.pos[0]);
	DDX_Text(pDX, IDC_FOSN_Longitude, fosn.pos[1]);
	DDX_Text(pDX, IDC_FOSN_Height, fosn.pos[2]);
	DDX_Text(pDX, IDC_FOSN_V_X, fosn.vel[0]);
	DDX_Text(pDX, IDC_FOSN_V_Y, fosn.vel[1]);
	DDX_Text(pDX, IDC_FOSN_V_Z, fosn.vel[2]);
	DDX_Text(pDX, IDC_FOSN_Angle_X, fosn.ang[0]);
	DDX_Text(pDX, IDC_FOSN_Angle_Y, fosn.ang[1]);
	DDX_Text(pDX, IDC_FOSN_Angle_Z, fosn.ang[2]);
	DDX_Text(pDX, IDC_FOSNREC_CNT, fosn.recnum);
	DDX_Text(pDX, IDC_ZT_Frame, ZT.Frame);
	DDX_Text(pDX, IDC_ZT_Count, ZT.cnt);
	DDX_Text(pDX, IDC_ZT_X, ZT.ang[0]);
	DDX_Text(pDX, IDC_ZT_Y, ZT.ang[1]);
	DDX_Text(pDX, IDC_ZT_Z, ZT.ang[2]);
	DDX_Text(pDX, IDC_CNT_ERR, m_cnt_err);
	DDX_Text(pDX, IDC_GPS_Latitude, gps.pos[0]);
	DDX_Text(pDX, IDC_GPS_Longitude, gps.pos[1]);
	DDX_Text(pDX, IDC_GPS_Height, gps.pos[2]);
	DDX_Text(pDX, IDC_GPS_Time, gps.time);
	DDX_Text(pDX, IDC_GPS_FLAG, gps.flag);
	DDX_Text(pDX, IDC_GPS_CNT, gps.cnt);
	DDX_Text(pDX, IDC_Refresh_Time, refresh_time);
	DDX_Text(pDX, IDC_Cal_Angle_X, INScal.ang[0]);
	DDX_Text(pDX, IDC_Cal_Angle_Y, INScal.ang[1]);
	DDX_Text(pDX, IDC_Cal_Angle_Z, INScal.ang[2]);
	DDX_Text(pDX, IDC_Cal_V_X, INScal.vel[0]);
	DDX_Text(pDX, IDC_Cal_V_Y, INScal.vel[1]);
	DDX_Text(pDX, IDC_Cal_V_Z, INScal.vel[2]);
	DDX_Text(pDX, IDC_Cal_Latitude, INScal.pos[0]);
	DDX_Text(pDX, IDC_Cal_Longitude, INScal.pos[1]);
	DDX_Text(pDX, IDC_Cal_HEIGHT, INScal.pos[2]);
	DDX_Text(pDX, IDC_ERR_Angle_X, INScal.err_ang[0]);
	DDX_Text(pDX, IDC_ERR_Angle_Y, INScal.err_ang[1]);
	DDX_Text(pDX, IDC_ERR_Angle_Z, INScal.err_ang[2]);
	DDX_Text(pDX, IDC_ERR_LocationX, INScal.err_pos[0]);
	DDX_Text(pDX, IDC_ERR_LocationY, INScal.err_pos[1]);
	DDX_Text(pDX, IDC_ERR_Location, INScal.err_pos[2]);
	DDX_Text(pDX, IDC_LATITUDE0, initial_latitude);
	DDX_Text(pDX, IDC_LONGITUDE0, initial_longitude);
	DDX_Text(pDX, IDC_HEIGHT0, initial_height);
	DDX_Text(pDX, IDC_RUNNING_TIME, m_cnt_s);
	DDX_Text(pDX, IDC_PHINS_Agnle_X, phins.ang[0]);
	DDX_Text(pDX, IDC_PHINS_Agnle_Y, phins.ang[1]);
	DDX_Text(pDX, IDC_PHINS_Agnle_Z, phins.ang[2]);
	DDX_Text(pDX, IDC_PHINS_CNT, phins.cnt);
	DDX_Text(pDX, IDC_PHINS_Height, phins.pos[2]);
	DDX_Text(pDX, IDC_PHINS_Latitude, phins.pos[0]);
	DDX_Text(pDX, IDC_PHINS_Longitude, phins.pos[1]);
	DDX_Text(pDX, IDC_PHINS_UTC, phins.utc);
	DDX_Text(pDX, IDC_PHINS_V_X, phins.vel[0]);
	DDX_Text(pDX, IDC_PHINS_V_Y, phins.vel[1]);
	DDX_Text(pDX, IDC_PHINS_V_Z, phins.vel[2]);
	DDX_Text(pDX, IDC_SAVETIME, m_save_time);
	DDX_Text(pDX, IDC_Kal_gyro_Bias_1, infor.gyro_bias_esti[0]);
	DDX_Text(pDX, IDC_Kal_gyro_Bias_2, infor.gyro_bias_esti[1]);
	DDX_Text(pDX, IDC_Kal_gyro_Bias_3, infor.gyro_bias_esti[2]);
	DDX_Text(pDX, IDC_Kal_acce_Bias_1, infor.acce_bias_esti[0]);
	DDX_Text(pDX, IDC_Kal_acce_Bias_2, infor.acce_bias_esti[1]);
	DDX_Text(pDX, IDC_Kal_acce_Bias_3, infor.acce_bias_esti[2]);
	DDX_Control(pDX, IDC_TEXT_MODE, m_TestMode);
	DDX_Text(pDX, IDC_STATE, sysc.state);
	DDX_Text(pDX, IDC_Data_F, edit_data_f);
}
BEGIN_MESSAGE_MAP(CWindowsMainControlV1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(WM_UPDATEDATA, updateData)
	ON_MESSAGE(WM_TIMESAVE, saveCancel)

	ON_BN_CLICKED(IDC_BTN_StartCard, &CWindowsMainControlV1Dlg::OnBnClickedBtnStartcard)
	ON_BN_CLICKED(IDC_BTN_StartPhins, &CWindowsMainControlV1Dlg::OnBnClickedBtnStartphins)
	ON_BN_CLICKED(IDC_BTN_StartCal, &CWindowsMainControlV1Dlg::OnBnClickedBtnStartcal)
	ON_BN_CLICKED(IDC_BTN_SAVEPATH, &CWindowsMainControlV1Dlg::OnBnClickedBtnSavepath)
	ON_BN_CLICKED(IDC_BTN_SAVEALL, &CWindowsMainControlV1Dlg::OnBnClickedBtnSaveall)
	ON_BN_CLICKED(IDC_BTN_SingleFile, &CWindowsMainControlV1Dlg::OnBnClickedBtnSinglefile)
	ON_BN_CLICKED(IDC_BTN_TimeSet, &CWindowsMainControlV1Dlg::OnBnClickedBtnTimeset)
	ON_BN_CLICKED(IDC_BTN_ReadMe, &CWindowsMainControlV1Dlg::OnBnClickedBtnReadme)
	ON_BN_CLICKED(IDC_BTN_SAVE_HELP, &CWindowsMainControlV1Dlg::OnBnClickedBtnSaveHelp)
	ON_BN_CLICKED(IDCANCEL, &CWindowsMainControlV1Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_Reset, &CWindowsMainControlV1Dlg::OnBnClickedBtnReset)

	ON_BN_CLICKED(IDC_BTN_SAVETIME, &CWindowsMainControlV1Dlg::OnBnClickedBtnSavetime)
	ON_BN_CLICKED(IDC_BTN_STOPREFRESH, &CWindowsMainControlV1Dlg::OnBnClickedBtnStoprefresh)
	ON_BN_CLICKED(IDC_BTN_Calibration, &CWindowsMainControlV1Dlg::OnBnClickedBtnCalibration)
	ON_CBN_SELCHANGE(IDC_TEXT_MODE, &CWindowsMainControlV1Dlg::OnCbnSelchangeTextMode)
	ON_CBN_SELCHANGE(IDC_COMBO_FineAlignMode, &CWindowsMainControlV1Dlg::OnCbnSelchangeComboFinealignmode)
END_MESSAGE_MAP()

// CWindowsMainControlV1Dlg ��Ϣ��������

BOOL CWindowsMainControlV1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵������ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ����Ӷ���ĳ�ʼ������
	ghWnd = this->ghWnd;
	init_Combo();
	init_mainmode();
	Init_Net();
	timeBeginPeriod(TimerRes);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}
void CWindowsMainControlV1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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
// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�
void CWindowsMainControlV1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CWindowsMainControlV1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/////��������***********������Ӧ****************��������//////
#pragma region ClBtn 
void CWindowsMainControlV1Dlg::OnBnClickedBtnStartcard()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

	is_cardReceive = !is_cardReceive;
	CWnd *h1;
	h1 = GetDlgItem(IDC_BTN_StartCard);		//ָ��ؼ���caption
	CString str;
	if (is_cardReceive)
	{
		NaviModeNum = m_NaviMode.GetCurSel();
		TestModeNum = m_TestMode.GetCurSel();
		switch (TestModeNum)
		{
		case 1:OnBnClickedBtnStartphins(); break;
		case 2: case 3: sysc.state = _T("��¼��ģʽ"); break;
		default:break;
		}
		Init_Card(ghWnd, NaviModeNum, 0x0f);
		Sio_Rx_ResetFIFO(hCard, 0);//��λ�忨
		Sio_Rx_ResetFIFO(hCard, 1);
		Sio_Rx_ResetFIFO(hCard, 2);
		CWinThread* Card_Thread;
		THREADPARAM  *phWndParam = new THREADPARAM;
		phWndParam->hwnd = m_hWnd;
		if (!(Card_Thread = AfxBeginThread(CardRec, (LPVOID)phWndParam)))
			return;

		UpdateData(true);
		str = _T("�忨����ֹͣ");
		h1->SetWindowText(str);
		GetDlgItem(IDC_BTN_SAVEPATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_StartCal)->EnableWindow(TRUE);
		m_TestMode.EnableWindow(FALSE);
		m_NaviMode.EnableWindow(FALSE);

	}
	else
	{
		str = _T("�忨����");
		h1->SetWindowText(str);
		GetDlgItem(IDC_BTN_SAVEPATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_StartCal)->EnableWindow(FALSE);
		m_TestMode.EnableWindow(TRUE);
		m_NaviMode.EnableWindow(TRUE);

	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnStartphins()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	is_start_phins = !is_start_phins;
	CWnd *h1;
	h1 = GetDlgItem(IDC_BTN_StartPhins);		//ָ��ؼ���caption
	CString str;
	if (is_start_phins)
	{		
		SOCKETPARAM  *pRecvParam = new SOCKETPARAM;
		pRecvParam->hwnd = m_hWnd;
		pRecvParam->sock = m_socketPHINSDataRec;
		if (!(phins_Thread = AfxBeginThread(PHINSThread, (LPVOID)pRecvParam)))
			return;
		str = _T("PHINS����ֹͣ");
		h1->SetWindowText(str);
	}
	else
	{
		str = _T("PHINS����");
		h1->SetWindowText(str);
		phins_Thread->PostThreadMessageW(WM_QUIT, NULL, NULL); 
	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnStartcal()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	TestModeNum = m_TestMode.GetCurSel();
	is_startCal = !is_startCal;
	CWnd *h1;
	h1 = GetDlgItem(IDC_BTN_StartCal);		//ָ��ؼ���caption
	CString str;
	if (is_startCal)
	{
		UpdateData(true);

		//������ʼ��
		init_basicnavi();
		sysc.Fs=edit_data_f;
		sysc.Ts = 1.0 / sysc.Fs;	
		//����ģʽ���г�ʼ��
		TestModeNum = m_TestMode.GetCurSel();
		CoarseModeNum = m_CoarseAignMode.GetCurSel();
		FineModeNum = m_FineAignMode.GetCurSel();
		PureINSModeNum = m_HeightMode.GetCurSel();
		NaviModeNum = m_NaviMode.GetCurSel();
		//��ʼλ�ò�������		
		CalVarInit(TestModeNum);

		if (5 == FineModeNum)
			CoarseModeNum=0;		

		if (0 == CoarseModeNum)
		{
			sysc.f_coarse_over = 1;
			sysc.coarse_time = 0;
		}
		else
		{
			init_coarsealign();
			sysc.f_coarse_over = 0;
		}

		if (0 == FineModeNum)
		{
			sysc.f_fine_over = 1;
			sysc.algn_time = sysc.coarse_time;
		}
		else
		{
			sysc.f_fine_over = 0;
			switch (FineModeNum)
			{
			case FINE_CMPS:
				init_cmp(); //��ʼ���޾�����	
				if (FineModeNum == 1)sysc.algn_time = sysc.coarse_time + sysc.fine_level + sysc.fine_azimuth;//��ˮƽ�ͺ���ʱ��������õ�����׼��ʱ�䣬�ټ��ϴֶ�׼��ʱ�䡣
				break;	
			case FINE_Yucia: Kal_Init_P_15(YA_POS); break;
			case FINE_ADRC: init_adrc(); break;
			default: break;
			}
		}

		sysc.f_navi_over = 0;
		switch (NaviModeNum)
		{
		case NAVI_HAISHI_BASIC:case NAVI_HAISHI_JZ:kalinitial(); break;
		case NAVI_SG:Kal_Init_P_15(YA_POS); break;
		case NAVI_VEL:Kal_Init_P_15(YA_VEL); break;
		case NAVI_VELANDAZ:Kal_Init_P_15(YA_VELANDAZ); break;
		default: break;
		}
		if (RS_para.delay5ms == 1)
		{
			isCreateTimer = true;
			TimerID = timeSetEvent(
				5,
				TimerRes,//��С�ֱ���
				TimeDalay,//�ص�����
				(DWORD)m_hWnd,//����
				TIME_PERIODIC);//�����Դ���
		}
		GetDlgItem(IDC_TEXT_MODE)->EnableWindow(FALSE);
		m_CoarseAignMode.EnableWindow(FALSE);
		m_FineAignMode.EnableWindow(FALSE);
		m_HeightMode.EnableWindow(FALSE);
		m_NaviMode.EnableWindow(FALSE);
		str = _T("����ֹͣ");
		h1->SetWindowText(str);
		isStartCalOk = true;

	}
	else
	{
		isStartCalOk = false;
		if (isCreateTimer == true)
		{
			timeKillEvent(TimerID);
			isCreateTimer = false;
		}
		sysc.f_coarse_over = 1;
		sysc.f_fine_over = 1;
		sysc.f_navi_over = 1;
		sysc.state = _T("�޽��㹤��");
		str = _T("���㿪ʼ");
		h1->SetWindowText(str);
		GetDlgItem(IDC_TEXT_MODE)->EnableWindow(TRUE);
		m_CoarseAignMode.EnableWindow(TRUE);
		m_FineAignMode.EnableWindow(TRUE);
		m_HeightMode.EnableWindow(TRUE);
		m_NaviMode.EnableWindow(TRUE);
		RS_para.reset();
		if (saveStart)
			OnBnClickedBtnSaveall();
	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnSavepath()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������	
	if (fid_zt != NULL) fclose(fid_zt);
	if (fid_Cal != NULL) fclose(fid_Cal);
	if (fid_FOSN != NULL) fclose(fid_FOSN);
	if (fid_gps != NULL) fclose(fid_gps);
	if (fid_PHINS != NULL) fclose(fid_PHINS);

	CString Filename = GetDirectory();
	if (Filename == _T(""))
		return;
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	CString Str1;
	Str1.Format(_T("%ld_%ld"), systime.wHour, systime.wMinute);//������ð�� %ld:%ld �����ļ������ķ������⣬ð�ż�������ַ���������
	if (!is_SingleFile)
	{
		CString DocName;
		if (TestModeNum != 1)
		{
			DocName = Filename + _T("zt") + Str1;
			fopen_s(&fid_zt, DocName + ".txt", "w+");
		}

		DocName = Filename + _T("cal") + Str1;
		fopen_s(&fid_Cal, DocName + ".txt", "w+");

		DocName = Filename + _T("fosn") + Str1;
		fopen_s(&fid_FOSN, DocName + ".txt", "w+");

		DocName = Filename + _T("gps") + Str1;
		fopen_s(&fid_gps, DocName + ".txt", "w+");

		DocName = Filename + _T("phins") + Str1;
		fopen_s(&fid_PHINS, DocName + ".txt", "w+");
	}
	if (is_SingleFile)
	{
		CString DocName;
		DocName = Filename + _T("data") + Str1;
		fopen_s(&fid_Cal, DocName + ".txt", "w+");
		fid_FOSN = NULL;
		fid_PHINS = NULL;
		fid_gps = NULL;
		fid_zt = NULL;
	}
	GetDlgItem(IDC_BTN_SAVEALL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_SingleFile)->EnableWindow(FALSE);
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnSaveall()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	saveStart = !saveStart;
	CWnd *h1;
	h1 = GetDlgItem(IDC_BTN_SAVEALL);		//ָ��ؼ���caption
	CString str;
	if (saveStart)
	{
		GetDlgItem(IDC_BTN_SAVEPATH)->EnableWindow(FALSE);
		str = _T("����ֹͣ");
		h1->SetWindowText(str);
	}
	else
	{
		if (fid_zt != NULL) fclose(fid_zt);
		if (fid_Cal != NULL) fclose(fid_Cal);
		if (fid_FOSN != NULL) fclose(fid_FOSN);
		if (fid_gps != NULL) fclose(fid_gps);
		if (fid_PHINS != NULL) fclose(fid_PHINS);
		if (RS_para.RdataFilefid != NULL) fclose(RS_para.RdataFilefid);

		GetDlgItem(IDC_BTN_SAVEPATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_SingleFile)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_SAVEALL)->EnableWindow(FALSE);
		str = _T("������ʼ");
		h1->SetWindowText(str);
	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnSinglefile()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	is_SingleFile = !is_SingleFile;
	CWnd *h1;
	h1 = GetDlgItem(IDC_BTN_SingleFile);		//ָ��ؼ���caption
	CString str;
	if (is_SingleFile)
	{
		str = _T("��ǰΪ���ļ�����");
		h1->SetWindowText(str);
	}
	else
	{
		str = _T("��ǰΪ���ļ�����");
		h1->SetWindowText(str);
	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnTimeset()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	refresh_time = GetDlgItemInt(IDC_Refresh_Time);
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnReadme()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	CString c;
	c = "!!!�޸ĸ��ֲ���ǰ������ѡ��ˢ����ͣ!!!";
	MessageBox(c, _T("������Windows����ƽ̨�������˵��������"), MB_OK);

}
void CWindowsMainControlV1Dlg::OnBnClickedBtnSaveHelp()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	CString c;
	/*���ļ������ʽ��\n\
		ZT����Ϊ��1 ֡�ţ�2 ZT���ռ�����3FOSN���ռ�����3~6ת̨������̬�ǡ�\n\
		cal���ݸ�ʽΪ��1 FOSN���ռ�����2~4��̬��5~7�ٶȣ�8~10λ�ã�11~13��̬��14~16λ����\n\
		fosn���ݸ�ʽΪ��1 FOSN���ռ�����2 FOSN����ʱ�䣬3~5���ݣ�6~8�ӱ���9~11��̬��12~14�ٶȣ�15~17λ��\n\
		gps���ݸ�ʽΪ��1 FOSN���ռ�����2 GPSʱ�䣬3~5 ��γ��\n\
		phins���ݸ�ʽΪ��1 PHINS���ռ�����2 FOSN���ռ�����3 PHINSʱ�䣬4~6��̬��7~9�ٶȣ�10~12λ��\n\n\*/
	c = "��¼��ǰѡ���ļ�ģʽ���߶��ļ�ģʽ�����ļ�ģʽ�Ѹ�ģ�����ݵ���¼ȡ\n\
��ѡ�񱣴�·��Ϊ�ļ���·����֮���Զ�����ϵͳʱ���Զ�����¼���ļ�\n\
����ת̨�ͳ���ģʽ�£�ֻ�����ݽ��㿪ʼ�Ż�¼�������Է����ȵ�¼���ٵ����\n\n\
�����ݵ�λΪ��/s,��̬��λΪ��\n\
���ļ������ʽ��\n\
����¼��ģʽ������PHINS��\n\
1~3   ¼��ͳ�ƣ�ת̨���ݺţ�fosnʱ��\n\
4~9   ���ݼӱ�����\n\
10~12 ת̨��̬\n\
13~15 FOSN��̬\n\n\
������ģʽ��\n\
1~5   ¼��ͳ�ƣ�ת̨/�๦�ܰ�֡��(����)��fosnʱ��,gpsʱ�䣬phinsʱ��\n\
6~11  ���ݼӱ�\n\
12~14 ������̬\n\
15~17 �����ٶ�\n\
18~20 ����λ��\n\
21~23 PHINS��̬\n\
24~26 PHINS�ٶ�\n\
27~29 PHINSλ��\n\
30~35 GPSλ�ã��ٶȣ�û�У�\n\
36~38 FOSN��̬\n\
39~41 FOSN�ٶ�\n\
42~44 FOSNλ�� \n\nby Dr.Yang";
	MessageBox(c, _T("������Windows����ƽ̨�������¼����ʽ˵��������"), MB_OK);
}
void CWindowsMainControlV1Dlg::OnBnClickedCancel()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

	closesocket(m_socketPHINSDataRec);
	WSACleanup();
	if (is_startCal)OnBnClickedBtnStartcal();
	if (fid_zt != NULL) fclose(fid_zt);
	if (fid_Cal != NULL) fclose(fid_Cal);
	if (fid_FOSN != NULL) fclose(fid_FOSN);
	if (fid_gps != NULL) fclose(fid_gps);
	if (fid_PHINS != NULL) fclose(fid_PHINS);
	if (RS_para.RdataFilefid != NULL) fclose(RS_para.RdataFilefid);
	timeEndPeriod(TimerRes);
	CDialogEx::OnCancel();
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnReset()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

	//1.�ر��ļ�ϵͳ
	if (saveStart)OnBnClickedBtnSaveall();

	//2.����������ֹ
	if (is_startCal) OnBnClickedBtnStartcal();

	//3.���ݽ���ֹͣ
	if (is_cardReceive) OnBnClickedBtnStartcard();
	if (is_start_phins) OnBnClickedBtnStartphins();

	//4.������ʼ��
	init_var();
	phins.reset();
	fosn.reset();
	gps.reset();
	sysc.reset();

}
void CWindowsMainControlV1Dlg::OnBnClickedBtnSavetime()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

	CWnd *h1;
	CString str;
	h1 = GetDlgItem(IDC_BTN_SAVETIME);		//ָ��ؼ���caption
	save_time = GetDlgItemInt(IDC_SAVETIME);
	m_save_time = save_time;
	is_timeSave = !is_timeSave;
	temp_cnt_s = m_cnt_s;//��¼���ʱ���Ѿ���ȥ��ʱ��
	if (is_timeSave)
	{
		str = _T("�رն�ʱ¼��");
		h1->SetWindowText(str);
		GetDlgItem(IDC_SAVETIME)->EnableWindow(FALSE);
	}
	else
	{
		str = _T("������ʱ¼��");
		h1->SetWindowText(str);
		GetDlgItem(IDC_SAVETIME)->EnableWindow(TRUE);

	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnStoprefresh()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	is_UpdateData = !is_UpdateData;
	CWnd *h1;
	h1 = GetDlgItem(IDC_BTN_STOPREFRESH);		//ָ��ؼ���caption
	CString str;
	if (is_UpdateData)
	{
		str = _T("ˢ����ͣ");
		h1->SetWindowText(str);

	}
	else
	{
		str = _T("ˢ�¼���");
		h1->SetWindowText(str);

	}
}
void CWindowsMainControlV1Dlg::OnBnClickedBtnCalibration()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	Calibration cali;
	cali.err_input = false;
	INT_PTR nRes;
	if (is_UpdateData)
		OnBnClickedBtnStoprefresh();
	nRes = cali.DoModal();
	if (IDOK == nRes)
	{
		calipara = cali.calipmt;
		amamul(3, 1, calipara.bias_acce, calipara.bias_acce, 9.78*0.000001);	// *ug	
		amamul(3, 1, calipara.bias_gyro, calipara.bias_gyro, D2R / 3600);         // ��/h -> rad/s
		calipara.Eang2mat();
		if (!is_UpdateData)
			OnBnClickedBtnStoprefresh();
	}
	else
	{
		if (!is_UpdateData)
			OnBnClickedBtnStoprefresh();
		return;
	}

}
////ʵ��ģʽ��ѡ�¼�
void CWindowsMainControlV1Dlg::OnCbnSelchangeTextMode()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������	
	INT_PTR nRes;

	if (m_TestMode.GetCurSel() == 6)
	{
		RS_para.reset();
		ReadSimuDlg ReadSimuDlg1;
		OnBnClickedBtnReset();
		nRes = ReadSimuDlg1.DoModal();
		if (nRes == 6)
		{
			if (RS_para.RdataFilefid != NULL) fclose(RS_para.RdataFilefid);
			RS_para.delay5ms = ReadSimuDlg1.Delay5ms;
			RS_para.ReadInitAtt = ReadSimuDlg1.ReadInitAtt;
			RS_para.ReadInitPos = ReadSimuDlg1.ReadInitPos;
			RS_para.RdataFile = ReadSimuDlg1.filename;
			RS_para.file_mode = ReadSimuDlg1.combonum;
			fopen_s(&RS_para.RdataFilefid, RS_para.RdataFile + "", "r");
			RS_para.canCal = 1;
			RS_para.RS_mode = true;
			GetDlgItem(IDC_BTN_SAVEPATH)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_StartCal)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_StartCard)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_StartPhins)->EnableWindow(FALSE);

			CWinThread* simu_Thread;
			THREADPARAM  *phWndParam = new THREADPARAM;
			phWndParam->hwnd = m_hWnd;
			if (!(simu_Thread = AfxBeginThread(SimulateThread, (LPVOID)phWndParam)))
				return;
		}
		else
		{
			MessageBox(_T("�ļ���ȡʧ��"));
			m_TestMode.SetCurSel(0);
			RS_para.reset();
			if (isCreateTimer == true)
			{
				timeKillEvent(TimerID);
				isCreateTimer = false;
			}
			GetDlgItem(IDC_BTN_SAVEPATH)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_StartCal)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_StartCard)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_StartPhins)->EnableWindow(TRUE);
		}
	}
	else RS_para.reset();

	if (m_TestMode.GetCurSel() == 7)
	{
		SimuDataDlg simuDlg;
		nRes = simuDlg.DoModal();
	}
}
//����׼�²�����ˮƽ�ͺ����׼ʱ��
void CWindowsMainControlV1Dlg::OnCbnSelchangeComboFinealignmode()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	FineModeNum = m_FineAignMode.GetCurSel();
	if (FineModeNum == 1)
	{
		GetDlgItem(IDC_FINE_LEVEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_FINE_AZIMUTH)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_FINE_LEVEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_FINE_AZIMUTH)->EnableWindow(FALSE);
	}
}
#pragma endregion ClBtn 

/////��������***********��ʼ������****************��������//////
#pragma region InitFunc
//haishiģʽ���ǳ���ģʽ��һЩֵԤ�裬��������ʵ�鲻�ò�ͣ�л�ѡ�
void CWindowsMainControlV1Dlg::init_mainmode()
{
	if (MAINMODE == 1)
	{
		m_NaviMode.SetCurSel(0);
		m_HeightMode.SetCurSel(0);
		edit_data_f = 200;
		UpdateData(false);
	}
	if (MAINMODE == 2)
	{
		m_NaviMode.SetCurSel(5);
		m_HeightMode.SetCurSel(4);
		edit_data_f = 100;
		UpdateData(false);
	}
}
////��������
void CWindowsMainControlV1Dlg::init_var()
{
	fid_FOSN = NULL;
	fid_PHINS = NULL;
	fid_gps = NULL;
	fid_zt = NULL;
	fid_Cal = NULL;

	is_cardReceive = 0;
	is_startCal = 0;
	//is_InitNaviVar=0;
	is_start_phins = 0;
	is_data_used = false;

	isStartCalOk = false;

	memset(bufFOSN, 0, sizeof(bufFOSN));
	
	m_cnt_err = 0;

	sysc.state = _T("��û��ʼZz");
	m_cnt_s = 0;
	saveStart = 0;

	is_timeSave = 0;
	m_save_time = 600;

	m_PRecNum = 0;

	if (MAINMODE == 2)
	{
		calipara.bias_gyro[0] = -3.94251980810677e-05;
		calipara.bias_gyro[1] = -4.75641514044445e-06;
		calipara.bias_gyro[2] = 1.35189552763437e-05;
	}
}
////combo��ʼ��
bool CWindowsMainControlV1Dlg::init_Combo()
{
	int judge_tf;
	int i;

	CString str2[] = { _T("��"),_T("������/ûд"),_T("����ϵ��/ûд"),_T("���̽�����"),_T("��Ԫ����/ûд"),_T("�Զ��巽��1"),_T("�Զ��巽��2") };
	for (i = 0; i < 7; i++)
	{
		judge_tf = m_CoarseAignMode.InsertString(i, str2[i]);
		if ((judge_tf == CB_ERR) || (judge_tf == CB_ERRSPACE))
		{
			MessageBox(_T("build baud error!"));
			return false;
		}
	}
	m_CoarseAignMode.SetCurSel(0);

	CString str3[] = { _T("��"),_T("�޾���"),_T("˫λ��/����"),_T("yucia����˫λ��"),_T("XXX"),_T("�Կ��Ŷ�׼"),_T("�Զ��巽��") };
	for (i = 0; i < 7; i++)
	{
		judge_tf = m_FineAignMode.InsertString(i, str3[i]);
		if ((judge_tf == CB_ERR) || (judge_tf == CB_ERRSPACE))
		{
			MessageBox(_T("build baud error!"));
			return false;
		}
	}
	m_FineAignMode.SetCurSel(0);

	CString str4[] = { _T("��������"),_T("�����"),_T("�ٶ�+λ��+��̬"),_T("GPSλ�����"),_T("�ٶ�+����"),_T("haishi���"),_T("haishi����") };
	for (i = 0; i < 7; i++)
	{
		judge_tf = m_NaviMode.InsertString(i, str4[i]);
		if ((judge_tf == CB_ERR) || (judge_tf == CB_ERRSPACE))
		{
			MessageBox(_T("build baud error!"));
			return false;
		}
	}
	m_NaviMode.SetCurSel(5);

	CString str5[] = { _T("������"),_T("������"),_T("�Ľ�����1"),_T("���򵼺�"),_T("haishi��λ") ,_T("haishi�״�λ") ,_T("haishi�޸˱�") };
	for (i = 0; i < 7; i++)
	{
		judge_tf = m_HeightMode.InsertString(i, str5[i]);
		if ((judge_tf == CB_ERR) || (judge_tf == CB_ERRSPACE))
		{
			MessageBox(_T("build baud error!"));
			return false;
		}
	}
	m_HeightMode.SetCurSel(3);

	CString str6[] = { _T("ת̨ʵ��"),_T("����ʵ��"),_T("��¼��������phins��"),_T("��¼������phins��"),_T("����1"),_T("����2"),_T("��������"),_T("ģ�����ݷ���") };
	for (i = 0; i < 8; i++)
	{
		judge_tf = m_TestMode.InsertString(i, str6[i]);
		if ((judge_tf == CB_ERR) || (judge_tf == CB_ERRSPACE))
		{
			MessageBox(_T("build baud error!"));
			return false;
		}
	}
	m_TestMode.SetCurSel(0);
	return true;
}
////�����ʼ��
bool CWindowsMainControlV1Dlg::Init_Net(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return false;
	}
	m_socketPHINSDataRec = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == m_socketPHINSDataRec)
	{
		MessageBox(_T(" ���ݽ���socket����ʧ��"));
		return false;
	}
	SOCKADDR_IN addrRecSock;
	addrRecSock.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrRecSock.sin_family = AF_INET;
	addrRecSock.sin_port = htons(8110);
	int retval = bind(m_socketPHINSDataRec, (SOCKADDR*)& addrRecSock, sizeof(SOCKADDR));
	if (SOCKET_ERROR == retval)
	{
		closesocket(m_socketPHINSDataRec);
		return false;
	}
	return true;
}
#pragma endregion InitFunc
#pragma region SmallFuc
////�ļ���·����ȡ
CString CWindowsMainControlV1Dlg::GetDirectory()////�����ļ���·��
{
	BROWSEINFO bi;
	char name[MAX_PATH];
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	bi.pszDisplayName = LPWSTR(name);
	bi.lpszTitle = _T("ѡ�񱣴���ļ���");
	bi.ulFlags = BIF_RETURNFSANCESTORS;
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (idl == NULL)
		return _T("");
	CString strDirectoryPath;
	SHGetPathFromIDList(idl, strDirectoryPath.GetBuffer(MAX_PATH));
	strDirectoryPath.ReleaseBuffer();
	if (strDirectoryPath.IsEmpty())
		return _T("");
	if (strDirectoryPath.Right(1) != "\\")
		strDirectoryPath += "\\";
	return strDirectoryPath;
}
////����ˢ����Ϣ��Ӧ����
LRESULT CWindowsMainControlV1Dlg::updateData(WPARAM wParam, LPARAM lParam)////��ʾˢ����Ӧ����
{
	UpdateData(false);
	return 0L;
}
////����ֹͣ��Ϣ��Ӧ����
LRESULT CWindowsMainControlV1Dlg::saveCancel(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedCancel();
	return 0L;
}
//200���ݼ�������(sysc.Fs������������200)
bool CWindowsMainControlV1Dlg::Rec200times(int &count200)
{
	if (count200 == sysc.Fs)
	{
		count200 = 1;
		return 1;
	}
	else
	{
		count200++;
		return 0;
	}
}
#pragma endregion SmallFuc

///////////�忨���ݽ��մ�������/////////////////
#pragma region DataPro 
int CWindowsMainControlV1Dlg::FOSNChannel()
{
	WORD rt = 0;
	int i;

	if (strm[2].IsProtocol)
	{
		//	Protocol Mode		
		if (Sio_Rx_IsFrameOver(hCard, 2))
		{
			Sio_ReadFrame(hCard, 2, bufFOSN, &rt);

			//���ݽ���
			int4ch.Ch[0] = bufFOSN[3];
			int4ch.Ch[1] = bufFOSN[4];
			int4ch.Ch[2] = bufFOSN[5];
			int4ch.Ch[3] = bufFOSN[6];
			fosn.time = int4ch.Int / 1000.0;
			//msecond=(unsigned int)IN4CH.Int%1000/1000.0;
			fosn.s = (int)fosn.time;
			fosn.ms = ((int)fosn.time - fosn.s) * 1000;

			/* ��������ϵΪǰ���� */
			for (i = 0; i < 12; i++)
				int12ch.Ch[i] = bufFOSN[i + 7];

			/***************XYZ�������****************/
			IMUout.gyro_b[1] = int12ch.Int[0] / 3.6*1e-6;
			IMUout.gyro_b[2] = int12ch.Int[1] / 3.6*1e-6;
			IMUout.gyro_b[0] = int12ch.Int[2] / 3.6*1e-6;

			for (i = 0; i < 12; i++)
				int12ch.Ch[i] = bufFOSN[i + 19];

			/***************XYZ����ٶ�****************/
			IMUout.acce_b[1] = int12ch.Int[0] * 1e-6;
			IMUout.acce_b[2] = int12ch.Int[1] * 1e-6;
			IMUout.acce_b[0] = int12ch.Int[2] * 1e-6;

			for (i = 0; i < 24; i++)
				fl6ch.Ch[i] = bufFOSN[i + 33];

			/********����ǡ�����ǡ�������************/
			fosn.ang[1] = fl6ch.FLo[0];
			fosn.ang[2] = fl6ch.FLo[1];
			fosn.ang[0] = fl6ch.FLo[2];

			/*************�����졢�����ٶ�************/
			fosn.vel[1] = fl6ch.FLo[3];
			fosn.vel[2] = fl6ch.FLo[4];
			fosn.vel[0] = fl6ch.FLo[5];

			for (i = 0; i < 12; i++)
				fl6ch.Ch[i] = bufFOSN[i + 57];
			/**************γ�ȡ����ȡ��߶�***********/
			fosn.pos[0] = fl6ch.FLo[0] * 57.29578;
			fosn.pos[1] = fl6ch.FLo[1] * 57.29578;
			fosn.pos[2] = fl6ch.FLo[2];

			if (is_startCal)fosn.recnum++;
			return 1;
		}
		else
			return 0;
	}
	return 0;
}
void CWindowsMainControlV1Dlg::ZTChannel()
{
	WORD rt = 0;
	static int old_buf2 = 0;
	if (strm[0].IsProtocol)
	{
		//	Protocol Mode
#ifndef CARD_DEBUG
		if (Sio_Rx_IsFrameOver(hCard, 0))
		{
			old_buf2 = BufZT[2];
			Sio_ReadFrame(hCard, 0, BufZT, &rt);

			if (BufZT[0] == 0xFF && BufZT[1] == 0xFF)
			{
				CString c;
				int temp[8];
				int i;
				float angFybt[3] = { 0.0 };
				short int jiaoyan = 0;
				int tem = 0;
				static int DrawNum = 0;
				for (i = 0; i < 3; i++)
				{
					temp[0] = BufZT[i * 4 + 3] / 16;
					temp[1] = BufZT[i * 4 + 3] % 16;
					temp[2] = BufZT[i * 4 + 1 + 3] / 16;
					temp[3] = BufZT[i * 4 + 1 + 3] % 16;
					temp[4] = BufZT[i * 4 + 2 + 3] / 16;
					temp[5] = BufZT[i * 4 + 2 + 3] % 16;
					temp[6] = BufZT[i * 4 + 3 + 3] / 16;
					temp[7] = BufZT[i * 4 + 3 + 3] % 16;
					angFybt[i] = temp[0] * 100 + temp[1] * 10 + temp[2] + temp[3] * 0.1 + temp[5] * 0.01 + temp[6] * 0.001 + temp[7] * 0.0001;
				}
				if (BufZT[2] - old_buf2 < 0) ZT.Frame += BufZT[2] - old_buf2 + 256;
				else ZT.Frame += BufZT[2] - old_buf2;

				if (is_startCal)
				{
					ZT.cnt++;
					m_cnt_err = ZT.cnt - fosn.recnum;
				}

				for (i = 0; i < 3; i++)
				{
					if (angFybt[i] > 180)
						angFybt[i] = angFybt[i] - 360.0;
				}
				ZT.ang[0] = angFybt[1];   //��ҡ��
				ZT.ang[1] = angFybt[0];   //��ҡ��
				ZT.ang[2] = angFybt[2];   //�����

			}
		}
#endif
	}
}
int CWindowsMainControlV1Dlg::GPSChannel()
{
	WORD rt = 0;
	//	Protocol Mode
	if (Sio_Rx_IsFrameOver(hCard, 1))
	{
		Sio_ReadFrame(hCard, 1, BufGPS, &rt);
		if (BufGPS[0] == 0xAA && BufGPS[1] == 0x44 && BufGPS[2] == 0x12 && BufGPS[4] == 0x2A)
		{
			int i;
			for (i = 0; i < 24; i++)
				gps_pv.ch[i] = BufGPS[i + 36];
			gps.pos[0] = gps_pv.db[0];//*D2R;
			gps.pos[1] = gps_pv.db[1];//*D2R;
			gps.pos[2] = gps_pv.db[2];	

			for (i = 0; i < 4; i++)
				int4ch.Ch[i] = BufGPS[i + 16];
			gps.time = int4ch.Int*0.001;
			if (is_startCal)
			{
				gps.cnt++;
			}
			if (BufGPS[28] == 0)
				return 1;
			else
				return 0;
		}
		else return 0;
	}
	else
		return 0;
}
void CWindowsMainControlV1Dlg::CalVarInit(char mode)
{
	if (mode == 1)
	{
		initial_latitude = phins.pos[0];
		initial_longitude = phins.pos[1];
		initial_height = phins.pos[2];
	}	
	infor.pos[0] = initial_latitude* D2R;
	infor.pos[1] = initial_longitude* D2R;
	infor.pos[2] = initial_height;
	infor.initial_pos[0] = initial_latitude* D2R;
	infor.initial_pos[1] = initial_longitude* D2R;
	infor.initial_pos[2] = initial_height;

}
void CWindowsMainControlV1Dlg::getfileData()
{
	double temp;
	if (feof(RS_para.RdataFilefid))//������β
	{
		RS_para.RS_mode = 0;
		return;
	}
	if (RS_para.file_mode == 1)//��Ӧ��¼��ģʽ������PHINS��
	{
		fscanf_s(RS_para.RdataFilefid, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			&m_PRecNum, &ZT.Frame, &fosn.time,
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2],
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2],
			&ZT.ang[0], &ZT.ang[1], &ZT.ang[2],
			&fosn.ang[0], &fosn.ang[1], &fosn.ang[2]);
	}
	if (RS_para.file_mode == 0)//��Ӧ����¼��ģʽ
	{
		fscanf_s(RS_para.RdataFilefid, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			&fosn.recnum, &ZT.cnt, &fosn.time, &gps.time, &phins.utc,
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2],
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2],
			&INScal.ang[0], &INScal.ang[1], &INScal.ang[2],
			&INScal.vel[0], &INScal.vel[1], &INScal.vel[2],
			&INScal.pos[0], &INScal.pos[1], &INScal.pos[2],
			&phins.ang[0], &phins.ang[1], &phins.ang[2],
			&phins.vel[0], &phins.vel[1], &phins.vel[2],
			&phins.pos[2], &phins.pos[0], &phins.pos[1],
			&gps.pos[0], &gps.pos[1], &gps.pos[2],
			0.0, 0.0, 0.0,
			&fosn.ang[0], &fosn.ang[1], &fosn.ang[2],
			&fosn.vel[0], &fosn.vel[1], &fosn.vel[2],
			&fosn.pos[0], &fosn.pos[1], &fosn.pos[2]);
	}
	if (RS_para.file_mode == 2)//��Ӧת̨ʵ��ģʽ������kalman���Ƶģ�
	{
		fscanf_s(RS_para.RdataFilefid, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			&fosn.recnum, &ZT.cnt,
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2],
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2],
			&INScal.ang[0], &INScal.ang[1], &INScal.ang[2],
			&INScal.vel[0], &INScal.vel[1], &INScal.vel[2],
			&INScal.pos[0], &INScal.pos[1], &INScal.pos[2],
			&ZT.ang[0], &ZT.ang[1], &ZT.ang[2],
			&fosn.ang[0], &fosn.ang[1], &fosn.ang[2]);
	}
	if (RS_para.file_mode == 3)//��Ӧ��������ģʽ1
	{
	                	//             |           |           |           |           |           |       |
		fscanf_s(RS_para.RdataFilefid, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			&fosn.time,
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2],
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2],
			&ZT.ang[0], &ZT.ang[1], &ZT.ang[2],
			&fosn.vel[0], &fosn.vel[1], &fosn.vel[2],
			&fosn.pos[0], &fosn.pos[1], &fosn.pos[2],
			&temp, &temp,
			&temp, &temp, &temp,
			&temp, &temp, &temp);
		IMUout.gyro_b[0] = IMUout.gyro_b[0]*R2D;
		IMUout.gyro_b[1] = IMUout.gyro_b[1]*R2D;
		IMUout.gyro_b[2] = IMUout.gyro_b[2]*R2D;
		ZT.ang[0] = ZT.ang[0]*R2D;
		ZT.ang[1] = ZT.ang[1]*R2D;
		ZT.ang[2] = ZT.ang[2]*R2D;		
		fosn.pos[0] = fosn.pos[0]*R2D;
		fosn.pos[1] = fosn.pos[1]*R2D;
		memcpy(phins.pos, fosn.pos, sizeof(fosn.pos));
	}
	if (RS_para.file_mode == 4)//��Ӧ��������ģʽ2
	{
		//               |           |           |           |           |           |       |
		fscanf_s(RS_para.RdataFilefid, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			&fosn.time,
			&temp, &temp, &temp,
			&temp, &temp, &temp,
			&ZT.ang[0], &ZT.ang[1], &ZT.ang[2],
			&fosn.vel[0], &fosn.vel[1], &fosn.vel[2],
			&fosn.pos[0], &fosn.pos[1], &fosn.pos[2],
			&temp, &temp,
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2],
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2]);
		IMUout.gyro_b[0] = IMUout.gyro_b[0]*R2D;
		IMUout.gyro_b[1] = IMUout.gyro_b[1]*R2D;
		IMUout.gyro_b[2] = IMUout.gyro_b[2]*R2D;
		ZT.ang[0] = ZT.ang[0]*R2D;
		ZT.ang[1] = ZT.ang[1]*R2D;
		ZT.ang[2] = ZT.ang[2]*R2D;
		fosn.pos[0] = fosn.pos[0]*R2D;
		fosn.pos[1] = fosn.pos[1]*R2D;
		memcpy(phins.pos, fosn.pos, sizeof(fosn.pos));
	}
	if (RS_para.file_mode == 5)//yucia���ݲ���
	{
		fscanf_s(RS_para.RdataFilefid, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			&fosn.time,
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2], 
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2],
			&ZT.ang[0], &ZT.ang[1], &ZT.ang[2],
			&phins.vel[0], &phins.vel[1], &phins.vel[2],
			&phins.pos[0], &phins.pos[1], &phins.pos[2]);
		ZT.ang[0] = ZT.ang[0]*R2D;
		ZT.ang[1] = ZT.ang[1]*R2D;
		ZT.ang[2] = ZT.ang[2]*R2D;			
	}
	if (RS_para.file_mode == 7)//haishi���ݲ���
	{
	//	1~3���ݣ�4~6�ӱ���7~9 ԭʼ������̬��10~12 ԭʼ�����ٶȣ� 13~15 �ⲿ�ο���̬��16~18�ⲿ�ο��ٶȣ�19γ�� 20����,21�ߣ�22����
		                    // 22��                |           |           |           |           |           |           |
		fscanf_s(RS_para.RdataFilefid, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %d\n",
			&IMUout.gyro_b[0], &IMUout.gyro_b[1], &IMUout.gyro_b[2],
			&IMUout.acce_b[0], &IMUout.acce_b[1], &IMUout.acce_b[2],
			&fosn.ang[0], &fosn.ang[1], &fosn.ang[2],
			&fosn.vel[0], &fosn.vel[1], &fosn.vel[2],
			&phins.ang[0], &phins.ang[1], &phins.ang[2],
			&phins.vel[0], &phins.vel[1], &phins.vel[2],
			&phins.pos[0], &phins.pos[1], &phins.pos[2],
			&phins.cnt);
		memcpy(ZT.ang, phins.ang, sizeof(phins.ang));
		memcpy(fosn.pos, phins.pos, sizeof(phins.pos));

	}
	fosn.s = (int)fosn.time;
	fosn.ms = (fosn.time - fosn.s) * 1000;
	//�״ζ�ȡ�ж��Ƿ���Ҫ���г�ʼ�����ݵ���̬��λ�õ����ݵ�һ֡��ȡ����̬��λ��
	if (RS_para.isstart == 0)
	{
		if (RS_para.ReadInitPos == 1)//��ȡ��ʼλ��
		{
			if (RS_para.file_mode == 2)
			{
				memcpy(infor.initial_pos, INScal.pos, sizeof(INScal.pos));
				memcpy(infor.pos, infor.initial_pos, sizeof(infor.initial_pos));
				initial_latitude = infor.pos[0];
				initial_longitude = infor.pos[1];
				initial_height = infor.pos[2];
			}
			if (RS_para.file_mode == 3 || RS_para.file_mode == 4 )
			{
				initial_latitude = fosn.pos[0];
				initial_longitude = fosn.pos[1];
				initial_height = fosn.pos[2];
				infor.pos[0] = initial_latitude* D2R;
				infor.pos[1] = initial_longitude* D2R;
				infor.pos[2] = initial_height;
				infor.initial_pos[0] = initial_latitude* D2R;
				infor.initial_pos[1] = initial_longitude* D2R;
				infor.initial_pos[2] = initial_height;
				
			}
			if (RS_para.file_mode == 5)
			{
				infor.pos[0] = phins.pos[0] * D2R;
				infor.pos[1] = phins.pos[1] * D2R;
				infor.pos[2] = phins.pos[2];
				initial_latitude = infor.pos[0] * R2D;
				initial_longitude = infor.pos[1] * R2D;
				initial_height = infor.pos[2];
			}
			if (RS_para.file_mode == 7)
			{				
				infor.pos[0] = phins.pos[0] * D2R;
				infor.pos[1] = phins.pos[1] * D2R;
				infor.pos[2] = phins.pos[2] * D2R;
				initial_latitude = phins.pos[0];
				initial_longitude = phins.pos[1];
				initial_height = phins.pos[2];
				memcpy(infor.vel_n, fosn.vel, sizeof(fosn.vel));

				infor.att_angle[0] = fosn.ang[0] * D2R;
				infor.att_angle[1] = fosn.ang[1] * D2R;
				infor.att_angle[2] = fosn.ang[2] * D2R;
				ang2cnb(infor.cnb_mat, infor.att_angle);
				cnb2q(infor.cnb_mat, infor.quart);
				optq(infor.quart);
			}
			RS_para.isstart = 1;
		}
		if (RS_para.ReadInitAtt == 1&& RS_para.file_mode<7)//��ȡ��ʼ��̬
		{
			infor.att_angle[0] = ZT.ang[0] *D2R;
			infor.att_angle[1] = ZT.ang[1] *D2R;
			infor.att_angle[2] = ZT.ang[2] *D2R;
			ang2cnb(infor.cnb_mat, infor.att_angle);
			cnb2q(infor.cnb_mat, infor.quart);
			optq(infor.quart);
		}
		RS_para.isstart = 1;
	}
}
void CWindowsMainControlV1Dlg::IMUdataCount()
{
	sysc.data_cnt++;
	if (0 == sysc.data_cnt % sysc.Fs)
	{
		sysc.cnt_s++;
		if (is_timeSave)
			m_save_time--;//����ʱ��ʾ
	}
	m_cnt_s = sysc.cnt_s;	
	is_data_used = 0;
}
void CWindowsMainControlV1Dlg::SaveData()
{
	if (saveStart && !is_SingleFile)
	{
		if (TestModeNum != 1)
		{
			fprintf_s(fid_zt, "%d,%d,%d,%lf,%lf,%lf\n",
				ZT.Frame, ZT.cnt, fosn.recnum,
				ZT.ang[0], ZT.ang[1], ZT.ang[2]);
			fflush(fid_zt);
		}

		fprintf_s(fid_Cal, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			fosn.recnum,
			INScal.ang[0], INScal.ang[1], INScal.ang[2],
			INScal.vel[0], INScal.vel[1], INScal.vel[2],
			INScal.pos[0], INScal.pos[1], INScal.pos[2],
			INScal.err_ang[0], INScal.err_ang[1], INScal.err_ang[2],
			INScal.err_pos[0], INScal.err_pos[1], INScal.err_pos[2]);
		fflush(fid_Cal);

		fprintf_s(fid_FOSN, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			fosn.recnum, fosn.time,
			IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
			IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
			fosn.ang[0], fosn.ang[1], fosn.ang[2],
			fosn.vel[0], fosn.vel[1], fosn.vel[2],
			fosn.pos[0], fosn.pos[1], fosn.pos[2]);
		fflush(fid_FOSN);

		fprintf_s(fid_gps, "%d,%lf,%lf,%lf,%lf\n",
			fosn.recnum, gps.time,
			gps.pos[0], gps.pos[1], gps.pos[2]);
		fflush(fid_gps);

		fprintf_s(fid_PHINS, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			phins.cnt, fosn.recnum, phins.utc,
			phins.ang[0], phins.ang[1], phins.ang[2],
			phins.vel[0], phins.vel[1], phins.vel[2],
			phins.pos[2], phins.pos[0], phins.pos[1]);
		fflush(fid_PHINS);

	}
	if (saveStart&&is_SingleFile)
	{
		if (TestModeNum == 0)//ת̨ʵ��
			if (FineModeNum == FINE_Yucia)
				fprintf_s(fid_Cal, "%d,%d,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
					fosn.recnum, ZT.cnt,
					IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
					IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
					INScal.ang[0], INScal.ang[1], INScal.ang[2],
					INScal.vel[0], INScal.vel[1], INScal.vel[2],
					INScal.pos[0], INScal.pos[1], INScal.pos[2],
					ZT.ang[0], ZT.ang[1], ZT.ang[2],
					fosn.ang[0], fosn.ang[1], fosn.ang[2],
					fkalman.X_vector[9], fkalman.X_vector[10], fkalman.X_vector[11],
					fkalman.X_vector[12], fkalman.X_vector[13], fkalman.X_vector[14]);
			else
				fprintf_s(fid_Cal, "%d,%d,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
					fosn.recnum, ZT.cnt,
					IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
					IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
					INScal.ang[0], INScal.ang[1], INScal.ang[2],
					INScal.vel[0], INScal.vel[1], INScal.vel[2],
					INScal.pos[0], INScal.pos[1], INScal.pos[2],
					ZT.ang[0], ZT.ang[1], ZT.ang[2],
					fosn.ang[0], fosn.ang[1], fosn.ang[2]);
#pragma region Datacz
		if (TestModeNum == 1)//����ʵ�����ݱ�׼��ʽ
			/*1~5 ֡��¼��ͳ�ƣ�ת̨֡��/�๦�ܰ�֡��/���ã�fosnʱ��,gpsʱ�䣬phinsʱ��
			6~11  ���ݼӱ������ݵ�λΪ ��/S��
			12~14 ������̬
			15~17 �����ٶ�
			18~20 ����λ��
			21~23 PHINS��̬
			24~26 PHINS�ٶ�
			27~29 PHINSλ��
			30~35 GPSλ�ã��ٶȣ�û�У�
			36~38 FOSN��̬
			39~41 FOSN�ٶ�
			42~44 FOSNλ��
												|����                 �ӱ�                |������̬            �ٶ�                  λ��                |PS��̬               �ٶ�                λ��                 |GPSλ��              �ٶ�                |FOSN��̬            �ٶ�                 λ��    */
			fprintf_s(fid_Cal, "%d,%d,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
				fosn.recnum, ZT.cnt, fosn.time, gps.time, phins.utc,
				IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
				IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
				INScal.ang[0], INScal.ang[1], INScal.ang[2],
				INScal.vel[0], INScal.vel[1], INScal.vel[2],
				INScal.pos[0], INScal.pos[1], INScal.pos[2],
				phins.ang[0], phins.ang[1], phins.ang[2],
				phins.vel[0], phins.vel[1], phins.vel[2],
				phins.pos[2], phins.pos[0], phins.pos[1],
				gps.pos[0], gps.pos[1], gps.pos[2],
				0.0, 0.0, 0.0,
				fosn.ang[0], fosn.ang[1], fosn.ang[2],
				fosn.vel[0], fosn.vel[1], fosn.vel[2],
				fosn.pos[0], fosn.pos[1], fosn.pos[2]);
#pragma endregion Datacz
		if (TestModeNum == 2)//3�ţ�3���ݣ�3�ӱ���3ת̨��̬��3FOSN��̬
			fprintf_s(fid_Cal, "%d,%d,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
				m_PRecNum, ZT.Frame, fosn.time,
				IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
				IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
				ZT.ang[0], ZT.ang[1], ZT.ang[2],
				fosn.ang[0], fosn.ang[1], fosn.ang[2]);
		if (TestModeNum == 3)//4�ţ�3���ݣ�3�ӱ���3ת̨��̬��3FOSN��̬��3Phins��̬�ٶ�λ��
			fprintf_s(fid_Cal, "%d,%d,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
				m_PRecNum, ZT.Frame, fosn.time, phins.utc,
				IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
				IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
				ZT.ang[0], ZT.ang[1], ZT.ang[2],
				fosn.ang[0], fosn.ang[1], fosn.ang[2],
				phins.ang[0], phins.ang[1], phins.ang[2],
				phins.vel[0], phins.vel[1], phins.vel[2],
				phins.pos[2], phins.pos[0], phins.pos[1]);
		if (TestModeNum == 6)
		{		
			if (FineModeNum == FINE_Yucia)
			{			
				fprintf_s(fid_Cal, "%lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
					fosn.time,
					IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
					IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
					INScal.ang[0], INScal.ang[1], INScal.ang[2],
					INScal.vel[0], INScal.vel[1], INScal.vel[2],
					INScal.pos[0], INScal.pos[1], INScal.pos[2],
					ZT.ang[0], ZT.ang[1], ZT.ang[2],
					fkalman.X_vector[9], fkalman.X_vector[10], fkalman.X_vector[11],
					fkalman.X_vector[12], fkalman.X_vector[13], fkalman.X_vector[14]);
				fflush(fid_Cal);
				return;
			}
			if (PureINSModeNum == PURE_SINS_TRANSVERSE)
			{
				fprintf_s(fid_Cal, "%lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
					fosn.time,
					IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
					IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
					inforS.att_angle[0], inforS.att_angle[1], inforS.att_angle[2],
					inforS.vel_n[0], inforS.vel_n[1], inforS.vel_n[2],
					inforS.lati, inforS.longi, inforS.high,
					inforS.att_angle_S[0], inforS.att_angle_S[1], inforS.att_angle_S[2],
					inforS.vel_S[0], inforS.vel_S[1], inforS.vel_S[2],
					inforS.lati_S, inforS.longi_S, inforS.high_S);
				fflush(fid_Cal);
				return;
			}				
			if (NaviModeNum == NAVI_HAISHI_BASIC || NaviModeNum == NAVI_HAISHI_JZ)
			{          //                               |                    |                    |                    |                    |                    |                    |                    |
				fprintf_s(fid_Cal, "%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
					INScal.ang[0], INScal.ang[1], INScal.ang[2],
					INScal.vel[0], INScal.vel[1], INScal.vel[2],
					INScal.pos[0], INScal.pos[1], INScal.pos[2],
					fosn.ang[0], fosn.ang[1], fosn.ang[2],
					fosn.vel[0], fosn.vel[1], fosn.vel[2],					
					phins.ang[0], phins.ang[1], phins.ang[2],
					phins.vel[0], phins.vel[1], phins.vel[2],
					phins.pos[0], phins.pos[1], phins.pos[2], 
					infor.vel_arm[0], infor.vel_arm[1], infor.vel_arm[2]);
				fflush(fid_Cal);
				return;
			}
			      //		       |                    |                    |                    |                    |                    |                    |                    |
			fprintf_s(fid_Cal, "%lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf,%.16lf\n",
				fosn.time,
				IMUout.gyro_b[0], IMUout.gyro_b[1], IMUout.gyro_b[2],
				IMUout.acce_b[0], IMUout.acce_b[1], IMUout.acce_b[2],
				INScal.ang[0], INScal.ang[1], INScal.ang[2],
				INScal.vel[0], INScal.vel[1], INScal.vel[2],
				INScal.pos[0], INScal.pos[1], INScal.pos[2],
				ZT.ang[0], ZT.ang[1], ZT.ang[2],
				fosn.vel[0], fosn.vel[1], fosn.vel[2],
				fosn.pos[0], fosn.pos[1], fosn.pos[2]);
		}
		fflush(fid_Cal);
	}
}
void CWindowsMainControlV1Dlg::DataTrans()
{
	avecmul(3, INScal.ang, infor.att_angle, R2D);	
	memcpy(INScal.vel, infor.vel_n, sizeof(infor.vel_n));	
	INScal.pos[0] = infor.pos[0]*R2D;
	INScal.pos[1] = infor.pos[1]*R2D;
	INScal.pos[2] = infor.pos[2];

	double temp_fixanlge[3] = { 0 };
	calipara.fixup(temp_fixanlge, infor.cnb_mat);
	if (0 == TestModeNum)
	{
		INScal.err_ang[0] = temp_fixanlge[0] * R2D - ZT.ang[0];
		INScal.err_ang[1] = temp_fixanlge[1] * R2D - ZT.ang[1];
		INScal.err_ang[2] = temp_fixanlge[2] * R2D - ZT.ang[2];
		INScal.err_pos[0] += infor.vel_n[0] * sysc.Ts;
		INScal.err_pos[1] += infor.vel_n[1] * sysc.Ts;
		INScal.err_pos[2] = sqrt(INScal.err_pos[0]*INScal.err_pos[0] + INScal.err_pos[1]*INScal.err_pos[1]);
	}
	if (1 == TestModeNum)
	{
		INScal.err_ang[0] = temp_fixanlge[0] * R2D - phins.ang[0];
		INScal.err_ang[1] = temp_fixanlge[1] * R2D - phins.ang[1];
		INScal.err_ang[2] = temp_fixanlge[2] * R2D - phins.ang[2];
		INScal.err_pos[0] = infor.pos[0] - phins.pos[0];
		INScal.err_pos[1] = infor.pos[1] - phins.pos[1];
		INScal.err_pos[2] = sqrt(INScal.err_pos[0]*INScal.err_pos[0] + INScal.err_pos[1]*INScal.err_pos[1]);
	}
	if (6 == TestModeNum)
	{
		INScal.err_ang[0] = temp_fixanlge[0] * R2D - ZT.ang[0];
		INScal.err_ang[1] = temp_fixanlge[1] * R2D - ZT.ang[1];
		INScal.err_ang[2] = temp_fixanlge[2] * R2D - ZT.ang[2];
		INScal.err_pos[0] = (infor.pos[0] - fosn.pos[0]*D2R)*RE;
		INScal.err_pos[1] = (infor.pos[1] - fosn.pos[1]*D2R)*RE;
		INScal.err_pos[2] = sqrt(INScal.err_pos[0]*INScal.err_pos[0] + INScal.err_pos[1]*INScal.err_pos[1]);
	}


}
#pragma endregion DataPro

//////////////////�����������߳�///////////////////////////////
#pragma region ProFunc
////�ֶ�׼
void CWindowsMainControlV1Dlg::CoarseThread()
{
	int j;
	sysc.coarse_time = sysc.coarse_time;
	if (!sysc.f_coarse_over)
	{
		if (is_data_used == 0)
		{
			/* �������ݶ�ȡ */
			for (j = 0; j < 3; j++)
			{
				/* �洢��һ֡���� */
				if (1 == sysc.data_cnt)  infor.gyro_old[j] = IMUout.gyro_b[j] * D2R;
				infor.gyro_old[j] = infor.gyro_wib_b[j]; // ����������û�и���֮ǰ����
				infor.gyro_wib_b[j] = IMUout.gyro_b[j] * D2R; // �µ���������
				infor.acce_b[j] = IMUout.acce_b[j];        // �ӱ�����	 	
			}
			calipara.IMUcalibrate(infor);
			switch (CoarseModeNum)
			{
			case COARSE_NG:coarse_ng(); sysc.state = _T("���̽�����"); break;
			}
			is_data_used = 1;

		}
	}
	return;
}
////����׼
void CWindowsMainControlV1Dlg::FineThread()
{
	int i, j;
	double temp_pos[3] = { 0 };	
	if (!sysc.f_fine_over&&sysc.f_coarse_over)
	{

		if (is_data_used == 0)
		{
			/* �������ݶ�ȡ */
			for (j = 0; j < 3; j++)
			{
				if (1 == sysc.data_cnt)  infor.gyro_old[j] = IMUout.gyro_b[j] * D2R;
				infor.gyro_old[j] = infor.gyro_wib_b[j]; // ����������û�и���֮ǰ����
				infor.gyro_wib_b[j] = IMUout.gyro_b[j] * D2R; // �µ���������
				infor.acce_b[j] = IMUout.acce_b[j];		 // �ӱ�����	
			}
			calipara.IMUcalibrate(infor);//��ƫ����
			for (i = 0; i < 3; i++)
			{
				infor.gyro_wib_b[i] = infor.gyro_wib_b[i] - cmp.wc_b[i];
				infor.acce_b[i] = infor.acce_b[i] - cmp.fc_b[i];
			}
			//������ģʽѡ��
			switch (PureINSModeNum)
			{
			case PURE_SINS_UNDUMP:sinscal_zundamp(sysc.Ts); break;
				//	case NAVI_SINS_DUMP :sinscal_zdamp(sysc.Ts );sysc.state = 0x40|g_SysMode.fine;break;
			}

			switch (FineModeNum)
			{
			case FINE_CMPS:fine_cmps(); sysc.state = _T("�޾�������׼"); break;
			case FINE_Yucia:
				switch (TestModeNum)
				{
				case 1:avecmul(3, temp_pos, infor.initial_pos, 1); break;
				case 2:
					avecmul(3, temp_pos, phins.pos, 1); 
					temp_pos[0] *= D2R;
					temp_pos[1] *= D2R;
					break;
				case 6:
					avecmul(3, temp_pos, phins.pos, 1); 
					temp_pos[0] *= D2R;
					temp_pos[1] *= D2R;
					break;
				default:break;
				}
				fine_yucia(fkalman, temp_pos, YA_POS);
				sysc.state = _T("����������׼");
				break;
			case FINE_ADRC:fine_adrc(); sysc.state = _T("DR.Yang��ϹJB��׼"); break;

			}
			is_data_used = 1;
		}
	}
	return;
}
////����
void CWindowsMainControlV1Dlg::NaviThread(void)
{
	int j;
	double tempob[3] = {0};//15/3�˲�����3ά�۲���
	double tempob_v[3] = { 0 }; //�ٶȹ۲���
	double tempob_att[3] = { 0 };//��̬�۲���
	double gyro[2][3] = { 0 };
	if (!sysc.f_navi_over&&sysc.f_coarse_over&&sysc.f_fine_over)
	{
		temp_test = false;
		if (is_data_used == 0)
		{
			/* �������ݶ�ȡ */
			for (j = 0; j < 3; j++)
			{
				if (1 == sysc.data_cnt)  infor.gyro_old[j] = IMUout.gyro_b[j] * D2R;
				infor.gyro_old[j] = infor.gyro_wib_b[j]; // ����������û�и���֮ǰ����
				infor.gyro_wib_b[j] = IMUout.gyro_b[j] * D2R; // �µ���������
				infor.acce_b[j] = IMUout.acce_b[j];		 // �ӱ�����			
			}
			calipara.IMUcalibrate(infor);
			//haishi����º͸˱۲����й�
			switch (PureINSModeNum)
			{
			case PURE_SINS_UNDUMP:
				sinscal_zundamp(sysc.Ts); 
				sysc.state = _T("����������"); 
				break;
			case PURE_SINS_HAISHI_P:
				infor.rp[0] = 0.05; infor.rp[1] = 50; infor.rp[2] = 1;
				break;
			case PURE_SINS_TRANSVERSE:
				
				sinscal_zundamp(sysc.Ts);
				for (int i = 0; i<3; i++)
				{
					gyro[0][i] = infor.gyro_old[i];
					gyro[1][i] = infor.gyro_wib_b[i];
				}
				for (int i = 0; i<3; i++) inforS.acce_b[i] = infor.acce_b[i];
				sinscal_TRANSVERSE(inforS, sysc.Ts, gyro);
				sysc.state = _T("������+���򴿹���");
				break;
			case PURE_SINS_HAISHI_L:
				infor.rp[0] = 0.05; infor.rp[1] = 15; infor.rp[2] = 15;
				break;
			case PURE_SINS_HAISHI_0RP:
				infor.rp[0] = 0; infor.rp[1] = 0; infor.rp[2] = 0;
				break;
			}
			switch (NaviModeNum)
			{
			case NAVI_SG:
				tempob[0] = gps.pos[0] * D2R;
				tempob[1] = gps.pos[1] * D2R;
				tempob[2] = gps.pos[1];
	
				navi_Kal_15_3(fkalman, tempob, YA_POS);
				sysc.state = _T("λ�����");
				break;
			case NAVI_VELANDAZ:
				avecmul(3, tempob_att, phins.ang, 1);//�۲����Ļ�÷�ʽ
				avecmul(3, tempob_v, phins.vel , 1);

				vecsub(3, tempob, infor.att_angle, tempob_att);				
				DeltaAtt2Phi(infor, tempob, tempob);//��̬���ǵ�ʧ׼�Ǵ���
				vecsub(2, tempob, infor.vel_n, tempob_v);
				navi_Kal_15_3(fkalman,tempob, YA_VELANDAZ);
				sysc.state = _T("�ٶ�+�������");
				break;
			case NAVI_VEL:
				avecmul(3, tempob_v, phins.vel, 1);//�۲����Ļ�÷�ʽ

				vecsub(2, tempob, infor.vel_n, tempob_v);
				navi_Kal_15_3(fkalman,tempob, YA_VELANDAZ);
				sysc.state = _T("�ٶ����");
				break;
			case NAVI_HAISHI_BASIC:
				navigation(phins.vel[0], phins.vel[1], phins.ang[2] * D2R, NAVI_HAISHI_BASIC); 
				sysc.state = _T("haishiһ�����"); 
				break;
			case NAVI_HAISHI_JZ:
				navigation(phins.vel[0], phins.vel[1], phins.ang[2] * D2R, NAVI_HAISHI_JZ); 
				sysc.state = _T("haishi�����˲�"); 
				break;
			default:break;
			}

			is_data_used = 1;
			datanavinum++;
			//	temp_test = true;
		}
	}
	return;
}
////�忨���ݽ����߳�
UINT CWindowsMainControlV1Dlg::CardRec(LPVOID pParam)
{
	HWND hwnd = ((THREADPARAM*)pParam)->hwnd;
	delete pParam;

	static int count200 = 1;//200�����ݵļ�����(sysc.Fs������200�����Ƕ��ٸ���ͨ��200)
	static long int rec_count = 0;
	while (!mquit)
	{
		if (is_cardReceive)
		{
			if (is_cardReset)
			{
				Sio_Rx_ResetFIFO(hCard, 0);//��λ�忨
				Sio_Rx_ResetFIFO(hCard, 1);
				Sio_Rx_ResetFIFO(hCard, 2);
				is_cardReset = 0;
			}
			if (FOSNChannel() == 0)
				continue;
			if (TestModeNum != 1)ZTChannel();
			if (Rec200times(count200)) gps.flag = GPSChannel();

			if (is_startCal&&isStartCalOk)
			{

				IMUdataCount();
				DataTrans();
				CoarseThread();
				FineThread();
				NaviThread();
			}
			rec_count++;
			if (1 == is_UpdateData)
			{
				if (rec_count % (refresh_time / 5) == 0)
				{
					::PostMessage(hwnd, WM_UPDATEDATA, NULL, NULL);
				}
			}
			if (is_timeSave&&is_startCal)
			{
				if (m_cnt_s - temp_cnt_s == save_time)
					::PostMessage(hwnd, WM_TIMESAVE, NULL, NULL);
			}

			if (TestModeNum < 2)//����ģʽ�£����㿪ʼ֮���¼��
			{
				if (is_startCal&&saveStart)
					SaveData();
			}
			else               //��¼��ģʽ�£���ֱ��¼��
			{
				if (saveStart)
				{
					m_PRecNum++;
					SaveData();
				}

			}

		}
		else
			return 0;
	}
	return 0;
}
UINT CWindowsMainControlV1Dlg::SimulateThread(LPVOID pParam)
{
	HWND hwnd = ((THREADPARAM*)pParam)->hwnd;
	delete pParam;

	static long int rec_count = 0;
	while (!mquit)
	{
		if (RS_para.canCal&&isStartCalOk)
		{
			getfileData();
			if (RS_para.RS_mode == 0)
			{
				if (RS_para.RdataFilefid != NULL) fclose(RS_para.RdataFilefid);
				sysc.state = _T("�����ļ����㵽ͷ��");
				::PostMessage(hwnd, WM_UPDATEDATA, NULL, NULL);
				rec_count = 0;
				return 0;
			}
			IMUdataCount();
			CoarseThread();
			FineThread();
			NaviThread();
			DataTrans();
			if (RS_para.delay5ms == 1) RS_para.canCal = 0;

			rec_count++;
			if (1 == is_UpdateData)
			{
				if (rec_count % (refresh_time / 5) == 0)
				{
					::PostMessage(hwnd, WM_UPDATEDATA, NULL, NULL);
				}
			}
			if (is_timeSave&&is_startCal)
			{
				if (m_cnt_s - temp_cnt_s == save_time)
					::PostMessage(hwnd, WM_TIMESAVE, NULL, NULL);
			}

			if (is_startCal&&saveStart)
				SaveData();
		}

	}
	return 0;
}
////phins���ݽ��յ������߳�
UINT CWindowsMainControlV1Dlg::PHINSThread(LPVOID pParam)
{
	int len = sizeof(SOCKADDR);
	SOCKADDR_IN addrFrom;
	SOCKET sock = ((SOCKETPARAM*)pParam)->sock;
	HWND hwnd = ((SOCKETPARAM*)pParam)->hwnd;
	delete pParam;
	while (is_start_phins)
	{
		int retval = recvfrom(sock, m_Recv_PHINS_Buff, 1024, 0, (SOCKADDR*)&addrFrom, &len);

		double binary_value_31_bite = 180.0 / powl(2, 31);

		if (retval == 42 && m_Recv_PHINS_Buff[0] == 0x71)
		{
			phins.cnt++;
			data.ch_tem[3] = m_Recv_PHINS_Buff[1];
			data.ch_tem[2] = m_Recv_PHINS_Buff[2];
			data.ch_tem[1] = m_Recv_PHINS_Buff[3];
			data.ch_tem[0] = m_Recv_PHINS_Buff[4];
			phins.utc = data.int_tem + ((UINT)m_Recv_PHINS_Buff[5])*0.01;		//UTC time

			data.ch_tem[3] = m_Recv_PHINS_Buff[6];
			data.ch_tem[2] = m_Recv_PHINS_Buff[7];
			data.ch_tem[1] = m_Recv_PHINS_Buff[8];
			data.ch_tem[0] = m_Recv_PHINS_Buff[9];
			phins.pos[0] = data.int_tem * binary_value_31_bite;		//γ��

			data.ch_tem[3] = m_Recv_PHINS_Buff[10];
			data.ch_tem[2] = m_Recv_PHINS_Buff[11];
			data.ch_tem[1] = m_Recv_PHINS_Buff[12];
			data.ch_tem[0] = m_Recv_PHINS_Buff[13];
			phins.pos[1] = data.int_tem * binary_value_31_bite;      //����


			data.ch_tem[3] = m_Recv_PHINS_Buff[14];
			data.ch_tem[2] = m_Recv_PHINS_Buff[15];
			data.ch_tem[1] = m_Recv_PHINS_Buff[16];
			data.ch_tem[0] = m_Recv_PHINS_Buff[17];
			phins.pos[2] = data.int_tem * 0.01;                       //�߶�


			Vel2CH.Ch[0] = m_Recv_PHINS_Buff[23];              //�����ٶ�
			Vel2CH.Ch[1] = m_Recv_PHINS_Buff[22];
			phins.vel[0] = Vel2CH.Int*0.01;

			Vel2CH.Ch[0] = m_Recv_PHINS_Buff[21];             //�����ٶ�
			Vel2CH.Ch[1] = m_Recv_PHINS_Buff[20];
			phins.vel[1] = Vel2CH.Int*0.01;

			Vel2CH.Ch[0] = m_Recv_PHINS_Buff[25];              //�����ٶ�
			Vel2CH.Ch[1] = m_Recv_PHINS_Buff[24];
			phins.vel[2] = -Vel2CH.Int*0.01;

			Ang2CH.Ch[1] = m_Recv_PHINS_Buff[28];
			Ang2CH.Ch[0] = m_Recv_PHINS_Buff[29];
			phins.ang[0] = Ang2CH.Int * 180.0 / 32768.0;	//��ҡ

			Ang2CH.Ch[1] = m_Recv_PHINS_Buff[26];
			Ang2CH.Ch[0] = m_Recv_PHINS_Buff[27];
			phins.ang[1] = Ang2CH.Int * 180.0 / 32768.0;	//��ҡ

			Ang2CH.Ch[1] = m_Recv_PHINS_Buff[30];
			Ang2CH.Ch[0] = m_Recv_PHINS_Buff[31];
			phins.ang[2] = -Ang2CH.Int * 180.0 / 32768.0;	//����


		}
	}

	return 0;
}
#pragma endregion ProFunc

//��ʱ���ص�����
void CALLBACK TimeDalay(UINT uID, UINT uMsg, DWORD dwUsers, DWORD dw1, DWORD dw2)
{
	RS_para.canCal = 1;
}






