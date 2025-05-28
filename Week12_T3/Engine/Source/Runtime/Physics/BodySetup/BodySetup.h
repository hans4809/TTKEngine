#pragma once
#include "BodySetupCore.h"
#include "Physics/AggregateGeom.h"

//class UPhysicalMaterial;

class UPrimitiveComponent;
class UPhysicalMaterial;
class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
public:
    UBodySetup();
    ~UBodySetup() override;

    UPROPERTY(EditAnywhere, FKAggregateGeom, AggGeom, = {})

    /** 
     * true일 경우(SkelMeshComp의 bEnableFullAnimWeightBodies가 true인 경우),
     * 이 본의 물리 시뮬레이션이 SkelMeshComp의 PhysicsWeight 설정과 상관없이
     * 스켈레탈 메시와 항상 블렌드됩니다.
     * 히트 리액션 등으로 물리 블렌딩을 켜고 끌 때에도
     * 클로스나 포니테일처럼 항상 물리를 적용할 본에 유용합니다.
     */
    UPROPERTY(EditAnywhere, bool, bAlwaysFullAnimWeight_DEPRECATED, = false)

    /**
     * 이 BodySetup을 PhysicsAsset(및 SkeletalMeshComponent)의
     * 경계 상자(AABB) 계산에 포함할지 여부입니다.
     * 경계를 갱신할 때 처리하는 BodySetup 수가 줄어들어
     * 성능이 향상됩니다.
     */
    UPROPERTY(EditAnywhere, bool, bConsiderForBounds, = false)

    /**
     * true일 경우(정적 메시만 해당),
     * 충돌이 활성화된 요소뿐 아니라
     * 메시의 모든 요소를 물리에 포함합니다.
     * 렌더 메시 전체를 사용해 고해상도 충돌을
     * 강제하고자 할 때 유용합니다.
     */
    UPROPERTY(EditAnywhere, bool , bMeshCollideAll, = false)

    /**
     * true일 경우, 씬 쿼리 시 물리 삼각 메시는 양면 폴리곤을 사용합니다.
     * 평면이나 단면 메시에서 양쪽 면 모두에서
     * 트레이스가 동작하도록 할 때 유용합니다.
     */
    UPROPERTY(EditAnywhere, bool, bDoubleSidedGeometry, = false)

    /** 
     * 비대칭(미러링되지 않은) 본에서의 충돌을 지원하기 위해
     * 필요한 데이터를 생성할지 여부입니다.
     */
    UPROPERTY(EditAnywhere, bool, bGenerateNonMirroredCollision, = false)

    /**
     * (cooked) 데이터가 여러 BodySetup 간에 공유되는지 여부입니다.
     * 폴리곤 단위 충돌에서 데이터 중복을 피하면서도
     * 지오메트리 변경을 위해 여러 BodySetup이 필요할 때 사용됩니다.
     */
    UPROPERTY(EditAnywhere, bool, bSharedCookedData, = false)

    /**
     * true일 경우 이 메시의 미러링된 인스턴스에서의 충돌을 지원하기 위해
     * 필요한 데이터를 생성합니다.
     * 충돌 데이터 크기를 절반으로 줄이지만,
     * 미러된 본의 충돌은 비활성화됩니다.
     */
    UPROPERTY(EditAnywhere, bool, bGenerateMirroredCollision, = false)

    /**
     * true일 경우, 물리 삼각 메시는 UV와
     * 패이스 리맵 테이블을 저장합니다.
     * 씬 쿼리에서 물리 머티리얼 마스크를 지원할 때 필요합니다.
     */
    UPROPERTY(EditAnywhere, bool, bSupportUVsAndFaceRemap, = false)

    /** 
     * (cooked) 데이터로부터 물리 볼록체 및 삼각 메쉬를
     * 생성했는지 여부를 나타내는 플래그입니다.
     */
    UPROPERTY(EditAnywhere, bool, bCreatedPhysicsMeshes, = false)

    /**
     * 물리 메쉬 생성에 실패했는지 여부를 나타내는 플래그입니다.
     * bCreatedPhysicsMeshes와 반대되는 값이 아니며,
     * 실패 시에도 bCreatedPhysicsMeshes는 true가 될 수 있습니다.
     */
    UPROPERTY(EditAnywhere, bool, bFailedToCreatePhysicsMeshes, = false)

    /**
     * 이 BodySetup이 (cooked)된 충돌 데이터를 보유하는지 여부입니다.
     * (cooked)된 데이터를 읽거나 쓸 때 설정되며,
     * 에디터 상의 에셋에 대해서는 권위 있는 정보가 아닙니다.
     */
    UPROPERTY(EditAnywhere, bool, bHasCookedCollisionData, = false)

    /**
     * 볼록체나 삼각 메쉬 형태를 절대 사용하지 않음을 나타냅니다.
     * 바이너리 데이터 검사를 건너뛰는 최적화를 위해 사용됩니다.
     *
     * TODO Chaos: 특정 메시에서 CreatePhysicsMeshes 호출을
     * 건너뛰기 위한 옵션입니다.
     * 장기적으로는 실제 메시 인스턴스가 필요할 때까지
     * CreatePhysicsMeshes를 호출하지 않는 방법이 더 좋습니다.
     */
    UPROPERTY(EditAnywhere, bool, bNeverNeedsCookedCollisionData, = false)

    /** 
     * 간단 충돌에 사용할 물리 머티리얼. 밀도, 마찰 등 정보를 인코딩함.
     */
    //UPROPERTY(EditAnywhere, UPhysicalMaterial*, PhysMaterial, = nullptr)

    /** 
     * 추가적인 페이스 리맵 테이블. 사용 가능한 경우, 충돌 메시에서 정적 메시로의
     * 페이스 인덱스 매핑을 결정할 때 물리 머티리얼 마스크와 함께 사용됨.
     */
    TArray<int32> FaceRemap;

    // /** Default properties of the body instance, copied into objects on instantiation, was URB_BodyInstance */
    // UPROPERTY(EditAnywhere, Category=Collision, meta=(FullyExpand = "true"))
    // FBodyInstance DefaultInstance;

    /** 
    * 이 바디 셋업의 빌드 스케일 (정적 메시 설정에서 정의됨)
    */
    UPROPERTY(FVector, BuildScale3D, = FVector::OneVector)

    /** 
     * 다른 BodySetup으로부터 본체 속성을 복사합니다.
     */
    void CopyBodyPropertiesFrom(const UBodySetup* FromSetup);

    /** 
     * 다른 BodySetup에서 충돌 도형을 이 셋업에 추가합니다.
     */
    void AddCollisionFrom(UBodySetup* FromSetup);
    void AddCollisionFrom(const FKAggregateGeom& FromAggGeom);
    bool AddCollisionElemFrom(const FKAggregateGeom& FromAggGeom, const EAggCollisionShape::Type ShapeType, const int32 ElemIndex);

    /** Mark internal cooked data dirty so next CreatePhysicsMeshes will re-cook */
    void InvalidatePhysicsData();
    /** 
     * 조리된(cooked) 데이터에서 물리 메쉬(ConvexMeshes, TriMesh & TriMeshNegX)를 생성합니다.
     */
    virtual void CreatePhysicsMeshes();


    /** 
     * 이 요소의 부피를 반환합니다.
     */
    virtual float GetVolume(const FVector& Scale) const;

    /** 
     * 주어진 스케일에 따른 이 요소의 부피를 반환합니다.
     */
    virtual float GetScaledVolume(const FVector& Scale) const;
    
    /** 
     * 질량을 계산합니다. (스케일, PhysMaterialOverride 등 추가 정보가 필요한 경우 컴포넌트를 전달할 수 있습니다.)
     */
    virtual float CalculateMass(const UPrimitiveComponent* Component = nullptr) const;

    /** 
     * 이 바디에 사용할 물리 머티리얼을 반환합니다. 지정된 것이 없으면 기본 엔진 머티리얼을 반환.
     */
    //class UPhysicalMaterial* GetPhysMaterial() const;

    /** 
     * 물리 메쉬(ConvexMeshes, TriMesh & TriMeshNegX)를 해제합니다.
     * BodySetup이 파괴되기 전에 반드시 호출해야 합니다.
     */
    void RemoveSimpleCollision();

    /**
     * 삼각 메쉬 충돌을 새 위치로 업데이트하고 BVH를 재적합(refit)합니다.
     * 전체 충돌 요리(cook)가 아니므로, 구조 변경이 아닌 위치 이동만 안전하게 지원합니다.
     * @param NewPositions    사용할 새 메쉬 위치 배열
     */
    void UpdateTriMeshVertices(const TArray<FVector> & NewPositions);

    /**  
     * BodySetup과 월드 위치 간 최단 거리를 계산합니다. 입력과 출력은 월드 공간 기준입니다.
     * @param WorldPosition      최단 거리를 구하려는 월드 공간상의 점
     * @param BodyToWorldTM      BodySetup을 월드 공간으로 변환하는 트랜스폼
     * @param bUseConvexShapes   true면 볼록체 요소도 검사 (기본값 false)
     * @return                   WorldPosition과 바디 셋업 간 거리. 0이면 내부에 있음을 의미.
     *
     * 주의: 이 함수는 삼각 메쉬 데이터를 무시합니다.
     */
    float GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BodyToWorldTM, bool bUseConvexShapes = false) const;


    /** 
     * BodySetup에서 주어진 월드 위치에 가장 가까운 점을 찾습니다. 입력과 출력은 월드 공간 기준입니다.
     * @param WorldPosition           최단 거리를 구하려는 월드 공간상의 점
     * @param BodyToWorldTM           BodySetup을 월드 공간으로 변환하는 트랜스폼
     * @param ClosestWorldPosition    WorldPosition에 가장 가까운 점 (월드 공간)
     * @param FeatureNormal           ClosestWorldPosition에 대응하는 표면 법선 벡터
     * @param bUseConvexShapes        true면 볼록체 요소도 검사 (기본값 false)
     * @return                        WorldPosition과 바디 셋업 간 거리. 0이면 내부에 있음을 의미.
     *
     * 주의: 이 함수는 삼각 메쉬 데이터를 무시합니다.
     */
    float GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BodyToWorldTM, FVector& ClosestWorldPosition, FVector& FeatureNormal, bool bUseConvexShapes = false) const;
    
    /*
     * 충돌 지오메트리를 제외한 모든 UPROPERTY 설정을 다른 BodySetup으로 복사합니다.
     * 정적 메시 재임포트 후 원래 데이터를 복원할 때 사용됩니다.
     */
    virtual void CopyBodySetupProperty(const UBodySetup* Other);
    
    // /** 
    //  * 이 BodySetup이 정의한 충돌 도형을 PxRigidBody에 추가합니다. 
    //  */
    // ENGINE_API void AddShapesToRigidActor_AssumesLocked(
    //     FBodyInstance* OwningInstance, 
    //     FVector& Scale3D, 
    //     UPhysicalMaterial* SimpleMaterial,
    //     TArray<UPhysicalMaterial*>& ComplexMaterials,
    //     TArray<FPhysicalMaterialMaskParams>& ComplexMaterialMasks,
    //     const FBodyCollisionData& BodyCollisionData,
    //     const FTransform& RelativeTM = FTransform::Identity, 
    //     TArray<FPhysicsShapeHandle>* NewShapes = NULL);
    //  
    // friend struct FIterateBodySetupHelper;
    UPhysicalMaterial* GetDefaultMaterial();
    UPhysicalMaterial* DefaultPhysicalMaterial;
};
