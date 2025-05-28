#include "World.h"

#include "Renderer/Renderer.h"
#include "PlayerCameraManager.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/UObjectIterator.h"
#include "Level.h"
#include "Engine/FBXLoader.h"
#include "Actors/ADodge.h"
#include "Contents/GameManager.h"
#include "Serialization/FWindowsBinHelper.h"

#include "GameFramework//PlayerController.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystemWorldManager.h"
#include "Script/LuaManager.h"
#include "UObject/UObjectArray.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Components/PrimitiveComponents/ParticleSystemComponent.h"
#include "Particles/Modules/ParticleModuleSpawn.h"
#include "Classes/Particles/ParticleLODLevel.h"
#include "Particles/Modules/ParticleModuleRequired.h"
#include "Particles/Modules/ParticleModuleVelocity.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/Modules/ParticleModuleLifetime.h"
#include "Particles/Modules/ParticleModuleLocation.h"
#include "Particles/Modules/ParticleModuleSize.h"
#include "Particles/TypeData/ParticleModuleTypeDataBase.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"

#include "DummyObject.h"
#include "Asset/AssetManager.h"
#include "Particles/ParticleSystem.h"
#include "Particles/Modules/ParticleModuleSubUV.h"
#include "Particles/Modules/ParticleModuleColor.h"

#include "PhysicsEngine/PhysScene_PhysX.h"
#include "PhysicsEngine/PhysXSDKManager.h"
#include "PhysicsEngine/BodyInstance.h"

#include <PxScene.h>
#include <PxRigidActor.h>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h> 

void UWorld::InitWorld()
{
    FParticleSystemWorldManager::OnWorldInit(this);
    LocalGizmo = nullptr;
    // TODO: Load Scene
    if (Level == nullptr)
    {
        Level = FObjectFactory::ConstructObject<ULevel>(this);
    }
    if (!InitializePhysicsScene())
    {
        UE_LOG(LogLevel::Error, "FATAL ERROR: UWorld::InitWorld - Failed to initialize physics scene!");

    }

    /*if (WorldType == EWorldType::Editor)
    {
        LoadScene("NewScene.scene");
    }
    else
    {
        CreateBaseObject(WorldType);
    }*/
    CreateBaseObject(WorldType);

}

void UWorld::LoadLevel(const FString& LevelName)
{
    // !TODO : 레벨 로드
    // 이름으로 레벨 로드한다
    // 실패 하면 현재 레벨 유지
}

void UWorld::CreateBaseObject(EWorldType::Type WorldType)
{
    if (WorldType == EWorldType::PIE)
    {
        return;
    }

    if (LocalGizmo == nullptr && WorldType)
    {
        LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>(this);
        LocalGizmo->SetActorLabel(TEXT("Gizmo"));
    }
    /* if (WorldType == EWorldType::Editor)
     {
         AActor* TestActor = SpawnActor<AActor>();

         UCubeComp* TestComp = TestActor->AddComponent<UCubeComp>(EComponentOrigin::Editor);
         FManagerOBJ::CreateStaticMesh("Assets/Primitives/Cube.obj");
         TestComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Cube.obj"));
     }*/
}


void UWorld::ReleaseBaseObject()
{
    LocalGizmo = nullptr;
}

