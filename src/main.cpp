// Entry point: creates the Windows window + DirectX 11 device and runs the GUI loop.
// Based on the official Dear ImGui "example_win32_directx11".
//
// Usage:  PersonalFinanceManager.exe          -> normal mode (data/finance.dat)
//         PersonalFinanceManager.exe --demo   -> sample data (data/demo.dat)
#include <string>
#include <windows.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "implot.h"
#include "gui/Gui.h"
#include "gui/Theme.h"

namespace {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTarget = nullptr;
    bool swapChainOccluded = false;
    UINT resizeWidth = 0, resizeHeight = 0;
    float dpiScale = 1.0f;

    void createRenderTarget() {
        ID3D11Texture2D* backBuffer = nullptr;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
        backBuffer->Release();
    }

    void cleanupRenderTarget() {
        if (renderTarget) { renderTarget->Release(); renderTarget = nullptr; }
    }

    bool createDevice(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2,
                                                    D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &deviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) {   // no GPU driver: use the software renderer
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2,
                                                D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &deviceContext);
        }
        if (res != S_OK) return false;
        createRenderTarget();
        return true;
    }

    void cleanupDevice() {
        cleanupRenderTarget();
        if (swapChain) { swapChain->Release(); swapChain = nullptr; }
        if (deviceContext) { deviceContext->Release(); deviceContext = nullptr; }
        if (device) { device->Release(); device = nullptr; }
    }

    // Run from the project folder so "assets/" and "data/" are found,
    // whether the exe is started from bin\ or from the project root.
    void useProjectFolder() {
        wchar_t path[MAX_PATH];
        DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
        if (length == 0 || length >= MAX_PATH) return;
        std::wstring dir(path);
        dir = dir.substr(0, dir.find_last_of(L"\\/"));
        for (int i = 0; i < 2; i++) {
            if (GetFileAttributesW((dir + L"\\assets").c_str()) != INVALID_FILE_ATTRIBUTES) {
                SetCurrentDirectoryW(dir.c_str());
                return;
            }
            dir = dir.substr(0, dir.find_last_of(L"\\/"));
        }
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) return 0;
            resizeWidth = (UINT)LOWORD(lParam);
            resizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize.x = (LONG)(1120 * dpiScale);
            info->ptMinTrackSize.y = (LONG)(720 * dpiScale);
            return 0;
        }
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;   // disable the ALT menu
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int main(int argc, char* argv[]) {
    bool demo = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--demo") demo = true;
    }
    useProjectFolder();

    ImGui_ImplWin32_EnableDpiAwareness();
    dpiScale = ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr),
                       LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr,
                       L"PersonalFinanceManager", nullptr };
    RegisterClassExW(&wc);
    int width = (int)(1360 * dpiScale), height = (int)(880 * dpiScale);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Personal Finance Manager", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, wc.hInstance, nullptr);
    if (!createDevice(hwnd)) {
        cleanupDevice();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        MessageBoxW(nullptr, L"Could not start DirectX 11.", L"Personal Finance Manager", MB_ICONERROR);
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;   // do not write imgui.ini
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, deviceContext);
    Theme::init(dpiScale);

    Gui::AppState app;
    app.nativeWindow = hwnd;
    Gui::startup(app, demo);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (swapChainOccluded && swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            Sleep(10);
            continue;
        }
        swapChainOccluded = false;
        if (resizeWidth != 0 && resizeHeight != 0) {
            cleanupRenderTarget();
            swapChain->ResizeBuffers(0, resizeWidth, resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            resizeWidth = resizeHeight = 0;
            createRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        Gui::frame(app);
        ImGui::Render();

        ImVec4 bg = Theme::colors().background;
        const float clear[4] = { bg.x, bg.y, bg.z, 1.0f };
        deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
        deviceContext->ClearRenderTargetView(renderTarget, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        HRESULT hr = swapChain->Present(1, 0);
        swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    Gui::shutdown(app);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    cleanupDevice();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
