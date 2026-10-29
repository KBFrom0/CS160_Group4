# Requirements

What every team member (and the grader) needs to build and run the app.

## 1. Operating system

- **Windows 10 or 11, 64-bit.** The app uses Win32 + DirectX 11, which are part of Windows.
  Nothing extra to install for graphics.

## 2. C++ compiler: MSYS2 (UCRT64) g++

The only thing you must install.

1. Download and run the installer from <https://www.msys2.org> (keep the default folder `C:\msys64`).
2. Open **"MSYS2 UCRT64"** from the Start menu and run:
   ```bash
   pacman -Syu
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc
   ```
3. Add `C:\msys64\ucrt64\bin` to the Windows **PATH**
   (Start → "Edit the system environment variables" → Environment Variables → Path → New).
4. Open a **new** terminal and check:
   ```bat
   g++ --version
   ```
   Any g++ with C++17 support works (tested with g++ 16.2).

## 3. Git

- [GitHub Desktop](https://desktop.github.com) or [Git for Windows](https://git-scm.com).
- Each member commits with **their own GitHub account** (see `CONTRIBUTING.md`).

## 4. Optional

- **VS Code** with the *C/C++* extension, compiler path `C:/msys64/ucrt64/bin/g++.exe`.
- **CMake** (only if you prefer it over `build.bat`).

## Already included in the repo (no install needed)

| Library | Version | Used for | License |
|---------|---------|----------|---------|
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.92.9b | Windows, buttons, tables, input fields | MIT |
| [ImPlot](https://github.com/epezent/implot) | 1.0 | Bar and donut charts | MIT |
| [Font Awesome Free](https://fontawesome.com) | 6.7.2 | Icons (`assets/fonts/fa-solid-900.ttf`) | SIL OFL 1.1 |
| [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders) | - | `ICON_FA_...` names | zlib |

The text font is **Segoe UI**, which ships with Windows.

These libraries only draw the interface. They do not store our data: all lists in our
code still use our own `DynamicArray<T>` (no STL containers).

## Build

```bat
build.bat          :: first time ~2 minutes (libraries), then a few seconds
run_demo.bat       :: build + open the app with sample data
```
