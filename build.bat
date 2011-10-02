@echo off

:: For traditional MinGW, set prefix32 to empty string
:: For mingw-w32, set prefix32 to i686-w64-mingw32-
:: For mingw-w64, set prefix64 to x86_64-w64-mingw32-

set prefix32=i686-w64-mingw32-
set prefix64=x86_64-w64-mingw32-
set l10n=en-US

taskkill /IM "Removable Drive Reminder.exe"

if not exist build. mkdir build

if "%1" == "all" (
	%prefix32%windres include\rdr.rc build\rdr.o
	%prefix32%gcc -o build\ini.exe include\ini.c -lshlwapi
	
	@echo.
	echo Building release build
	%prefix32%gcc -o "build\Removable Drive Reminder.exe" rdr.c build\rdr.o -mwindows -lshlwapi -lwininet -lcomctl32 -O2 -s
	if not exist "build\Removable Drive Reminder.exe". exit /b
	
	if "%2" == "x64" (
		if not exist "build\x64". mkdir "build\x64"
		%prefix64%windres -o build\x64\rdr.o include\rdr.rc
		%prefix64%gcc -o "build\x64\Removable Drive Reminder.exe" rdr.c build\x64\rdr.o -mwindows -lshlwapi -lwininet -lcomctl32 -O2 -s
		if not exist "build\x64\Removable Drive Reminder.exe". exit /b
	)
	
	for %%f in (%l10n%) do (
		@echo.
		echo Putting together %%f
		if not exist "build\%%f\Removable Drive Reminder". mkdir "build\%%f\Removable Drive Reminder"
		copy "build\Removable Drive Reminder.exe" "build\%%f\Removable Drive Reminder"
		copy "Removable Drive Reminder.ini" "build\%%f\Removable Drive Reminder"
		build\ini "build\%%f\Removable Drive Reminder\Removable Drive Reminder.ini" "Removable Drive Reminder" Language %%f
		if "%2" == "x64" (
			if not exist "build\x64\%%f\Removable Drive Reminder". mkdir "build\x64\%%f\Removable Drive Reminder"
			copy "build\x64\Removable Drive Reminder.exe" "build\x64\%%f\Removable Drive Reminder"
			copy "build\%%f\Removable Drive Reminder\Removable Drive Reminder.ini" "build\x64\%%f\Removable Drive Reminder"
		)
	)
	
	@echo.
	echo Building installer
	if "%2" == "x64" (
		makensis /V2 /Dx64 installer.nsi
	) else (
		makensis /V2 installer.nsi
	)
) else if "%1" == "x64" (
	if not exist "build\x64". mkdir "build\x64"
	%prefix64%windres -o build\x64\rdr.o include\rdr.rc
	%prefix64%gcc -o "Removable Drive Reminder.exe" rdr.c build\x64\rdr.o -mwindows -lshlwapi -lwininet -lcomctl32 -g -DDEBUG
) else (
	%prefix32%windres include\rdr.rc build\rdr.o
	%prefix32%gcc -o "Removable Drive Reminder.exe" rdr.c build\rdr.o -mwindows -lshlwapi -lwininet -lcomctl32 -g -DDEBUG
	
	if "%1" == "run" (
		start "" "Removable Drive Reminder.exe"
	)
)
