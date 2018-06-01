// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"
#include "ProcessConnection.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

#if defined(DEBUG) || defined(_DEBUG)
	//启动控制台
	StartConsole();
#endif
	GetDlgItem(IDC_STATIC_PID).SetWindowText(_T("选择PID"));
	GetDlgItem(IDC_EDIT_PID).SetWindowText(_T(""));
	GetDlgItem(IDOK).SetWindowText(_T("刷新"));
	GetDlgItem(IDCANCEL).SetWindowText(_T("关闭"));
	GetDlgItem(ID_APP_ABOUT).SetWindowText(_T("关于"));
	
	CListViewCtrl listViewTcp = (CListViewCtrl)GetDlgItem(IDC_LIST_TCP);
	LV_COLUMN lvcTcp = { 0 };
	
	while (listViewTcp.DeleteColumn(0));

	lvcTcp.cx = 100;
	lvcTcp.iSubItem = 0;
	lvcTcp.mask = LVCF_TEXT | LVCF_WIDTH;
	lvcTcp.pszText = _T("进程名称");
	listViewTcp.InsertColumn(listViewTcp.GetHeader().GetItemCount(), &lvcTcp);
	lvcTcp.pszText = _T("进程ID");
	listViewTcp.InsertColumn(listViewTcp.GetHeader().GetItemCount(), &lvcTcp);
	lvcTcp.pszText = _T("本地TCP连接");
	listViewTcp.InsertColumn(listViewTcp.GetHeader().GetItemCount(), &lvcTcp);
	lvcTcp.pszText = _T("远程连接");
	listViewTcp.InsertColumn(listViewTcp.GetHeader().GetItemCount(), &lvcTcp);
	lvcTcp.pszText = _T("状态");
	listViewTcp.InsertColumn(listViewTcp.GetHeader().GetItemCount(), &lvcTcp);

	CListViewCtrl listViewUdp = (CListViewCtrl)GetDlgItem(IDC_LIST_UDP);
	LV_COLUMN lvcUdp = { 0 };
	while (listViewUdp.DeleteColumn(0));
	
	lvcUdp.cx = 100;
	lvcUdp.mask = LVCF_TEXT | LVCF_WIDTH;
	lvcUdp.iSubItem = 0;

	lvcUdp.pszText = _T("进程名称");
	listViewUdp.InsertColumn(listViewUdp.GetHeader().GetItemCount(), &lvcUdp);
	lvcUdp.pszText = _T("进程ID");
	listViewUdp.InsertColumn(listViewUdp.GetHeader().GetItemCount(), &lvcUdp);
	lvcUdp.pszText = _T("本地UDP连接");
	listViewUdp.InsertColumn(listViewUdp.GetHeader().GetItemCount(), &lvcUdp);

	SendMessage(WM_COMMAND, IDOK);

	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	// if UI is the last thread, no need to wait
	if(_Module.GetLockCount() == 1)
	{
		_Module.m_dwTimeOut = 0L;
		_Module.m_dwPause = 0L;
	}
	_Module.Unlock();

	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 

	CProcessConnection pc;
	DWORD dwPid = GetDlgItemInt(IDC_EDIT_PID);
	if (dwPid <= (0L))
	{
		dwPid = (-1L);
	}
	pc.GetInfo2(dwPid);

	USES_CONVERSION;
	
	CListViewCtrl listViewTcp = (CListViewCtrl)GetDlgItem(IDC_LIST_TCP);

	listViewTcp.DeleteAllItems();
	for (auto it : pc.m_tcp_list)
	{
		LV_ITEM lvi = { 0 };
		_TCHAR tzPid[64] = { 0 };

		lvi.iItem = listViewTcp.GetItemCount();
		lvi.iSubItem = 0;
		lvi.mask = LVIF_TEXT;

		listViewTcp.AddItem(lvi.iItem, lvi.iSubItem, A2W(it.czProcessName));

		lvi.iSubItem++;
		wsprintf(tzPid, _T("%ld"), it.dwProcessId);
		lvi.pszText = tzPid;
		listViewTcp.SetItem(&lvi);

		lvi.iSubItem++;
		lvi.pszText = A2W(it.szLocalPort);
		listViewTcp.SetItem(&lvi);

		lvi.iSubItem++;
		lvi.pszText = A2W(it.szRemotePort);
		listViewTcp.SetItem(&lvi);

		lvi.iSubItem++;
		lvi.pszText = A2W(it.szStatus);
		listViewTcp.SetItem(&lvi);
	}

	CListViewCtrl listViewUdp = (CListViewCtrl)GetDlgItem(IDC_LIST_UDP);
	listViewUdp.DeleteAllItems();
	for (auto it : pc.m_udp_list)
	{
		LV_ITEM lvi = { 0 };
		_TCHAR tzPid[64] = { 0 };

		lvi.iItem = listViewUdp.GetItemCount();
		lvi.iSubItem = 0;
		lvi.mask = LVIF_TEXT;
		
		listViewUdp.AddItem(lvi.iItem, lvi.iSubItem, A2W(it.czProcessName));

		lvi.iSubItem++;
		wsprintf(tzPid, _T("%ld"), it.dwProcessId);
		lvi.pszText = tzPid;
		listViewUdp.SetItem(&lvi);

		lvi.iSubItem++;
		lvi.pszText = A2W(it.szLocalPort);
		listViewUdp.SetItem(&lvi);
	}

	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
