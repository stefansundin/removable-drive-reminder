;Copyright (C) 2011  Stefan Sundin (recover89@gmail.com)
;
;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.

;Requires AccessControl plug-in
;http://nsis.sourceforge.net/AccessControl_plug-in

!define APP_NAME      "Removable Drive Reminder"
!define APP_VERSION   "0.1"
!define APP_URL       "http://code.google.com/p/removable-drive-reminder/"
!define APP_UPDATEURL "http://removable-drive-reminder.googlecode.com/svn/wiki/latest-stable.txt"

; Libraries

!include "MUI2.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "StrFunc.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"
${StrLoc}

; General

Name "${APP_NAME} ${APP_VERSION}"
OutFile "build/${APP_NAME}-${APP_VERSION}.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
InstallDirRegKey HKLM "Software\${APP_NAME}" "Install_Dir"
RequestExecutionLevel admin
ShowInstDetails hide
ShowUninstDetails show
SetCompressor /SOLID lzma

; Interface

!define MUI_LANGDLL_REGISTRY_ROOT "HKLM" 
!define MUI_LANGDLL_REGISTRY_KEY "Software\${APP_NAME}" 
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"

!define MUI_COMPONENTSPAGE_NODESC

!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION "Launch"

; Pages

Page custom PageLocation PageLocationLeave
Page custom PageUpgrade PageUpgradeLeave
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipPage
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipPage
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Variables

Var UpgradeState
Var LocationState
Var AutostartSectionState

; Languages

!include "localization\installer.nsh"
!insertmacro MUI_RESERVEFILE_LANGDLL

!macro Lang lang id
${If} $LANGUAGE == ${id}
	WriteINIStr "$INSTDIR\${APP_NAME}.ini" "${APP_NAME}" "Language" "${lang}"
${EndIf}
!macroend

; Functions

!macro AddTray un
Function ${un}AddTray
	;Add tray icon if program is running
	FindWindow $0 "${APP_NAME}" ""
	IntCmp $0 0 done
		DetailPrint "Adding tray icon."
		System::Call "user32::RegisterWindowMessage(t 'AddTray') i .r1"
		SendMessage $0 $1 0 0 /TIMEOUT=500
	done:
FunctionEnd
!macroend
!insertmacro AddTray ""
!insertmacro AddTray "un."

!macro CloseApp un
Function ${un}CloseApp
	;Close app if running
	FindWindow $0 "${APP_NAME}" ""
	IntCmp $0 0 done
		DetailPrint "Closing running ${APP_NAME}."
		SendMessage $0 ${WM_CLOSE} 0 0 /TIMEOUT=500
		waitloop:
			Sleep 10
			FindWindow $0 "${APP_NAME}" ""
			IntCmp $0 0 closed waitloop waitloop
	closed:
	Sleep 100 ;Sleep a little extra to let Windows do its thing.
	done:
FunctionEnd
!macroend
!insertmacro CloseApp ""
!insertmacro CloseApp "un."

; Install location page

Var Flashbox
Var Combobox
Var Uninstallbox

Function PageLocation
	Call HideBackButton
	
	nsDialogs::Create 1018
	!insertmacro MUI_HEADER_TEXT "$(L10N_LOCATION_TITLE)" "$(L10N_LOCATION_SUBTITLE)"
	${NSD_CreateLabel} 0 0 100% 20u "$(L10N_LOCATION_HEADER)"
	
	${NSD_CreateRadioButton} 0 45 100% 10u "$(L10N_LOCATION_FLASH)"
	Pop $Flashbox
	${NSD_Check} $Flashbox
	${NSD_OnClick} $Flashbox "UpdateNextButton"
	
	${NSD_CreateLabel} 16 62 100% 10u "$(L10N_LOCATION_FLASH2)"
	${NSD_CreateLabel} 16 83 35 10u "$(L10N_LOCATION_FLASH3)"
	
	${NSD_CreateDropList} 55 80 45 30
	Pop $Combobox
	Call RefreshDrives
	
	${NSD_CreateButton} 115 79 80 23 "$(L10N_LOCATION_REFRESH)"
	Pop $0
	${NSD_OnClick} $0 "RefreshDrives"
	
	${NSD_CreateRadioButton} 0 120 100% 10u "$(L10N_LOCATION_SYSTEM)"
	Pop $0
	${NSD_OnClick} $0 "UpdateNextButton"
	${NSD_CreateLabel} 16 137 100% 20u "$(L10N_LOCATION_SYSTEM2)"
	
	IfFileExists $INSTDIR 0 +4
		${NSD_CreateRadioButton} 0 183 100% 10u "$(L10N_UPGRADE_UNINSTALL)"
		Pop $Uninstallbox
		${NSD_OnClick} $Uninstallbox "UpdateNextButton"
	
	nsDialogs::Show
