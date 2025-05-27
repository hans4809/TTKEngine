#pragma once
#include "UnrealEd/EditorPanel.h"

class SLevelEditor;

class PhysicsGraphPreviewEditorPanel : public UEditorPanel
{
public:
    void Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight);
    virtual void Render() override;
    void DrawSceneComponentTree(USceneComponent* Component, UActorComponent*& PickedComponent);
    void DrawActorComponent(UActorComponent* Component, UActorComponent*& PickedComponent);
    virtual void OnResize(HWND hWnd) override;
private:
    SLevelEditor* activeLevelEditor = nullptr;
    float Width = 0, Height = 0;
};