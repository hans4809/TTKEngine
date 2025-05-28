#pragma once
#include "UnrealEd/EditorPanel.h"
#include "Physics/ShapeElems/ShapeElem.h"
#include "PhysicsCore/PhysicsAssetTypes.h"
class SLevelEditor;
class FRefSkeletal;
class FBoneNode;
class FBone;
class UPhysicsAsset;



class PhysicsTreePreviewEditorPanel : public UEditorPanel {
public:
    void Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight);
    virtual void Render() override;
    
    virtual void OnResize(HWND hWnd) override;

    void DrawBoneNodeRecursive(FRefSkeletal& RefSkeletal, int32 BoneIndex, const TArray<FBoneNode>& BoneTree, const TArray<FBone>& RawBones,
        UPhysicsAsset* PhysicsAsset);
public:
    // 선택된 노드 정보들
    static EPhysicsNodeType SelectedType;
    static int32 SelectedBoneIndex;
    static int32 SelectedConstraintIndex;
    static int32 SelectedBodyIndex;
    static EAggCollisionShape::Type SelectedPrimType;
    static int32 SelectedPrimIndex;

private:
    SLevelEditor* activeLevelEditor = nullptr;
    float Width = 0, Height = 0;

    ImVec4 HighlightColor;
};