#include "ConvexElem.h"
#include "BoxElem.h"
EAggCollisionShape::Type FKConvexElem::StaticShapeType = EAggCollisionShape::Convex;

FKConvexElem::FKConvexElem()
    : FKShapeElem(EAggCollisionShape::Convex)
    , ElemBox(FBoundingBox())
    , Transform(FTransform::Identity)
{
}

FKConvexElem::FKConvexElem(const FKConvexElem& Other)
    : FKShapeElem(Other) // 부모 클래스 복사 생성자 호출
{
    // VertexData, IndexData, ElemBox, Transform 등은 복사
    VertexData = Other.VertexData;
    IndexData = Other.IndexData;
    ElemBox = Other.ElemBox;
    Transform = Other.Transform;
    CookedPxConvexMesh = Other.CookedPxConvexMesh;

    if (CookedPxConvexMesh)
    {
        CookedPxConvexMesh->acquireReference(); // 참조 카운트 증가
    }
}

FKConvexElem::~FKConvexElem()
{
}

const FKConvexElem& FKConvexElem::operator=(const FKConvexElem& Other)
{
    if (this != &Other)
    {
        FKShapeElem::operator=(Other);    // 부모 클래스 대입
        ResetCookedPhysXData();           // 현재 객체의 쿠킹된 데이터 정리
        VertexData = Other.VertexData;
        IndexData = Other.IndexData;
        ElemBox = Other.ElemBox;
        Transform = Other.Transform;
        CookedPxConvexMesh = Other.CookedPxConvexMesh; // 새 포인터 복사
        if (CookedPxConvexMesh)
        {
            CookedPxConvexMesh->acquireReference(); // 참조 카운트 증가
        }
    }
    return *this;
}
void FKConvexElem::AddCachedSolidConvexGeom(TArray<FVertexSimple>& VertexBuffer, TArray<uint32>& IndexBuffer, const FColor VertexColor) const
{
}

/** Reset the hull to empty all arrays */
void FKConvexElem::Reset()
{
    VertexData.Empty();
    IndexData.Empty(); // 인덱스 데이터도 있다면
    ElemBox.Min = FVector::ZeroVector;
    ElemBox.Max = FVector::ZeroVector;
    ResetCookedPhysXData();
    Transform = FTransform::Identity;
}

void FKConvexElem::UpdateElemBox()
{
    if (VertexData.IsEmpty())
    {
        ElemBox = FBoundingBox(FVector::ZeroVector, FVector::ZeroVector);
    }
    else
    {
        ElemBox = FBoundingBox(VertexData);
    }
}

FBoundingBox FKConvexElem::CalcAABB(const FTransform& BoneTM, const FVector& Scale3D) const
{
    // Zero out rotation and location so we transform by scale along
    const FTransform LocalToWorld = FTransform(FQuat::Identity, FVector::ZeroVector, Scale3D) * BoneTM;

    return ElemBox.TransformBy((Transform * LocalToWorld).ToMatrixWithScale());
}

void FKConvexElem::GetPlanes(TArray<FPlane>& Planes) const
{
}

bool FKConvexElem::HullFromPlanes(const TArray<FPlane>& InPlanes, const TArray<FVector>& SnapVerts, float InSnapDistance)
{
    return true;
}

void FKConvexElem::ConvexFromBoxElem(const FKBoxElem& InBox)
{
    Reset(); // 기존 데이터 초기화

    FVector HalfExtents(InBox.X * 0.5f, InBox.Y * 0.5f, InBox.Z * 0.5f); // InBox.X 등이 전체 크기라고 가정
    FTransform BoxTransform = InBox.GetTransform(); // FKBoxElem이 GetTransform()을 통해 로컬 트랜스폼 반환 가정

    // 박스의 8개 로컬 꼭짓점 계산
    FVector Corners[8];
    Corners[0] = FVector(-HalfExtents.X, -HalfExtents.Y, -HalfExtents.Z);
    Corners[1] = FVector(HalfExtents.X, -HalfExtents.Y, -HalfExtents.Z);
    Corners[2] = FVector(HalfExtents.X, HalfExtents.Y, -HalfExtents.Z);
    Corners[3] = FVector(-HalfExtents.X, HalfExtents.Y, -HalfExtents.Z);
    Corners[4] = FVector(-HalfExtents.X, -HalfExtents.Y, HalfExtents.Z);
    Corners[5] = FVector(HalfExtents.X, -HalfExtents.Y, HalfExtents.Z);
    Corners[6] = FVector(HalfExtents.X, HalfExtents.Y, HalfExtents.Z);
    Corners[7] = FVector(-HalfExtents.X, HalfExtents.Y, HalfExtents.Z);

    VertexData.Reserve(8);
    for (int i = 0; i < 8; ++i)
    {
        VertexData.Add(BoxTransform.TransformPosition(Corners[i]));
    }

    Transform = FTransform::Identity;
    UpdateElemBox(); // Invalidate (re-cook) PxConvexMesh
    ResetCookedPhysXData(); // 다음 CreatePhysicsMeshes 호출 시 새로 쿠킹되도록
}

