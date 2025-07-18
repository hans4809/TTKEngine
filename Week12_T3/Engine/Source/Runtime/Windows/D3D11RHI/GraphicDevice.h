#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxguid")

#define _TCHAR_DEFINED
#define SAFE_RELEASE(p) if(p) { p->Release(); p = nullptr; }
#include <d3d11.h>
#include <d3d11shader.h>
#include <filesystem>

#include "Container/Array.h"
#include "Container/Map.h"
#include "Container/String.h"

enum class EShaderStage;

struct FConstantBufferInfo
{
    FString Name;
    uint32 ByteWidth;
    uint32 BindSlot;
};

struct FWindowData
{
    IDXGISwapChain* SwapChain = nullptr;
    
    UINT ScreenWidth = 0;
    UINT ScreenHeight = 0;

    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
    
    ID3D11Texture2D* PingPongFrameBuffers[2] = { nullptr, nullptr };
    ID3D11RenderTargetView* PingPongRTVs[2] = { nullptr, nullptr };
    ID3D11ShaderResourceView* PingPongSRVs[2] = { nullptr, nullptr };

    // Depth-Stencil 관련 변수
    ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
    ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    ID3D11Texture2D* DepthCopyTexture = nullptr;
    ID3D11ShaderResourceView* DepthCopySRV = nullptr;

    float GetAspectRatio() const
    {
        return static_cast<float>(ScreenWidth) / static_cast<float>(ScreenHeight);
    }
};

class FGraphicsDevice
{
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    TMap<HWND, FWindowData> SwapChains;
    
    int CurrentPingPongIndex = 0; // 현재 쓰기용 버퍼 인덱스
    HWND CurrentAppWnd = nullptr;
    
    // ID3D11RasterizerState* RasterizerStateSOLID = nullptr;
    // ID3D11RasterizerState* RasterizerStateWIREFRAME = nullptr;
    
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)

    void Initialize(HWND hWindow);
    void AddWindow(HWND hWindow);
    void RemoveWindow(HWND hWindow);

    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateSwapChain(HWND AppWnd);
    void CreateDepthStencilBuffer(HWND AppWnd);
    bool CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) const;
    bool CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) const;
    bool CreateBlendState(const D3D11_BLEND_DESC* pBlendState, ID3D11BlendState** ppBlendState) const;
    void CreateDepthCopyTexture(HWND AppWnd);
    void CreateFrameBuffer(const HWND hWindow);

    void ReleaseSwapChain(HWND hWindow);
    void ReleaseDevice();

    // PingPong 관련 함수들
    void SwapPingPongBuffers();

    const FWindowData* GetCurrentWindowData() const;
    ID3D11RenderTargetView* GetCurrentRenderTargetView() const;
    ID3D11ShaderResourceView* GetPreviousShaderResourceView() const;
    
    void ReleaseFrameBuffer(HWND AppWnd);
    void ReleaseDepthStencilResources(HWND AppWnd);
    void Release();
    void SwapBuffer(HWND AppWnd) const;
    void Prepare(HWND AppWnd);
    void OnResize(HWND AppWindow);
    
    void BindSampler(EShaderStage stage, uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const;
    void BindSamplers(uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const;
    
    // void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil) const;
    // ID3D11ShaderResourceView* GetCopiedShaderResourceView() const;

    FWindowData GetDefaultWindowData() { return SwapChains.begin()->Value;}

private:
    //ID3D11RasterizerState* CurrentRasterizer = nullptr;
public:
    static bool CompileVertexShader(const std::filesystem::path& InFilePath, const D3D_SHADER_MACRO* pDefines, ID3DBlob** ppCode);
    static bool CompilePixelShader(const std::filesystem::path& InFilePath, const D3D_SHADER_MACRO* pDefines, ID3DBlob** ppCode);
    static bool CompileComputeShader(const FString& InFileName, const D3D_SHADER_MACRO* pDefines, ID3DBlob** ppCode);

    bool CreateVertexShader(const std::filesystem::path& InFilePath, const D3D_SHADER_MACRO* pDefines, ID3DBlob** ppCode, ID3D11VertexShader** ppVShader) const;
    bool CreatePixelShader(const std::filesystem::path& InFilePath, const D3D_SHADER_MACRO* pDefines, ID3DBlob** ppCode, ID3D11PixelShader** ppPS) const;
    bool CreateComputeShader(const FString& InFileName, const D3D_SHADER_MACRO* pDefines, ID3DBlob** ppCode, ID3D11ComputeShader** ppComputeShader) const;

    void ExtractVertexShaderInfo(ID3DBlob* shaderBlob, TArray<FConstantBufferInfo>& OutCBInfos, ID3D11InputLayout*& OutInputLayout) const;
    static void ExtractPixelShaderInfo(ID3DBlob* shaderBlob, TArray<FConstantBufferInfo>& OutCBInfos);
    static TArray<FConstantBufferInfo> ExtractConstantBufferInfos(ID3D11ShaderReflection* InReflector, const D3D11_SHADER_DESC& InShaderDecs);
    ID3D11InputLayout* ExtractInputLayout(ID3DBlob* InShaderBlob, ID3D11ShaderReflection* InReflector, const D3D11_SHADER_DESC& InShaderDecs) const;

    /**
     * Shader ByteCode와 Reflector를 사용하여 (Instance Buffer도 고려한)InputLayout을 생성합니다.
     *
     * @param InShaderBlob      Complie된 Shader ByteCode(Vertex Shader)의 Blob 포인터
     * @param InReflector       Shader Reflection Interface 포인터 (ID3D11ShaderReflection*)
     * @param InShaderDescs     Shader Description을 담고 있는 D3D11_SHADER_DESC 구조체(입력 파라미터 개수 등)
     * @param OutInputLayout    생성된 ID3D11InputLayout 포인터를 저장할 변수의 주소
     *                          (이 함수 호출 후, 이 포인터를 통해 입력 레이아웃 객체를 얻음)
     *
     * 함수 내부에서는 InReflector->GetInputParameterDesc를 사용해 각 입력 시그니처를 분석하고,
     * D3D11_INPUT_ELEMENT_DESC 배열을 구성한 뒤, CreateInputLayout을 통해 InputLayout을 생성합니다.
     * 생성된 객체는 OutInputLayout에 할당되며, 호출자는 이후 이 레이아웃을 IASetInputLayout 등에 바인딩할 수 있습니다.
     */
    void ExtractInputLayout(ID3DBlob* InShaderBlob, ID3D11ShaderReflection* InReflector, const D3D11_SHADER_DESC& InShaderDescs, ID3D11InputLayout*& OutInputLayout) const;
};

