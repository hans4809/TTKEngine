#pragma once
#pragma once

#include <PxSimulationEventCallback.h>
#include <PxFiltering.h>

class FPhysScene_PhysX;
class FBodyInstance;
class UPrimitiveComponent;
class AActor;

class FSimulationEventCallback : public physx::PxSimulationEventCallback 
{
public:
    // 생성자 (선택적으로 물리 씬 포인터 등을 받을 수 있음)
    FSimulationEventCallback(FPhysScene_PhysX* OwningScene = nullptr);
    virtual ~FSimulationEventCallback();

    // PxSimulationEventCallback 인터페이스 메서드 오버라이드
    virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
    virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
    virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
    virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
    virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;

private:
    FPhysScene_PhysX* OwningPhysScene;

    FBodyInstance* GetBodyInstanceFromActor(physx::PxActor* Actor) const;
    UPrimitiveComponent* GetComponentFromActor(physx::PxActor* Actor) const;
    AActor* GetGameActorFromActor(physx::PxActor* Actor) const;
};