void UWorld::SyncPhysicsActor(physx::PxActor* PActor)
{
    switch (PActor->getType())
    {
    case physx::PxActorType::eRIGID_DYNAMIC:
    {
        FBodyInstance* BodyInst = static_cast<FBodyInstance*>(PActor->userData);
        if (BodyInst == nullptr) return;

        auto* OwnerComp = BodyInst->GetOwnerComponent();

        if (OwnerComp == nullptr) return;
        if (physx::PxRigidActor* Rigid = PActor->is<physx::PxRigidActor>())
        {
            physx::PxTransform PxT = Rigid->getGlobalPose();

            FVector Location = FVector::PToFVector(PxT.p);
            FQuat   Rotation = FQuat::PToFQuat(PxT.q);

            AActor* Actor = OwnerComp->GetOwner();
            if (Actor && !Actor->IsActorBeingDestroyed())
            {
                Actor->SetActorLocation(Location);
                Actor->SetActorRotation(Rotation.Rotator());
            }
            else
            {
                CurrentPhysicsScene->RemoveObject(BodyInst);
                Actor = nullptr;
            }
        }
    }
    break;

    case physx::PxActorType::eRIGID_STATIC:
    {
        // 정적 강체는 위치 변경이 없으므로 기본적으로 스킵
    }
    break;
    default:
    {
        // 기타 타입(eAGGREGATE, 사용자 정의 등)은 필요에 따라 처리
    }
    break;
    }
}
void UWorld::SyncPhysicsActors()
{
    if (!CurrentPhysicsScene)
        return;


    physx::PxActorTypeFlags Flags =
        physx::PxActorTypeFlag::eRIGID_DYNAMIC |
        physx::PxActorTypeFlag::eRIGID_STATIC;

    physx::PxU32 NumActors = CurrentPhysicsScene->GetNbActors(Flags);
    TArray<physx::PxActor*> PxActors;
    PxActors.SetNum(NumActors);
    CurrentPhysicsScene->GetActors(Flags, PxActors.GetData(), NumActors);

    for (physx::PxActor* PActor : PxActors)
    {
        SyncPhysicsActor(PActor);
    }
}
void UWorld::Tick(ELevelTick tickType, float deltaSeconds)
{
    if (tickType == LEVELTICK_ViewportsOnly)
    {
        if (LocalGizmo)
        {
            LocalGizmo->Tick(deltaSeconds);
        }

        FGameManager::Get().EditorTick(deltaSeconds);
    }
    if (CurrentPhysicsScene )//&& (tickType == ELevelTick::LEVELTICK_All))
    {
        CurrentPhysicsScene->Simulate(deltaSeconds); //내부에서 FetchResult 호출
        SyncPhysicsActors();
    }

    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    if (tickType == LEVELTICK_All)
    {
        FLuaManager::Get().BeginPlay();
        TSet<AActor*> PendingActors = Level->PendingBeginPlayActors;
        for (AActor* Actor : PendingActors)
        {
            Actor->BeginPlay();
            Level->PendingBeginPlayActors.Remove(Actor);
        }

        TArray CopyActors = Level->GetActors();
        for (AActor* Actor : CopyActors)
        {
            Actor->Tick(deltaSeconds);
        }

        FGameManager::Get().Tick(deltaSeconds);
    }

    FParticleSystemWorldManager::Get(this)->Tick(deltaSeconds, tickType);
}

bool UWorld::InitializePhysicsScene()
{
    if (CurrentPhysicsScene)
    {
        FPhysXSDKManager::GetInstance().Pvd->disconnect();
        FPhysXSDKManager::GetInstance().ConnectPVD();
        UE_LOG(LogLevel::Display, " UWorld::InitializePhysicsScene - Physics scene already initialized.");
        return true;
    }

    // 1. PhysX SDK가 초기화되었는지 확인하고 PxPhysics* 가져오기
    if (!FPhysXSDKManager::GetInstance().Initalize())
    {
        UE_LOG(LogLevel::Error, "UWorld::InitializePhysicsScene - FPhysXSDKManager is not initialized.");
        return false;
    }
    physx::PxPhysics* PxSDK = FPhysXSDKManager::GetInstance().GetPhysicsSDK();
    physx::PxPvd* PxPvd = FPhysXSDKManager::GetInstance().Pvd;
    if (!PxSDK)
    {
        UE_LOG(LogLevel::Error, "UWorld::InitializePhysicsScene - Failed to get PxPhysics SDK from FPhysXSDKManager.");
        return false;
    }

    // 2. FPhysScene_PhysX 인스턴스 생성 (FPhysScene 인터페이스 포인터로 받음)
    //    FPhysScene_PhysX 생성자는 PxPhysics*와 UWorld* (this)를 받을 수 있음
    FPhysScene_PhysX* NewPhysXScene = new FPhysScene_PhysX(PxSDK, PxPvd, this);
    CurrentPhysicsScene = NewPhysXScene; // FPhysScene* 타입으로 업캐스팅하여 저장

    // 3. 생성된 물리 씬 초기화 (이 내부에서 PxScene 생성)
    if (!CurrentPhysicsScene->Initialize())
    {
        UE_LOG(LogLevel::Error, "UWorld::InitializePhysicsScene - Failed to initialize FPhysScene_PhysX.");
        delete CurrentPhysicsScene;
        CurrentPhysicsScene = nullptr;
        return false;
    }
    UE_LOG(LogLevel::Display, "UWorld::InitializePhysicsScene - Physics scene initialized successfully.");
    return true;
}

