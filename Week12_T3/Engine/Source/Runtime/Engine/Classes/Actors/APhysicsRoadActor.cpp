#include "APhysicsRoadActor.h"

#include "Components/PrimitiveComponents/MeshComponents/StaticMeshComponents/StaticMeshComponent.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"
#include "Engine/Asset/AssetManager.h"
#include "PhysicsEngine/PhysicsMaterial.h"
#include "PhysicsEngine/PhysXSDKManager.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleCreate.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleFilterShader.h"

APhysicsRoadActor::APhysicsRoadActor()
{
}

APhysicsRoadActor::~APhysicsRoadActor()
{
}

void APhysicsRoadActor::Init()
{
    //Create a plane to drive on.
    GetStaticMeshComponent()->SetStaticMesh(UAssetManager::Get().Get<UStaticMesh>(TEXT("Cube")));
    GetStaticMeshComponent()->SetRelativeLocation(FVector(0, 0, -5.0f));
    GetStaticMeshComponent()->SetRelativeScale(FVector(1000.0f, 1000.0f, 1.0f));
    physx::PxFilterData groundPlaneSimFilterData(snippetvehicle::COLLISION_FLAG_GROUND, snippetvehicle::COLLISION_FLAG_GROUND_AGAINST, 0, 0);
    gGroundPlane = snippetvehicle::createDrivablePlane(groundPlaneSimFilterData, FPhysXSDKManager::GetInstance().GetDefaultMaterial()->GetPxMaterial(), FPhysXSDKManager::GetInstance().GetPhysicsSDK());
}
