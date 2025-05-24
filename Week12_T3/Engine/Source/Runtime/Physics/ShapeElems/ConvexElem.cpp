#include "ConvexElem.h"

EAggCollisionShape::Type FKConvexElem::StaticShapeType = EAggCollisionShape::Convex;

FKConvexElem::FKConvexElem()
    : FKShapeElem(EAggCollisionShape::Convex)
    , ElemBox(FBoundingBox())
    , Transform(FTransform::Identity)
{
}

FKConvexElem::FKConvexElem(const FKConvexElem& Other)
{
    CloneElem(Other);
}

FKConvexElem::~FKConvexElem()
{
}

const FKConvexElem& FKConvexElem::operator=(const FKConvexElem& Other)
{
    //ResetChaosConvexMesh();
    CloneElem(Other);
    return *this;
}

void FKConvexElem::AddCachedSolidConvexGeom(TArray<FVertexSimple>& VertexBuffer, TArray<uint32>& IndexBuffer, const FColor VertexColor) const
{
}

/** Reset the hull to empty all arrays */
void FKConvexElem::Reset()
{
    VertexData.Empty();
    ElemBox.Min = FVector::ZeroVector;
    ElemBox.Max = FVector::ZeroVector;
}

void FKConvexElem::UpdateElemBox()
{
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
    float MinScale, MinScaleAbs;
    FVector Scale3DAbs;
    SetupNonUniformHelper(BoneToWorldTM.GetScale() * GetTransform().GetScale(), MinScale, MinScaleAbs, Scale3DAbs);

    const FTransform LocalToWorldTM = GetTransform() * BoneToWorldTM;
    const FVector LocalPosition = LocalToWorldTM.InverseTransformPositionNoScale(WorldPosition);

    // if (ChaosConvex)
    // {
    //     Chaos::FVec3 OutNormal;
    //     Chaos::FReal Phi = ChaosConvex->PhiWithNormalScaled(LocalPosition, Scale3DAbs, OutNormal);
    //     Normal = LocalToWorldTM.TransformVectorNoScale(OutNormal);
    //     ClosestWorldPosition = WorldPosition - Normal * Phi;
    //     return Phi > UE_SMALL_NUMBER ? Phi : 0.f;
    // }
    return 0.f;
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

void FKConvexElem::CloneElem(const FKConvexElem& Other)
{
    Super::CloneElem(Other);
    VertexData = Other.VertexData;
    IndexData = Other.IndexData;
    ElemBox = Other.ElemBox;
    Transform = Other.Transform;
}
