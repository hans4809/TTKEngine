#pragma once
#include "RigidBodyIndexPair.h"
#include "Engine/Asset/Asset.h"
#include "Math/BoundingBox.h"
#include "ShapeElems/ShapeElem.h"

class UBodySetup;
class USkeletalMeshComponent;
struct FRefSkeletal;
class USkeletalMesh;
class UPhysicsConstraintTemplate;

class UPhysicsAsset : public UAsset
{
    DECLARE_CLASS(UPhysicsAsset, UAsset)
public:
    UPhysicsAsset();
    ~UPhysicsAsset() override;

    UPROPERTY(EditAnywhere, USkeletalMesh*, DefaultSkeletalMesh, = nullptr)

    UPROPERTY(EditAnywhere, USkeletalMesh*, PreviewSkeletalMesh, = nullptr)

    UPROPERTY(EditAnywhere, TArray<FName>, PhysicalAnimationProfiles, = {})

    UPROPERTY(EditAnywhere, TArray<FName>, ConstraintProfiles, = {})

    UPROPERTY(EditAnywhere, FName, CurrentPhysicalAnimationProfileName, = TEXT("None"))

    UPROPERTY(EditAnywhere, FName, CurrentConstraintProfileName, = TEXT("None"))

    /** bConsiderForBounds로 표시된 바디의 인덱스 */
    UPROPERTY(EditAnywhere, TArray<int32>, BoundsBodies, = {})
    
    /** 
     * RB_ConstraintSetup 객체의 배열입니다.
     * 두 바디 사이의 조인트 정보(각 바디에 대한 상대 위치, 제한 등)를 저장합니다.
     */
    UPROPERTY(EditAnywhere, TArray<UPhysicsConstraintTemplate*>, ConstraintSetup, = {})
    
    /** BodyName별 BodySetup 인덱스를 캐싱하여 FindBodyIndex를 빠르게 수행할 수 있게 합니다 */
    using NameToIntMapType = TMap<FName, int32>;
    UPROPERTY(EditAnywhere, NameToIntMapType, BodySetupIndexMap, = {})


    /** 
     * 어느 바디 쌍 사이의 충돌이 비활성화되었는지 나타내는 테이블입니다. 내부적으로 사용됩니다.
     * 물리 엔진 내부에서도 접근하므로 물리 연산 중에는 변경하면 안전하지 않습니다.
     */
    using FRigidBodyIndexPairToboolMapType = TMap<FRigidBodyIndexPair, bool>;
    UPROPERTY(EditAnywhere, FRigidBodyIndexPairToboolMapType, CollisionDisableTable, = {})

    UPROPERTY(EditAnywhere, TArray<UBodySetup*>, BodySetups, = {})

public: 
    void Initialize();
    
    const TArray<FName>& GetPhysicalAnimationProfileNames() const
    {
        return PhysicalAnimationProfiles;
    }

    const TArray<FName>& GetConstraintProfileNames() const
    {
        return ConstraintProfiles;
    }
    
    // 이 그래픽 본을 제어하는 물리 본의 인덱스를 찾습니다.
    int32 FindControllingBodyIndex(const class USkeletalMesh* skelMesh, int32 BoneIndex) const;
    int32 FindParentBodyIndex(const class USkeletalMesh * skelMesh, int32 StartBoneIndex) const;
    int32 FindParentBodyIndex(FRefSkeletal& RefSkeleton, const int32 StartBoneIndex) const;
    
    int32 FindConstraintIndex(FName ConstraintName) const;
    int32 FindConstraintIndex(FName Bone1Name, FName Bone2Name) const;
    FName FindConstraintBoneName(int32 ConstraintIndex) const;

    /** 지정된 이름의 본 아래(포함) 모든 바디 인덱스를 가져오는 유틸리티 */
    void GetBodyIndicesBelow(TArray<int32>& OutBodyIndices, FName InBoneName, const USkeletalMesh* InSkelMesh, bool bIncludeParent = true) const;
    void GetBodyIndicesBelow(TArray<int32>& OutBodyIndices, const FName InBoneName, FRefSkeletal& RefSkeleton, const bool bIncludeParent = true) const;

