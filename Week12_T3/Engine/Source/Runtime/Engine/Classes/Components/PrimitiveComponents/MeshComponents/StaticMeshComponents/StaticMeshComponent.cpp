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
            return staticMesh->GetMaterials()[ElementIndex]->Material;
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

    for (const FMaterialSlot* Material : staticMesh->GetMaterials())
    {
        MaterialNames.Emplace(Material->MaterialSlotName);
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

    OBJ::FStaticMeshRenderData* renderData = staticMesh->GetRenderData();

    FVertexSimple* vertices = renderData->Vertices.GetData();
    int vCount = renderData->Vertices.Num();
    UINT* indices = renderData->Indices.GetData();
    int iCount = renderData->Indices.Num();

    if (!vertices) return 0;
    BYTE* pbPositions = reinterpret_cast<BYTE*>(renderData->Vertices.GetData());

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
    AABB = FBoundingBox(staticMesh->GetRenderData()->BoundingBoxMin, staticMesh->GetRenderData()->BoundingBoxMax);
    VBIBTopologyMappingName = staticMesh->GetRenderData()->DisplayName;
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

    Info->StaticMeshPath = staticMesh->GetRenderData()->PathName;
}
void UStaticMeshComponent::LoadAndConstruct(const FActorComponentInfo& Info)
{
    Super::LoadAndConstruct(Info);

    const FStaticMeshComponentInfo& StaticMeshInfo = static_cast<const FStaticMeshComponentInfo&>(Info);
    UStaticMesh* Mesh = FManagerOBJ::CreateStaticMesh(FString::ToFString(StaticMeshInfo.StaticMeshPath));
    SetStaticMesh(Mesh);

}
UObject* UStaticMeshComponent::Duplicate(UObject* InOuter)
{
    UStaticMeshComponent* NewComp = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComp->DuplicateSubObjects(this, InOuter);
    NewComp->PostDuplicate();
    return NewComp;
}

void UStaticMeshComponent::DuplicateSubObjects(const UObject* Source, UObject* InOuter)
{
    UMeshComponent::DuplicateSubObjects(Source, InOuter);
}

void UStaticMeshComponent::PostDuplicate() {}

void UStaticMeshComponent::TickComponent(float DeltaTime)
{
    //Timer += DeltaTime * 0.005f;
    //SetLocation(GetWorldLocation()+ (FVector(1.0f,1.0f, 1.0f) * sin(Timer)));
}

void UStaticMeshComponent::OnCreatePhysicsState()
{
    //임시 테스트 용 ---
    FTransform ShapeLocalPose = FTransform::Identity;

    BodyInstance = new FBodyInstance();
    BodyInstance->Initialize(this, FPhysXSDKManager::GetInstance().GetPhysicsSDK());
    BodyInstance->CreatePhysicsState(FTransform::Identity, EPhysBodyType::Dynamic);
    UPhysicalMaterial* MyMaterial = new UPhysicalMaterial(FPhysXSDKManager::GetInstance().GetPhysicsSDK(), 1, 1, 1);

    BodyInstance->AddBoxGeometry(FVector(.5f, .5f, .5f), MyMaterial, ShapeLocalPose);
    
    AActor* Owner = GetOwner();
    
    if (!Owner) return;
    
    FPhysScene* PhysScene = Owner->GetWorld()->GetPhysicsScene();
    BodyInstance->AddObject(PhysScene);

} 