#if defined(DEBUG) || defined(_DEBUG)
	CloseConsole();
#endif
	DestroyWindow();
	::PostQuitMessage(nVal);
}


LRESULT CMainDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	RECT rcWindow = { 0 };
	RECT rc = { 0 };
	int nLastBottom = 0;
	int nLastLeft = 0;

	GetClientRect(&rcWindow);

	GetDlgItem(IDOK).GetClientRect(&rc);
	rc.top = 4;
	rc.bottom = rc.top + rc.bottom;
	rc.left = rcWindow.right - rc.right - 4;
	rc.right = rcWindow.right - 4;
	GetDlgItem(IDC_STATIC_PID).MoveWindow(&rc, FALSE);
	nLastBottom = rc.bottom;
	nLastLeft = rc.left;

	GetDlgItem(IDOK).GetClientRect(&rc);
	rc.top = 4 + nLastBottom;
	rc.bottom = rc.top + rc.bottom;
	rc.left = rcWindow.right - rc.right - 4;
	rc.right = rcWindow.right - 4;
	GetDlgItem(IDC_EDIT_PID).MoveWindow(&rc, FALSE);
	nLastBottom = rc.bottom;

	GetDlgItem(IDOK).GetClientRect(&rc);
	rc.top = 4 + nLastBottom;
	rc.bottom = rc.top + rc.bottom;
	rc.left = rcWindow.right - rc.right - 4;
	rc.right = rcWindow.right - 4;
	GetDlgItem(IDOK).MoveWindow(&rc, FALSE);
	nLastBottom = rc.bottom;

	GetDlgItem(IDCANCEL).GetClientRect(&rc);
	rc.top = 4 + nLastBottom;
	rc.bottom = rc.top + rc.bottom;
	rc.left = rcWindow.right - rc.right - 4;
	rc.right = rcWindow.right - 4;
	GetDlgItem(IDCANCEL).MoveWindow(&rc, FALSE);
	nLastBottom = rc.bottom;

	GetDlgItem(ID_APP_ABOUT).GetClientRect(&rc);
	rc.top = 4 + nLastBottom;
	rc.bottom = rc.top + rc.bottom;
	rc.left = rcWindow.right - rc.right - 4;
	rc.right = rcWindow.right - 4;
	GetDlgItem(ID_APP_ABOUT).MoveWindow(&rc, FALSE);
	nLastBottom = rc.bottom;

	rc.left = 4;
	rc.top = 4;
	rc.right = nLastLeft - 4;
	rc.bottom = (rcWindow.bottom - 12) / 2 + rc.top;
	GetDlgItem(IDC_LIST_TCP).MoveWindow(&rc, FALSE);
	nLastBottom = rc.bottom;
	for (UINT uIdx = 0; uIdx < ((CListViewCtrl)GetDlgItem(IDC_LIST_TCP)).GetHeader().GetItemCount(); uIdx++)
	{
		((CListViewCtrl)GetDlgItem(IDC_LIST_TCP)).SetColumnWidth(uIdx, (rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL) - 4) / ((CListViewCtrl)GetDlgItem(IDC_LIST_TCP)).GetHeader().GetItemCount());
	}

	rc.left = 4;
	rc.top = 4 + nLastBottom;
	rc.right = nLastLeft - 4;
	rc.bottom = rcWindow.bottom - 4;
	GetDlgItem(IDC_LIST_UDP).MoveWindow(&rc, FALSE);
	for (UINT uIdx = 0; uIdx < ((CListViewCtrl)GetDlgItem(IDC_LIST_UDP)).GetHeader().GetItemCount(); uIdx++)
	{
		((CListViewCtrl)GetDlgItem(IDC_LIST_UDP)).SetColumnWidth(uIdx, (rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL) - 4) / ((CListViewCtrl)GetDlgItem(IDC_LIST_TCP)).GetHeader().GetItemCount());
	}

	Invalidate();
	UpdateWindow();

	return 0;
}
