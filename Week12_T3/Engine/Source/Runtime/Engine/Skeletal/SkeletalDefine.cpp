#include "SkeletalDefine.h"
#include "Components/Material/Material.h"
#include "Engine/FLoaderOBJ.h"

FVector FSkeletalVertex::SkinVertexPosition(const TArray<FBone>& bones) const
{
    FVector result = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 4; ++i)
    {
        int boneIndex;
        float weight;
        if (i == 0)
        {
            boneIndex = BoneIndices0;
            weight = BoneWeights0;
        }
        else if (i == 1)
        {
            boneIndex = BoneIndices1;
            weight = BoneWeights1;
        }
        else if (i == 2)
        {
            boneIndex = BoneIndices2;
            weight = BoneWeights2;
        }
        else if (i == 3)
        {
            boneIndex = BoneIndices3;
            weight = BoneWeights3;
        }
        
        if (weight > 0.0f && boneIndex >= 0 && boneIndex < bones.Num())
        {
            const FMatrix& SkinMat = bones[boneIndex].SkinningMatrix;
            FVector transformed = SkinMat.TransformPosition(Position.xyz());
            result = result + (transformed * weight);
        }
    }

    return result;
}

void FRefSkeletal::Serialize(FArchive& Ar) const
{
    Ar << Name
        << RawVertices
        << RawBones
        << BoneTree
        << RootBoneIndices
        << BoneNameToIndexMap
        << MaterialSubsets;
    Ar << Materials.Num();
    for (const auto& material : Materials)
    {
        Ar << *material;
    }
}

void FRefSkeletal::Deserialize(FArchive& Ar)
{
    int MaterialCount;
    Ar >> Name
        >> RawVertices
        >> RawBones
        >> BoneTree
        >> RootBoneIndices
        >> BoneNameToIndexMap
        >> MaterialSubsets;
    Ar >> MaterialCount;
    Materials.SetNum(MaterialCount);
    for (auto& material : Materials)
    {
        material = FObjectFactory::ConstructObject<UMaterial>(nullptr);
        Ar >> *material;
        FManagerOBJ::RegisterMaterial(material->GetMaterialInfo().MTLName, material);
    }
}

FName FRefSkeletal::GetBoneName(const int32 BoneIndex) const
{
    return RawBones[BoneIndex].BoneName;
}

int32 FRefSkeletal::GetParentIndex(const int32 InBoneIndex)
{
    // 유효한 인덱스면 RawBones에 저장된 ParentIndex 반환, 아니면 INDEX_NONE 반환
    if (RawBones.IsValidIndex(InBoneIndex))
    {
        return RawBones[InBoneIndex].ParentIndex;
    }
    return INDEX_NONE;
}

int32 FRefSkeletal::FindBoneIndex(const FName BoneName) const
{
    int BoneIndex = INDEX_NONE;
    if( BoneName != NAME_None )
    {
        const int32* IndexPtr = BoneNameToIndexMap.Find(BoneName.ToString());
        if( IndexPtr )
        {
            BoneIndex = *IndexPtr;
        }
    }
    return BoneIndex;
}

bool FRefSkeletal::BoneIsChildOf(const int32 ChildBoneIndex, const int32 ParentBoneIndex)
{
    if (ParentBoneIndex != INDEX_NONE)
    {
        // Bones are in strictly increasing order.
        // So child must have an index greater than its parent.
        if (ChildBoneIndex > ParentBoneIndex)
        {
            int32 BoneIndex = GetParentIndex(ChildBoneIndex);
            do
            {
                if (BoneIndex == ParentBoneIndex)
                {
                    return true;
                }
                BoneIndex = GetParentIndex(BoneIndex);

            } while (BoneIndex != INDEX_NONE);
        }
    }

    return false;
}

bool FRefSkeletal::IsValidIndex(int32 Index) const
{
    return RawBones.IsValidIndex(Index);
}

void FSkeletalVertex::SkinningVertex(const TArray<FBone>& bones)
{
    Position = FVector4(SkinVertexPosition(bones), 1.0f);
}
