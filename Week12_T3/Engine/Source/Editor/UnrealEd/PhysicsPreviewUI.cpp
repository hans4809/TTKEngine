#include "PhysicsPreviewUI.h"
#include <LevelEditor/SLevelEditor.h>

void FPhysicsPreviewUI::Initialize(SLevelEditor* LevelEditor, float Width, float Height) const
{

}

void FPhysicsPreviewUI::Render() const
{
    for (const auto& Panel : Panels)
    {
        if (Panel.Value->bIsVisible)
        {
            Panel.Value->Render();
        }
    }
}

void FPhysicsPreviewUI::OnResize(HWND hWnd) const
{
    for (auto& Panel : Panels)
    {
        Panel.Value->OnResize(hWnd);
    }
}

void FPhysicsPreviewUI::SetWorld(UWorld* InWorld)
{
    PreviewWorld = InWorld;
    for (auto& [_, Panel] : Panels)
    {
        Panel->SetWorld(PreviewWorld);
    }
}