FunctionEnd

Function PageLocationLeave
	${NSD_GetState} $Uninstallbox $0
	${If} $0 == ${BST_CHECKED}
		Exec "$INSTDIR\Uninstall.exe"
		Quit
	${EndIf}
	
	${NSD_GetState} $Flashbox $LocationState
	${NSD_GetText} $Combobox $0
	${If} $LocationState == ${BST_CHECKED}
		StrCpy $INSTDIR "$0"
	${EndIf}
FunctionEnd

Function RefreshDrives
	SendMessage $Combobox ${CB_RESETCONTENT} 0 0
	${GetDrives} "FDD" "ListDrives"
	Call UpdateNextButton
FunctionEnd

Function ListDrives
	${NSD_CB_AddString} $Combobox "$9"
	${NSD_CB_SelectString} $Combobox "$9"
	Push $0
FunctionEnd

Function UpdateNextButton
	${NSD_GetState} $Flashbox $0
	${NSD_GetText} $Combobox $1
	
	GetDlgItem $3 $HWNDPARENT 1
	${If} $0 == ${BST_CHECKED}
	${AndIf} $1 == ""
		EnableWindow $3 0
	${Else}
		EnableWindow $3 1
	${EndIf}
FunctionEnd

; Detect previous installation

Var Upgradebox

Function PageUpgrade
	${If} $LocationState == ${BST_CHECKED}
		Abort
	${EndIf}
	
	IfFileExists $INSTDIR +2
		Abort
	
	nsDialogs::Create 1018
	!insertmacro MUI_HEADER_TEXT "$(L10N_UPGRADE_TITLE)" "$(L10N_UPGRADE_SUBTITLE)"
	${NSD_CreateLabel} 0 0 100% 20u "$(L10N_UPGRADE_HEADER)"
	
	${NSD_CreateRadioButton} 0 45 100% 10u "$(L10N_UPGRADE_UPGRADE)"
	Pop $Upgradebox
	${NSD_Check} $Upgradebox
	${NSD_CreateLabel} 16 62 100% 20u "$(L10N_UPGRADE_INI)"
	
	${NSD_CreateRadioButton} 0 95 100% 10u "$(L10N_UPGRADE_INSTALL)"
	
	nsDialogs::Show
FunctionEnd

Function PageUpgradeLeave
	${NSD_GetState} $Upgradebox $UpgradeState
FunctionEnd

; Installer

Section "$(L10N_UPDATE_SECTION)" sec_update
	NSISdl::download "${APP_UPDATEURL}" "$TEMP\${APP_NAME}-updatecheck"
	Pop $0
	StrCmp $0 "success" +3
		DetailPrint "Update check failed. Error: $0."
		Goto done
	FileOpen $0 "$TEMP\${APP_NAME}-updatecheck" r
	IfErrors done
	FileRead $0 $1
	FileClose $0
	Delete /REBOOTOK "$TEMP\${APP_NAME}-updatecheck"
	
	;Make sure the response is valid
	StrCpy $3 "Version: "
	StrLen $4 $3
	StrCpy $5 $1 $4
	StrCmpS $3 $5 0 done
	
	;New version available?
	StrCpy $6 $1 "" $4
	StrCmp $6 ${APP_VERSION} done
		MessageBox MB_ICONINFORMATION|MB_YESNO "$(L10N_UPDATE_DIALOG)" /SD IDNO IDNO done
			ExecShell "open" "${APP_URL}"
			Quit
	done:
SectionEnd

Section "${APP_NAME}" sec_app
	SectionIn RO
	
	;Close app if running
	Call CloseApp
	
	SetOutPath "$INSTDIR"
	
	;Rename old ini file if it exists
	IfFileExists "${APP_NAME}.ini" 0 +3
		Delete "${APP_NAME}-old.ini"
		Rename "${APP_NAME}.ini" "${APP_NAME}-old.ini"
	
	;Install files
	!ifdef x64
	${If} ${RunningX64}
		File "build\x64\${APP_NAME}.exe"
	${Else}
		File "build\${APP_NAME}.exe"
	${EndIf}
	!else
	File "build\${APP_NAME}.exe"
	!endif
	File "${APP_NAME}.ini"
	File "beep.wav"
	${If} $LocationState == ${BST_CHECKED}
		IfFileExists "autorun.inf" +2 0
			;We don't want to ruin people's autorun.inf
			File "autorun.inf"
	${EndIf}
	
	!insertmacro Lang en-US ${LANG_ENGLISH}
	
	${If} $LocationState == ${BST_UNCHECKED}
		;Update registry
		WriteRegStr HKLM "Software\${APP_NAME}" "Install_Dir" "$INSTDIR"
		WriteRegStr HKLM "Software\${APP_NAME}" "Version" "${APP_VERSION}"
		
		;Create uninstaller
		WriteUninstaller "Uninstall.exe"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" '"$INSTDIR\${APP_NAME}.exe"'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "HelpLink" "${APP_URL}"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "Stefan Sundin"
		WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
		WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
	${EndIf}
