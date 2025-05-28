#pragma once
#include "Engine/StaticMeshActor.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleCreate.h"

class FVehicle4W;

class APhysicsVehicleActor : public AStaticMeshActor
{
    DECLARE_CLASS(APhysicsVehicleActor, AStaticMeshActor)
public:
    APhysicsVehicleActor();
    ~APhysicsVehicleActor() override;
    void Init();
    void PostEditChangeProperty(const FProperty* PropertyThatChanged) override;
    void BeginPlay() override;
    void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    bool Destroy() override;
    void Tick(float DeltaTime) override;

    FVehicle4W* GetVehicle4W() const { return Vehicle; }
private:
    FVehicle4W* Vehicle = nullptr;
};
