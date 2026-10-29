@echo off
rem Build, then run the app. Extra arguments are passed to the app, e.g. run.bat --demo
cd /d "%~dp0"
call "%~dp0build.bat"
if errorlevel 1 (
    pause
    exit /b 1
)
"%~dp0bin\PersonalFinanceManager.exe" %*
