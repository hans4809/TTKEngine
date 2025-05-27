#pragma once
#include "FBaseRenderPass.h"
#include "Container/Array.h"

class FEditorViewportClient;

class FDepthOfFieldRenderPass : public FBaseRenderPass
{
public:
    FDepthOfFieldRenderPass(const FDepthOfFieldRenderPass&) = delete;
    FDepthOfFieldRenderPass& operator=(const FDepthOfFieldRenderPass&) = delete;
    FDepthOfFieldRenderPass(FDepthOfFieldRenderPass&&) = delete;
    FDepthOfFieldRenderPass& operator=(FDepthOfFieldRenderPass&&) = delete;
    
public:
    explicit FDepthOfFieldRenderPass(const FName& InShaderName);
    ~FDepthOfFieldRenderPass() override = default;

    void AddRenderObjectsToRenderPass(UWorld* World) override;
    void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;
    void ClearRenderObjects() override;
    
    void UpdateConstantBuffer(const std::shared_ptr<FEditorViewportClient>& InViewportClient) const;
    void UpdateCameraConstant(const std::shared_ptr<FEditorViewportClient>& InViewportClient) const;

private:
    class ID3D11Buffer* DOFCBuffer = nullptr;
    class ID3D11Buffer* CameraCBuffer = nullptr;
    
    
    TArray<class APostProcessVolume*> PostProcessVolumes;
};
