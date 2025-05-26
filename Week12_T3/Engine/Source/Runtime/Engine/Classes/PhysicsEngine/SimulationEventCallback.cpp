
#include "SimulationEventCallback.h"
#include "PhysScene_PhysX.h"
#include "BodyInstance.h"
#include "Components/PrimitiveComponents/PrimitiveComponent.h"
#include "GameFramework/Actor.h" 
#include "UserInterface/Console.h"
#include <PxActor.h>
#include <PxRigidActor.h>

FSimulationEventCallback::FSimulationEventCallback(FPhysScene_PhysX* OwningScene)
    :OwningPhysScene(OwningScene)
{
}

FSimulationEventCallback::~FSimulationEventCallback() {}

void FSimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
    UE_LOG(LogLevel::Display, " FSimulationEventCallback::onConstraintBreak called");

    // 필요한 로직 구현 (예: 특정 조인트 파괴 시 효과음, 게임 상태 변경 등)
    /*
        for (physx::PxU32 i = 0; i < count; ++i)
        {
            // constraints[i].constraint, constraints[i].externalReference, constraints[i].type, constraints[i].actor0, constraints[i].actor1
        }
    */
}

void FSimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
    UE_LOG(LogLevel::Display, " FSimulationEventCallback::OnWake called");
    for (physx::PxU32 i = 0; i < count; ++i) 
    {
        AActor* GameActor = GetGameActorFromActor(actors[i]);
        if (GameActor) 
        {
            // GameActor->OnPhysicsWake(); // 게임 액터에 깨어남 이벤트 전달
        }
    }
}

void FSimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
    UE_LOG(LogLevel::Display, " FSimulationEventCallback::onSleep called");

    for (physx::PxU32 i = 0; i < count; ++i) 
    {
        AActor* GameActor = GetGameActorFromActor(actors[i]);
        if (GameActor) 
        {
            // GameActor->OnPhysicsSleep(); // 게임 액터에 잠듦 이벤트 전달
        }
    }
}

void FSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    UE_LOG(LogLevel::Warning, " FSimulationEventCallback::onContact called");
    
    FBodyInstance* BodyInstanceA = GetBodyInstanceFromActor(pairHeader.actors[0]);
    FBodyInstance* BodyInstanceB = GetBodyInstanceFromActor(pairHeader.actors[1]);
    if (!BodyInstanceA || !BodyInstanceB) 
    {
        // 하나 이상의 액터가 유효한 BodyInstance를 가지고 있지 않음 (예: 씬 경계 등)
        return;
    }
    UPrimitiveComponent* CompA = BodyInstanceA->GetOwnerComponent();
    UPrimitiveComponent* CompB = BodyInstanceB->GetOwnerComponent();

    for (physx::PxU32 i = 0; i < nbPairs; ++i)
    {
        const physx::PxContactPair& cp = pairs[i];

        if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            UE_LOG(LogLevel::Warning, "eNOTIFY_TOUCH_FOUND");
           // if (CompA) CompA->HandleContact(CompB, cp, physx::PxPairFlag::eNOTIFY_TOUCH_FOUND); // 컴포넌트에 이벤트 전달
           // if (CompB) CompB->HandleContact(CompA, cp, physx::PxPairFlag::eNOTIFY_TOUCH_FOUND);
        }
        if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) 
        {
            UE_LOG(LogLevel::Warning, " eNOTIFY_TOUCH_LOST");
           // if (CompA) CompA->HandleContact(CompB, cp, physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
           // if (CompB) CompB->HandleContact(CompA, cp, physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
        }
        if (cp.events & physx::PxPairFlag::eNOTIFY_CONTACT_POINTS)
        {
            UE_LOG(LogLevel::Warning, " eNOTIFY_CONTACT_POINTS");
        }
        // ... 기타 PxPairFlag 확인 (eNOTIFY_THRESHOLD_FORCE_FOUND 등)
    }
}

void FSimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
    UE_LOG(LogLevel::Display, " FSimulationEventCallback::onTrigger called");

    for (physx::PxU32 i = 0; i < count; ++i)
    {
        const physx::PxTriggerPair& tp = pairs[i];

        // tp.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER (또는 OTHER) 로 셰이프 제거 확인 가능

        FBodyInstance* TriggerBodyInstance = GetBodyInstanceFromActor(tp.triggerActor);
        FBodyInstance* OtherBodyInstance = GetBodyInstanceFromActor(tp.otherActor);

        if (!TriggerBodyInstance || !OtherBodyInstance) continue;

        UPrimitiveComponent* TriggerComp = TriggerBodyInstance->GetOwnerComponent();
        UPrimitiveComponent* OtherComp = OtherBodyInstance->GetOwnerComponent();

        if (tp.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) 
        {
            //if (TriggerComp) TriggerComp->HandleTriggerOverlap(OtherComp, tp.triggerShape, tp.otherShape, physx::PxPairFlag::eNOTIFY_TOUCH_FOUND);
            //if (OtherComp) OtherComp->HandleTriggerOverlap(TriggerComp, tp.otherShape, tp.triggerShape, physx::PxPairFlag::eNOTIFY_TOUCH_FOUND); // 상대방에게도 알릴 수 있음
        }
        if (tp.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) 
        {
            //if (TriggerComp) TriggerComp->HandleTriggerOverlap(OtherComp, tp.triggerShape, tp.otherShape, physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
           //if (OtherComp) OtherComp->HandleTriggerOverlap(TriggerComp, tp.otherShape, tp.triggerShape, physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
        }
    }
}

void FSimulationEventCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
{
    UE_LOG(LogLevel::Display, " FSimulationEventCallback::onAdvance called");
}

FBodyInstance* FSimulationEventCallback::GetBodyInstanceFromActor(physx::PxActor* Actor) const
{
    if (Actor && Actor->userData)
    {
        return static_cast<FBodyInstance*>(Actor->userData);
    }
    return nullptr;
}

UPrimitiveComponent* FSimulationEventCallback::GetComponentFromActor(physx::PxActor* Actor) const
{
    FBodyInstance* BI = GetBodyInstanceFromActor(Actor);
    return BI ? BI->GetOwnerComponent() : nullptr;
    return nullptr;
}

AActor* FSimulationEventCallback::GetGameActorFromActor(physx::PxActor* Actor) const
{
    UPrimitiveComponent* Comp = GetComponentFromActor(Actor);
    return Comp ? Comp->GetOwner() : nullptr;
}