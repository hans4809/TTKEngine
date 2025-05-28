#pragma once
#include "Engine/StaticMeshActor.h"

class FVehicle4W;

class APhysicsVehicleActor : public AStaticMeshActor
{
    DECLARE_CLASS(APhysicsVehicleActor, AStaticMeshActor)
public:
    APhysicsVehicleActor();
    ~APhysicsVehicleActor() override;
    void PostEditChangeProperty(const FProperty* PropertyThatChanged) override;
    void BeginPlay() override;
    void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    bool Destroy() override;

private:
    FVehicle4W*             PxVehicle = nullptr;
};
