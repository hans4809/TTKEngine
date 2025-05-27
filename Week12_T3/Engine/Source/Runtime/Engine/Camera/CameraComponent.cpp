#include "CameraComponent.h"

#include "EditorEngine.h"
#include "LaunchEngineLoop.h"
#include "Math/JungleMath.h"
#include "Engine/World.h"
#include "UnrealEd/EditorViewportClient.h"

#include "Classes/GameFramework/Actor.h"

UCameraComponent::UCameraComponent()
{
}

void UCameraComponent::InitializeComponent()
{
	Super::InitializeComponent();
	//RelativeLocation = FVector(0.0f, 0.0f, 0.5f);
	FOV = 60.f;
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    SetRelativeRotation(RelativeRotation);
}

void UCameraComponent::PostDuplicate()
{
    Super::PostDuplicate();
}

FMatrix UCameraComponent::GetViewMatrix() const
{
    //FVector CameraPos = GetWorldLocation();
    FVector CameraPos = GetWorldMatrix().TransformPosition(GetRelativeLocation());
    FVector CameraForward = GetWorldForwardVector();
    //FVector4 CameraForward4 = FMatrix::TransformVector(FVector4(1, 0, 0, 0), GetWorldMatrix());
    //FVector CameraForward = FVector(CameraForward4.x, CameraForward4.y, CameraForward4.z);
    FVector CameraUP = FVector(0, 0, 1);

    return JungleMath::CreateViewMatrix(CameraPos, CameraPos + CameraForward, CameraUP);
}