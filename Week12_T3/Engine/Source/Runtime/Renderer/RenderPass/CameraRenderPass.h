#pragma once
#include "FBaseRenderPass.h"
#include "Container/Array.h"

class FCameraRenderPass : public FBaseRenderPass
{
public:
    FCameraRenderPass(const FCameraRenderPass&) = delete;
    FCameraRenderPass& operator=(const FCameraRenderPass&) = delete;
    FCameraRenderPass(FCameraRenderPass&&) = delete;
    FCameraRenderPass& operator=(FCameraRenderPass&&) = delete;
    
public:
    explicit FCameraRenderPass(const FName& InShaderName);
    ~FCameraRenderPass() override = default;

    void AddRenderObjectsToRenderPass(UWorld* World) override;
    void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;
    void ClearRenderObjects() override;
    
    void UpdateConstantBuffer(class UEditorEngine* Engine) const;
    void UpdateCameraConstant(const std::shared_ptr<FViewportClient>& InViewportClient) const;

private:
    class ID3D11Buffer* DOFCBuffer = nullptr;
    class ID3D11Buffer* CameraCBuffer = nullptr;
    
    
    TArray<class APostProcessVolume*> PostProcessVolumes;
};
