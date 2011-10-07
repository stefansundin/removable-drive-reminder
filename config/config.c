/*
	Copyright (C) 2011  Stefan Sundin (recover89@gmail.com)
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
*/

#include <commctrl.h>
#include <prsht.h>
#include <windowsx.h>

//Boring stuff
BOOL CALLBACK PropSheetProc(HWND, UINT, LPARAM);
INT_PTR CALLBACK GeneralPageDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AdvancedPageDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutPageDialogProc(HWND, UINT, WPARAM, LPARAM);
void LinkProc(HWND, UINT, WPARAM, LPARAM);
HWND g_cfgwnd = NULL;

//Include stuff
#include "resource.h"
#include "../include/autostart.c"

extern int on_removable;

//Entry point
void OpenConfig(int startpage) {
	if (IsWindow(g_cfgwnd)) {
		PropSheet_SetCurSel(g_cfgwnd, 0, startpage);
		SetForegroundWindow(g_cfgwnd);
		return;
	}
	
	//Define the pages
	struct {
		int pszTemplate;
		DLGPROC pfnDlgProc;
	} pages[] = {
		{ IDD_GENERALPAGE,   GeneralPageDialogProc },
		{ IDD_ADVANCEDPAGE,  AdvancedPageDialogProc },
		{ IDD_ABOUTPAGE,     AboutPageDialogProc },
	};
	
	PROPSHEETPAGE psp[sizeof(pages)/sizeof(pages[0])] = {0};
	int i;
	for (i=0; i < sizeof(pages)/sizeof(pages[0]); i++) {
		psp[i].dwSize      = sizeof(PROPSHEETPAGE);
		psp[i].hInstance   = g_hinst;
		psp[i].pszTemplate = MAKEINTRESOURCE(pages[i].pszTemplate);
		psp[i].pfnDlgProc  = pages[i].pfnDlgProc;
	}
	
	//Define the property sheet
	PROPSHEETHEADER psh = {0};
	psh.dwSize          = sizeof(PROPSHEETHEADER);
	psh.dwFlags         = PSH_PROPSHEETPAGE | PSH_USECALLBACK | PSH_USEHICON | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	psh.hwndParent      = NULL;
	psh.hInstance       = g_hinst;
	psh.hIcon           = LoadImage(g_hinst, L"app_icon", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
	psh.pszCaption      = APP_NAME;
	psh.nPages          = sizeof(pages)/sizeof(pages[0]);
	psh.ppsp            = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback     = PropSheetProc;
	psh.nStartPage      = startpage;
	
	//Open the property sheet
	PropertySheet(&psh);
}

void UpdateSettings() {
	PostMessage(g_hwnd, WM_UPDATESETTINGS, 0, 0);
}

void UpdateL10n() {
	//Update window title
	PropSheet_SetTitle(g_cfgwnd, 0, l10n->title);
	
	//Update tab titles
	HWND tc = PropSheet_GetTabControl(g_cfgwnd);
	int numrows_prev = TabCtrl_GetRowCount(tc);
	wchar_t *titles[] = { l10n->tab_general, l10n->tab_advanced, l10n->tab_about };
	int i;
	for (i=0; i < sizeof(titles)/sizeof(titles[0]); i++) {
		TCITEM ti;
		ti.mask = TCIF_TEXT;
		ti.pszText = titles[i];
		TabCtrl_SetItem(tc, i, &ti);
	}
}

BOOL CALLBACK PropSheetProc(HWND hwnd, UINT msg, LPARAM lParam) {
	if (msg == PSCB_INITIALIZED) {
		g_cfgwnd = hwnd;
		UpdateL10n();
		
		//OK button replaces Cancel button
		SendMessage(g_cfgwnd, PSM_CANCELTOCLOSE, 0, 0);
		EnableWindow(GetDlgItem(g_cfgwnd,IDCANCEL), TRUE); //Re-enable to enable escape key
		WINDOWPLACEMENT wndpl;
		wndpl.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(GetDlgItem(g_cfgwnd,IDCANCEL), &wndpl);
		SetWindowPlacement(GetDlgItem(g_cfgwnd,IDOK), &wndpl);
		ShowWindow(GetDlgItem(g_cfgwnd,IDCANCEL), SW_HIDE);
	}
}

INT_PTR CALLBACK GeneralPageDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int updatel10n = 0;
	if (msg == WM_INITDIALOG) {
		wchar_t txt[30];
		GetPrivateProfileString(APP_NAME, L"PlaySound", L"", txt, sizeof(txt)/sizeof(wchar_t), inipath);
		Button_SetCheck(GetDlgItem(hwnd,IDC_PLAYSOUND), wcslen(txt)>0?BST_CHECKED:BST_UNCHECKED);
		GetPrivateProfileString(APP_NAME, L"IgnoreDriveLetters", L"", txt, sizeof(txt)/sizeof(wchar_t), inipath);
		SetDlgItemText(hwnd, IDC_IGNORE, txt);
		
		int i;
		for (i=0; languages[i].code != NULL; i++) {
			ComboBox_AddString(GetDlgItem(hwnd,IDC_LANGUAGE), languages[i].strings->lang);
			if (l10n == languages[i].strings) {
				ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_LANGUAGE), i);
			}
		}
	}
	else if (msg == WM_COMMAND) {
		int id = LOWORD(wParam);
		int event = HIWORD(wParam);
		HWND control = GetDlgItem(hwnd, id);
		int val = Button_GetCheck(control);
		
		if (id == IDC_PLAYSOUND) {
			WritePrivateProfileString(APP_NAME, L"PlaySound", val?L"beep.wav":L"", inipath);
		}
		else if (id == IDC_IGNORE && event == EN_KILLFOCUS) {
			wchar_t txt[30];
			Edit_GetText(control, txt, sizeof(txt)/sizeof(wchar_t));
			WritePrivateProfileString(APP_NAME, L"IgnoreDriveLetters", txt, inipath);
		}
		else if (id == IDC_LANGUAGE && event == CBN_SELCHANGE) {
			int i = ComboBox_GetCurSel(control);
			if (languages[i].code == NULL) {
				ShellExecute(NULL, L"open", L"http://code.google.com/p/removable-drive-reminder/wiki/Translate", NULL, NULL, SW_SHOWNORMAL);
				for (i=0; l10n != languages[i].strings; i++) {}
				ComboBox_SetCurSel(control, i);
			}
			else {
				l10n = languages[i].strings;
				WritePrivateProfileString(APP_NAME, L"Language", languages[i].code, inipath);
				updatel10n = 1;
				UpdateL10n();
			}
		}
		else if (id == IDC_AUTOSTART) {
			SetAutostart(val, 0);
			EnableWindow(GetDlgItem(hwnd,IDC_AUTOSTART_HIDE), val);
			if (!val) {
				Button_SetCheck(GetDlgItem(hwnd,IDC_AUTOSTART_HIDE), BST_UNCHECKED);
			}
		}
		else if (id == IDC_AUTOSTART_HIDE) {
			SetAutostart(1, val);
		}
		UpdateSettings();
	}
	else if (msg == WM_NOTIFY) {
		LPNMHDR pnmh = (LPNMHDR)lParam;
		if (pnmh->code == PSN_SETACTIVE) {
			updatel10n = 1;
			
			//Autostart
			int autostart=0, hidden=0;
			CheckAutostart(&autostart, &hidden);
			Button_SetCheck(GetDlgItem(hwnd,IDC_AUTOSTART), autostart?BST_CHECKED:BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(hwnd,IDC_AUTOSTART_HIDE), hidden?BST_CHECKED:BST_UNCHECKED);
			Button_Enable(GetDlgItem(hwnd,IDC_AUTOSTART_HIDE), autostart);
			if (on_removable) {
				Button_Enable(GetDlgItem(hwnd,IDC_AUTOSTART), 0);
				Button_Enable(GetDlgItem(hwnd,IDC_AUTOSTART_HIDE), 0);
			}
		}
	}
	if (updatel10n) {
		//Update text
		SetDlgItemText(hwnd, IDC_GENERAL_BOX,         l10n->general_box);
		SetDlgItemText(hwnd, IDC_PLAYSOUND,           l10n->general_playsound);
		SetDlgItemText(hwnd, IDC_IGNORE_HEADER,       l10n->general_ignore);
		SetDlgItemText(hwnd, IDC_LANGUAGE_HEADER,     l10n->general_language);
		SetDlgItemText(hwnd, IDC_AUTOSTART_BOX,       l10n->general_autostart_box);
		SetDlgItemText(hwnd, IDC_AUTOSTART,           l10n->general_autostart);
		SetDlgItemText(hwnd, IDC_AUTOSTART_HIDE,      l10n->general_autostart_hide);
		SetDlgItemText(hwnd, IDC_AUTOSAVE,            l10n->general_autosave);
		
		//Language
		ComboBox_DeleteString(GetDlgItem(hwnd,IDC_LANGUAGE), sizeof(languages)/sizeof(languages[0])-1);
		ComboBox_AddString(GetDlgItem(hwnd,IDC_LANGUAGE), l10n->general_helptranslate);
	}
	return FALSE;
}

