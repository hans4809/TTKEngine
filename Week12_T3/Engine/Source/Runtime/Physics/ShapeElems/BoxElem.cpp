#include "BoxElem.h"
#include "Define.h"

EAggCollisionShape::Type FKBoxElem::StaticShapeType = EAggCollisionShape::Box;

FKBoxElem::FKBoxElem()
    : FKShapeElem(EAggCollisionShape::Box)
    , TM(FMatrix())
    , Orientation( FQuat::Identity )
    , Center( FVector::ZeroVector )
    , Rotation( FQuat::Identity )
    , X(1), Y(1), Z(1)
    {}

FKBoxElem::FKBoxElem(const float InScale)
    : FKShapeElem(EAggCollisionShape::Box)
    , TM(FMatrix())
    , Orientation(FQuat::Identity)
    , Center(FVector::ZeroVector)
    , Rotation(FQuat::Identity)
    , X(InScale), Y(InScale), Z(InScale)
    {}

FKBoxElem::FKBoxElem(const float InX, const float InY, const float InZ)
    : FKShapeElem(EAggCollisionShape::Box)
    , TM(FMatrix())
    , Orientation( FQuat::Identity )
    , Center( FVector::ZeroVector )
    , Rotation( FQuat::Identity )
    , X(InX), Y(InY), Z(InZ)
    {}

FKBoxElem::~FKBoxElem()
{
}

bool operator==(const FKBoxElem& LHS, const FKBoxElem& RHS)
{
    return ( LHS.Center == RHS.Center &&
        LHS.Rotation == RHS.Rotation &&
        LHS.X == RHS.X &&
        LHS.Y == RHS.Y &&
        LHS.Z == RHS.Z );
}

FTransform FKBoxElem::GetTransform() const
{
    return FTransform(Rotation, Center, FVector::OneVector);
}

void FKBoxElem::SetTransform(const FTransform& InTransform)
{
    Rotation = InTransform.Rotator().ToQuaternion();
    Center = InTransform.GetLocation();
}

float FKBoxElem::GetVolume(const FVector& Scale3D) const
{
    return GetScaledVolume(Scale3D);
}

float FKBoxElem::GetScaledVolume(const FVector& Scale3D) const
{
    return FMath::Abs(Scale3D.X * Scale3D.Y * Scale3D.Z * X * Y * Z);
}

FBoundingBox FKBoxElem::CalcAABB(const FTransform& BoneTM, float Scale) const
{
    FTransform ElemTM = GetTransform();
    ElemTM.Location *= Scale;
    FMatrix MultipliedMat =  ElemTM.ToMatrixWithScale() * BoneTM.ToMatrixWithScale();
    
    FVector Extent(0.5f * Scale * X, 0.5f * Scale * Y, 0.5f * Scale * Z);
    FBoundingBox LocalBox(-Extent, Extent);

    return LocalBox.TransformBy(ElemTM.ToMatrixWithScale());
}

void FKBoxElem::ScaleElem(FVector DeltaSize, float MinSize)
{
    // Sizes are lengths, so we double the delta to get similar increase in size.
    X = FMath::Max(X + 2 * DeltaSize.X, MinSize);
    Y = FMath::Max(Y + 2 * DeltaSize.Y, MinSize);
    Z = FMath::Max(Z + 2 * DeltaSize.Z, MinSize);
}

FKBoxElem FKBoxElem::GetFinalScaled(const FVector& Scale3D, const FTransform& RelativeTM) const
{
    float MinScale, MinScaleAbs;
    FVector Scale3DAbs;

    SetupNonUniformHelper(Scale3D * RelativeTM.GetScale(), MinScale, MinScaleAbs, Scale3DAbs);

    FKBoxElem ScaledBox = *this;
    ScaledBox.X *= Scale3DAbs.X;
    ScaledBox.Y *= Scale3DAbs.Y;
    ScaledBox.Z *= Scale3DAbs.Z;

    FTransform ScaleTransform(FQuat::Identity, FVector::ZeroVector, Scale3D); 
    FTransform BoxTransform = GetTransform() * RelativeTM * ScaleTransform;
    ScaledBox.SetTransform(BoxTransform);

    return ScaledBox;
}

float FKBoxElem::GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BoneToWorldTM) const
{
    const FKBoxElem& ScaledBox = GetFinalScaled(BoneToWorldTM.GetScale(), FTransform::Identity);
    const FTransform LocalToWorldTM = GetTransform() * BoneToWorldTM;
    const FVector LocalPosition = LocalToWorldTM.InverseTransformPositionNoScale(WorldPosition);
    const FVector LocalPositionAbs = LocalPosition.GetAbs();

    const FVector HalfPoint(ScaledBox.X*0.5f, ScaledBox.Y*0.5f, ScaledBox.Z*0.5f);
    const FVector Delta = LocalPositionAbs - HalfPoint;
    const FVector Errors = FVector(FMath::Max(Delta.X, 0.0f), FMath::Max(Delta.Y, 0.0f), FMath::Max(Delta.Z, 0.0f));
    const float Error = Errors.Magnitude();

    return Error > SMALL_NUMBER ? Error : 0.f;
}

float FKBoxElem::GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BoneToWorldTM, FVector& ClosestWorldPosition, FVector& Normal) const
{
    const FKBoxElem& ScaledBox = GetFinalScaled(BoneToWorldTM.GetScale(), FTransform::Identity);
    const FTransform LocalToWorldTM = GetTransform() * BoneToWorldTM;
    const FVector LocalPosition = LocalToWorldTM.InverseTransformPositionNoScale(WorldPosition);

    const float HalfX = ScaledBox.X * 0.5f;
    const float HalfY = ScaledBox.Y * 0.5f;
    const float HalfZ = ScaledBox.Z * 0.5f;
	
    const FVector ClosestLocalPosition(FMath::Clamp(LocalPosition.X, -HalfX, HalfX), FMath::Clamp(LocalPosition.Y, -HalfY, HalfY), FMath::Clamp(LocalPosition.Z, -HalfZ, HalfZ));
    ClosestWorldPosition = LocalToWorldTM.TransformPositionNoScale(ClosestLocalPosition);

    const FVector LocalDelta = LocalPosition - ClosestLocalPosition;
    float Error = LocalDelta.Magnitude();
	
    bool bIsOutside = Error > SMALL_NUMBER;
	
    const FVector LocalNormal = bIsOutside ? LocalDelta.GetSafeNormal() : FVector::ZeroVector;

    ClosestWorldPosition = LocalToWorldTM.TransformPositionNoScale(ClosestLocalPosition);
    Normal = LocalToWorldTM.TransformVectorNoScale(LocalNormal);
	
    return bIsOutside ? Error : 0.f;
}
