#include "SkeletalMesh.h"

#include "Define.h"
#include "Renderer/RenderResourceManager.h"
#include "LaunchEngineLoop.h"
#include "Renderer/Renderer.h"
#include "Engine/FBXLoader.h"
#include "Animation/Skeleton.h"
#include "Components/Material/Material.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/Asset/AssetManager.h"

uint32 USkeletalMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < MaterialSlots.Num(); materialIndex++) {
        if (MaterialSlots[materialIndex].MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void USkeletalMesh::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (const FMaterialSlot& Material : MaterialSlots)
    {
        Out.Emplace(Material.Material);
    }
}

void USkeletalMesh::SetData(const FSkeletalMeshRenderData& InRenderData, USkeleton* InSkeleton)
{
    SkeletalMeshRenderData = InRenderData;
    Skeleton = InSkeleton;

    const uint32 verticeNum = SkeletalMeshRenderData.Vertices.Num();
    if (verticeNum <= 0) return;

    FRenderResourceManager* renderResourceManager = GEngineLoop.Renderer.GetResourceManager();
    SkeletalMeshRenderData.VB = renderResourceManager->CreateDynamicVertexBuffer<FSkeletalVertex>(SkeletalMeshRenderData.Vertices);
    
    const uint32 indexNum = SkeletalMeshRenderData.Indices.Num();
    if (indexNum > 0)
    {
        SkeletalMeshRenderData.IB = renderResourceManager->CreateIndexBuffer(SkeletalMeshRenderData.Indices);
    }

    MaterialSlots.Empty();
    for (int materialIndex = 0; materialIndex < Skeleton->GetRefSkeletal().Materials.Num(); materialIndex++) {
        FMaterialSlot newMaterialSlot;
        // Change

        newMaterialSlot.Material = FManagerOBJ::GetMaterial(Skeleton->GetRefSkeletal().Materials[materialIndex]->GetMaterialInfo().MTLName);
        newMaterialSlot.MaterialSlotName = Skeleton->GetRefSkeletal().Materials[materialIndex]->GetMaterialInfo().MTLName;
        
        MaterialSlots.Add(newMaterialSlot);
    }
}

void USkeletalMesh::UpdateBoneHierarchy()
{
    // 먼저 루트 뼈들의 글로벌 트랜스폼을 설정
    for (int32 RootIndex : Skeleton->GetRefSkeletal().RootBoneIndices)
    {
        // 루트 뼈는 로컬 트랜스폼이 곧 글로벌 트랜스폼이 됨
        SkeletalMeshRenderData.Bones[RootIndex].GlobalTransform =
            SkeletalMeshRenderData.Bones[RootIndex].LocalTransform;

        SkeletalMeshRenderData.Bones[RootIndex].SkinningMatrix =
            SkeletalMeshRenderData.Bones[RootIndex].InverseBindPoseMatrix * SkeletalMeshRenderData.Bones[RootIndex].GlobalTransform;
        
        // 재귀적으로 자식 뼈들의 글로벌 트랜스폼을 업데이트
        UpdateChildBones(RootIndex);
    }
}

void USkeletalMesh::UpdateChildBones(int ParentIndex)
{
    // BoneTree 구조를 사용하여 현재 부모 뼈의 모든 자식을 찾음
    const FBoneNode& ParentNode = Skeleton->GetRefSkeletal().BoneTree[ParentIndex];
    
    // 모든 자식 뼈를 순회
    for (int32 ChildIndex : ParentNode.ChildIndices)
    {
        // 자식의 글로벌 트랜스폼은 부모의 글로벌 트랜스폼과 자식의 로컬 트랜스폼을 결합한 것
        SkeletalMeshRenderData.Bones[ChildIndex].GlobalTransform
        = SkeletalMeshRenderData.Bones[ChildIndex].LocalTransform * SkeletalMeshRenderData.Bones[ParentIndex].GlobalTransform;

        SkeletalMeshRenderData.Bones[ChildIndex].SkinningMatrix
        = SkeletalMeshRenderData.Bones[ChildIndex].InverseBindPoseMatrix * SkeletalMeshRenderData.Bones[ChildIndex].GlobalTransform;
        
        // 재귀적으로 이 자식의 자식들도 업데이트
        UpdateChildBones(ChildIndex);
    }
}

// USkeletalMesh 클래스에 추가할 함수
void USkeletalMesh::RotateBoneByName(const FString& BoneName, float DeltaAngleInDegrees, const FVector& RotationAxis)
{
    // 이름으로 본 인덱스 찾기
    int targetBoneIndex = FindBoneIndexByName(BoneName);

    // 본을 찾지 못한 경우
    if (targetBoneIndex < 0)
        return;

    RotateBoneByIndex(targetBoneIndex, DeltaAngleInDegrees, RotationAxis);
}

void USkeletalMesh::RotateBoneByIndex(int32 BoneIndex, float DeltaAngleInDegrees, const FVector& RotationAxis, bool bIsChildUpdate)
{
    // 본을 찾지 못한 경우
    if (BoneIndex < 0)
        return;

    // 회전 행렬 생성 및 적용
    ApplyRotationToBone(BoneIndex, DeltaAngleInDegrees, RotationAxis);

    if (bIsChildUpdate)
    {
        // 계층 구조 업데이트
        UpdateBoneHierarchy();

        // 스키닝 적용 및 버퍼 업데이트
        UpdateSkinnedVertices();
    }
}

int USkeletalMesh::FindBoneIndexByName(const FString& BoneName) const
{
    if (SkeletalMeshRenderData.Bones.Num() == 0)
        return -1;

    for (int i = 0; i < SkeletalMeshRenderData.Bones.Num(); i++)
    {
        if (SkeletalMeshRenderData.Bones[i].BoneName == BoneName)
            return i;
    }

    return -1;
}

void USkeletalMesh::ApplyRotationToBone(int BoneIndex, float DeltaAngleInDegrees, const FVector& RotationAxis)
{
    if (BoneIndex < 0 || BoneIndex >= SkeletalMeshRenderData.Bones.Num())
        return;

    float angleInRadians = DeltaAngleInDegrees * 3.14159f / 180.0f;
    FMatrix rotationMatrix = FMatrix::Identity;

    // X축 회전
    if (RotationAxis.X != 0.0f)
    {
        rotationMatrix.M[1][1] = cos(angleInRadians);
        rotationMatrix.M[1][2] = -sin(angleInRadians);
        rotationMatrix.M[2][1] = sin(angleInRadians);
        rotationMatrix.M[2][2] = cos(angleInRadians);
    }
    // Y축 회전
    else if (RotationAxis.Y != 0.0f)
    {
        rotationMatrix.M[0][0] = cos(angleInRadians);
        rotationMatrix.M[0][2] = sin(angleInRadians);
        rotationMatrix.M[2][0] = -sin(angleInRadians);
        rotationMatrix.M[2][2] = cos(angleInRadians);
    }
    // Z축 회전
    else if (RotationAxis.Z != 0.0f)
    {
        rotationMatrix.M[0][0] = cos(angleInRadians);
        rotationMatrix.M[0][1] = -sin(angleInRadians);
        rotationMatrix.M[1][0] = sin(angleInRadians);
        rotationMatrix.M[1][1] = cos(angleInRadians);
    }

    // 본 로컬 변환에 회전 적용
    SkeletalMeshRenderData.Bones[BoneIndex].LocalTransform =
        rotationMatrix * SkeletalMeshRenderData.Bones[BoneIndex].LocalTransform;
}

bool USkeletalMesh::LoadFromFile(const FString& FilePath)
{
    if (FFBXLoader::ParseSkeletalMeshFromFBX(FilePath, SkeletalMeshRenderData) == false)
        return false;

    FString AssetName = FString(std::filesystem::path(FilePath).stem().string());
    Skeleton = UAssetManager::Get().Get<USkeleton>(AssetName);
    if (Skeleton == nullptr)
        return false;
    
    return true;
}

bool USkeletalMesh::SerializeToFile(std::ostream& Out)
{
    return UAsset::SerializeToFile(Out);
}

bool USkeletalMesh::DeserializeFromFile(std::istream& In)
{
    return UAsset::DeserializeFromFile(In);
}

void USkeletalMesh::PostLoad()
{
    const uint32 verticeNum = SkeletalMeshRenderData.Vertices.Num();
    if (verticeNum <= 0) return;

    FRenderResourceManager* renderResourceManager = GEngineLoop.Renderer.GetResourceManager();
    SkeletalMeshRenderData.VB = renderResourceManager->CreateDynamicVertexBuffer<FSkeletalVertex>(SkeletalMeshRenderData.Vertices);
    
    const uint32 indexNum = SkeletalMeshRenderData.Indices.Num();
    if (indexNum > 0)
    {
        SkeletalMeshRenderData.IB = renderResourceManager->CreateIndexBuffer(SkeletalMeshRenderData.Indices);
    }

    MaterialSlots.Empty();
    for (int materialIndex = 0; materialIndex < Skeleton->GetRefSkeletal().Materials.Num(); materialIndex++) {
        FMaterialSlot newMaterialSlot;
        // Change

        newMaterialSlot.Material = FManagerOBJ::GetMaterial(Skeleton->GetRefSkeletal().Materials[materialIndex]->GetMaterialInfo().MTLName);
        newMaterialSlot.MaterialSlotName = Skeleton->GetRefSkeletal().Materials[materialIndex]->GetMaterialInfo().MTLName;
        
        MaterialSlots.Add(newMaterialSlot);
    }
}

void USkeletalMesh::UpdateSkinnedVertices()
{
    if (SkeletalMeshRenderData.Vertices.Num() <= 0)
        return;


    if (Skeleton->GetRefSkeletal().RawVertices.Num() == SkeletalMeshRenderData.Vertices.Num())
    {
        for (int i = 0; i < SkeletalMeshRenderData.Vertices.Num(); i++)
        {
            SkeletalMeshRenderData.Vertices[i].Position = Skeleton->GetRefSkeletal().RawVertices[i].Position;
        }
    }

    // 스키닝 적용
    for (auto& Vertex : SkeletalMeshRenderData.Vertices)
    {
        Vertex.SkinningVertex(SkeletalMeshRenderData.Bones);
    }

    FRenderResourceManager* renderResourceManager = GEngineLoop.Renderer.GetResourceManager();
    SkeletalMeshRenderData.VB = renderResourceManager->CreateDynamicVertexBuffer<FSkeletalVertex>(SkeletalMeshRenderData.Vertices);
}