float FKConvexElem::GetVolume(const FVector& Scale) const
{
    return GetScaledVolume(Scale);
}

float FKConvexElem::GetScaledVolume(const FVector& Scale3D) const
{
    float Volume = 0.0f;

    // if (ChaosConvex != nullptr)
    // {
    //     Volume = FMath::Abs(Scale3D.X * Scale3D.Y * Scale3D.Z) * ChaosConvex->GetVolume();
    // }

    return Volume;
}

void FKConvexElem::ScaleElem(FVector DeltaSize, float MinSize)
{
    FTransform ScaledTransform = GetTransform();
    ScaledTransform.SetScale(ScaledTransform.GetScale() + DeltaSize);
    SetTransform(ScaledTransform);
}

float FKConvexElem::GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BoneToWorldTM, FVector& ClosestWorldPosition,
    FVector& Normal) const
{
    if (!CookedPxConvexMesh) return FLT_MAX;

    const FTransform ConvexWorldTM = Transform * BoneToWorldTM; // 컨벡스 요소의 최종 월드 트랜스폼
    physx::PxTransform PxConvexPose = ConvexWorldTM.ToPxTransform();
    physx::PxVec3 PxWorldPos = WorldPosition.ToPxVec3();

    // 스케일링된 컨벡스 지오메트리 생성
    physx::PxMeshScale PxScale(ConvexWorldTM.GetScale().ToPxVec3()); // 스케일 가져오기
    physx::PxConvexMeshGeometry ConvexGeom(CookedPxConvexMesh, PxScale);

    physx::PxVec3 PxClosestPoint;
    physx::PxVec3 PxNormal;
    float PxDistance;

    // PxGeometryQuery::pointDistance는 가장 가까운 거리를 반환. 법선은 직접 계산해야 할 수 있음.
    // 또는 PhysX에 더 적합한 함수가 있는지 확인 필요. (예: PxShape의 raycast, sweep 등 활용)
    // 가장 간단하게는, PxShape를 임시로 만들어서 쿼리하는 방법도 있지만 비효율적.
    // 여기서는 이 함수의 정확한 PhysX 구현은 생략.
    // PhiWithNormal 같은 기능은 Chaos에 특화된 것일 수 있음.

    //// 임시 반환
    //ClosestWorldPosition = WorldPosition; // 실제 계산 필요
    //Normal = FVector::UpVector;       // 실제 계산 필요
    //return FLT_MAX;                   // 실제 계산 필요
    return 0;
}

float FKConvexElem::GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BoneToWorldTM) const
{
    float MinScale, MinScaleAbs;
    FVector Scale3DAbs;
    SetupNonUniformHelper(BoneToWorldTM.GetScale() * GetTransform().GetScale(), MinScale, MinScaleAbs, Scale3DAbs);

    const FTransform LocalToWorldTM = GetTransform() * BoneToWorldTM;
    const FVector LocalPosition = LocalToWorldTM.InverseTransformPositionNoScale(WorldPosition);
    // if (ChaosConvex)
    // {
    //     Chaos::FVec3 OutNormal;
    //     Chaos::FReal Phi = ChaosConvex->PhiWithNormalScaled(LocalPosition, Scale3DAbs, OutNormal);
    //     return Phi > SMALL_NUMBER ? Phi : 0.f;
    // }
    return 0.f;
}

void FKConvexElem::ResetCookedPhysXData()
{
    if (CookedPxConvexMesh)
    {
        CookedPxConvexMesh->release(); // PhysX 객체 참조 카운트 감소 및 해제
        CookedPxConvexMesh = nullptr;
    }
}

void FKConvexElem::CloneElem(const FKConvexElem& Other)
{
    Super::CloneElem(Other);
    VertexData = Other.VertexData;
    IndexData = Other.IndexData;
    ElemBox = Other.ElemBox;
    Transform = Other.Transform;
}
