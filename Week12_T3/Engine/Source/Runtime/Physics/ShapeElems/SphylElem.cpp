#include "SphylElem.h"

#include "Math/BoundingBox.h"

EAggCollisionShape::Type FKSphylElem::StaticShapeType = EAggCollisionShape::Sphyl;

FKSphylElem::FKSphylElem()
    : FKShapeElem(EAggCollisionShape::Sphyl)
    , TM(FMatrix())
    , Orientation(FQuat::Identity)
    , Center(FVector::ZeroVector)
    , Rotation(FQuat::Identity)
    , Radius(1.0f), Length(1.0f)
    {}

FKSphylElem::FKSphylElem(const float InRadius, const float InLength)
    : FKShapeElem(EAggCollisionShape::Sphyl)
    , TM(FMatrix())
    , Orientation(FQuat::Identity)
    , Center(FVector::ZeroVector)
    , Rotation(FQuat::Identity)
    , Radius(InRadius), Length(InLength)
    {}

FKSphylElem::~FKSphylElem()
{
}

bool operator==(const FKSphylElem& LHS, const FKSphylElem& RHS)
{
    return ( LHS.Center == RHS.Center &&
        LHS.Rotation == RHS.Rotation &&
        LHS.Radius == RHS.Radius &&
        LHS.Length == RHS.Length );
}

FTransform FKSphylElem::GetTransform() const
{
    return FTransform(Rotation, Center, FVector::OneVector);
}

void FKSphylElem::SetTransform(const FTransform& InTransform)
{
    Rotation = InTransform.Rotation;
    Center   = InTransform.GetLocation();
}

float FKSphylElem::GetVolume(const FVector& Scale) const
{
    return GetScaledVolume(Scale);
}

float FKSphylElem::GetScaledVolume(const FVector& Scale3D) const
{  
    float ScaledRadius = GetScaledRadius(Scale3D);
    float ScaledLength = GetScaledCylinderLength(Scale3D);
    return PI * FMath::Square(ScaledRadius) * (1.3333f * ScaledRadius + ScaledLength);
}

FBoundingBox FKSphylElem::CalcAABB(const FTransform& BoneTM, const float Scale) const
{
    FTransform ElemTM = GetTransform();
    ElemTM.ScaleTranslation(FVector(Scale));
    ElemTM *= BoneTM;

    const FVector SphylCenter = ElemTM.GetLocation();

    // Get sphyl axis direction
    const FVector Axis = ElemTM.GetScaledAxis( EAxis::Z );
    // Get abs of that vector
    const FVector AbsAxis(FMath::Abs(Axis.X), FMath::Abs(Axis.Y), FMath::Abs(Axis.Z));
    // Scale by length of sphyl
    const FVector AbsDist = AbsAxis;

    const FVector MaxPos = SphylCenter + AbsDist;
    const FVector MinPos = SphylCenter - AbsDist;
    const FVector Extent(Scale * Radius);

    const FBoundingBox Result(MinPos - Extent, MaxPos + Extent);

    return Result;
}

void FKSphylElem::ScaleElem(const FVector DeltaSize, const float MinSize)
{
    float DeltaRadius = DeltaSize.X;
    if (FMath::Abs(DeltaSize.Y) > FMath::Abs(DeltaRadius))
    {
        DeltaRadius = DeltaSize.Y;
    }

    float DeltaHeight = DeltaSize.Z;
    float radius = FMath::Max(Radius + DeltaRadius, MinSize);
    float length = Length + DeltaHeight;

    length += Radius - radius;
    length = FMath::Max(0.f, length);

    Radius = radius;
    Length = length;
}

