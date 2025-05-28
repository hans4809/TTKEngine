#pragma once
#include "Engine/Asset/Asset.h"
#include "Skeletal/SkeletalDefine.h"

class USkeleton;
struct FMaterialSlot;
class UPhysicsAsset;

class USkeletalMesh : public UAsset
{
    DECLARE_CLASS(USkeletalMesh, UAsset)
public:
    USkeletalMesh() = default;
    virtual ~USkeletalMesh() override = default;

    FSkeletalMeshRenderData& GetRenderData() { return SkeletalMeshRenderData; }
    USkeleton* GetSkeleton() const { return Skeleton;}
    const TArray<FMaterialSlot>& GetMaterials() const { return MaterialSlots; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*>& Out) const;

    void SetData(const FSkeletalMeshRenderData& InRenderData, USkeleton* InSkeleton);
    
    void UpdateBoneHierarchy();
    // 정점 스키닝을 업데이트하는 함수
    void UpdateSkinnedVertices();

    // 버텍스 버퍼를 업데이트하는 함수
    void RotateBoneByName(const FString& BoneName, float DeltaAngleInDegrees, const FVector& RotationAxis);
    void RotateBoneByIndex(int32 BoneIndex, float DeltaAngleInDegrees, const FVector& RotationAxis, bool bIsChildUpdate = true);
    int FindBoneIndexByName(const FString& BoneName) const;

    void ResetToOriginalPose();

public:
    UPhysicsAsset* GetPhysicsAsset() const;
    void SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset);

private:
    UPROPERTY(FSkeletalMeshRenderData, SkeletalMeshRenderData, = {})
    UPROPERTY(EditAnywhere | DuplicateTransient, USkeleton*, Skeleton, = nullptr)
    UPROPERTY(EditAnywhere, TArray<FMaterialSlot>, MaterialSlots, = {})
    UPROPERTY(EditAnywhere | DuplicateTransient, UPhysicsAsset*, MyPhysicsAsset, = nullptr)

    void UpdateChildBones(int ParentIndex);
    void ApplyRotationToBone(int BoneIndex, float DeltaAngleInDegrees, const FVector& RotationAxis);

public:
    bool LoadFromFile(const FString& FilePath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;
    void PostLoad() override;
};

