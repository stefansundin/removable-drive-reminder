;AltDrag - en-US localization by Stefan Sundin (recover89@gmail.com)
;
;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.

!insertmacro MUI_LANGUAGE "English" ;English name of this language
!define LANG ${LANG_ENGLISH}

LangString L10N_UPGRADE_TITLE     ${LANG} "Already Installed"
LangString L10N_UPGRADE_SUBTITLE  ${LANG} "Choose how you want to install ${APP_NAME}."
LangString L10N_UPGRADE_HEADER    ${LANG} "${APP_NAME} is already installed on this system. Select the operation you want to perform and click Next to continue."
LangString L10N_UPGRADE_UPGRADE   ${LANG} "&Upgrade ${APP_NAME} to ${APP_VERSION}."
LangString L10N_UPGRADE_INI       ${LANG} "Your existing settings will be copied to ${APP_NAME}-old.ini."
LangString L10N_UPGRADE_INSTALL   ${LANG} "&Install to a new location."
LangString L10N_UPGRADE_UNINSTALL ${LANG} "&Uninstall ${APP_NAME}."

LangString L10N_LOCATION_TITLE    ${LANG} "Install Location"
LangString L10N_LOCATION_SUBTITLE ${LANG} "Choose where you want to install ${APP_NAME}."
LangString L10N_LOCATION_HEADER   ${LANG} "${APP_NAME} can be installed either on a flash drive or on the system. Select the operation you want to perform and click Next to continue."
LangString L10N_LOCATION_FLASH    ${LANG} "Install to a &flash drive."
LangString L10N_LOCATION_FLASH2   ${LANG} "This will help you remember your flash drive wherever you go."
LangString L10N_LOCATION_FLASH3   ${LANG} "Drive:"
LangString L10N_LOCATION_SYSTEM   ${LANG} "Install to the &system."
LangString L10N_LOCATION_SYSTEM2  ${LANG} "This is usable for system administrators. Be sure to read the wiki to learn how to do easy deployment."

LangString L10N_UPDATE_SECTION    ${LANG} "Check for update before installing"
LangString L10N_UPDATE_DIALOG     ${LANG} "A new version is available.$\nAbort install and go to website?"
LangString L10N_SHORTCUT          ${LANG} "Start Menu Shortcut"
LangString L10N_AUTOSTART         ${LANG} "Autostart"
LangString L10N_AUTOSTART_HIDE    ${LANG} "Hide tray"

!undef LANG
