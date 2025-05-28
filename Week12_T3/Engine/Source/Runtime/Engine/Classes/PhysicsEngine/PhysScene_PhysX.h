#pragma once
#include "PhysicsCore/PhysScene.h"
#include "Math/Vector.h"

class FVehicle4W;
class UWorld;
class UPrimitiveComponent;
class FBodyInstance;
class FSimulationEventCallback;

namespace physx
{
    class PxVehicleDrive4W;
    class PxSerializationRegistry;
    class PxPvd;

};

class FPhysScene_PhysX : public FPhysScene
{
public:
    FPhysScene_PhysX(physx::PxPhysics* InPxSDK, physx::PxPvd* InPvd, UWorld* InOwningWorld);

    virtual ~FPhysScene_PhysX() override;

    virtual bool Initialize() override;

    virtual void Shutdown() override;
    // FPhysScene 인터페이스 구현
    virtual void AddObject(FBodyInstance* BodyInstance) override;

    void AddVehicle(physx::PxVehicleDrive4W* InVehicle4W) override;
    void AddActor(physx::PxActor* InRigidActor) override;

    virtual void RemoveObject(FBodyInstance* BodyInstance) override;

    virtual void Simulate(float DeltaTime) override;

    virtual bool RaycastSingle(const FVector& Origin, const FVector& Direction, float MaxDistance, FHitResult& OutHit,
        const physx::PxQueryFilterData& FilterData, physx::PxHitFlags InQueryFlags = physx::PxHitFlags(physx::PxHitFlag::eDEFAULT)) override;

    virtual void SetGravity(const FVector& NewGravity);

    virtual physx::PxU32 GetNbActors(physx::PxActorTypeFlags Flags) const override;

    virtual void GetActors(physx::PxActorTypeFlags Flags, physx::PxActor** OutActors, physx::PxU32 MaxCount) const override;
private:

    physx::PxPhysics* PxSDK; // PxScene 생성에 사용된 SDK

    physx::PxPvd* Pvd;

    UWorld* OwningEngineWorld;     // 이 씬을 사용하는 엔진 월드

    physx::PxScene* PxSceneInstance;

    physx::PxCpuDispatcher* CpuDispatcher;

    FSimulationEventCallback* EventCallback;

    FVehicle4W* Vehicle4W;
};