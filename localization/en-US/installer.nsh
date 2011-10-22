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
LangString L10N_LOCATION_HEADER   ${LANG} "${APP_NAME} can be installed either on the system or a flash drive. Select the operation you want to perform and click Next to continue."
LangString L10N_LOCATION_SYSTEM   ${LANG} "Install to the &system."
LangString L10N_LOCATION_SYSTEM2  ${LANG} "If you are a system administrator, be sure to read the wiki to learn how to easily do mass deployment."
LangString L10N_LOCATION_FLASH    ${LANG} "Install to a &flash drive."
LangString L10N_LOCATION_FLASH2   ${LANG} "This will help you remember your flash drive wherever you go. Remember that you have to start the program manually after plugging in the flash drive."
LangString L10N_LOCATION_FLASH3   ${LANG} "Drive:"
LangString L10N_LOCATION_REFRESH  ${LANG} "Refresh list"

LangString L10N_UPDATE_DIALOG     ${LANG} "A new version is available.$\nAbort install and go to website?"

!undef LANG
