#include "APhysicsVehicleActor.h"

APhysicsVehicleActor::APhysicsVehicleActor()
{
}

APhysicsVehicleActor::~APhysicsVehicleActor()
{
}

void APhysicsVehicleActor::PostEditChangeProperty(const FProperty* PropertyThatChanged)
{
    AActor::PostEditChangeProperty(PropertyThatChanged);
}

void APhysicsVehicleActor::BeginPlay()
{
    AActor::BeginPlay();
}

void APhysicsVehicleActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    AActor::EndPlay(EndPlayReason);
}

bool APhysicsVehicleActor::Destroy()
{
    return AActor::Destroy();
}
