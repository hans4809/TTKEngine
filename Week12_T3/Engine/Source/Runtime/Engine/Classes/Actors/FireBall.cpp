#include "FireBall.h"

#include "Components/GameFramework/ProjectileMovementComponent.h"
#include "Components/LightComponents/PointLightComponent.h"
#include "Components/PrimitiveComponents/UBillboardComponent.h"
#include "Components/PrimitiveComponents/MeshComponents/StaticMeshComponents/FireBallComponent.h"

AFireBall::AFireBall()
    : Super()
{
    FireBallComponent = AddComponent<UFireBallComponent>(EComponentOrigin::Constructor);
    LightComp = AddComponent<UPointLightComponent>(EComponentOrigin::Constructor);
    LightComp->SetIntensity(3.f);
    LightComp->SetRadius(20.f);
    ProjMovementComp = AddComponent<UProjectileMovementComponent>(EComponentOrigin::Constructor);
    BillboardComponent = AddComponent<UBillboardComponent>(EComponentOrigin::Constructor);
    BillboardComponent->SetTexture(L"Assets/Texture/spotLight.png");
}

void AFireBall::BeginPlay()
{
    Super::BeginPlay();
}

void AFireBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AFireBall::Destroyed()
{
    Super::Destroyed();
}

void AFireBall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool AFireBall::Destroy()
{
    return Super::Destroy();
}

void AFireBall::PostDuplicate()
{
    Super::PostDuplicate();
}

void AFireBall::LoadAndConstruct(const TArray<std::unique_ptr<FActorComponentInfo>>& InfoArray)
{
    AActor::LoadAndConstruct(InfoArray);
}

FActorInfo AFireBall::GetActorInfo()
{
    return AActor::GetActorInfo();
}
