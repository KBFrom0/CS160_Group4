cmake_minimum_required(VERSION 3.10)
project(PersonalFinanceManager CXX)

# Windows only (Win32 + DirectX 11). See REQUIREMENTS.md.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI third_party/imgui)
add_library(gui_libs STATIC
    ${IMGUI}/imgui.cpp ${IMGUI}/imgui_draw.cpp ${IMGUI}/imgui_tables.cpp ${IMGUI}/imgui_widgets.cpp
    ${IMGUI}/backends/imgui_impl_win32.cpp ${IMGUI}/backends/imgui_impl_dx11.cpp
    ${IMGUI}/misc/cpp/imgui_stdlib.cpp
    third_party/implot/implot.cpp third_party/implot/implot_items.cpp)
target_compile_definitions(gui_libs PUBLIC IMGUI_DISABLE_DEMO_WINDOWS)
target_include_directories(gui_libs PUBLIC
    ${IMGUI} ${IMGUI}/backends ${IMGUI}/misc/cpp third_party/implot third_party/icons)

file(GLOB LOGIC_SOURCES src/*.cpp)
list(FILTER LOGIC_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
file(GLOB GUI_SOURCES src/gui/*.cpp)

add_executable(PersonalFinanceManager WIN32 src/main.cpp ${LOGIC_SOURCES} ${GUI_SOURCES})
target_include_directories(PersonalFinanceManager PRIVATE include)
target_link_libraries(PersonalFinanceManager PRIVATE gui_libs d3d11 d3dcompiler dxgi dwmapi gdi32 imm32)

add_executable(run_tests tests/test_core.cpp ${LOGIC_SOURCES})
target_include_directories(run_tests PRIVATE include)

if(MSVC)
    target_compile_options(PersonalFinanceManager PRIVATE /utf-8)
    target_compile_options(gui_libs PRIVATE /utf-8)
    # MSVC uses WinMain for WIN32 apps; use the normal main() instead.
    target_link_options(PersonalFinanceManager PRIVATE /ENTRY:mainCRTStartup)
else()
    target_compile_options(PersonalFinanceManager PRIVATE -Wall -Wextra)
    target_link_options(PersonalFinanceManager PRIVATE -static)
endif()