    /** 지정된 본 아래에서 가장 가까운 바디 인덱스를 가져옵니다 */
    void GetNearestBodyIndicesBelow(TArray<int32>& OutBodyIndices, FName InBoneName, const USkeletalMesh* InSkelMesh) const;

    /** 월드 공간 기준 AABB를 계산합니다 */
    FBoundingBox CalcAABB(const USkeletalMeshComponent* MeshComponent, const FTransform& LocalToWorld) const;

    /** 본 인덱스, SpaceBases, LocalToWorld를 통해 본 트랜스폼을 구합니다 */
    FTransform GetSkelBoneTransform(int32 BoneIndex, const TArray<FTransform>& SpaceBases, const FTransform& LocalToWorld);
    
    void GetCollisionMesh(int32 ViewIndex, const FRefSkeletal& RefSkeleton);

    // 지정된 두 바디 인덱스 사이의 충돌을 비활성화합니다
    void DisableCollision(int32 BodyIndexA, int32 BodyIndexB);

    // 지정된 두 바디 인덱스 사이의 충돌을 활성화합니다
    void EnableCollision(int32 BodyIndexA, int32 BodyIndexB);

    // 두 바디 간 충돌 활성 여부를 확인합니다
    bool IsCollisionEnabled(int32 BodyIndexA, int32 BodyIndexB) const;

    // 바디별 프리미티브 충돌 필터링 모드를 설정합니다
    void SetPrimitiveCollision(int32 BodyIndex, EAggCollisionShape::Type PrimitiveType, int32 PrimitiveIndex, ECollisionEnabled::Type CollisionEnabled);

    // 바디별 프리미티브 충돌 필터링 모드를 가져옵니다
    ECollisionEnabled::Type GetPrimitiveCollision(int32 BodyIndex, EAggCollisionShape::Type PrimitiveType, int32 PrimitiveIndex) const;

    // 프리미티브 볼륨이 질량 계산에 기여할지 여부를 설정합니다
    void SetPrimitiveContributeToMass(int32 BodyIndex, EAggCollisionShape::Type PrimitiveType, int32 PrimitiveIndex, bool bContributesToMass);
    
    // 프리미티브 볼륨이 질량 계산에 기여하는지 여부를 가져옵니다
    bool GetPrimitiveContributeToMass(int32 BodyIndex, EAggCollisionShape::Type PrimitiveType, int32 PrimitiveIndex) const;

    /** bConsiderForBounds로 표시된 바디 인덱스를 BoundsBodies 배열에 캐싱하고 업데이트합니다 */
    void UpdateBoundsBodiesArray();

    /** BodySetup 배열 인덱스 맵을 업데이트합니다 */
    void UpdateBodySetupIndexMap();

    int32 FindBodyIndex(FName BodyName) const;
    
    /** 특정 바디에 연결된 모든 제약(Constraint)을 찾습니다.
     *
     * @param BodyIndex    제약을 찾을 바디의 인덱스
     * @param Constraints  검색된 제약 인덱스를 반환할 배열
     **/
    void BodyFindConstraints(int32 BodyIndex, TArray<int32>& Constraints);

    /** IPreviewMeshProviderInterface 인터페이스 */
    virtual void SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty = true);
    virtual USkeletalMesh* GetPreviewMesh() const;

    /* Body 자동 생성*/
    void AutoGenerateBodies();

    /* Constraint 자동 생성 */
    void AutoGenerateConstraints();
    
    /* Constraint 재귀 생성*/
    void GenerateConstraintRecursive(const FRefSkeletal& RefSkeletal, int32 ParentBoneIndex);
    bool LoadFromFile(const FString& filepath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;
    void PostLoad() override;

    void GetBodySetups(TArray<class UBodySetup*>& OutBodySetup) const;
};
