#pragma once
#include "UnrealEd/EditorPanel.h"

class SLevelEditor;
class USkeletalMeshComponent;
class UPhysicsAsset;

class PhysicsDetailPreviewEditorPanel : public UEditorPanel
{
public:
    void Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight);
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    SLevelEditor* activeLevelEditor = nullptr;
    float Width = 0, Height = 0;

private:
    void DrawBoneDetails(int32 BoneIndex, USkeletalMeshComponent* SkelComp, UPhysicsAsset* PhysicsAsset);
    void DrawPropertiesForObject(UObject* Obj);
};