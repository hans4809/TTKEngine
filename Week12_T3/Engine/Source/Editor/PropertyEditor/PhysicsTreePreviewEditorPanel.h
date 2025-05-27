#pragma once
#include "UnrealEd/EditorPanel.h"

class SLevelEditor;
class FRefSkeletal;
class FBoneNode;
class FBone;
class PhysicsTreePreviewEditorPanel : public UEditorPanel {
public:
    void Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight);
    virtual void Render() override;
    
    virtual void OnResize(HWND hWnd) override;

    void DrawBoneNodeRecursive(FRefSkeletal& RefSkeletal, int32 BoneIndex, const TArray<FBoneNode>& BoneTree, const TArray<FBone>& RawBones);
private:
    SLevelEditor* activeLevelEditor = nullptr;
    float Width = 0, Height = 0;

    int32 SelectedBoneIndex = -1;
};