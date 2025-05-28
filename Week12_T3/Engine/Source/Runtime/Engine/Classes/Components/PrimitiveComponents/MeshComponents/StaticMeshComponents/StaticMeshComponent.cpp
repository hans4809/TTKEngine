#include "StaticMeshComponent.h"

#include "Engine/World.h"
#include "Launch/EditorEngine.h"
#include "UObject/ObjectFactory.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Classes/Engine/FLoaderOBJ.h"
#include "Components/Mesh/StaticMesh.h"
#include "CoreUObject/UObject/Casts.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysXSDKManager.h"
#include "PhysicsEngine/PhysicsMaterial.h"
#include "PhysicsEngine/PhysScene_PhysX.h"

#include "Physics/BodySetup/BodySetup.h"
#include <PxRigidActor.h>
#include <PxScene.h>

#include "Engine/Asset/AssetManager.h"


uint32 UStaticMeshComponent::GetNumMaterials() const
{
    if (staticMesh == nullptr) return 0;

    return staticMesh->GetMaterials().Num();
}

UMaterial* UStaticMeshComponent::GetMaterial(uint32 ElementIndex) const
{
    if (staticMesh != nullptr)
    {
        if (OverrideMaterials[ElementIndex] != nullptr)
        {
            return OverrideMaterials[ElementIndex];
        }

        if (staticMesh->GetMaterials().IsValidIndex(ElementIndex))
        {
            return staticMesh->GetMaterials()[ElementIndex].Material;
        }
    }
    return nullptr;
}

uint32 UStaticMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
    if (staticMesh == nullptr) return -1;

    return staticMesh->GetMaterialIndex(MaterialSlotName);
}

TArray<FName> UStaticMeshComponent::GetMaterialSlotNames() const
{
    TArray<FName> MaterialNames;
    if (staticMesh == nullptr) return MaterialNames;

    for (FMaterialSlot Material : staticMesh->GetMaterials())
    {
        MaterialNames.Emplace(Material.MaterialSlotName);
    }

    return MaterialNames;
}

void UStaticMeshComponent::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    if (staticMesh == nullptr) return;
    staticMesh->GetUsedMaterials(Out);
    for (int materialIndex = 0; materialIndex < GetNumMaterials(); materialIndex++)
    {
        if (OverrideMaterials[materialIndex] != nullptr)
        {
            Out[materialIndex] = OverrideMaterials[materialIndex];
        }
    }
}

int UStaticMeshComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    if (!AABB.IntersectRay(rayOrigin, rayDirection, pfNearHitDistance)) return 0;
    int nIntersections = 0;
    if (staticMesh == nullptr) return 0;

    FStaticMeshRenderData renderData = staticMesh->GetRenderData();

    FVertexSimple* vertices = renderData.Vertices.GetData();
    int vCount = renderData.Vertices.Num();
    UINT* indices = renderData.Indices.GetData();
    int iCount = renderData.Indices.Num();

    if (!vertices) return 0;
    BYTE* pbPositions = reinterpret_cast<BYTE*>(renderData.Vertices.GetData());

    int nPrimitives = (!indices) ? (vCount / 3) : (iCount / 3);
    float fNearHitDistance = FLT_MAX;
    for (int i = 0; i < nPrimitives; i++) {
        int idx0, idx1, idx2;
        if (!indices) {
            idx0 = i * 3;
            idx1 = i * 3 + 1;
            idx2 = i * 3 + 2;
        }
        else {
            idx0 = indices[i * 3];
            idx2 = indices[i * 3 + 1];
            idx1 = indices[i * 3 + 2];
        }

        // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
        uint32 stride = sizeof(FVertexSimple);
        FVector v0 = *reinterpret_cast<FVector*>(pbPositions + idx0 * stride);
        FVector v1 = *reinterpret_cast<FVector*>(pbPositions + idx1 * stride);
        FVector v2 = *reinterpret_cast<FVector*>(pbPositions + idx2 * stride);

        float fHitDistance;
        if (IntersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, fHitDistance)) {
            if (fHitDistance < fNearHitDistance) {
                pfNearHitDistance = fNearHitDistance = fHitDistance;
            }
            nIntersections++;
        }

    }
    return nIntersections;
}


void UStaticMeshComponent::SetStaticMesh(UStaticMesh* value)
{
    staticMesh = value;
    OverrideMaterials.SetNum(value->GetMaterials().Num());
    AABB = FBoundingBox(staticMesh->GetRenderData().BoundingBoxMin, staticMesh->GetRenderData().BoundingBoxMax);
    VBIBTopologyMappingName = staticMesh->GetRenderData().DisplayName;
}

std::unique_ptr<FActorComponentInfo> UStaticMeshComponent::GetComponentInfo()
{
    auto Info = std::make_unique<FStaticMeshComponentInfo>();
    SaveComponentInfo(*Info);

    return Info;
}

