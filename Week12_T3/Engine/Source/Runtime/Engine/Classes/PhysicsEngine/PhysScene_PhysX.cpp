#include "PhysScene_PhysX.h"
#include "UserInterface/Console.h"

#include "SimulationEventCallback.h"
#include "PhysicsEngine/BodyInstance.h"

#include <PxPhysicsAPI.h> 
#include <thread>
#include "PhysicsEngine/PhysXSDKManager.h"

#include <PxDefaultCpuDispatcher.h>
#include <PxScene.h>
#include <PxPhysics.h>

FPhysScene_PhysX::FPhysScene_PhysX(physx::PxPhysics* InPxSDK, physx::PxPvd* InPvd, UWorld* InOwningWorld)
    :PxSDK(InPxSDK), Pvd(InPvd), OwningEngineWorld(InOwningWorld)
{

}

FPhysScene_PhysX::~FPhysScene_PhysX()
{
    Shutdown();
}

bool FPhysScene_PhysX::Initialize()
{
    if (!PxSDK)
    {
        UE_LOG(LogLevel::Error, TEXT("FPhysScene_PhysX::Initialize: PxSDK is null."));
        return false;
    }

    if (PxSceneInstance)
    {
        UE_LOG(LogLevel::Warning, TEXT("FPhysScene_PhysX::Initialize: Scene already initialized."));
        return true; // 이미 초기화됨
    }

    physx::PxSceneDesc sceneDesc(PxSDK->getTolerancesScale());

    sceneDesc.gravity = physx::PxVec3(0.0f, 0.f, -0.3f);

    unsigned int numCores = std::thread::hardware_concurrency();

    CpuDispatcher = physx::PxDefaultCpuDispatcherCreate(numCores > 0 ? numCores - 1 : 0);

    if (!CpuDispatcher)
    {
        UE_LOG(LogLevel::Error, TEXT("FPhysScene_PhysX::Initialize: Failed to create CpuDispatcher."));
        return false;
    }
    sceneDesc.cpuDispatcher = CpuDispatcher;


    // 이벤트 콜백 설정
    EventCallback = new FSimulationEventCallback(this);

    sceneDesc.simulationEventCallback = EventCallback;

    // 씬 플래그 설정
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    PxSceneInstance = PxSDK->createScene(sceneDesc);

    if (!PxSceneInstance)
    {
        UE_LOG(LogLevel::Error, TEXT("FPhysScene_PhysX::Initialize: Failed to create PxScene."));
        CpuDispatcher = nullptr;

        delete EventCallback;
        EventCallback = nullptr;

        return false;
    }

    physx::PxPvdSceneClient* pvdClient = PxSceneInstance->getScenePvdClient();

    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    UE_LOG(LogLevel::Display, TEXT("FPhysScene_PhysX: Initialized successfully."));
    return true;
}

void FPhysScene_PhysX::Shutdown()
{
    if (PxSceneInstance)
    {
        PxSceneInstance->release();
        PxSceneInstance = nullptr;
    }

    // CpuDispatcher 해제
    if (CpuDispatcher)
    {
        CpuDispatcher = nullptr;
    }

    if (EventCallback)
    {
        delete EventCallback;
        EventCallback = nullptr;
    }
}

