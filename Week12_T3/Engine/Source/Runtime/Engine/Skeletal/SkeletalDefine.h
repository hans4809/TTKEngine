#pragma once
#include "Define.h"
#include "Container/Array.h"
#include "Container/Map.h"
#include "Math/BoundingBox.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"


#pragma region Skeletal;

class UMaterial;

struct FBone
{
    DECLARE_STRUCT(FBone)
    
    UPROPERTY(VisibleAnywhere, FString, BoneName, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, FMatrix, SkinningMatrix, = FMatrix::Identity)
    UPROPERTY(VisibleAnywhere, FMatrix, InverseBindPoseMatrix, = FMatrix::Identity)
    UPROPERTY(VisibleAnywhere, FMatrix, GlobalTransform, = FMatrix::Identity)
    UPROPERTY(VisibleAnywhere, FMatrix, LocalTransform, = FMatrix::Identity)
    UPROPERTY(VisibleAnywhere, int, ParentIndex, = -1)

    void Serialize(FArchive& Ar) const
    {
        Ar << BoneName
            << SkinningMatrix
            << InverseBindPoseMatrix
            << GlobalTransform
            << LocalTransform
            << ParentIndex;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> BoneName
            >> SkinningMatrix
            >> InverseBindPoseMatrix
            >> GlobalTransform
            >> LocalTransform
            >> ParentIndex;
    }
};

struct FBoneNode
{
    DECLARE_STRUCT(FBoneNode)
    UPROPERTY(VisibleAnywhere, FString, BoneName, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, int, BoneIndex, = -1)
    UPROPERTY(VisibleAnywhere, TArray<int>, ChildIndices, = {})

    void Serialize(FArchive& Ar) const
    {
        Ar << BoneName << BoneIndex << ChildIndices;
    }
    
    void Deserialize(FArchive& Ar)
    {
        Ar >> BoneName >> BoneIndex >> ChildIndices;
    }
};

struct FSkeletalVertex
{
    DECLARE_STRUCT(FSkeletalVertex)

    FSkeletalVertex()
    {
    }
    
    UPROPERTY(FVector4, Position)
    UPROPERTY(FVector, Normal)
    UPROPERTY(FVector4, Tangent)
    UPROPERTY(FVector2D, TexCoord)
    UPROPERTY(int32, BoneIndices0)
    UPROPERTY(int32, BoneIndices1)
    UPROPERTY(int32, BoneIndices2)
    UPROPERTY(int32, BoneIndices3)
    UPROPERTY(float, BoneWeights0)
    UPROPERTY(float, BoneWeights1)
    UPROPERTY(float, BoneWeights2)
    UPROPERTY(float, BoneWeights3)
    
    void SkinningVertex(const TArray<FBone>& bones);

    void Serialize(FArchive& Ar) const
    {
        Ar << Position << Normal << Tangent << TexCoord;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> Position >> Normal >> Tangent >> TexCoord;
    }
    
private:
    FVector SkinVertexPosition(const TArray<FBone>& bones) const;
};


struct FRefSkeletal
{
    DECLARE_STRUCT(FRefSkeletal)
    
    // Tree structure for bones
    UPROPERTY(VisibleAnywhere, FString, Name, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, TArray<FSkeletalVertex>, RawVertices, = {})
    UPROPERTY(VisibleAnywhere, TArray<FBone>, RawBones, ={})
    UPROPERTY(VisibleAnywhere, TArray<FBoneNode>, BoneTree, = {})
    UPROPERTY(VisibleAnywhere, TArray<int>, RootBoneIndices, = {})
    using StringToIntMapType = TMap<FString, int>;
    UPROPERTY(VisibleAnywhere, StringToIntMapType, BoneNameToIndexMap, = {})
    UPROPERTY(VisibleAnywhere, TArray<UMaterial*>, Materials, ={})
    UPROPERTY(VisibleAnywhere, TArray<FMaterialSubset>, MaterialSubsets, = {})

    void Serialize(FArchive& Ar) const;

    void Deserialize(FArchive& Ar);

    FName GetBoneName(const int32 BoneIndex) const;
    int32 GetParentIndex(int32 InBoneIndex);
    int32 FindBoneIndex(FName BoneName) const;
    bool BoneIsChildOf(const int32 ChildBoneIndex, const int32 ParentBoneIndex);

    bool IsValidIndex(int32 Index) const;
};

struct FSkeletalMeshRenderData
{
    DECLARE_STRUCT(FSkeletalMeshRenderData)
    
    // @todo PreviewName과 FilePath 분리하기
    UPROPERTY(VisibleAnywhere, FString, Name, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, TArray<FSkeletalVertex>, Vertices, = {})
    UPROPERTY(VisibleAnywhere, TArray<uint32>, Indices, = {})
    UPROPERTY(VisibleAnywhere, TArray<FBone>, Bones, = {})
    UPROPERTY(VisibleAnywhere, FBoundingBox, BoundingBox, = {})
    ID3D11Buffer* VB = nullptr;
    ID3D11Buffer* IB = nullptr;

    void Serialize(FArchive& Ar) const
    {
        Ar << Name;
        Ar << Vertices;
        Ar << Indices;
        Ar << Bones;
        Ar << BoundingBox;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> Name >> Vertices >> Indices >> Bones >> BoundingBox;
    }
};
#pragma endregion