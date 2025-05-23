#include "Texture.h"

#include <wincodec.h>

#include "DDSTextureLoader.h"
#include "LaunchEngineLoop.h"

bool UTexture::LoadFromFile(const FString& InFilepath)
{
    if (InFilepath.IsEmpty()) return false;

    if (std::filesystem::path(InFilepath).extension().string() == ".dds")
    {
        LoadTextureFromDDS(InFilepath);
    }
    else
    {
        LoadTextureFromFile(InFilepath);
    }

    return true;
}

bool UTexture::LoadTextureFromDDS(const FString& InFilepath)
{
    FGraphicsDevice* GraphicDevice = &GEngineLoop.GraphicDevice;

    ID3D11Resource* texture = nullptr;

    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        GraphicDevice->Device, GraphicDevice->DeviceContext,
        InFilepath.ToWideString().c_str(),
        &texture,
        &TextureSRV
    );
    
    if (FAILED(hr) || texture == nullptr) abort();

#pragma region WidthHeight
    
    hr = texture->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&Texture);
    if (FAILED(hr) || Texture == nullptr) {
        std::wcerr << L"Failed to query ID3D11Texture2D interface!" << std::endl;
        texture->Release();
        abort();
        return false;
    }

    // 🔹 텍스처 크기 가져오기
    D3D11_TEXTURE2D_DESC texDesc;
    Texture->GetDesc(&texDesc);
    uint32 width = static_cast<uint32>(texDesc.Width);
    uint32 height = static_cast<uint32>(texDesc.Height);

#pragma endregion WidthHeight
    Width = width;
    Height = height;
    Path = InFilepath;

    Console::GetInstance().AddLog(LogLevel::Warning, "Texture File Load Successs");

    return true;
}

bool UTexture::LoadTextureFromFile(const FString& InFilepath)
{
    FGraphicsDevice* GraphicDevice = &GEngineLoop.GraphicDevice;
    
    IWICImagingFactory* wicFactory = nullptr;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* frame = nullptr;
	IWICFormatConverter* converter = nullptr;

	// WIC 팩토리 생성
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) return false;

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	if (FAILED(hr)) return false;


	// 이미지 파일 디코딩
	hr = wicFactory->CreateDecoderFromFilename(InFilepath.ToWideString().c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr)) return false;


	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr)) return false;

	// WIC 포맷 변환기 생성 (픽셀 포맷 변환)
	hr = wicFactory->CreateFormatConverter(&converter);
	if (FAILED(hr)) return false;

	hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) return false;

	// 이미지 크기 가져오기
	UINT width, height;
	frame->GetSize(&width, &height);
	
	// 픽셀 데이터 로드
	BYTE* imageData = new BYTE[width * height * 4];
	hr = converter->CopyPixels(nullptr, width * 4, width * height * 4, imageData);
	if (FAILED(hr)) {
		delete[] imageData;
		return false;
	}

	// DirectX 11 텍스처 생성
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageData;
	initData.SysMemPitch = width * 4;
	hr = GraphicDevice->Device->CreateTexture2D(&textureDesc, &initData, &Texture);
	delete[] imageData;
	if (FAILED(hr)) return false;

	// Shader Resource View 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = GraphicDevice->Device->CreateShaderResourceView(Texture, &srvDesc, &TextureSRV);

	// 리소스 해제
	wicFactory->Release();
	decoder->Release();
	frame->Release();
	converter->Release();
    
    Width = width;
    Height = height;
    Path = InFilepath;

	Console::GetInstance().AddLog(LogLevel::Warning, "Texture File Load Successs");

    // COM 언인니셜라이즈
    CoUninitialize();

    return true;
}

bool UTexture::SerializeToFile(std::ostream& Out)
{
    return true;
}

bool UTexture::DeserializeFromFile(std::istream& In)
{
    return true;
}