FKSphylElem FKSphylElem::GetFinalScaled(const FVector& Scale3D, const FTransform& RelativeTM) const
{
    FKSphylElem ScaledSphylElem = *this;

    float MinScale, MinScaleAbs;
    FVector Scale3DAbs;

    SetupNonUniformHelper(Scale3D * RelativeTM.GetScale(), MinScale, MinScaleAbs, Scale3DAbs);

    ScaledSphylElem.Radius = GetScaledRadius(Scale3DAbs);
    ScaledSphylElem.Length = GetScaledCylinderLength(Scale3DAbs);

    const FTransform ScaleTransform(FQuat::Identity, FVector::ZeroVector, Scale3D);
    const FTransform RotationTransform(ScaledSphylElem.Rotation, FVector::ZeroVector, Scale3D);
    const FTransform ScaledRotationTransform = RotationTransform * ScaleTransform;
	
    const FVector LocalOrigin = RelativeTM.TransformPosition(Center) * Scale3D;
    ScaledSphylElem.Center = LocalOrigin;
    ScaledSphylElem.Rotation = FQuat(RelativeTM.GetRotation() * ScaledRotationTransform.GetRotation());

    return ScaledSphylElem;
}

float FKSphylElem::GetScaledRadius(const FVector& Scale3D) const
{
    const FVector Scale3DAbs = Scale3D.GetAbs();
    const float RadiusScale = FMath::Max(Scale3DAbs.X, Scale3DAbs.Y);
    return FMath::Clamp(Radius * RadiusScale, 0.1f, GetScaledHalfLength(Scale3DAbs));
}

float FKSphylElem::GetScaledCylinderLength(const FVector& Scale3D) const
{
    return FMath::Max(0.1f, (GetScaledHalfLength(Scale3D) - GetScaledRadius(Scale3D)) * 2.f);
}

float FKSphylElem::GetScaledHalfLength(const FVector& Scale3D) const
{
    return FMath::Max((Length + Radius * 2.0f) * FMath::Abs(Scale3D.Z) * 0.5f, 0.1f);
}

float FKSphylElem::GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BoneToWorldTM) const
{
	const FKSphylElem ScaledSphyl = GetFinalScaled(BoneToWorldTM.GetScale(), FTransform::Identity);

	const FTransform LocalToWorldTM = GetTransform() * BoneToWorldTM;
	const FVector ErrorScale = LocalToWorldTM.GetScale();
	const FVector LocalPosition = LocalToWorldTM.InverseTransformPositionNoScale(WorldPosition);
	const FVector LocalPositionAbs = LocalPosition.GetAbs();
	
	
	const FVector Target(LocalPositionAbs.X, LocalPositionAbs.Y, FMath::Max(LocalPositionAbs.Z - ScaledSphyl.Length * 0.5f, 0.f));	//If we are above half length find closest point to cap, otherwise to cylinder
	const float Error = FMath::Max(Target.Magnitude() - ScaledSphyl.Radius, 0.f);

	return Error > SMALL_NUMBER ? Error : 0.f;
}

float FKSphylElem::GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BoneToWorldTM, FVector& ClosestWorldPosition, FVector& Normal) const
{
	const FKSphylElem ScaledSphyl = GetFinalScaled(BoneToWorldTM.GetScale(), FTransform::Identity);

	const FTransform LocalToWorldTM = GetTransform() * BoneToWorldTM;
	const FVector ErrorScale = LocalToWorldTM.GetScale();
	const FVector LocalPosition = LocalToWorldTM.InverseTransformPositionNoScale(WorldPosition);
	
	const float HalfLength = 0.5f * ScaledSphyl.Length;
	const float TargetZ = FMath::Clamp(LocalPosition.Z, -HalfLength, HalfLength);	//We want to move to a sphere somewhere along the capsule axis

	const FVector WorldSphere = LocalToWorldTM.TransformPositionNoScale(FVector(0.f, 0.f, TargetZ));
	const FVector Dir = WorldSphere - WorldPosition;
	const float DistToCenter = Dir.Magnitude();
	const float DistToEdge = FMath::Max(DistToCenter - ScaledSphyl.Radius, 0.f);

	bool bIsOutside = DistToCenter > SMALL_NUMBER;
	if (bIsOutside)
	{
		Normal = -Dir.GetSafeNormal();
	}
	else
	{
		Normal = FVector::ZeroVector;
	}

	ClosestWorldPosition = WorldPosition - Normal*DistToEdge;

	return bIsOutside ? DistToEdge : 0.f;
}
