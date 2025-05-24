#include "ShapeElem.h"

EAggCollisionShape::Type FKShapeElem::StaticShapeType = EAggCollisionShape::Unknown;

FKShapeElem::FKShapeElem()
    : RestOffset(0.f)
    , bIsGenerated(false)
    , ShapeType(EAggCollisionShape::Unknown)
    , bContributeToMass(true)
    , CollisionEnabled(ECollisionEnabled::QueryAndPhysics)
    {}

FKShapeElem::FKShapeElem(const EAggCollisionShape::Type InShapeType)
    : RestOffset(0.f)
    , bIsGenerated(false)
    , ShapeType(InShapeType)
    , bContributeToMass(true)
    , CollisionEnabled(ECollisionEnabled::QueryAndPhysics)
    {}

FKShapeElem::FKShapeElem(const FKShapeElem& Copy)
    : RestOffset(Copy.RestOffset)
    , bIsGenerated(Copy.bIsGenerated)
    , Name(Copy.Name)
    , ShapeType(Copy.ShapeType)
    , bContributeToMass(Copy.bContributeToMass)
    , CollisionEnabled(ECollisionEnabled::QueryAndPhysics)
    {}

const FKShapeElem& FKShapeElem::operator=(const FKShapeElem& Other)
{
    CloneElem(Other);
    return *this;
}

const FName& FKShapeElem::GetName() const
{
    return Name;
}

void FKShapeElem::SetName(const FName& InName)
{
    Name = InName;
}

EAggCollisionShape::Type FKShapeElem::GetShapeType() const
{
    return ShapeType;
}

bool FKShapeElem::GetContributeToMass() const
{
    return bContributeToMass;
}

void FKShapeElem::SetContributeToMass(bool bInContributeToMass)
{
    bContributeToMass = bInContributeToMass;
}

void FKShapeElem::SetCollisionEnabled(ECollisionEnabled::Type InCollisionEnabled)
{
    CollisionEnabled = InCollisionEnabled;
}

ECollisionEnabled::Type FKShapeElem::GetCollisionEnabled() const
{
    return CollisionEnabled;
}

void FKShapeElem::CloneElem(const FKShapeElem& Other)
{
    RestOffset        = Other.RestOffset;
    ShapeType         = Other.ShapeType;
    Name              = Other.Name;
    bContributeToMass = Other.bContributeToMass;
    CollisionEnabled  = Other.CollisionEnabled;
    bIsGenerated      = Other.bIsGenerated;
}

FKShapeElem::~FKShapeElem()
{
}

void SetupNonUniformHelper(FVector InScale3D, double& OutMinScale, double& OutMinScaleAbs, FVector& OutScale3DAbs)
{
    // if almost zero, set min scale
    // @todo fixme
    if (InScale3D.IsNearlyZero())
    {
        // set min scale
        InScale3D = FVector(0.1f);
    }

    OutScale3DAbs = InScale3D.GetAbs();
    OutMinScaleAbs = OutScale3DAbs.GetMin();

    OutMinScale = FMath::Max3(InScale3D.X, InScale3D.Y, InScale3D.Z) < 0.f ? -OutMinScaleAbs : OutMinScaleAbs;	//if all three values are negative make minScale negative

    if (FMath::IsNearlyZero(OutMinScale))
    {
        // only one of them can be 0, we make sure they have mini set up correctly
        OutMinScale = 0.1f;
        OutMinScaleAbs = 0.1f;
    }
}

void SetupNonUniformHelper(const FVector InScale3D, float& OutMinScale, float& OutMinScaleAbs, FVector& OutScale3DAbs)
{
    double OutMinScaleD, OutMinScaleAbsD;
    SetupNonUniformHelper(InScale3D, OutMinScaleD, OutMinScaleAbsD, OutScale3DAbs);
    OutMinScale = static_cast<float>(OutMinScaleD);	// LWC_TODO: Precision loss?
    OutMinScaleAbs = static_cast<float>(OutMinScaleAbsD);
}
