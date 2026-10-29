@echo off
rem Build with g++ (MSYS2 UCRT64). See REQUIREMENTS.md.
rem   build.bat         -> build the app into bin\
rem   build.bat test    -> also build and run the unit tests
rem   build.bat clean   -> delete bin\ (forces the libraries to be rebuilt)
rem To build AND run the app, use run.bat / run_demo.bat instead.
setlocal enabledelayedexpansion
cd /d "%~dp0"
set RESULT=0

if /I "%~1"=="clean" (
    if exist bin rmdir /s /q bin
    echo Cleaned.
    goto end
)

where g++ >nul 2>nul
if errorlevel 1 (
    echo ERROR: g++ was not found. Install MSYS2 and add C:\msys64\ucrt64\bin to PATH.
    echo See REQUIREMENTS.md
    set RESULT=1
    goto end
)

if not exist bin\obj mkdir bin\obj
if not exist data mkdir data

set INC=-Iinclude -Ithird_party\imgui -Ithird_party\imgui\backends -Ithird_party\imgui\misc\cpp -Ithird_party\implot -Ithird_party\icons
set LIBS=-mwindows -static -ld3d11 -ld3dcompiler -ldxgi -ldwmapi -lgdi32 -limm32

rem ---- 1. Libraries: compiled once and cached in bin\obj (takes ~2 minutes the first time) ----
for %%f in (
    third_party\imgui\imgui.cpp
    third_party\imgui\imgui_draw.cpp
    third_party\imgui\imgui_tables.cpp
    third_party\imgui\imgui_widgets.cpp
    third_party\imgui\backends\imgui_impl_win32.cpp
    third_party\imgui\backends\imgui_impl_dx11.cpp
    third_party\imgui\misc\cpp\imgui_stdlib.cpp
    third_party\implot\implot.cpp
    third_party\implot\implot_items.cpp
) do (
    if not exist bin\obj\%%~nf.o (
        echo Compiling library %%~nxf ...
        g++ -std=c++17 -O2 -DIMGUI_DISABLE_DEMO_WINDOWS %INC% -c %%f -o bin\obj\%%~nf.o
        if errorlevel 1 (
            echo BUILD FAILED
            set RESULT=1
            goto end
        )
    )
)

rem ---- 2. The app ----
echo Building app...
g++ -std=c++17 -O2 -Wall -Wextra %INC% src\*.cpp src\gui\*.cpp bin\obj\*.o -o bin\PersonalFinanceManager.exe %LIBS%
if errorlevel 1 (
    echo BUILD FAILED
    set RESULT=1
    goto end
)
echo Built bin\PersonalFinanceManager.exe

rem ---- 3. Unit tests (logic only, no GUI) ----
if /I "%~1"=="test" (
    set SOURCES=
    for %%f in (src\*.cpp) do if /I not "%%~nxf"=="main.cpp" set SOURCES=!SOURCES! %%f
    g++ -std=c++17 -Wall -Wextra -Iinclude tests\test_core.cpp !SOURCES! -o bin\run_tests.exe
    if errorlevel 1 (
        echo TEST BUILD FAILED
        set RESULT=1
        goto end
    )
    bin\run_tests.exe
    if errorlevel 1 set RESULT=1
)

:end
rem Keep the window open when this file was double-clicked.
echo %cmdcmdline% | find /i "%~nx0" >nul && pause
exit /b %RESULT%
