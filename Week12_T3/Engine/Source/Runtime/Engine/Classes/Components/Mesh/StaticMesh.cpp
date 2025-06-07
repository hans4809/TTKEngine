#include "StaticMesh.h"

#include "LaunchEngineLoop.h"
#include "Engine/FLoaderOBJ.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderResourceManager.h"

#include "UObject/ObjectFactory.h"
#include "Physics/BodySetup/BodySetup.h"

UStaticMesh::UStaticMesh()
{

}

UStaticMesh::~UStaticMesh()
{
}

uint32 UStaticMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex].MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void UStaticMesh::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (FMaterialSlot Material : materials)
    {
        Out.Emplace(Material.Material);
    }
}

void UStaticMesh::SetData(const FStaticMeshRenderData& renderData)
{
    staticMeshRenderData = renderData;

    ID3D11Buffer* VB = nullptr; 
    ID3D11Buffer* IB = nullptr;

    const uint32 verticeNum = staticMeshRenderData.Vertices.Num();
    if (verticeNum <= 0) return;

    FRenderResourceManager* renderResourceManager = GEngineLoop.Renderer.GetResourceManager();
    VB = renderResourceManager->CreateImmutableVertexBuffer<FVertexSimple>(staticMeshRenderData.Vertices);
    renderResourceManager->AddOrSetVertexBuffer(staticMeshRenderData.DisplayName, VB);
    GEngineLoop.Renderer.MappingVBTopology(staticMeshRenderData.DisplayName, staticMeshRenderData.DisplayName, sizeof(FVertexSimple), verticeNum);
    
    const uint32 indexNum = staticMeshRenderData.Indices.Num();
    if (indexNum > 0)
    {
        IB = renderResourceManager->CreateIndexBuffer(staticMeshRenderData.Indices);
        renderResourceManager->AddOrSetIndexBuffer(staticMeshRenderData.DisplayName, IB);
    }
    GEngineLoop.Renderer.MappingVBTopology(staticMeshRenderData.DisplayName, staticMeshRenderData.DisplayName, sizeof(FVertexSimple), verticeNum);
    GEngineLoop.Renderer.MappingIB(staticMeshRenderData.DisplayName, staticMeshRenderData.DisplayName, indexNum);

    materials.Empty();
    for (int materialIndex = 0; materialIndex < staticMeshRenderData.Materials.Num(); materialIndex++) {
        FMaterialSlot newMaterialSlot;
        UMaterial* newMaterial = FManagerOBJ::CreateMaterial(staticMeshRenderData.Materials[materialIndex]);

        newMaterialSlot.Material = newMaterial;
        newMaterialSlot.MaterialSlotName = staticMeshRenderData.Materials[materialIndex].MTLName;

        materials.Add(newMaterialSlot);
    }
}

void UStaticMesh::SetBodySetup(UBodySetup* bodySetup)
{
    BodySetup = bodySetup;
}

UBodySetup* UStaticMesh::GetBodySetup()
{
    return BodySetup;
}

bool UStaticMesh::LoadFromFile(const FString& FilePath)
{
    if (FLoaderOBJ::ParseOBJ(FilePath, staticMeshRenderData) == false)
        return false;

    return true;
}

bool UStaticMesh::SerializeToFile(std::ostream& Out)
{
    return UAsset::SerializeToFile(Out);
}

bool UStaticMesh::DeserializeFromFile(std::istream& In)
{
    return UAsset::DeserializeFromFile(In);
}

