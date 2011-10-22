;AltDrag - sv-SE localization by Stefan Sundin (recover89@gmail.com)
;
;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.

!insertmacro MUI_LANGUAGE "Swedish"
!define LANG ${LANG_SWEDISH}

LangString L10N_UPGRADE_TITLE     ${LANG} "Redan Installerad"
LangString L10N_UPGRADE_SUBTITLE  ${LANG} "Välj hur du vill installera ${APP_NAME}."
LangString L10N_UPGRADE_HEADER    ${LANG} "${APP_NAME} är redan installerad på den här datorn. Välj vad du vill göra och klicka på Nästa för att fortsätta."
LangString L10N_UPGRADE_UPGRADE   ${LANG} "&Uppgradera ${APP_NAME} till ${APP_VERSION}."
LangString L10N_UPGRADE_INI       ${LANG} "Dina existerande inställningar kommer att kopieras till ${APP_NAME}-old.ini."
LangString L10N_UPGRADE_INSTALL   ${LANG} "&Installera till en ny plats."
LangString L10N_UPGRADE_UNINSTALL ${LANG} "&Avinstallera ${APP_NAME}."

LangString L10N_LOCATION_TITLE    ${LANG} "Installeringsplats"
LangString L10N_LOCATION_SUBTITLE ${LANG} "Välj vart du vill installera ${APP_NAME}."
LangString L10N_LOCATION_HEADER   ${LANG} "${APP_NAME} kan antingen installeras på datorn eller på ett USB minne. Välj vad du vill göra och klicka på Nästa för att fortsätta."
LangString L10N_LOCATION_SYSTEM   ${LANG} "Installera på &datorn."
LangString L10N_LOCATION_SYSTEM2  ${LANG} "Om du är en nätverksadministratör, läs på wikin för att lära dig hur du enkelt installerar programmet i en stor skala."
LangString L10N_LOCATION_FLASH    ${LANG} "Installera på ett &USB minne."
LangString L10N_LOCATION_FLASH2   ${LANG} "Detta hjälper dig att komma ihåg USB minnet vart du än går. Kom ihåg att du måste starta programmet manuellt efter att du anslutit USB minnet."
LangString L10N_LOCATION_FLASH3   ${LANG} "Enhet:"
LangString L10N_LOCATION_REFRESH  ${LANG} "Uppdatera lista"

LangString L10N_UPDATE_DIALOG     ${LANG} "En ny version finns tillgänglig.$\nAvbryt installation och gå till hemsida?"

!undef LANG
