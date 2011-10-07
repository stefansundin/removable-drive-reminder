
struct strings {
	wchar_t *reminder;
	wchar_t *oops;
	wchar_t *gotit;
	
	/* tray */
	wchar_t *tray_enabled;
	wchar_t *tray_disabled;
	wchar_t *menu_enable;
	wchar_t *menu_disable;
	wchar_t *menu_hide;
	wchar_t *menu_update;
	wchar_t *menu_config;
	wchar_t *menu_about;
	wchar_t *menu_exit;
	
	/* update */
	wchar_t *update_balloon;
	wchar_t *update_dialog;
	wchar_t *update_nonew;
	
	/* config */
	wchar_t *lang;
	wchar_t *title;
	wchar_t *tab_general;
	wchar_t *tab_advanced;
	wchar_t *tab_about;
	wchar_t *general_box;
	wchar_t *general_playsound;
	wchar_t *general_ignore;
	wchar_t *general_language;
	wchar_t *general_helptranslate;
	wchar_t *general_autostart_box;
	wchar_t *general_autostart;
	wchar_t *general_autostart_hide;
	wchar_t *general_autosave;
	wchar_t *advanced_box;
	wchar_t *advanced_checkonstartup;
	wchar_t *advanced_beta;
	wchar_t *advanced_checknow;
	wchar_t *about_box;
	wchar_t *about_version;
	wchar_t *about_author;
	wchar_t *about_license;
	wchar_t *about_donations_box;
	wchar_t *about_donations_plea;
	wchar_t *about_donate;
};

#include "en-US/strings.h"

struct {
	wchar_t *code;
	struct strings *strings;
} languages[] = {
	{ L"en-US", &en_US },
	{ NULL }
};

struct strings *l10n = &en_US;
