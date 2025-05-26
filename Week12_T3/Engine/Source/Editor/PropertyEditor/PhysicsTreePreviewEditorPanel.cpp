#include "PhysicsTreePreviewEditorPanel.h"
#include "LevelEditor/SLevelEditor.h"
#include <Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h>
#include "Classes/GameFramework/Actor.h"
#include <Actors/SkeletalMeshActor.h>
#include "Classes/Animation/Skeleton.h"

void PhysicsTreePreviewEditorPanel::Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight)
{
    activeLevelEditor = levelEditor;
    Width = InWidth;
    Height = InHeight;

}

void PhysicsTreePreviewEditorPanel::Render()
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
    if (EditorEngine == nullptr)
    {
        return;
    }

    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = 5.0f;
    float PanelPosY = 45.0f;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Skeleton Tree", nullptr, PanelFlags);

    // 프리뷰 월드에서는 오로지 하나의 액터만 선택 가능 (스켈레탈 메쉬 프리뷰인 경우, 스켈레탈 메쉬 액터)
    AActor* PickedActor = nullptr;

    if (!World->GetSelectedActors().IsEmpty())
    {
        PickedActor = *World->GetSelectedActors().begin();
    }
    
    USkeletalMeshComponent* SkeletalMeshComp;
    ASkeletalMeshActor* SkeletalActor = Cast<ASkeletalMeshActor>(PickedActor);
    if (SkeletalActor) {
        SkeletalMeshComp = SkeletalActor->GetSkeletalMeshComponent();
        if (SkeletalMeshComp) {
            USkeleton* SkeletonAsset = SkeletalMeshComp->GetSkeletalMesh()->GetSkeleton();
            if (SkeletonAsset)
            {
                FRefSkeletal* RefSkeletal = SkeletonAsset->GetRefSkeletal();
                if (RefSkeletal)
                {
                    // 2BoneTree 루트부터 그리기
                    for (int32 RootIndex : RefSkeletal->RootBoneIndices)
                    {
                        DrawBoneNodeRecursive(RefSkeletal, RootIndex, RefSkeletal->BoneTree, RefSkeletal->RawBones);
                    }
                }
            }
        }
    }

    ImGui::End();
}

void PhysicsTreePreviewEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}

void PhysicsTreePreviewEditorPanel::DrawBoneNodeRecursive(FRefSkeletal* RefSkeletal, int32 BoneIndex, const TArray<FBoneNode>& BoneTree, const TArray<FBone>& RawBones)
{
    const FBone& Bone = RawBones[BoneIndex];
    std::stringstream ss;
    ss << Bone.BoneName.ToAnsiString() << "##Bone" << BoneIndex;

    bool bOpen = ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen | 
        ImGuiTreeNodeFlags_OpenOnArrow|
        ImGuiTreeNodeFlags_AllowItemOverlap
    );

    if (ImGui::IsItemClicked())
    {
        // 📌 노드 클릭 시 상세 패널로 전달 (임시 전역/싱글톤으로 선택된 BoneIndex 저장)
        SelectedBoneIndex = BoneIndex;
    }

    if (bOpen)
    {
        if (BoneTree.IsValidIndex(BoneIndex))
        {
            const TArray<int>& ChildIndices = BoneTree[BoneIndex].ChildIndices;
            for (int32 ChildIndex : ChildIndices)
            {
                DrawBoneNodeRecursive(RefSkeletal, ChildIndex, BoneTree, RawBones);
            }
        }
        ImGui::TreePop();
    }
}

