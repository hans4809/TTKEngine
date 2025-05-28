#include "PhysicsTreePreviewEditorPanel.h"
#include "LevelEditor/SLevelEditor.h"
#include <Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h>
#include "Classes/GameFramework/Actor.h"
#include <Actors/SkeletalMeshActor.h>
#include "Physics/PhysicsAsset.h"
#include "Physics/PhysicsConstraintTemplate.h"
#include "Classes/Animation/Skeleton.h"

EPhysicsNodeType PhysicsTreePreviewEditorPanel::SelectedType = EPhysicsNodeType::None;
int32 PhysicsTreePreviewEditorPanel::SelectedBoneIndex = -1;
int32 PhysicsTreePreviewEditorPanel::SelectedConstraintIndex = -1;
int32 PhysicsTreePreviewEditorPanel::SelectedBodyIndex = -1;
EAggCollisionShape::Type PhysicsTreePreviewEditorPanel::SelectedPrimType = EAggCollisionShape::Sphere;

int32 PhysicsTreePreviewEditorPanel::SelectedPrimIndex = -1;
void PhysicsTreePreviewEditorPanel::Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight)
{
    activeLevelEditor = levelEditor;
    Width = InWidth;
    Height = InHeight;
    HighlightColor = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);

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
        UPhysicsAsset* PhysAsset = SkeletalMeshComp->GetSkeletalMesh()->GetPhysicsAsset();

        if (SkeletalMeshComp && SkeletalMeshComp->GetSkeletalMesh()) {
            USkeleton* SkeletonAsset = SkeletalMeshComp->GetSkeletalMesh()->GetSkeleton();
            if (SkeletonAsset)
            {
                FRefSkeletal& RefSkeletal = SkeletonAsset->GetRefSkeletal();
               
                // 2BoneTree 루트부터 그리기
                for (int32 RootIndex : RefSkeletal.RootBoneIndices)
                {
                    DrawBoneNodeRecursive(RefSkeletal, RootIndex, RefSkeletal.BoneTree, RefSkeletal.RawBones, PhysAsset);
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

void PhysicsTreePreviewEditorPanel::DrawBoneNodeRecursive(FRefSkeletal& RefSkeletal, int32 BoneIndex, const TArray<FBoneNode>& BoneTree, const TArray<FBone>& RawBones, UPhysicsAsset* PhysicsAsset)
{
    const FBone& Bone = RawBones[BoneIndex];
    std::stringstream ss;
    ss << Bone.BoneName.ToAnsiString() << "##Bone" << BoneIndex;

    ImGuiTreeNodeFlags BoneFlags = ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_AllowItemOverlap;

    bool isSelected = (SelectedType == EPhysicsNodeType::Bone && SelectedBoneIndex == BoneIndex);

    if (isSelected) {
        BoneFlags |= ImGuiTreeNodeFlags_Selected;
        ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
    }

    bool bOpen = ImGui::TreeNodeEx(ss.str().c_str(), BoneFlags);

    if (isSelected) {
        ImGui::PopStyleColor(3);
    }

    if (ImGui::IsItemClicked())
    {
        SelectedType = EPhysicsNodeType::Bone;
        SelectedBoneIndex = BoneIndex;
        SelectedConstraintIndex = -1;
        SelectedBodyIndex = -1;
    }

    // Bodies under this bone
    int32 BodyIdx = PhysicsAsset->FindBodyIndex(Bone.BoneName);
    if (BodyIdx != INDEX_NONE)
    {
        std::string BLabel = "Body##Body" + std::to_string(BodyIdx);


        ImGuiTreeNodeFlags BodyFlags = ImGuiTreeNodeFlags_Leaf;

        bool isSelected = (SelectedType == EPhysicsNodeType::Body && SelectedBodyIndex == BoneIndex);

        if (isSelected) {
            BoneFlags |= ImGuiTreeNodeFlags_Selected;
            ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
        }

        ImGui::TreeNodeEx(BLabel.c_str(), BodyFlags);

        if (isSelected) {
            ImGui::PopStyleColor(3);
        }

        if (ImGui::IsItemClicked())
        {
            SelectedType = EPhysicsNodeType::Body;
            SelectedBodyIndex = BodyIdx;
        }
        ImGui::TreePop();

        // primitives inside body
        UBodySetup* BS = PhysicsAsset->BodySetups[BodyIdx];
        // spheres

        
        for (int32 Pi = 0; Pi < BS->AggGeom.SphereElems.Num(); ++Pi)
        {
            std::string PLabel = "Sphere##P" + std::to_string(BodyIdx) + "_" + std::to_string(Pi);

            ImGuiTreeNodeFlags PrimitiveFlags = ImGuiTreeNodeFlags_Leaf;

            bool isSelected = (SelectedType == EPhysicsNodeType::Primitive 
                && SelectedPrimIndex == Pi
                && PhysicsTreePreviewEditorPanel::SelectedPrimType == EAggCollisionShape::Sphere);

            if (isSelected) {
                BoneFlags |= ImGuiTreeNodeFlags_Selected;
                ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
            }

            ImGui::TreeNodeEx(PLabel.c_str(), PrimitiveFlags);

            if (isSelected) {
                ImGui::PopStyleColor(3);
            }

            if (ImGui::IsItemClicked())
            {
                SelectedType = EPhysicsNodeType::Primitive;
                SelectedBodyIndex = BodyIdx;
                SelectedPrimType = EAggCollisionShape::Sphere;
                SelectedPrimIndex = Pi;
            }
            ImGui::TreePop();
        }
        for (int32 Pi = 0; Pi < BS->AggGeom.BoxElems.Num(); ++Pi)
        {
            std::string PLabel = "Box##P" + std::to_string(BodyIdx) + "_" + std::to_string(Pi);

            ImGuiTreeNodeFlags PrimitiveFlags = ImGuiTreeNodeFlags_Leaf;

            bool isSelected = (SelectedType == EPhysicsNodeType::Primitive
                && SelectedPrimIndex == Pi
                && PhysicsTreePreviewEditorPanel::SelectedPrimType == EAggCollisionShape::Box);

            if (isSelected) {
                BoneFlags |= ImGuiTreeNodeFlags_Selected;
                ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
            }

            ImGui::TreeNodeEx(PLabel.c_str(), ImGuiTreeNodeFlags_Leaf);

            if (isSelected) {
                ImGui::PopStyleColor(3);
            }

            if (ImGui::IsItemClicked())
            {
                SelectedType = EPhysicsNodeType::Primitive;
                SelectedBodyIndex = BodyIdx;
                SelectedPrimType = EAggCollisionShape::Box;
                SelectedPrimIndex = Pi;
            }
            ImGui::TreePop();
        }
        for (int32 Pi = 0; Pi < BS->AggGeom.SphylElems.Num(); ++Pi)
        {
            std::string PLabel = "SphylElems##P" + std::to_string(BodyIdx) + "_" + std::to_string(Pi);
            
            ImGuiTreeNodeFlags PrimitiveFlags = ImGuiTreeNodeFlags_Leaf;

            bool isSelected = (SelectedType == EPhysicsNodeType::Primitive
                && SelectedPrimIndex == Pi
                && PhysicsTreePreviewEditorPanel::SelectedPrimType == EAggCollisionShape::Sphyl);

            if (isSelected) {
                BoneFlags |= ImGuiTreeNodeFlags_Selected;
                ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
            }


            ImGui::TreeNodeEx(PLabel.c_str(), PrimitiveFlags);

            if (isSelected) {
                ImGui::PopStyleColor(3);
            }
            
            if (ImGui::IsItemClicked())
            {
                SelectedType = EPhysicsNodeType::Primitive;
                SelectedBodyIndex = BodyIdx;
                SelectedPrimType = EAggCollisionShape::Sphyl;
                SelectedPrimIndex = Pi;
            }
            ImGui::TreePop();
        }
        for (int32 Pi = 0; Pi < BS->AggGeom.ConvexElems.Num(); ++Pi)
        {
            std::string PLabel = "Convex##P" + std::to_string(BodyIdx) + "_" + std::to_string(Pi);

            ImGuiTreeNodeFlags PrimitiveFlags = ImGuiTreeNodeFlags_Leaf;

            bool isSelected = (SelectedType == EPhysicsNodeType::Primitive
                && SelectedPrimIndex == Pi
                && PhysicsTreePreviewEditorPanel::SelectedPrimType == EAggCollisionShape::Convex);

            if (isSelected) {
                BoneFlags |= ImGuiTreeNodeFlags_Selected;
                ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
            }

            ImGui::TreeNodeEx(PLabel.c_str(), PrimitiveFlags);

            if (isSelected) {
                ImGui::PopStyleColor(3);
            }

            if (ImGui::IsItemClicked())
            {
                SelectedType = EPhysicsNodeType::Primitive;
                SelectedBodyIndex = BodyIdx;
                SelectedPrimType = EAggCollisionShape::Convex;
                SelectedPrimIndex = Pi;
            }
            ImGui::TreePop();
        }
        // Constraints under this bone
        TArray<int32> Constraints;
        PhysicsAsset->BodyFindConstraints(BoneIndex, Constraints);
        for (int32 Ci : Constraints)
        {
            UPhysicsConstraintTemplate* CT = PhysicsAsset->ConstraintSetup[Ci];
            std::string CLabel = std::string(CT->JointName.ToString().ToAnsiString() + "##Const" + std::to_string(Ci));

            ImGuiTreeNodeFlags ConstraintFlags = ImGuiTreeNodeFlags_Leaf;

            bool isSelected = (SelectedType == EPhysicsNodeType::Constraint
                && SelectedPrimIndex == Ci);

            if (isSelected) {
                BoneFlags |= ImGuiTreeNodeFlags_Selected;
                ImGui::PushStyleColor(ImGuiCol_Header, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HighlightColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, HighlightColor);
            }

            ImGui::TreeNodeEx(CLabel.c_str(), ConstraintFlags);

            if (isSelected) {
                ImGui::PopStyleColor(3);
            }

            if (ImGui::IsItemClicked())
            {
                SelectedType = EPhysicsNodeType::Constraint;
                SelectedConstraintIndex = Ci;
            }
            ImGui::TreePop();
        }
    }

    if (bOpen)
    {
        if (BoneTree.IsValidIndex(BoneIndex))
        {
            for (int32 ChildIndex : BoneTree[BoneIndex].ChildIndices)
            {
                DrawBoneNodeRecursive(RefSkeletal, ChildIndex, BoneTree, RawBones, PhysicsAsset);
            }
        }
        ImGui::TreePop();
    }
}