void FPhysScene_PhysX::AddObject(FBodyInstance* BodyInstance)
{
    if (PxSceneInstance && BodyInstance && BodyInstance->IsPhysicsStateCreated())
    {
        physx::PxRigidActor* ActorToAdd = BodyInstance->GetPxRigidActor();

        // 씬에 추가하기 전에 액터의 상태를 확인하거나 설정할 수 있음
        ActorToAdd->setActorFlag(physx::PxActorFlag::eVISUALIZATION, true);

        /*
        * eVISUALIZATION: PhysX Visual Debugger (PVD)에 시각화 정보(모양, 위치 등)를 전송
        * eDISABLE_GRAVITY: 해당 액터에 중력이 적용되지 않도록 합니다.
        * eSEND_SLEEP_NOTIFIES: 해당 액터가 잠들거나 깨어날 때 onWake/onSleep 콜백이 호출되도록 합니다. (씬 플래그 eENABLE_ACTIVE_ACTORS도 필요할 수 있음)
        * eDISABLE_SIMULATION: 해당 액터에 대한 물리 시뮬레이션을 완전히 비활성화합니다. 액터는 씬에 존재하지만 어떤 물리적 상호작용도 하지 않습니다.
          (씬에서 제거하는 것과는 다름)
        */

        if (ActorToAdd)
        {
            PxSceneInstance->addActor(*ActorToAdd);
        }
        else
        {
            UE_LOG(LogLevel::Warning, TEXT("FPhysScene_PhysX::AddActorToScene: PxRigidActor is null in BodyInstance."));
        }

    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FPhysScene_PhysX::AddActorToScene: Invalid PxScene, BodyInstance, or PxRigidActor."));
    }
}

void FPhysScene_PhysX::RemoveObject(FBodyInstance* BodyInstance)
{
    if (PxSceneInstance && BodyInstance && BodyInstance->GetPxRigidActor())
    {
        physx::PxRigidActor* ActorToRemove = BodyInstance->GetPxRigidActor();
        PxSceneInstance->removeActor(*ActorToRemove);

    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FPhysScene_PhysX::RemoveActorFromScene: Invalid PxScene, BodyInstance, or PxRigidActor."));
    }
}

void FPhysScene_PhysX::Simulate(float DeltaTime)
{
    if (DeltaTime <= 0) return;
    PxSceneInstance->simulate(DeltaTime);
    PxSceneInstance->fetchResults(true); //블로킹 방식으로 결과 가져오기
}

bool FPhysScene_PhysX::RaycastSingle(const FVector& Origin, const FVector& Direction, float MaxDistance, FHitResult& OutHit, const physx::PxQueryFilterData& FilterData, physx::PxHitFlags InQueryFlags)
{
    if (!PxSceneInstance)
    {
        return false;
    }

    physx::PxVec3 PxOrigin = Origin.ToPxVec3();
    physx::PxVec3 PxUnitDir = Direction.ToPxVec3().getNormalized();
    physx::PxRaycastBuffer PxHitBuffer;

    // PxHitFlag::eDEFAULT를 직접 사용하거나, 인자로 받은 InQueryFlags 사용
    bool bBlockingHit = PxSceneInstance->raycast(PxOrigin, PxUnitDir, MaxDistance, PxHitBuffer, InQueryFlags, FilterData);

    if (bBlockingHit)
    {
        // PxHitBuffer.block (PxRaycastHit 타입)에는 ePOSITION, eNORMAL, eFACE_INDEX 정보가 채워져 있을 것
        OutHit = FHitResult::FromPxRaycastHit(PxHitBuffer.block);
    }

    return bBlockingHit;
}

void FPhysScene_PhysX::SetGravity(const FVector& NewGravity)
{
    if (PxSceneInstance)
    {
        PxSceneInstance->setGravity(NewGravity.ToPxVec3());
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FPhysScene_PhysX::SetGravity: PxSceneInstance is null."));
    }
}

physx::PxScene* FPhysScene_PhysX::GetPxScene()
{
    return PxSceneInstance;
}

physx::PxU32 FPhysScene_PhysX::GetNbActors(physx::PxActorTypeFlags Flags) const
{
    // PhysX PxScene 객체의 getNbActors 호출
    return PxSceneInstance->getNbActors(Flags);
}

void FPhysScene_PhysX::GetActors(physx::PxActorTypeFlags Flags, physx::PxActor** OutActors, physx::PxU32 MaxCount) const
{
    PxSceneInstance->getActors(Flags, OutActors, MaxCount);
}