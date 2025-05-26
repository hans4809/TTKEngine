#include "CameraRenderPass.h"

#include "EditorEngine.h"
#include "LaunchEngineLoop.h"
#include "Actors/APostProcessVolume.h"
#include "D3D11RHI/CBStructDefine.h"
#include "Engine/FEditorStateManager.h"
#include "Renderer/Renderer.h"
#include "Windows/D3D11RHI/CBStructDefine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

FCameraRenderPass::FCameraRenderPass(const FName& InShaderName) : FBaseRenderPass(InShaderName)
{
    FRenderResourceManager* RenderResourceManager = FEngineLoop::Renderer.GetResourceManager();
    DOFCBuffer = RenderResourceManager->CreateConstantBuffer(sizeof(FPostProcessConstants));
    CameraCBuffer = RenderResourceManager->CreateConstantBuffer(sizeof(FFogCameraConstant));
}

void FCameraRenderPass::AddRenderObjectsToRenderPass(UWorld* World)
{
    for (AActor* Actor : TObjectRange<AActor>())
    {
        if (Actor->GetWorld() != World)
        {
            continue;
        }
                
        if (APostProcessVolume* Volume = Cast<APostProcessVolume>(Actor))
        {
            PostProcessVolumes.Add(Volume);
        }
    }
}

void FCameraRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewportClient)
{
    if (PostProcessVolumes.IsEmpty()) return;
    
    FBaseRenderPass::Prepare(InViewportClient);
    
    const FRenderer& Renderer = FEngineLoop::Renderer;
    FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    Graphics.SwapPingPongBuffers();

    const auto CurRTV = Graphics.GetCurrentRenderTargetView();
    Graphics.DeviceContext->OMSetRenderTargets(1, &CurRTV, nullptr);
    Graphics.DeviceContext->OMSetDepthStencilState(Renderer.GetDepthStencilState(EDepthStencilState::DepthNone), 0);

    Graphics.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics.DeviceContext->CopyResource(Graphics.GetCurrentWindowData()->DepthCopyTexture, Graphics.GetCurrentWindowData()->DepthStencilBuffer);
        
    ID3D11SamplerState* Sampler = Renderer.GetSamplerState(ESamplerType::PostProcess);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::PostProcess), 1, &Sampler);

    const auto PreviousSRV = Graphics.GetPreviousShaderResourceView();
    Graphics.DeviceContext->PSSetShaderResources(0, 1, &Graphics.GetCurrentWindowData()->DepthCopySRV);
    Graphics.DeviceContext->PSSetShaderResources(1, 1, &PreviousSRV);
}

void FCameraRenderPass::Execute(std::shared_ptr<FViewportClient> InViewportClient)
{
    if (PostProcessVolumes.IsEmpty()) return;
    
    FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    
    auto viewPort = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
    {
        UpdateScreenConstant(InViewportClient);
        UpdateCameraConstant(InViewportClient);
        UpdateConstantBuffer(EditorEngine);
    }
    Graphics.DeviceContext->Draw(6, 0);
}

void FCameraRenderPass::ClearRenderObjects()
{
    PostProcessVolumes.Empty();
}

void FCameraRenderPass::UpdateConstantBuffer(class UEditorEngine* Engine) const
{
    const FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    FRenderResourceManager* renderResourceManager = FEngineLoop::Renderer.GetResourceManager();
    
    FPostProcessConstants Params;
    Params.FocalLength = PostProcessVolumes[0]->PostProcess.FocalLength;
    Params.Aperture = PostProcessVolumes[0]->PostProcess.Aperture;
    Params.FocusDistance = PostProcessVolumes[0]->PostProcess.FocusDistance;
    Params.SensorWidth = PostProcessVolumes[0]->PostProcess.SensorWidth;
    Params.MaxCoCRadius = PostProcessVolumes[0]->PostProcess.MaxCoCRadius;
    Params.SampleCount = PostProcessVolumes[0]->PostProcess.SampleCount;
    
    renderResourceManager->UpdateConstantBuffer(DOFCBuffer, &Params);
    Graphics.DeviceContext->PSSetConstantBuffers(1, 1, &DOFCBuffer);
}

void FCameraRenderPass::UpdateCameraConstant(const std::shared_ptr<FViewportClient>& InViewportClient) const
{
    const FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    FRenderResourceManager* renderResourceManager = FEngineLoop::Renderer.GetResourceManager();
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);

    FFogCameraConstant CameraConstants;
    CameraConstants.InvProjMatrix = FMatrix::Inverse(curEditorViewportClient->GetProjectionMatrix());
    CameraConstants.InvViewMatrix = FMatrix::Inverse(curEditorViewportClient->GetViewMatrix());
    CameraConstants.CameraPos = curEditorViewportClient->ViewTransformPerspective.GetLocation();
    CameraConstants.NearPlane = curEditorViewportClient->GetNearClip();
    CameraConstants.CameraForward = curEditorViewportClient->ViewTransformPerspective.GetForwardVector();
    CameraConstants.FarPlane = curEditorViewportClient->GetFarClip();

    renderResourceManager->UpdateConstantBuffer(CameraCBuffer, &CameraConstants);

    Graphics.DeviceContext->PSSetConstantBuffers(2, 1, &CameraCBuffer);
}
