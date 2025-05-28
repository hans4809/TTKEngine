#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "GizmoCircleComponent.h"
#include "GizmoRectangleComponent.h"
#include "Engine/World.h"
#include "Engine/FLoaderOBJ.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorPlayer.h"
#include <PxPhysicsAPI.h>

#include "Engine/Asset/AssetManager.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"

UTransformGizmo::UTransformGizmo()
{
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_loc_x.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_loc_y.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_loc_z.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_rot_x.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_rot_y.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_rot_z.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_scale_x.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_scale_y.obj");
    // FManagerOBJ::CreateStaticMesh("Assets/gizmo_scale_z.obj");

    SetRootComponent(
        AddComponent<USceneComponent>(EComponentOrigin::Constructor)
    );

    UGizmoArrowComponent* LocationX = AddComponent<UGizmoArrowComponent>(EComponentOrigin::Constructor);
    LocationX->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_loc_x")));
	LocationX->SetupAttachment(RootComponent);
    LocationX->SetGizmoType(UGizmoBaseComponent::EGizmoType::ArrowX);
	ArrowArr.Add(LocationX);

    UGizmoArrowComponent* LocationY = AddComponent<UGizmoArrowComponent>(EComponentOrigin::Constructor);
    LocationY->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_loc_y")));
    LocationY->SetupAttachment(RootComponent);
    LocationY->SetGizmoType(UGizmoBaseComponent::EGizmoType::ArrowY);
    ArrowArr.Add(LocationY);

    UGizmoArrowComponent* LocationZ = AddComponent<UGizmoArrowComponent>(EComponentOrigin::Constructor);
    LocationZ->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_loc_z")));
    LocationZ->SetupAttachment(RootComponent);
    LocationZ->SetGizmoType(UGizmoBaseComponent::EGizmoType::ArrowZ);
    ArrowArr.Add(LocationZ);

    UGizmoRectangleComponent* ScaleX = AddComponent<UGizmoRectangleComponent>(EComponentOrigin::Constructor);
    ScaleX->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_scale_x")));
    ScaleX->SetupAttachment(RootComponent);
    ScaleX->SetGizmoType(UGizmoBaseComponent::EGizmoType::ScaleX);
    RectangleArr.Add(ScaleX);

    UGizmoRectangleComponent* ScaleY = AddComponent<UGizmoRectangleComponent>(EComponentOrigin::Constructor);
    ScaleY->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_scale_y")));
    ScaleY->SetupAttachment(RootComponent);
    ScaleY->SetGizmoType(UGizmoBaseComponent::EGizmoType::ScaleY);
    RectangleArr.Add(ScaleY);

    UGizmoRectangleComponent* ScaleZ = AddComponent<UGizmoRectangleComponent>(EComponentOrigin::Constructor);
    ScaleZ->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_scale_z")));
    ScaleZ->SetupAttachment(RootComponent);
    ScaleZ->SetGizmoType(UGizmoBaseComponent::EGizmoType::ScaleZ);
    RectangleArr.Add(ScaleZ);

    UGizmoCircleComponent* CircleX = AddComponent<UGizmoCircleComponent>(EComponentOrigin::Constructor);
    CircleX->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_rot_x")));
    CircleX->SetupAttachment(RootComponent);
    CircleX->SetGizmoType(UGizmoBaseComponent::EGizmoType::CircleX);
    CircleArr.Add(CircleX);

    UGizmoCircleComponent* CircleY = AddComponent<UGizmoCircleComponent>(EComponentOrigin::Constructor);
    CircleY->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_rot_y")));
    CircleY->SetupAttachment(RootComponent);
    CircleY->SetGizmoType(UGizmoBaseComponent::EGizmoType::CircleY);
    CircleArr.Add(CircleY);

    UGizmoCircleComponent* CircleZ = AddComponent<UGizmoCircleComponent>(EComponentOrigin::Constructor);
    CircleZ->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("gizmo_rot_z")));
    CircleZ->SetupAttachment(RootComponent);
    CircleZ->SetGizmoType(UGizmoBaseComponent::EGizmoType::CircleZ);
    CircleArr.Add(CircleZ);
}

void UTransformGizmo::Tick(const float DeltaTime)
{
    if (GetWorld()->WorldType != EWorldType::Editor
        && GetWorld()->WorldType != EWorldType::EditorPreview)
    {
        return;
    }

	Super::Tick(DeltaTime);

    TSet<AActor*> SelectedActors = GetWorld()->GetSelectedActors();

    // @todo 단일 선택에 대한 케이스 분리
    if (!SelectedActors.IsEmpty()) 
    {
        const AActor* PickedActor = *SelectedActors.begin();
        if (PickedActor == nullptr)
        {
            return;
        }

        SetActorLocation(PickedActor->GetActorLocation());

        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EControlMode ControlMode = EditorEngine->GetLevelEditor()->GetActiveViewportClientData().GetControlMode();
            if (ControlMode == ECoordiMode::CDM_LOCAL)
            {
                // TODO: 임시로 RootComponent의 정보로 사용
                SetActorRotation(PickedActor->GetActorRotation());
            }
            else if (ControlMode == ECoordiMode::CDM_WORLD)
            {
                SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
            }
        }
    }
}
