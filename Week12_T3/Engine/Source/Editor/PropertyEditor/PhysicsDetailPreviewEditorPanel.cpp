#include "PhysicsDetailPreviewEditorPanel.h"
#include "EditorEngine.h"
#include "LevelEditor/SLevelEditor.h"
#include "PhysicsCore/PhysicsAssetTypes.h"
#include "PhysicsTreePreviewEditorPanel.h"
#include <Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h>
#include <Actors/SkeletalMeshActor.h>
#include "Physics/PhysicsAsset.h"

void PhysicsDetailPreviewEditorPanel::Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight)
{
    activeLevelEditor = levelEditor;
    Width = InWidth;
    Height = InHeight;
}

void PhysicsDetailPreviewEditorPanel::Render()
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);

    if (EditorEngine == nullptr)
    {
        return;
    }

    AActor* PickedActor = nullptr;

    if (!World->GetSelectedActors().IsEmpty())
    {
        PickedActor = *World->GetSelectedActors().begin();
    }

    USkeletalMeshComponent* SkeletalMeshComp;
    ASkeletalMeshActor* SkeletalActor = Cast<ASkeletalMeshActor>(PickedActor);
    UPhysicsAsset* PhysicsAsset = nullptr;
    
    if (SkeletalActor) {
        SkeletalMeshComp = SkeletalActor->GetSkeletalMeshComponent();
        PhysicsAsset = SkeletalMeshComp->GetSkeletalMesh()->GetPhysicsAsset();
    }


    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = (Height) * 0.3f + 15.0f;

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
    ImGui::Begin("Detail", nullptr, PanelFlags);

    using EType = EPhysicsNodeType;
    UObject* ToInspect = nullptr;

    switch (PhysicsTreePreviewEditorPanel::SelectedType) {
    case EType::Bone:
        break;
    case EType::Constraint:
        ToInspect = PhysicsAsset->ConstraintSetup[PhysicsTreePreviewEditorPanel::SelectedConstraintIndex];
        break;
    case EType::Body:
        break;
    case EType::Primitive:
        break;
    default:
    }

    

    ImGui::End();
}

void PhysicsDetailPreviewEditorPanel::DrawSceneComponentTree(USceneComponent* Component, UActorComponent*& PickedComponent)
{
}

void PhysicsDetailPreviewEditorPanel::DrawActorComponent(UActorComponent* Component, UActorComponent*& PickedComponent)
{
}

void PhysicsDetailPreviewEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