void UStaticMeshComponent::SaveComponentInfo(FActorComponentInfo& OutInfo)
{
    FStaticMeshComponentInfo* Info = static_cast<FStaticMeshComponentInfo*>(&OutInfo);
    Super::SaveComponentInfo(*Info);

    Info->StaticMeshPath = staticMesh->GetRenderData().PathName;
}
void UStaticMeshComponent::LoadAndConstruct(const FActorComponentInfo& Info)
{
    Super::LoadAndConstruct(Info);

    const FStaticMeshComponentInfo& StaticMeshInfo = static_cast<const FStaticMeshComponentInfo&>(Info);
    UStaticMesh* StaticMesh = UAssetManager::Get().Get<UStaticMesh>(StaticMeshInfo.VBIBTopologyMappingName.ToString());

    SetStaticMesh(StaticMesh);
    //UStaticMesh* Mesh = FManagerOBJ::CreateStaticMesh(StaticMeshInfo.StaticMeshPath);
    //SetStaticMesh(Mesh);

}

void UStaticMeshComponent::PostDuplicate()
{
    OnCreatePhysicsState();
}

void UStaticMeshComponent::TickComponent(float DeltaTime)
{
    //Timer += DeltaTime * 0.005f;
    //SetLocation(GetWorldLocation()+ (FVector(1.0f,1.0f, 1.0f) * sin(Timer)));
}

void UStaticMeshComponent::DestroyPhysicsState()
{
    if (BodyInstance.GetPxRigidActor())
    {
        physx::PxScene* CurrentScene = BodyInstance.GetPxRigidActor()->getScene();
        if (CurrentScene)
        {
            physx::PxRigidActor* RigidActor = BodyInstance.GetPxRigidActor();
            CurrentScene->removeActor(*RigidActor);
        }
    }
    BodyInstance.ReleasePhysicsState();
}
void UStaticMeshComponent::OnCreatePhysicsState()
{

    BodyInstance.Initialize(this, FPhysXSDKManager::GetInstance().GetPhysicsSDK());

    FTransform ComponentWorldTransform = GetWorldTransform();
    FTransform LocalTransform = FTransform::Identity;
 
    bool bActorCreated = BodyInstance.CreatePhysicsState(GetWorldTransform(), BodyType);

    if (!bActorCreated || !BodyInstance.IsPhysicsStateCreated())
    {
        UE_LOG(LogLevel::Error, TEXT("OnCreatePhysicsState: Failed to create PxActor for %s"), *GetName());
        return;
    }

    if (!staticMesh)
    {
        return;
    }

    UBodySetup* Setup = staticMesh->GetBodySetup();

    if (Setup)
    {
        UPhysicalMaterial* PhysMat = GetPhysicalMaterial();
        UPhysicalMaterial* Mat = Setup->GetDefaultMaterial();
        switch (ShapeType)
        {
        case EPhysBodyShapeType::Sphere:
        {
            for (const FKSphereElem& Elem : Setup->AggGeom.SphereElems)
            {
                FVector Scale = ComponentWorldTransform.GetScale();
                float ScaledRadius = Elem.Radius * Scale.Magnitude();
                BodyInstance.AddSphereGeometry(ScaledRadius, Mat, LocalTransform);
            }
        }
        break;
        case EPhysBodyShapeType::Box:
        {
            for (const FKBoxElem& Elem : Setup->AggGeom.BoxElems)
            {
                FVector HalfExtents(Elem.X * 0.5f, Elem.Y * 0.5f, Elem.Z * 0.5f);

                FVector Scale = ComponentWorldTransform.GetScale();
                FVector ScaledHalfExtents = HalfExtents * Scale;

                BodyInstance.AddBoxGeometry(ScaledHalfExtents, Mat, LocalTransform);
            }
        }
        break;
        case EPhysBodyShapeType::Sphyl:
        {
            for (const FKSphylElem& Elem : Setup->AggGeom.SphylElems)
            {
                FVector Scale = ComponentWorldTransform.GetScale();

                float Radius = Elem.Radius * Scale.Magnitude(); // Sphyl의 반지름

                float HalfHeight = Elem.Length * 0.5f * Scale.Magnitude();
                BodyInstance.AddCapsuleGeometry(Radius, HalfHeight, Mat, LocalTransform);
            }
        }
        break;
        case EPhysBodyShapeType::Convex:
        {
            for (const FKConvexElem& Elem : Setup->AggGeom.ConvexElems)
            {

                BodyInstance.AddConvexGeometry(Elem.CookedPxConvexMesh, Mat, LocalTransform, ComponentWorldTransform.GetScale());
            }
        }
        break;
        default:
            break;
        }
        // 5. Dynamic 바디의 경우 질량 및 관성 설정
        if (BodyType != EPhysBodyType::Static)
        {
            BodyInstance.UpdateMassAndInertia(Setup->CalculateMass(this));
        }
    }

    AActor* Owner = GetOwner();

    if (!Owner) return;

    FPhysScene* PhysScene = Owner->GetWorld()->GetPhysicsScene();
    BodyInstance.AddObject(PhysScene);

}