/*
	Copyright (C) 2011  Stefan Sundin (recover89@gmail.com)
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
*/

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0600

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shlwapi.h>
#include <dbt.h>

//App
#define APP_NAME            L"Removable Drive Reminder"
#define APP_VERSION         "0.1"
#define APP_URL             L"http://code.google.com/p/removable-drive-reminder/"
#define APP_UPDATE_STABLE   L"http://removable-drive-reminder.googlecode.com/svn/wiki/latest-stable.txt"
#define APP_UPDATE_UNSTABLE L"http://removable-drive-reminder.googlecode.com/svn/wiki/latest-unstable.txt"

//Messages
#define WM_TRAY                WM_USER+1
#define SWM_TOGGLE             WM_APP+1
#define SWM_HIDE               WM_APP+2
#define SWM_UPDATE             WM_APP+3
#define SWM_CONFIG             WM_APP+4
#define SWM_ABOUT              WM_APP+5
#define SWM_EXIT               WM_APP+6

//Stuff missing in MinGW
#ifndef NIIF_USER
#define NIIF_USER 4
#define NIN_BALLOONSHOW        WM_USER+2
#define NIN_BALLOONHIDE        WM_USER+3
#define NIN_BALLOONTIMEOUT     WM_USER+4
#define NIN_BALLOONUSERCLICK   WM_USER+5
#endif
//Vista+ shutdown stuff
typedef BOOL WINAPI (*_ShutdownBlockReasonCreate)(HWND, LPCWSTR);
typedef BOOL WINAPI (*_ShutdownBlockReasonDestroy)(HWND);
_ShutdownBlockReasonCreate ShutdownBlockReasonCreate;
_ShutdownBlockReasonDestroy ShutdownBlockReasonDestroy;

//Boring stuff
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hinst = NULL;
HWND g_hwnd = NULL;
UINT WM_TASKBARCREATED = 0;
UINT WM_UPDATESETTINGS = 0;
UINT WM_ADDTRAY = 0;
UINT WM_HIDETRAY = 0;
UINT WM_OPENCONFIG = 0;
wchar_t inipath[MAX_PATH];
#define ENABLED() enabled

//Cool stuff
int enabled = 1;
int blocking = 0;
int on_removable = 0;
int vista = 0;
int countdown = 0;

//Timers
#define COUNTDOWN_TIMER WM_APP+1
#define DESTROY_TIMER   WM_APP+2
#define BEEP_TIMER      WM_APP+3

//Include stuff
#include "localization/strings.h"
#include "include/error.c"
#include "include/tray.c"
#include "include/update.c"
#include "config/config.c"

