#include "ResourceManager.h"
#include <wincodec.h>
#include <ranges>
#include "Define.h"
#include "D3D11RHI/GraphicDevice.h"
#include "DirectXTK/Include/DDSTextureLoader.h"
#include "Engine/FLoaderOBJ.h"
#include "UserInterface/Console.h"

void FResourceManager::Initialize(FGraphicsDevice* device)
{
    GraphicDevice = device;
}

void FResourceManager::Release()
{
    for (auto& Pair : Textures)
    {
        if (Pair.Value)
            Pair.Value->Release();
    }
    Textures.Empty();
}

UTexture* FResourceManager::GetDefaultWhiteTexture()
{
    static const FWString Key   = L"DefaultWhite1x1";
    if (auto Found = Textures.Find(Key))
    {
        return *Found;
    }

    ID3D11Device*           device  = GraphicDevice->Device;
    ID3D11DeviceContext*    ctx     = GraphicDevice->DeviceContext;

    UINT32 whitePixel = 0xFFFFFFFF; // RGBA = (1,1,1,1)
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width              = 1;
    desc.Height             = 1;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem           = &whitePixel;
    initData.SysMemPitch       = sizeof(whitePixel);
    initData.SysMemSlicePitch  = sizeof(whitePixel);

    ID3D11Texture2D* tex2D = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &initData, &tex2D);
    if (FAILED(hr) || !tex2D)
    {
        return nullptr;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex2D, nullptr, &srv);
    if (FAILED(hr) || !srv)
    {
        tex2D->Release();  // SRV 생성 실패 시엔 Texture만 해제
        return nullptr;
    }

    UTexture* DefaultTex = FObjectFactory::ConstructObject<UTexture>(nullptr);
    DefaultTex->TextureSRV = srv;
    DefaultTex->Texture = tex2D;
    DefaultTex->Width = 1;
    DefaultTex->Height = 1;
    DefaultTex->Path = Key;

    // 캐싱
    Textures.Add(Key, DefaultTex);
    return DefaultTex;
}