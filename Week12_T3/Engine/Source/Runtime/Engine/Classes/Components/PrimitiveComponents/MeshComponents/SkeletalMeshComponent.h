#pragma once
#include "MeshComponent.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Physics/BodySetup/BodySetup.h"

class UAnimInstance;
class UStaticMeshComponent;
class UAnimationAsset;
class UAnimSingleNodeInstance;
class UAnimSequence;

enum class EAnimationMode : uint8
{
    AnimationBlueprint,
    AnimationSingleNode,
};


class USkeletalMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, UMeshComponent)

public:
    USkeletalMeshComponent() = default;
    
    virtual void PostDuplicate() override;
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual uint32 GetNumMaterials() const override;
    virtual UMaterial* GetMaterial(uint32 ElementIndex) const override;
    virtual uint32 GetMaterialIndex(FName MaterialSlotName) const override;
    virtual TArray<FName> GetMaterialSlotNames() const override;
    virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const override;
    
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    
    USkeletalMesh* GetSkeletalMesh() const { return SkeletalMesh; }
    void SetSkeletalMesh(USkeletalMesh* value);
    USkeletalMesh* LoadSkeletalMesh(const FString& FileName);

    UAnimInstance* GetAnimInstance() const { return AnimInstance; }
    void SetAnimInstance(UAnimInstance* InAnimInstance) { AnimInstance = InAnimInstance; }
    
    int32 GetBoneIndex(FName BoneName) const;
    
    /**
     * 본 인덱스를 이용해 월드 공간 상의 본 트랜스폼을 가져옵니다.
     * 컴포넌트 트랜스폼도 함께 지정할 수 있습니다.
     *
     * @param BoneIndex 가져올 본의 인덱스
     *
     * @return 지정된 인덱스의 본 트랜스폼 (월드 공간 기준)
     */
    FTransform GetBoneTransform(int32 BoneIndex) const;
    
    UAnimSingleNodeInstance* GetSingleNodeInstance() const;
    void CreateBoneComponents();
    void UpdateBoneHierarchy();
    void ResetToOriginPos();
    void UpdateChildBones(int ParentIndex);
    
    int SelectedSubMeshIndex = -1;

    UPROPERTY(EditAnywhere, bool, bConsiderAllBodiesForBounds, = true)
    
    TArray<FMatrix> BoneLocalTransforms;
    TArray<FMatrix> BoneWorldTransforms;
    TArray<FMatrix> BoneSkinningMatrices;
public:
    void PlayAnimation(UAnimSequence* NewAnimToPlay, bool bLooping);

    void SetAnimSequence(UAnimSequence* NewAnimToPlay);

    UAnimSequence* GetAnimSequence() const;
        
    void Play(bool bLooping);

    void Stop();

    void SetPlaying(bool bPlaying);

    bool IsPlaying() const;

    void SetReverse(bool bIsReverse);

    bool IsReverse() const;

    void SetPlayRate(float Rate);

    float GetPlayRate() const;

    void SetLooping(bool bIsLooping);

    bool IsLooping() const;

    int GetCurrentKey() const;

    void SetCurrentKey(int InKey);

    void SetElapsedTime(float InElapsedTime);

    float GetElapsedTime() const;

    int32 GetLoopStartFrame() const;

    void SetLoopStartFrame(int32 InLoopStartFrame);

    int32 GetLoopEndFrame() const;

    void SetLoopEndFrame(int32 InLoopEndFrame);

    bool bIsAnimationEnabled() const { return bPlayAnimation; }

    void SetAnimationMode(EAnimationMode InAnimationMode);

    EAnimationMode GetAnimationMode() const { return AnimationMode; }
    int32 FindBodyIndex(FName BoneName) const;
    void CreatePhysicsState();
private:

    UPROPERTY(EditAnywhere, EAnimationMode, AnimationMode, = EAnimationMode::AnimationSingleNode)
    //EAnimationMode AnimationMode;
    bool bPlayAnimation;
private:
    TArray<UStaticMeshComponent*> BoneComponents;
    bool bCPUSkinned = false;

    void SkinningVertex();

protected:
    UPROPERTY(EditAnywhere | DuplicateTransient, USkeletalMesh*, SkeletalMesh, = nullptr)
    UPROPERTY(EditAnywhere, UAnimInstance*, AnimInstance, = nullptr)
    
    float animTime = 0.f;

    /** Array of FBodyInstance objects, storing per-instance state about about each body. */
    TArray<struct FBodyInstance*> Bodies;

    /** Array of FConstraintInstance structs, storing per-instance state about each constraint. */
    TArray<struct FConstraintInstance*> Constraints;
};