SectionEnd

Section "$(L10N_SHORTCUT)" sec_shortcut
	CreateShortCut "$SMPROGRAMS\${APP_NAME}.lnk" "$INSTDIR\${APP_NAME}.exe" "" "$INSTDIR\${APP_NAME}.exe" 0
SectionEnd

SectionGroup /e "$(L10N_AUTOSTART)"
	Section /o "$(L10N_AUTOSTART)" sec_autostart
	SectionEnd
	Section /o "$(L10N_AUTOSTART_HIDE)" sec_hide
	SectionEnd
SectionGroupEnd

Function Launch
	Exec "$INSTDIR\${APP_NAME}.exe"
FunctionEnd

;Used when upgrading to skip the components and directory pages
Function SkipPage
	${If} $UpgradeState == ${BST_CHECKED}
	${OrIf} $LocationState == ${BST_CHECKED}
		!insertmacro UnselectSection ${sec_shortcut}
		Abort
	${EndIf}
FunctionEnd

Function HideBackButton
	GetDlgItem $0 $HWNDPARENT 3
	ShowWindow $0 ${SW_HIDE}
FunctionEnd

Function .onInit
	;Detect x64
	!ifdef x64
	${If} ${RunningX64}
		SectionSetText ${sec_app} "${APP_NAME} (x64)"
		;Only set x64 installation dir if not already installed
		IfFileExists $INSTDIR +2
			StrCpy $INSTDIR "$PROGRAMFILES64\${APP_NAME}"
	${EndIf}
	!endif
	;Display language selection and add tray if program is running
	!insertmacro MUI_LANGDLL_DISPLAY
	Call AddTray
	;If silent, deselect check for update
	IfSilent 0 autostart_check
		!insertmacro UnselectSection ${sec_update}
	autostart_check:
	;Determine current autostart setting
	StrCpy $AutostartSectionState 0
	ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"
	IfErrors done
		!insertmacro SelectSection ${sec_autostart}
		${StrLoc} $0 $0 "-hide" "<"
		${If} $0 != ""
			StrCpy $AutostartSectionState 1
			!insertmacro SelectSection ${sec_hide}
		${EndIf}
	done:
FunctionEnd

Function .onSelChange
	;Hide tray automatically checks Autostart
	${If} ${SectionIsSelected} ${sec_hide}
		${If} $AutostartSectionState == 0
			StrCpy $AutostartSectionState 1
			!insertmacro SelectSection ${sec_autostart}
		${ElseIfNot} ${SectionIsSelected} ${sec_autostart}
			StrCpy $AutostartSectionState 0
			!insertmacro UnselectSection ${sec_hide}
		${EndIf}
	${Else}
		StrCpy $AutostartSectionState 0
	${EndIf}
FunctionEnd

Function .onInstSuccess
	;Set or remove autostart
	${If} ${SectionIsSelected} ${sec_hide}
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}" '"$INSTDIR\${APP_NAME}.exe" -hide'
	${ElseIf} ${SectionIsSelected} ${sec_autostart}
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}" '"$INSTDIR\${APP_NAME}.exe"'
	${Else}
		DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"
	${EndIf}
	;Run program if silent
	IfSilent 0 +2
		Call Launch
FunctionEnd

; Uninstaller

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	Call un.AddTray
FunctionEnd

Section "Uninstall"
	Call un.CloseApp

	Delete /REBOOTOK "$INSTDIR\${APP_NAME}.exe"
	Delete /REBOOTOK "$INSTDIR\${APP_NAME}.ini"
	Delete /REBOOTOK "$INSTDIR\${APP_NAME}-old.ini"
	Delete /REBOOTOK "$INSTDIR\beep.wav"
	Delete /REBOOTOK "$INSTDIR\Uninstall.exe"
	RMDir  /REBOOTOK "$INSTDIR"

	Delete /REBOOTOK "$SMPROGRAMS\${APP_NAME}.lnk"

	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"
	DeleteRegKey /ifempty HKLM "Software\${APP_NAME}"
	DeleteRegKey /ifempty HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd
