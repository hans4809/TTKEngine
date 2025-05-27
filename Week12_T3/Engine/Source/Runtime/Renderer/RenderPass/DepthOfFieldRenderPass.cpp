#include "DepthOfFieldRenderPass.h"

#include "LaunchEngineLoop.h"
#include "Actors/APostProcessVolume.h"
#include "D3D11RHI/CBStructDefine.h"
#include "Engine/FEditorStateManager.h"
#include "Renderer/Renderer.h"
#include "Windows/D3D11RHI/CBStructDefine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

FDepthOfFieldRenderPass::FDepthOfFieldRenderPass(const FName& InShaderName) : FBaseRenderPass(InShaderName)
{
    FRenderResourceManager* RenderResourceManager = FEngineLoop::Renderer.GetResourceManager();
    DOFCBuffer = RenderResourceManager->CreateConstantBuffer(sizeof(FDepthOfFieldConstants));
    CameraCBuffer = RenderResourceManager->CreateConstantBuffer(sizeof(FFogCameraConstant));
}

void FDepthOfFieldRenderPass::AddRenderObjectsToRenderPass(UWorld* World)
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

void FDepthOfFieldRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewportClient)
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

void FDepthOfFieldRenderPass::Execute(std::shared_ptr<FViewportClient> InViewportClient)
{
    if (PostProcessVolumes.IsEmpty()) return;
    
    FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    
    auto EditorViewPort = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);

    UpdateScreenConstant(InViewportClient);
    UpdateCameraConstant(EditorViewPort);
    UpdateConstantBuffer(EditorViewPort);
    
    Graphics.DeviceContext->Draw(6, 0);
}

void FDepthOfFieldRenderPass::ClearRenderObjects()
{
    PostProcessVolumes.Empty();
}

void FDepthOfFieldRenderPass::UpdateConstantBuffer(const std::shared_ptr<FEditorViewportClient>& InViewportClient) const
{
    const FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    FRenderResourceManager* renderResourceManager = FEngineLoop::Renderer.GetResourceManager();
    
    FDepthOfFieldConstants Params;
    Params.Aperture = PostProcessVolumes[0]->PostProcess.Aperture;
    Params.FocusDistance = PostProcessVolumes[0]->PostProcess.FocusDistance;
    Params.SensorWidth = PostProcessVolumes[0]->PostProcess.SensorWidth;
    Params.MaxCoCRadius = PostProcessVolumes[0]->PostProcess.MaxCoCRadius;
    Params.SampleCount = PostProcessVolumes[0]->PostProcess.SampleCount;

    float CalcFocal = (PostProcessVolumes[0]->PostProcess.SensorWidth * 0.5) / tanf(InViewportClient->GetViewFOV() * (PI/180.0f) * 0.5f);
    Params.FocalLength = CalcFocal;
    
    float ShowFlag = InViewportClient->GetViewMode() == EViewModeIndex::VMI_DepthOfField ? 1.0f : 0.0f;
    Params.UserData = FVector2D { ShowFlag, 0.0f };
    
    renderResourceManager->UpdateConstantBuffer(DOFCBuffer, &Params);
    Graphics.DeviceContext->PSSetConstantBuffers(1, 1, &DOFCBuffer);
}

void FDepthOfFieldRenderPass::UpdateCameraConstant(const std::shared_ptr<FEditorViewportClient>& InViewportClient) const
{
    const FGraphicsDevice& Graphics = FEngineLoop::GraphicDevice;
    FRenderResourceManager* renderResourceManager = FEngineLoop::Renderer.GetResourceManager();

    FFogCameraConstant CameraConstants;
    CameraConstants.InvProjMatrix = FMatrix::Inverse(InViewportClient->GetProjectionMatrix());
    CameraConstants.InvViewMatrix = FMatrix::Inverse(InViewportClient->GetViewMatrix());
    CameraConstants.CameraPos = InViewportClient->ViewTransformPerspective.GetLocation();
    CameraConstants.NearPlane = InViewportClient->GetNearClip();
    CameraConstants.CameraForward = InViewportClient->ViewTransformPerspective.GetForwardVector();
    CameraConstants.FarPlane = InViewportClient->GetFarClip();

    renderResourceManager->UpdateConstantBuffer(CameraCBuffer, &CameraConstants);

    Graphics.DeviceContext->PSSetConstantBuffers(2, 1, &CameraCBuffer);
}
