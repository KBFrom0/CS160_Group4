@echo off
rem Build, then run the app.
cd /d "%~dp0"
call "%~dp0build.bat"
if errorlevel 1 (
    pause
    exit /b 1
)
"%~dp0bin\PersonalFinanceManager.exe" %*
