@echo off
rem Quick build without CMake. Requires g++ in PATH (MinGW / MSYS2).
rem   build.bat        -> build the app into bin\
rem   build.bat test   -> also build and run the unit tests
setlocal enabledelayedexpansion
cd /d "%~dp0"
if not exist bin mkdir bin
if not exist data mkdir data

g++ -std=c++17 -Wall -Wextra -Iinclude src\*.cpp -o bin\PersonalFinanceManager.exe
if errorlevel 1 exit /b 1
echo Built bin\PersonalFinanceManager.exe

if /I "%~1"=="test" (
    set LIBS=
    for %%f in (src\*.cpp) do if /I not "%%~nxf"=="main.cpp" set LIBS=!LIBS! %%f
    g++ -std=c++17 -Wall -Wextra -Iinclude tests\test_core.cpp !LIBS! -o bin\run_tests.exe
    if errorlevel 1 exit /b 1
    bin\run_tests.exe
)
