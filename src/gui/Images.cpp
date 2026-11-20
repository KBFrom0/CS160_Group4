#include "gui/Images.h"

#include <cstdint>
#include <string>
#include <windows.h>
#include <wincodec.h>
#include <d3d11.h>

namespace {
    ID3D11Device* device = nullptr;
    IWICImagingFactory* factory = nullptr;

    template <typename T>
    void release(T*& p) {
        if (p) { p->Release(); p = nullptr; }
    }
}

namespace Images {

void init(void* d3dDevice) {
    device = static_cast<ID3D11Device*>(d3dDevice);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
}

bool load(const char* path, Texture& out) {
    out = Texture();
    if (device == nullptr || factory == nullptr) return false;

    wchar_t widePath[MAX_PATH];
    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, widePath, MAX_PATH) == 0) return false;

    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool ok = SUCCEEDED(factory->CreateDecoderFromFilename(widePath, nullptr, GENERIC_READ,
                                                           WICDecodeMetadataCacheOnLoad, &decoder)) &&
              SUCCEEDED(decoder->GetFrame(0, &frame)) &&
              SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
              SUCCEEDED(converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
                                              nullptr, 0.0, WICBitmapPaletteTypeCustom));
    UINT width = 0, height = 0;
    unsigned char* pixels = nullptr;
    if (ok) {
        converter->GetSize(&width, &height);
        pixels = new unsigned char[width * height * 4];
        ok = SUCCEEDED(converter->CopyPixels(nullptr, width * 4, width * height * 4, pixels));
    }

    ID3D11ShaderResourceView* view = nullptr;
    if (ok) {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA sub;
        ZeroMemory(&sub, sizeof(sub));
        sub.pSysMem = pixels;
        sub.SysMemPitch = width * 4;

        ID3D11Texture2D* texture = nullptr;
        ok = SUCCEEDED(device->CreateTexture2D(&desc, &sub, &texture));
        if (ok) {
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            ZeroMemory(&viewDesc, sizeof(viewDesc));
            viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipLevels = 1;
            ok = SUCCEEDED(device->CreateShaderResourceView(texture, &viewDesc, &view));
        }
        release(texture);
    }

    delete[] pixels;
    release(converter);
    release(frame);
    release(decoder);
    if (!ok) return false;

    out.id = (ImTextureID)(intptr_t)view;
    out.width = (int)width;
    out.height = (int)height;
    return true;
}

void drawCircle(ImDrawList* draw, const Texture& texture, const ImVec2& center, float radius) {
    if (texture.id == 0) return;
    // Crop the middle square so the circle is not stretched.
    ImVec2 uvMin(0, 0), uvMax(1, 1);
    if (texture.width > texture.height) {
        float cut = (1.0f - (float)texture.height / texture.width) * 0.5f;
        uvMin.x = cut;
        uvMax.x = 1.0f - cut;
    } else if (texture.height > texture.width) {
        float cut = (1.0f - (float)texture.width / texture.height) * 0.5f;
        uvMin.y = cut;
        uvMax.y = 1.0f - cut;
    }
    // Trim a thin border: the member photos have dark corners around their circle.
    const float inset = 0.04f;
    float w = uvMax.x - uvMin.x, h = uvMax.y - uvMin.y;
    uvMin = ImVec2(uvMin.x + w * inset, uvMin.y + h * inset);
    uvMax = ImVec2(uvMax.x - w * inset, uvMax.y - h * inset);
    draw->AddImageRounded(texture.id, ImVec2(center.x - radius, center.y - radius),
                          ImVec2(center.x + radius, center.y + radius), uvMin, uvMax,
                          IM_COL32_WHITE, radius);
}

}
