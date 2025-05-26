#include "SphereElem.h"

#include "Math/BoundingBox.h"

EAggCollisionShape::Type FKSphereElem::StaticShapeType = EAggCollisionShape::Sphere;

FKSphereElem::FKSphereElem()
    : FKShapeElem(EAggCollisionShape::Sphere)
    , TM(FMatrix())
    , Center(FVector::ZeroVector)
    , Radius(1)
    {}

FKSphereElem::FKSphereElem(const float InRadius)
    : FKShapeElem(EAggCollisionShape::Sphere)
    , TM(FMatrix())
    , Center(FVector::ZeroVector)
    , Radius(InRadius)
    {}

bool operator==(const FKSphereElem& LHS, const FKSphereElem& RHS)
{
    return (LHS.Center == RHS.Center &&
        LHS.Radius == RHS.Radius);
}

FTransform FKSphereElem::GetTransform() const
{
    return FTransform(FQuat::Identity, Center, FVector::OneVector);
}

void FKSphereElem::SetTransform(const FTransform& InTransform)
{
    Center = InTransform.GetLocation();
}

float FKSphereElem::GetVolume(const FVector& Scale) const
{
    return GetScaledVolume(Scale);
}

float FKSphereElem::GetScaledVolume(const FVector& Scale) const
{
    return 1.3333f * PI * FMath::Pow(Radius * Scale.GetAbsMin(), 3);
}

FBoundingBox FKSphereElem::CalcAABB(const FTransform& BoneTM, const float Scale) const
{
    FTransform ElemTM = GetTransform();
    ElemTM.Location *= Scale;
    const FMatrix MultipliedMat =  ElemTM.ToMatrixWithScale() * BoneTM.ToMatrixWithScale();

    const FVector BoxCenter = MultipliedMat.GetOrigin();
    const FVector BoxExtents(Radius * Scale);

    return FBoundingBox(BoxCenter - BoxExtents, BoxCenter + BoxExtents);
}

void FKSphereElem::ScaleElem(FVector DeltaSize, float MinSize)
{
    // Find element with largest magnitude, btu preserve sign.
    float DeltaRadius = DeltaSize.X;
    if (FMath::Abs(DeltaSize.Y) > FMath::Abs(DeltaRadius))
        DeltaRadius = DeltaSize.Y;
    else if (FMath::Abs(DeltaSize.Z) > FMath::Abs(DeltaRadius))
        DeltaRadius = DeltaSize.Z;

    Radius = FMath::Max(Radius + DeltaRadius, MinSize);
}

FKSphereElem FKSphereElem::GetFinalScaled(const FVector& Scale3D, const FTransform& RelativeTM) const
{
    float MinScale, MinScaleAbs;
    FVector Scale3DAbs;

    SetupNonUniformHelper(Scale3D * RelativeTM.GetScale(), MinScale, MinScaleAbs, Scale3DAbs);

    FKSphereElem ScaledSphere = *this;
    ScaledSphere.Radius *= MinScaleAbs;

    ScaledSphere.Center = RelativeTM.ToMatrixWithScale().TransformPosition(Center) * Scale3D;
    
    return ScaledSphere;
}

float FKSphereElem::GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& LocalToWorldTM) const
{
    FKSphereElem ScaledSphere = GetFinalScaled(LocalToWorldTM.GetScale(), FTransform::Identity);

    const FVector Dir = LocalToWorldTM.ToMatrixNoScale().TransformPosition(ScaledSphere.Center) - WorldPosition;
    const float DistToCenter = Dir.Magnitude();
    const float DistToEdge = DistToCenter - ScaledSphere.Radius;
	
    return DistToEdge > SMALL_NUMBER ? DistToEdge : 0.f;
}

float FKSphereElem::GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& LocalToWorldTM, FVector& ClosestWorldPosition,
    FVector& Normal) const
{
    FKSphereElem ScaledSphere = GetFinalScaled(LocalToWorldTM.GetScale(), FTransform::Identity);

    const FVector Dir = LocalToWorldTM.ToMatrixNoScale().TransformPosition(ScaledSphere.Center) - WorldPosition;
    const float DistToCenter = Dir.Magnitude();
    const float DistToEdge = FMath::Max(DistToCenter - ScaledSphere.Radius, 0.f);

    if(DistToCenter > SMALL_NUMBER)
    {
        Normal = -Dir.GetSafeNormal();
    }
    else
    {
        Normal = FVector::ZeroVector;
    }
	
    ClosestWorldPosition = WorldPosition - Normal*DistToEdge;

    return DistToEdge;
}
