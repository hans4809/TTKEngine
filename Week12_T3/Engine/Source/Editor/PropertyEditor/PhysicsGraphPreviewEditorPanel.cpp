#include "PhysicsGraphPreviewEditorPanel.h"
#include "EditorEngine.h"
#include "LevelEditor/SLevelEditor.h"
void PhysicsGraphPreviewEditorPanel::Initialize(SLevelEditor* levelEditor, float InWidth, float InHeight)
{
    activeLevelEditor = levelEditor;
    Width = InWidth;
    Height = InHeight;
}

void PhysicsGraphPreviewEditorPanel::Render()
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);

    if (EditorEngine == nullptr)
    {
        return;
    }

    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.3f;

    float PanelPosX = 5.0f;
    float PanelPosY = (Height) * 0.7f - 15.0f;

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
    ImGui::Begin("Graph", nullptr, PanelFlags);

    // 프리뷰 월드에서는 오로지 하나의 액터만 선택 가능 (스켈레탈 메쉬 프리뷰인 경우, 스켈레탈 메쉬 액터)
    AActor* PickedActor = nullptr;

    if (!World->GetSelectedActors().IsEmpty())
    {
        PickedActor = *World->GetSelectedActors().begin();
    }

    ImGui::End();
}

void PhysicsGraphPreviewEditorPanel::DrawSceneComponentTree(USceneComponent* Component, UActorComponent*& PickedComponent)
{
}

void PhysicsGraphPreviewEditorPanel::DrawActorComponent(UActorComponent* Component, UActorComponent*& PickedComponent)
{
}

void PhysicsGraphPreviewEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