//Entry point
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	g_hinst = hInst;
	
	//Check command line
	if (!strcmp(szCmdLine,"-hide")) {
		hide = 1;
	}
	
	//Look for previous instance
	WM_UPDATESETTINGS = RegisterWindowMessage(L"UpdateSettings");
	WM_OPENCONFIG = RegisterWindowMessage(L"OpenConfig");
	WM_ADDTRAY = RegisterWindowMessage(L"AddTray");
	WM_HIDETRAY = RegisterWindowMessage(L"HideTray");
	HWND previnst = FindWindow(APP_NAME, NULL);
	if (previnst != NULL) {
		PostMessage(previnst, WM_UPDATESETTINGS, 0, 0);
		if (!hide) {
			PostMessage(previnst, WM_OPENCONFIG, 0, 0);
		}
		PostMessage(previnst, (hide?WM_HIDETRAY:WM_ADDTRAY), 0, 0);
		return 0;
	}
	
	//Create window
	WNDCLASSEX wnd = {sizeof(WNDCLASSEX), 0, WindowProc, 0, 0, hInst, NULL, NULL, (HBRUSH)(COLOR_WINDOW+1), NULL, APP_NAME, NULL};
	RegisterClassEx(&wnd);
	g_hwnd = CreateWindowEx(0, wnd.lpszClassName, APP_NAME, 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
	
	//Load settings
	GetModuleFileName(NULL, inipath, sizeof(inipath)/sizeof(wchar_t));
	PathRemoveFileSpec(inipath);
	wcscat(inipath, L"\\"APP_NAME".ini");
	wchar_t txt[10];
	GetPrivateProfileString(APP_NAME, L"Language", L"en-US", txt, sizeof(txt)/sizeof(wchar_t), inipath);
	int i;
	for (i=0; languages[i].code != NULL; i++) {
		if (!wcsicmp(txt,languages[i].code)) {
			l10n = languages[i].strings;
			break;
		}
	}
	
	//Tray icon
	InitTray();
	UpdateTray();
	
	//Associate icon to make it appear in the Vista+ shutdown dialog
	HICON app_icon = LoadImage(g_hinst, L"app_icon", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
	SendMessage(g_hwnd, WM_SETICON, ICON_BIG, (LPARAM)app_icon);
	
	//Check if we are in Vista+ and load functions if we are
	OSVERSIONINFO vi = { sizeof(OSVERSIONINFO) };
	GetVersionEx(&vi);
	if (vi.dwMajorVersion >= 6) {
		//Load user32.dll
		HINSTANCE user32 = GetModuleHandle(L"user32.dll");
		if (user32 == NULL) {
			Error(L"GetModuleHandle('user32.dll')", L"Failed to load user32.dll.", GetLastError(), TEXT(__FILE__), __LINE__);
		}
		else {
			//Get address to ShutdownBlockReasonCreate()
			ShutdownBlockReasonCreate = (_ShutdownBlockReasonCreate)GetProcAddress(user32, "ShutdownBlockReasonCreate");
			if (ShutdownBlockReasonCreate == NULL) {
				Error(L"GetProcAddress('ShutdownBlockReasonCreate')", L"Failed to load Vista+ specific function.", GetLastError(), TEXT(__FILE__), __LINE__);
			}
			//ShutdownBlockReasonDestroy()
			ShutdownBlockReasonDestroy = (_ShutdownBlockReasonDestroy)GetProcAddress(user32, "ShutdownBlockReasonDestroy");
			if (ShutdownBlockReasonDestroy == NULL) {
				Error(L"GetProcAddress('ShutdownBlockReasonDestroy')", L"Failed to load Vista+ specific function.", GetLastError(), TEXT(__FILE__), __LINE__);
			}
			vista = 1;
		}
	}
	
	//Make Windows query this program first
	if (SetProcessShutdownParameters(0x4FF,0) == 0) {
		Error(L"SetProcessShutdownParameters(0x4FF)", L"This means that programs started before "APP_NAME" will be closed before the shutdown can be stopped.", GetLastError(), TEXT(__FILE__), __LINE__);
	}
	
	//Check if we reside on a removable drive
	wsprintf(txt, L"%c:\\", inipath[0]); // "X:\"
	if (GetDriveType(txt) == DRIVE_REMOVABLE) {
		on_removable = 1;
	}
	
	//Register for safely remove hardware events
	if (on_removable) {
		wsprintf(txt, L"\\\\.\\%c:", inipath[0]); // "\\.\X:"
		HANDLE vol = CreateFile(txt, 0, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (vol == INVALID_HANDLE_VALUE) {
			#ifdef DEBUG
			Error(L"CreateFile()", L"Failed to register for safely remove hardware events.", GetLastError(), TEXT(__FILE__), __LINE__);
			#endif
		}
		else {
			DEV_BROADCAST_HANDLE dev = { sizeof(DEV_BROADCAST_HANDLE), DBT_DEVTYP_HANDLE, 0, vol };
			HDEVNOTIFY devnotify = RegisterDeviceNotification(g_hwnd, &dev, DEVICE_NOTIFY_WINDOW_HANDLE);
			#ifdef DEBUG
			if (devnotify == NULL) {
				Error(L"RegisterDeviceNotification()", L"Failed to register for safely remove hardware events.", GetLastError(), TEXT(__FILE__), __LINE__);
			}
			#endif
			CloseHandle(vol);
		}
	}
	
	//Check for update
	GetPrivateProfileString(L"Update", L"CheckOnStartup", L"0", txt, sizeof(txt)/sizeof(wchar_t), inipath);
	if (_wtoi(txt)) {
		CheckForUpdate(0);
	}
	
	//Message loop
	MSG msg;
	while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

int CheckDrives() {
	DWORD drives = GetLogicalDrives();
	int bitmask = 1;
	wchar_t driveletters[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	wchar_t txt[30];
	GetPrivateProfileString(APP_NAME, L"IgnoreDriveLetters", L"", txt, sizeof(txt)/sizeof(wchar_t), inipath);
	
	wchar_t *letter;
	for (letter=driveletters; letter != '\0'; letter++) {
		if (drives&bitmask && wcschr(txt,*letter) == NULL) {
			wchar_t drive[] = L"X:\\";
			drive[0] = *letter;
			if (GetDriveType(drive) == DRIVE_REMOVABLE) {
				return 1;
			}
		}
		bitmask = bitmask << 1;
	}
	
	return 0;
}

void ToggleState() {
	enabled = !enabled;
	if (ENABLED()) {
		SendMessage(g_hwnd, WM_UPDATESETTINGS, 0, 0);
	}
	else {
		if (vista) {
			ShutdownBlockReasonDestroy(g_hwnd);
		}
	}
	UpdateTray();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_TRAY) {
		if (lParam == WM_LBUTTONDOWN || lParam == WM_LBUTTONDBLCLK) {
			ToggleState();
			if (lParam == WM_LBUTTONDBLCLK && !(GetAsyncKeyState(VK_SHIFT)&0x8000)) {
				SendMessage(hwnd, WM_OPENCONFIG, 0, 0);
			}
		}
		else if (lParam == WM_MBUTTONDOWN) {
			ShellExecute(NULL, L"open", inipath, NULL, NULL, SW_SHOWNORMAL);
		}
		else if (lParam == WM_RBUTTONDOWN) {
			ShowContextMenu(hwnd);
		}
		else if (lParam == NIN_BALLOONUSERCLICK) {
			hide=0;
			SendMessage(hwnd, WM_COMMAND, SWM_UPDATE, 0);
		}
		else if (lParam == NIN_BALLOONTIMEOUT) {
			if (hide) {
				RemoveTray();
			}
		}
	}
	else if (msg == WM_UPDATESETTINGS) {
		wchar_t txt[10];
		//Language
		GetPrivateProfileString(APP_NAME, L"Language", L"en-US", txt, sizeof(txt)/sizeof(wchar_t), inipath);
		int i;
		for (i=0; languages[i].code != NULL; i++) {
			if (!wcsicmp(txt,languages[i].code)) {
				l10n = languages[i].strings;
				break;
			}
		}
	}
	else if (msg == WM_ADDTRAY) {
		hide = 0;
		UpdateTray();
	}
	else if (msg == WM_HIDETRAY) {
		hide = 1;
		RemoveTray();
	}
	else if (msg == WM_OPENCONFIG && (lParam || !hide)) {
		OpenConfig(wParam);
	}
	else if (msg == WM_TASKBARCREATED) {
		tray_added = 0;
		UpdateTray();
	}
	else if (msg == WM_COMMAND) {
		int wmId=LOWORD(wParam), wmEvent=HIWORD(wParam);
		if (wmId == SWM_TOGGLE) {
			ToggleState();
		}
		else if (wmId == SWM_HIDE) {
			hide = 1;
			RemoveTray();
		}
		else if (wmId == SWM_UPDATE) {
			if (MessageBox(NULL, l10n->update_dialog, APP_NAME, MB_ICONINFORMATION|MB_YESNO|MB_SYSTEMMODAL) == IDYES) {
				ShellExecute(NULL, L"open", APP_URL, NULL, NULL, SW_SHOWNORMAL);
			}
		}
		else if (wmId == SWM_CONFIG) {
			SendMessage(hwnd, WM_OPENCONFIG, 0, 0);
		}
		else if (wmId == SWM_ABOUT) {
			SendMessage(hwnd, WM_OPENCONFIG, 2, 0);
		}
		else if (wmId == SWM_EXIT) {
			DestroyWindow(hwnd);
		}
	}
	else if (msg == WM_DESTROY) {
		showerror = 0;
		RemoveTray();
		PostQuitMessage(0);
	}
	else if (msg == WM_QUERYENDSESSION && !(GetAsyncKeyState(VK_SHIFT)&0x8000)) {
		if (ENABLED() && CheckDrives()) {
			blocking = 1;
			hide = 0;
			UpdateTray();
			SetTimer(hwnd, BEEP_TIMER, 10, NULL);
			
			if (vista) {
				countdown = 31;
				SendMessage(hwnd, WM_TIMER, COUNTDOWN_TIMER, 0);
				SetTimer(hwnd, COUNTDOWN_TIMER, 1000, NULL);
				return FALSE;
			}
			
			//WinXP
			SetTimer(hwnd, DESTROY_TIMER, 4000, NULL);
			MessageBox(NULL, l10n->reminder, APP_NAME, MB_ICONWARNING|MB_OK);
			return TRUE;
		}
		else {
			if (vista) {
				ShutdownBlockReasonDestroy(hwnd);
			}
			return TRUE;
		}
	}
	else if (msg == WM_ENDSESSION && wParam == FALSE) {
		//The log off was aborted
		KillTimer(hwnd, COUNTDOWN_TIMER);
		blocking = 0;
		if (vista) {
			ShutdownBlockReasonDestroy(hwnd);
		}
	}
	else if (msg == WM_TIMER) {
		if (wParam == COUNTDOWN_TIMER) {
			countdown--;
			if (countdown == 0) {
				KillTimer(hwnd, COUNTDOWN_TIMER);
				ShutdownBlockReasonCreate(hwnd, l10n->oops);
				SetTimer(hwnd, DESTROY_TIMER, 3000, NULL);
			}
			else {
				wchar_t txt[100];
				wsprintf(txt, L"(%d) %s", countdown, l10n->reminder);
				ShutdownBlockReasonCreate(hwnd, txt);
				CheckDrives();
			}
		}
		else if (wParam == DESTROY_TIMER) {
			//We use a timer for this so the goodbye message can be seen.
			DestroyWindow(hwnd);
		}
		else if (wParam == BEEP_TIMER) {
			KillTimer(hwnd, BEEP_TIMER);
			wchar_t txt[MAX_PATH];
			GetPrivateProfileString(APP_NAME, L"PlaySound", L"", txt, sizeof(txt)/sizeof(wchar_t), inipath);
			if (wcslen(txt) > 0) {
				PlaySound(txt, NULL, SND_FILENAME);
			}
		}
	}
	else if (msg == WM_DEVICECHANGE && wParam == DBT_DEVICEREMOVECOMPLETE && ENABLED() && blocking) {
		KillTimer(hwnd, COUNTDOWN_TIMER);
		ShutdownBlockReasonCreate(hwnd, l10n->gotit);
		SetTimer(hwnd, DESTROY_TIMER, 3000, NULL);
	}
	else if (msg == WM_DEVICECHANGE && wParam == DBT_DEVICEQUERYREMOVE) {
		//Quickly exit if the user uses the safely remove hardware tray icon
		showerror = 0;
		RemoveTray();
		TerminateProcess(GetCurrentProcess(), 0);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
