#include "APhysicsVehicleActor.h"

#include "Components/PrimitiveComponents/MeshComponents/StaticMeshComponents/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"
#include "Engine/Asset/AssetManager.h"
#include "Physics/Vehicle4W.h"
#include "PhysicsEngine/PhysicsMaterial.h"
#include "PhysicsEngine/PhysXSDKManager.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleConcurrency.h"

APhysicsVehicleActor::APhysicsVehicleActor()
{
}

APhysicsVehicleActor::~APhysicsVehicleActor()
{
}

void APhysicsVehicleActor::Init()
{
    GetStaticMeshComponent()->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("Dodge")));
    const FVector Min = GetStaticMeshComponent()->GetStaticMesh()->GetRenderData().BoundingBoxMin;
    const FVector Max = GetStaticMeshComponent()->GetStaticMesh()->GetRenderData().BoundingBoxMax;

    Vehicle = new FVehicle4W();
    FVector ChassisDims = Max - Min;
    FPhysXSDKManager* sdkManager = &FPhysXSDKManager::GetInstance();
    const snippetvehicle::VehicleDesc desc = FPhysXSDKManager::InitVehicleDesc(sdkManager->GetDefaultMaterial()->GetPxMaterial(), ChassisDims);
    Vehicle->Initialize(desc, sdkManager->GetPhysicsSDK(), sdkManager->GetCooking());
    
    GetStaticMeshComponent()->InitializeBodyInstance();
    GetStaticMeshComponent()->GetBodyInstance().SetRigidActor(Vehicle->GetVehicle()->getRigidDynamicActor());
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

void APhysicsVehicleActor::Tick(float DeltaTime)
{
    AStaticMeshActor::Tick(DeltaTime);
    
    Vehicle->Update(DeltaTime);
}