INT_PTR CALLBACK AdvancedPageDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_INITDIALOG) {
		wchar_t txt[10];
		GetPrivateProfileString(L"Update", L"CheckOnStartup", L"0", txt, sizeof(txt)/sizeof(wchar_t), inipath);
		Button_SetCheck(GetDlgItem(hwnd,IDC_CHECKONSTARTUP), _wtoi(txt)?BST_CHECKED:BST_UNCHECKED);
		GetPrivateProfileString(L"Update", L"Beta", L"0", txt, sizeof(txt)/sizeof(wchar_t), inipath);
		Button_SetCheck(GetDlgItem(hwnd,IDC_BETA), _wtoi(txt)?BST_CHECKED:BST_UNCHECKED);
	}
	else if (msg == WM_COMMAND) {
		if (wParam == IDC_CHECKNOW) {
			CheckForUpdate(1);
		}
		else {
			wchar_t txt[10];
			int val = Button_GetCheck(GetDlgItem(hwnd,wParam));
			if (wParam == IDC_CHECKONSTARTUP) {
				WritePrivateProfileString(L"Update", L"CheckOnStartup", _itow(val,txt,10), inipath);
			}
			else if (wParam == IDC_BETA) {
				WritePrivateProfileString(L"Update", L"Beta", _itow(val,txt,10), inipath);
			}
		}
	}
	else if (msg == WM_NOTIFY) {
		LPNMHDR pnmh = (LPNMHDR)lParam;
		if (pnmh->code == PSN_SETACTIVE) {
			//Update text
			SetDlgItemText(hwnd, IDC_ADVANCED_BOX,    l10n->advanced_box);
			SetDlgItemText(hwnd, IDC_CHECKONSTARTUP,  l10n->advanced_checkonstartup);
			SetDlgItemText(hwnd, IDC_BETA,            l10n->advanced_beta);
			SetDlgItemText(hwnd, IDC_CHECKNOW,        l10n->advanced_checknow);
		}
	}
	return FALSE;
}

