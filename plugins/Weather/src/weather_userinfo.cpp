/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/*
This file contain the source that is related to display contact
information, including the one shows in user detail and the brief
information
*/

#include "stdafx.h"

//============  BRIEF INFORMATION  ============
//
static int BriefDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_HEADERBAR:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;

	case IDC_MTEXT:
	case IDC_DATALIST:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_MUPDATE:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_MTOGGLE:
	case IDC_MWEBPAGE:
	case IDCANCEL:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

// set the title of the dialog and on the which rectangle
// also load brief info into message box
static void LoadBriefInfoText(HWND hwndDlg, MCONTACT hContact)
{
	WEATHERINFO winfo;
	wchar_t str[4096];

	// load weather information from the contact into the WEATHERINFO struct
	winfo = LoadWeatherInfo(hContact);
	// check if data exist.  If not, display error message box
	if (!(BOOL)db_get_b(hContact, WEATHERPROTONAME, "IsUpdated", FALSE))
		wcsncpy(str, WEATHER_NO_INFO, _countof(str) - 1);
	else
		// set the display text and show the message box
		GetDisplay(&winfo, opt.bText, str);
	SetDlgItemText(hwndDlg, IDC_MTEXT, str);

	GetDisplay(&winfo, L"%c, %t", str);
	SetWindowText(hwndDlg, winfo.city);
	SetDlgItemText(hwndDlg, IDC_HEADERBAR, str);
}

// dialog process for more data in the user info window
// lParam = contact handle
static INT_PTR CALLBACK DlgProcMoreData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const unsigned tabstops = 48;
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		// save the contact handle for later use
		hContact = lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);

		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_AUTOURLDETECT, (WPARAM)TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETEVENTMASK, 0, ENM_LINK);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETMARGINS, EC_LEFTMARGIN, 5);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETTABSTOPS, 1, (LPARAM)&tabstops);

		// get the list to display
		{
			LV_COLUMN lvc = { 0 };
			HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);
			RECT aRect = { 0 };
			GetClientRect(hList, &aRect);

			// managing styles
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			ListView_SetExtendedListViewStyleEx(hList,
				LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP,
				LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

			// inserting columns
			lvc.cx = LIST_COLUMN;
			lvc.pszText = TranslateT("Variable");
			ListView_InsertColumn(hList, 0, &lvc);

			lvc.cx = aRect.right - LIST_COLUMN - GetSystemMetrics(SM_CXVSCROLL) - 3;
			lvc.pszText = TranslateT("Information");
			ListView_InsertColumn(hList, 1, &lvc);

			// inserting data
			SendMessage(hwndDlg, WM_UPDATEDATA, 0, 0);
		}
		TranslateDialogDefault(hwndDlg);

		// prevent dups of the window
		WindowList_Add(hDataWindowList, hwndDlg, hContact);

		// restore window position
		Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, WEATHERPROTONAME, "BriefInfo_");
		return TRUE;

	case WM_UPDATEDATA:
		ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_DATALIST));
		LoadBriefInfoText(hwndDlg, hContact);
		DBDataManage(hContact, WDBM_DETAILDISPLAY, (WPARAM)hwndDlg, 0);

		// set icons
		Window_FreeIcon_IcoLib(hwndDlg);
		Window_SetProtoIcon_IcoLib(hwndDlg, WEATHERPROTONAME, db_get_w(hContact, WEATHERPROTONAME, "StatusIcon", 0));

		RedrawWindow(GetDlgItem(hwndDlg, IDC_HEADERBAR), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case WM_SIZE:
		{
			RECT rc;
			HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);
			GetWindowRect(hList, &rc);
			ListView_SetColumnWidth(hList, 1, ListView_GetColumnWidth(hList, 1) + (int)LOWORD(lParam) - (rc.right - rc.left));

			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_BRIEF), BriefDlgResizer);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;

			// The minimum width in points
			mmi->ptMinTrackSize.x = 350;
			// The minimum height in points
			mmi->ptMinTrackSize.y = 300;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			// close the info window
			DestroyWindow(hwndDlg);
			break;

		case IDC_MUPDATE:
			{
				HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);

				// update current data
				// set the text to "updating"
				SetDlgItemText(hwndDlg, IDC_MTEXT, TranslateT("Retrieving new data, please wait..."));
				ListView_DeleteAllItems(hList);

				LV_ITEM lvi = { 0 };
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = 1;
				lvi.pszText = L"";
				lvi.iItem = ListView_InsertItem(hList, &lvi);
				lvi.pszText = TranslateT("Retrieving new data, please wait...");
				ListView_SetItemText(hList, lvi.iItem, 1, lvi.pszText);
				UpdateSingleStation(hContact, 0);
				break;
			}

		case IDC_MWEBPAGE:
			LoadForecast(hContact, 0);	// read complete forecast
			break;

		case IDC_MTOGGLE:
			if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_DATALIST)))
				SetDlgItemText(hwndDlg, IDC_MTOGGLE, TranslateT("More Info"));
			else
				SetDlgItemText(hwndDlg, IDC_MTOGGLE, TranslateT("Brief Info"));
			ShowWindow(GetDlgItem(hwndDlg, IDC_DATALIST), (int)!IsWindowVisible(
				GetDlgItem(hwndDlg, IDC_DATALIST)));
			ShowWindow(GetDlgItem(hwndDlg, IDC_MTEXT), (int)!IsWindowVisible(GetDlgItem(hwndDlg, IDC_MTEXT)));
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			if (pNmhdr->idFrom == IDC_MTEXT && pNmhdr->code == EN_LINK) {
				ENLINK *enlink = (ENLINK *)lParam;
				switch (enlink->msg) {
				case WM_LBUTTONUP:
					TEXTRANGE tr;
					tr.chrg = enlink->chrg;
					tr.lpstrText = (LPTSTR)mir_alloc(sizeof(wchar_t)*(tr.chrg.cpMax - tr.chrg.cpMin + 8));
					SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					Utils_OpenUrlW(tr.lpstrText);
					mir_free(tr.lpstrText);
					break;
				}
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Utils_SaveWindowPosition(hwndDlg, NULL, WEATHERPROTONAME, "BriefInfo_");
		WindowList_Remove(hDataWindowList, hwndDlg);
		break;
	}

	return FALSE;
}

