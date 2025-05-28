#pragma once
#include "Math/Vector.h"
#include "HitResult.h"
#include <PxScene.h>    
#include <PxQueryFiltering.h> 
namespace physx
{
    class PxPhysics;
    class PxScene;
    class PxCpuDispatcher;
    class PxRigidActor;
    struct PxQueryFilterData;
}


class FBodyInstance;

class FPhysScene 
{
public:
    virtual ~FPhysScene() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void AddObject(FBodyInstance* BodyInstance) = 0;
    virtual void RemoveObject(FBodyInstance* BodyInstance) = 0;
    virtual void Simulate(float DeltaTime) = 0;
    virtual physx::PxU32 GetNbActors(physx::PxActorTypeFlags Flags) const = 0;
    virtual void GetActors(physx::PxActorTypeFlags Flags, physx::PxActor** OutActors, physx::PxU32 MaxCount) const = 0;
    // virtual void FetchResults(bool bBlock) = 0; // Simulate에 통합될 수도 있음
    
    // 가장 가까운 첫 번째 교차점에 대한 정보만을 반환
    virtual bool RaycastSingle(const FVector& Origin, const FVector& Direction, float MaxDistance, FHitResult& OutHit,
        const physx::PxQueryFilterData& FilterData, physx::PxHitFlags InQueryFlags = physx::PxHitFlags(physx::PxHitFlag::eDEFAULT)) = 0;
    virtual void SetGravity(const FVector& NewGravity) = 0;
    virtual physx::PxScene* GetPxScene() = 0; // PxScene 생성에 사용된 SDK
        
protected:
    physx::PxScene* PxSceneInstance;

};