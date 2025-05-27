#include "DirectionalLightActor.h"

#include "Components/LightComponents/DirectionalLightComponent.h"
#include "Components/PrimitiveComponents/UBillboardComponent.h"


ADirectionalLightActor::ADirectionalLightActor()
    : Super()
{
    LightComponent = AddComponent<UDirectionalLightComponent>(EComponentOrigin::Constructor);
    BillboardComponent->SetTexture(L"S_LightDirectional");
}

void ADirectionalLightActor::BeginPlay()
{
    Super::BeginPlay();
}

void ADirectionalLightActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADirectionalLightActor::Destroyed()
{
    Super::Destroyed();
}

void ADirectionalLightActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool ADirectionalLightActor::Destroy()
{
    return Super::Destroy();
}

void ADirectionalLightActor::PostDuplicate()
{
    Super::PostDuplicate();
}

void ADirectionalLightActor::LoadAndConstruct(const TArray<std::unique_ptr<FActorComponentInfo>>& InfoArray)
{
    Super::LoadAndConstruct(InfoArray);
}

FActorInfo ADirectionalLightActor::GetActorInfo()
{
    return Super::GetActorInfo();
}
