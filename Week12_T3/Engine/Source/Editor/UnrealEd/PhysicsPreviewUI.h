#pragma once
#include "Container/Map.h"
#include "Container/String.h"
#include "Engine/World.h"
#include "Engine/Particles/ParticleEmitterInstances.h"
#include "Engine/Classes/Particles/ParticleSystemWorldManager.h"
#include "Engine/Classes/Particles/ParticleSystem.h"
#include "Particles/ParticleLODLevel.h"
#include "EditorViewportClient.h"
#include "EditorPanel.h"

class SLevelEditor;

class FPhysicsPreviewUI
{
public:
    FPhysicsPreviewUI() = default;
    ~FPhysicsPreviewUI() = default;

    void Initialize(SLevelEditor* LevelEditor, float Width, float Height);
    void Render() const;
    void OnResize(HWND hWnd) const;
    void SetWorld(UWorld* InWorld);

    HWND ActiveHandle;
    
private:
    UWorld* PreviewWorld = nullptr;
    TMap<FString, std::shared_ptr<UEditorPanel>> Panels;
};