INT_PTR CALLBACK AboutPageDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_COMMAND) {
		if (wParam == IDC_DONATE) {
			ShellExecute(NULL, L"open", L"http://code.google.com/p/removable-drive-reminder/wiki/Donate", NULL, NULL, SW_SHOWNORMAL);
		}
	}
	else if (msg == WM_NOTIFY) {
		LPNMHDR pnmh = (LPNMHDR)lParam;
		if (pnmh->code == PSN_SETACTIVE) {
			//Update text
			SetDlgItemText(hwnd, IDC_ABOUT_BOX,       l10n->about_box);
			SetDlgItemText(hwnd, IDC_VERSION,         l10n->about_version);
			SetDlgItemText(hwnd, IDC_AUTHOR,          l10n->about_author);
			SetDlgItemText(hwnd, IDC_LICENSE,         l10n->about_license);
			SetDlgItemText(hwnd, IDC_DONATIONS_BOX,   l10n->about_donations_box);
			SetDlgItemText(hwnd, IDC_DONATIONS_PLEA,  l10n->about_donations_plea);
			SetDlgItemText(hwnd, IDC_DONATE,          l10n->about_donate);
		}
	}
	
	LinkProc(hwnd, msg, wParam, lParam);
	return FALSE;
}

void LinkProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_NOTIFY) {
		NMLINK *link = (NMLINK*)lParam;
		if (link->hdr.code == NM_CLICK || link->hdr.code == NM_RETURN) {
			ShellExecute(NULL, L"open", link->item.szUrl, NULL, NULL, SW_SHOWDEFAULT);
		}
	}
}

