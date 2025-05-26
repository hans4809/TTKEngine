#include "RotatingMovementComponent.h"

#include "Components/PrimitiveComponents/PrimitiveComponent.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

URotatingMovementComponent::URotatingMovementComponent()
{
}


void URotatingMovementComponent::TickComponent(float DeltaTime)
{
    const FRotator OldRotation = UpdatedComponent->GetWorldRotation();
    const FRotator DeltaRotation = FRotator(RotationRate * DeltaTime / 1000.0f);
    FRotator NewRotation = OldRotation.Add(DeltaRotation.Pitch, DeltaRotation.Yaw, DeltaRotation.Roll);

    UpdatedComponent->SetRelativeRotation(NewRotation);
}

void URotatingMovementComponent::PostDuplicate()
{
    UMovementComponent::PostDuplicate();
}

std::unique_ptr<FActorComponentInfo> URotatingMovementComponent::GetComponentInfo()
{
    auto Info = std::make_unique<FRotatingMovementComponentInfo>();
    SaveComponentInfo(*Info);
    
    return Info;
}

void URotatingMovementComponent::SaveComponentInfo(FActorComponentInfo& OutInfo)
{
    FRotatingMovementComponentInfo* Info = static_cast<FRotatingMovementComponentInfo*>(&OutInfo);
    Super::SaveComponentInfo(*Info);
    Info->RotationRate = RotationRate;
}

void URotatingMovementComponent::LoadAndConstruct(const FActorComponentInfo& Info)
{
    Super::LoadAndConstruct(Info);
    const FRotatingMovementComponentInfo& RotatingMovementInfo = static_cast<const FRotatingMovementComponentInfo&>(Info);
    RotationRate = RotatingMovementInfo.RotationRate;
}