void UStaticMesh::PostLoad()
{
    ID3D11Buffer* VB = nullptr; 
    ID3D11Buffer* IB = nullptr;

    const uint32 verticeNum = staticMeshRenderData.Vertices.Num();
    if (verticeNum <= 0) return;

    FRenderResourceManager* renderResourceManager = GEngineLoop.Renderer.GetResourceManager();
    VB = renderResourceManager->CreateImmutableVertexBuffer<FVertexSimple>(staticMeshRenderData.Vertices);
    renderResourceManager->AddOrSetVertexBuffer(staticMeshRenderData.DisplayName, VB);
    GEngineLoop.Renderer.MappingVBTopology(staticMeshRenderData.DisplayName, staticMeshRenderData.DisplayName, sizeof(FVertexSimple), verticeNum);
    
    const uint32 indexNum = staticMeshRenderData.Indices.Num();
    if (indexNum > 0)
    {
        IB = renderResourceManager->CreateIndexBuffer(staticMeshRenderData.Indices);
        renderResourceManager->AddOrSetIndexBuffer(staticMeshRenderData.DisplayName, IB);
    }
    GEngineLoop.Renderer.MappingVBTopology(staticMeshRenderData.DisplayName, staticMeshRenderData.DisplayName, sizeof(FVertexSimple), verticeNum);
    GEngineLoop.Renderer.MappingIB(staticMeshRenderData.DisplayName, staticMeshRenderData.DisplayName, indexNum);

    materials.Empty();
    for (int materialIndex = 0; materialIndex < staticMeshRenderData.Materials.Num(); materialIndex++)
    {
        FMaterialSlot newMaterialSlot;
        UMaterial* newMaterial = FManagerOBJ::CreateMaterial(staticMeshRenderData.Materials[materialIndex]);

        newMaterialSlot.Material = newMaterial;
        newMaterialSlot.MaterialSlotName = staticMeshRenderData.Materials[materialIndex].MTLName;

        materials.Add(newMaterialSlot);
    }

    UBodySetup* newBodySetup = FObjectFactory::ConstructObject<UBodySetup>(this);

    SetBodySetup(newBodySetup);

    const FVector Min = staticMeshRenderData.BoundingBoxMin;
    const FVector Max = staticMeshRenderData.BoundingBoxMax;
    const FVector Center = (Min + Max) * 0.5f; // 메시 로컬 공간에서의 AABB 중심
    const FVector Extents = (Max - Min) * 0.5f; // AABB의 half-extents

    // (A) AABB 기반 박스 추가
    {
        FKBoxElem BoxElem;
        BoxElem.Center = Center; // 또는 FVector::ZeroVector 만약 BoxElem.Transform으로 위치 관리
        BoxElem.Rotation = FQuat::Identity;
        BoxElem.X = Extents.X * 2.0f; // 전체 크기
        BoxElem.Y = Extents.Y * 2.0f;
        BoxElem.Z = Extents.Z * 2.0f;
        newBodySetup->AggGeom.BoxElems.Add(BoxElem);
    }

    // (B) AABB 기반 스피어 추가
    {
        FKSphereElem SphereElem;
        SphereElem.Center = Center;
        SphereElem.Radius = Extents.X;
        newBodySetup->AggGeom.SphereElems.Add(SphereElem);
    }

    // (C) AABB 기반 캡슐 추가 (예시: Z축이 긴 캡슐)
    {
        FKSphylElem SphylElem;
        SphylElem.Center = Center;
        SphylElem.Radius = FMath::Max(Extents.X, Extents.Y);
        SphylElem.Length = Extents.Z;
        SphylElem.Rotation = FQuat(FVector::XAxisVector, FMath::DegreesToRadians(-90.0f));
        newBodySetup->AggGeom.SphylElems.Add(SphylElem);
    }

    {
        FKConvexElem ConvexElem;

        for (const FVertexSimple& vert : staticMeshRenderData.Vertices)
        {
            ConvexElem.VertexData.Add(FVector(vert.x, vert.y, vert.z));
        }

        ConvexElem.SetTransform(FTransform::Identity);
        ConvexElem.UpdateElemBox();
        newBodySetup->AggGeom.ConvexElems.Add(ConvexElem);
    }

    // (4) PhysicsCooking: PhysX SDK를 위해 물리 메쉬 생성
    newBodySetup->InvalidatePhysicsData();   // 내부 데이터 초기화 플래그
    newBodySetup->CreatePhysicsMeshes();     // Cook(PhysX Collision Data) 호출

}