void UWorld::ShutdownPhysicsScene()
{
    if (CurrentPhysicsScene)
    {
        CurrentPhysicsScene->Shutdown();
        delete CurrentPhysicsScene;
        CurrentPhysicsScene = nullptr;
    }
}
FPhysScene* UWorld::GetPhysicsScene()
{
    if (CurrentPhysicsScene)
    {
        return CurrentPhysicsScene;
    }
    else
        InitializePhysicsScene();

    return CurrentPhysicsScene;
}

void UWorld::Release()
{
    FParticleSystemWorldManager::OnWorldCleanup(this);

    if (WorldType == EWorldType::Editor)
    {
        //SaveScene("Assets/Scenes/AutoSave.Scene");
    }
    
    TArray<AActor*> Actors = Level->GetActors();
    for (AActor* Actor : Actors)
    {
        Actor->Destroy();
    }
    
    if (LocalGizmo)
    {
        LocalGizmo->Destroy();
    }
    
    ShutdownPhysicsScene();
    GUObjectArray.MarkRemoveObject(Level);
    // TODO Level -> Release로 바꾸기
    // Level->Release();
    GUObjectArray.MarkRemoveObject(this);

    pickingGizmo = nullptr;
    ReleaseBaseObject();
}

void UWorld::ClearScene()
{
    // 1. PickedActor제거
    SelectedActors.Empty();
    // 2. 모든 Actor Destroy

    for (AActor* actor : TObjectRange<AActor>())
    {
        if (actor->GetWorld() == this)
        {
            DestroyActor(actor);
        }
    }
    Level->GetActors().Empty();
    Level->PendingBeginPlayActors.Empty();
    ReleaseBaseObject();
}

void UWorld::PostDuplicate()
{
    UObject::PostDuplicate();
}

UWorld* UWorld::GetWorld() const
{
    return const_cast<UWorld*>(this);
}

void UWorld::LoadScene(const FString& FileName)
{
    ClearScene(); // 기존 오브젝트 제거
    CreateBaseObject(WorldType);
    FArchive ar;
    FWindowsBinHelper::LoadFromBin(FileName, ar);

    ar >> *this;
}

AActor* UWorld::SpawnActorByClass(UClass* ActorClass, UObject* InOuter, bool bCallBeginPlay)
{
    if (ActorClass == nullptr)
        return nullptr;

    AActor* Actor = Cast<AActor>(ActorClass->GetDefaultObject());
    if (Actor == nullptr)
        return nullptr;


    if (bCallBeginPlay)
    {
        Level->PendingBeginPlayActors.Add(Actor);
    }
    else
    {
        Level->GetActors().Add(Actor);
    }

    return Actor;
}