// dialog process for the weather tab under user info
// lParam = current contact
static INT_PTR CALLBACK DlgProcUIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WEATHERINFO w;
	wchar_t str[MAX_TEXT_SIZE];

	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_MOREDETAIL, BUTTONSETASFLATBTN, TRUE, 0);
		// save the contact handle for later use
		hContact = lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);
		// load weather info for the contact
		w = LoadWeatherInfo(lParam);
		SetDlgItemText(hwndDlg, IDC_INFO1, GetDisplay(&w, TranslateT("Current condition for %n"), str));

		SendDlgItemMessage(hwndDlg, IDC_INFOICON, STM_SETICON,
			(WPARAM)Skin_LoadProtoIcon(WEATHERPROTONAME,
				db_get_w(hContact, WEATHERPROTONAME, "StatusIcon", 0)), 0);

		{	// bold and enlarge the current condition
			LOGFONT lf;
			HFONT hNormalFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_INFO2, WM_GETFONT, 0, 0);
			GetObject(hNormalFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			lf.lfWidth = 7;
			lf.lfHeight = 15;
			SendDlgItemMessage(hwndDlg, IDC_INFO2, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);
		}
		// set the text for displaying other current weather conditions data
		GetDisplay(&w, L"%c     %t", str);
		SetDlgItemText(hwndDlg, IDC_INFO2, str);
		SetDlgItemText(hwndDlg, IDC_INFO3, w.feel);
		SetDlgItemText(hwndDlg, IDC_INFO4, w.pressure);
		GetDisplay(&w, L"%i  %w", str);
		SetDlgItemText(hwndDlg, IDC_INFO5, str);
		SetDlgItemText(hwndDlg, IDC_INFO6, w.dewpoint);
		SetDlgItemText(hwndDlg, IDC_INFO7, w.sunrise);
		SetDlgItemText(hwndDlg, IDC_INFO8, w.sunset);
		SetDlgItemText(hwndDlg, IDC_INFO9, w.high);
		SetDlgItemText(hwndDlg, IDC_INFO10, w.low);
		GetDisplay(&w, TranslateT("Last update on:   %u"), str);
		SetDlgItemText(hwndDlg, IDC_INFO11, str);
		SetDlgItemText(hwndDlg, IDC_INFO12, w.humid);
		SetDlgItemText(hwndDlg, IDC_INFO13, w.vis);
		break;

	case WM_DESTROY:
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(hwndDlg, IDC_INFOICON, STM_SETICON, 0, 0));
		DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_INFO2, WM_GETFONT, 0, 0));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MOREDETAIL:
			HWND hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
			if (hMoreDataDlg == nullptr)
				hMoreDataDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BRIEF), nullptr, DlgProcMoreData, hContact);
			else {
				SetForegroundWindow(hMoreDataDlg);
				SetFocus(hMoreDataDlg);
			}
			ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 0);
			ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 1);
		}
		break;
	}
	return 0;
}

//============  CONTACT INFORMATION  ============
//
// initialize user info
// lParam = current contact
int UserInfoInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_plugin.getInst();
	odp.position = 100000000;
	odp.szTitle.a = WEATHERPROTONAME;

	if (lParam == 0) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO);
		odp.pfnDlgProc = DlgProcINIPage;
		g_plugin.addUserInfo(wParam, &odp);
	}
	else {
		// check if it is a weather contact
		if (IsMyContact(lParam)) {
			// register the contact info page
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_USERINFO);
			odp.pfnDlgProc = DlgProcUIPage;
			odp.flags = ODPF_BOLDGROUPS;
			g_plugin.addUserInfo(wParam, &odp);
		}
	}
	return 0;
}


// show brief information dialog
// wParam = current contact
int BriefInfo(WPARAM wParam, LPARAM)
{
	// make sure that the contact is actually a weather one
	if (IsMyContact(wParam)) {
		HWND hMoreDataDlg = WindowList_Find(hDataWindowList, wParam);
		if (hMoreDataDlg != nullptr) {
			SetForegroundWindow(hMoreDataDlg);
			SetFocus(hMoreDataDlg);
		}
		else hMoreDataDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_BRIEF), nullptr, DlgProcMoreData, (LPARAM)wParam);

		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 0);
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 1);
		SetDlgItemText(hMoreDataDlg, IDC_MTOGGLE, TranslateT("More Info"));
		return 1;
	}
	return 0;
}

INT_PTR BriefInfoSvc(WPARAM wParam, LPARAM lParam)
{
	return BriefInfo(wParam, lParam);
}
