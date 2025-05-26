#include "Light.h"
#include "Components/PrimitiveComponents/UBillboardComponent.h"
#include "Components/LightComponents/LightComponent.h"

ALight::ALight()
    : Super()
{
    BillboardComponent = AddComponent<UBillboardComponent>(EComponentOrigin::Constructor);
}

ALight::~ALight()
{
}

void ALight::BeginPlay()
{
    Super::BeginPlay();
}

void ALight::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
}

void ALight::Destroyed()
{
    Super::Destroyed();
}

void ALight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool ALight::Destroy()
{
    return Super::Destroy();
}

void ALight::PostDuplicate()
{
    Super::PostDuplicate();
}

void ALight::LoadAndConstruct(const TArray<std::unique_ptr<FActorComponentInfo>>& InfoArray)
{
    Super::LoadAndConstruct(InfoArray);
}

FActorInfo ALight::GetActorInfo()
{
    return Super::GetActorInfo();
}