void UWorld::DuplicateSelectedActors()
{
    TSet<AActor*> newSelectedActors;
    for (AActor* Actor : SelectedActors)
    {
        AActor* DupedActor = Cast<AActor>(Actor->Duplicate(this));
        FString TypeName = DupedActor->GetActorLabel().Left(DupedActor->GetActorLabel().Find("_", ESearchCase::IgnoreCase, ESearchDir::FromEnd));
        DupedActor->SetActorLabel(TypeName);
        FVector DupedLocation = DupedActor->GetActorLocation();
        DupedActor->SetActorLocation(FVector(DupedLocation.X + 50, DupedLocation.Y + 50, DupedLocation.Z));
        Level->GetActors().Add(DupedActor);
        Level->PendingBeginPlayActors.Add(DupedActor);
        newSelectedActors.Add(DupedActor);
    }
    SelectedActors = newSelectedActors;
}
void UWorld::DuplicateSelectedActorsOnLocation()
{
    TSet<AActor*> newSelectedActors;
    for (AActor* Actor : SelectedActors)
    {
        AActor* DupedActor = Cast<AActor>(Actor->Duplicate(this));
        FString TypeName = DupedActor->GetActorLabel().Left(DupedActor->GetActorLabel().Find("_", ESearchCase::IgnoreCase, ESearchDir::FromEnd));
        DupedActor->SetActorLabel(TypeName);
        Level->GetActors().Add(DupedActor);
        Level->PendingBeginPlayActors.Add(DupedActor);
        newSelectedActors.Add(DupedActor);
    }
    SelectedActors = newSelectedActors;
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TArray<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    Level->GetActors().Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

void UWorld::SetPickingGizmo(UObject* Object)
{
    pickingGizmo = Cast<USceneComponent>(Object);
}

void UWorld::Serialize(FArchive& ar) const
{
    int ActorCount = Level->GetActors().Num();
    ar << ActorCount;
    for (AActor* Actor : Level->GetActors())
    {
        FActorInfo ActorInfo = (Actor->GetActorInfo());
        ar << ActorInfo;
    }
}

void UWorld::Deserialize(FArchive& ar)
{
    int ActorCount;
    ar >> ActorCount;
    for (int i = 0; i < ActorCount; i++)
    {
        FActorInfo ActorInfo;
        ar >> ActorInfo;
        UClass* ActorClass = UClassRegistry::Get().FindClassByName(ActorInfo.Type);
        if (ActorClass)
        {
            AActor* Actor = SpawnActorByClass(ActorClass, this, true);
            if (Actor)
            {
                Actor->LoadAndConstruct(ActorInfo.ComponentInfos);
                Level->GetActors().Add(Actor);
            }
        }
    }
    Level->PostLoad();

}

/*************************임시******************************/
bool UWorld::IsPIEWorld() const
{
    return false;
}

void UWorld::BeginPlay()
{
    // FGameManager::Get().BeginPlay();

    // TODO : 나중에 제대로 구현
    // if (PlayerController == nullptr)
    // {
    //     PlayerController = SpawnActor<APlayerController>();
    //
    //     bool bCharacterExist = false;
    //     for (AActor* Actor : Level->GetActors())
    //     {
    //         if (ACharacter* Character = Cast<ACharacter>(Actor))
    //         {
    //             bCharacterExist = true;
    //             PlayerController->Possess(Character);
    //             break;
    //         }
    //     }
    //
    //     if (bCharacterExist == false)
    //     {
    //         ACharacter* Character = SpawnActor<ACharacter>();
    //         PlayerController->Possess(Character);
    //         Character->SetActorScale(FVector(0.2f, 0.2f, 0.2f));
    //     }
    //
    //     APlayerCameraManager* PlayerCameraManager = SpawnActor<APlayerCameraManager>();
    //     PlayerController->SetPlayerCameraManager(PlayerCameraManager);
    // }
}

void UWorld::DuplicateSubObjects(const UObject* Source, UObject* InOuter, FObjectDuplicator& Duplicator)
{
    UObject::DuplicateSubObjects(Source, InOuter, Duplicator);
}

// AActor* SpawnActorByName(const FString& ActorName, UObject* InOuter, bool bCallBeginPlay)
// {
//     // GetWorld 문제 발생 여지 많음.
//     UClass* ActorClass = UClassRegistry::Get().FindClassByName(ActorName);
//     return GetWorld()->SpawnActorByClass(ActorClass, InOuter, bCallBeginPlay);
// }

/**********************************************************/
