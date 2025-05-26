#include "AggregateGeom.h"

float SelectMinScale(const FVector Scale)
{
    float Min = Scale.X, AbsMin = FMath::Abs(Scale.X);

    float  CompareAbsMin  = FMath::Abs(Scale.Y);
    if (  CompareAbsMin < AbsMin )
    {
        AbsMin = CompareAbsMin;
        Min = Scale.Y;
    }

    CompareAbsMin  = FMath::Abs(Scale.Z);
    if (  CompareAbsMin < AbsMin )
    {
        AbsMin = CompareAbsMin;
        Min = Scale.Z;
    }

    return Min;
}


int32 FKAggregateGeom::GetElementCount(const EAggCollisionShape::Type Type) const
{
    switch (Type)
    {
    case EAggCollisionShape::Box:
        return BoxElems.Num();
    case EAggCollisionShape::Convex:
        return ConvexElems.Num();
    case EAggCollisionShape::Sphyl:
        return SphylElems.Num();
    case EAggCollisionShape::Sphere:
        return SphereElems.Num();
    default:
        return 0;
    }
}

FKShapeElem* FKAggregateGeom::GetElement(const EAggCollisionShape::Type Type, const int32 Index)
{
    switch (Type)
    {
    case EAggCollisionShape::Sphere:
        if (SphereElems.IsValidIndex(Index)) { return &SphereElems[Index]; }
    case EAggCollisionShape::Box:
        if (BoxElems.IsValidIndex(Index)) { return &BoxElems[Index]; }
    case EAggCollisionShape::Sphyl:
        if (SphylElems.IsValidIndex(Index)) { return &SphylElems[Index]; }
    case EAggCollisionShape::Convex:
        if (ConvexElems.IsValidIndex(Index)) { return &ConvexElems[Index]; }
    default:
        return nullptr;
    }
}

FKShapeElem* FKAggregateGeom::GetElement(const int32 InIndex)
{
    int Index = InIndex;
    if (Index < SphereElems.Num()) { return &SphereElems[Index]; }
    Index -= SphereElems.Num();
    if (Index < BoxElems.Num()) { return &BoxElems[Index]; }
    Index -= BoxElems.Num();
    if (Index < SphylElems.Num()) { return &SphylElems[Index]; }
    Index -= SphylElems.Num();
    if (Index < ConvexElems.Num()) { return &ConvexElems[Index]; }
    Index -= ConvexElems.Num();
    return nullptr;
}

const FKShapeElem* FKAggregateGeom::GetElement(const int32 InIndex) const
{
    int Index = InIndex;
    if (Index < SphereElems.Num()) { return &SphereElems[Index]; }
    Index -= SphereElems.Num();
    if (Index < BoxElems.Num()) { return &BoxElems[Index]; }
    Index -= BoxElems.Num();
    if (Index < SphylElems.Num()) { return &SphylElems[Index]; }
    Index -= SphylElems.Num();
    if (Index < ConvexElems.Num()) { return &ConvexElems[Index]; }
    Index -= ConvexElems.Num();
    return nullptr;
}

const FKShapeElem* FKAggregateGeom::GetElementByName(const FName InName) const
{
    if (const FKShapeElem* FoundSphereElem = GetElementByName<FKSphereElem>(SphereElems, InName))
    {
        return FoundSphereElem;
    }
    else if (const FKShapeElem* FoundBoxElem = GetElementByName<FKBoxElem>(BoxElems, InName))
    {
        return FoundBoxElem;
    }
    else if (const FKShapeElem* FoundSphylElem = GetElementByName<FKSphylElem>(SphylElems, InName))
    {
        return FoundSphylElem;
    }
    else if (const FKShapeElem* FoundConvexElem = GetElementByName<FKConvexElem>(ConvexElems, InName))
    {
        return FoundConvexElem;
    }
    return nullptr;
}

int32 FKAggregateGeom::GetElementIndexByName(const FName InName) const
{
    int32 FoundIndex = GetElementIndexByName<FKSphereElem>(SphereElems, InName);
    int32 StartIndex = 0;
    if (FoundIndex != INDEX_NONE)
    {
        return FoundIndex + StartIndex;
    }
    StartIndex += SphereElems.Num();

    FoundIndex = GetElementIndexByName<FKBoxElem>(BoxElems, InName);
    if (FoundIndex != INDEX_NONE)
    {
        return FoundIndex + StartIndex;
    }
    StartIndex += BoxElems.Num();

    FoundIndex = GetElementIndexByName<FKSphylElem>(SphylElems, InName);
    if (FoundIndex != INDEX_NONE)
    {
        return FoundIndex + StartIndex;
    }
    StartIndex += SphylElems.Num();

    FoundIndex = GetElementIndexByName<FKConvexElem>(ConvexElems, InName);
    if (FoundIndex != INDEX_NONE)
    {
        return FoundIndex + StartIndex;
    }
    StartIndex += ConvexElems.Num();

    return INDEX_NONE;
}

void FKAggregateGeom::GetAggGeom(const FTransform& Transform, const FColor Color, const UMaterial* MatInst, bool bPerHullColor, bool bDrawSolid,
    bool bOutputVelocity, int32 ViewIndex) const
{
    // Debug Draw 용인듯?
}

FBoundingBox FKAggregateGeom::CalcAABB(const FTransform& Transform) const
{
    const FVector Scale3D = Transform.GetScale();
    FTransform BoneTM = Transform;
    BoneTM.RemoveScaling();

    FBoundingBox Box = FBoundingBox();

    // Instead of ignore if not uniform, I'm getting Min of the abs value
    // the reason for below function is for negative scale
    // say if you have scale of (-1, 2, -3), you'd like to get -1;
    const float ScaleFactor = SelectMinScale(Scale3D);

    for(int32 i=0; i<SphereElems.Num(); i++)
    {
        Box += SphereElems[i].CalcAABB(BoneTM, ScaleFactor);
    }

    for(int32 i=0; i<BoxElems.Num(); i++)
    {
        Box += BoxElems[i].CalcAABB(BoneTM, ScaleFactor);
    }

    for(int32 i=0; i<SphylElems.Num(); i++)
    {
        Box += SphylElems[i].CalcAABB(BoneTM, ScaleFactor);
    }

    // Accumulate convex element bounding boxes.
    for(int32 i=0; i<ConvexElems.Num(); i++)
    {
        Box += ConvexElems[i].CalcAABB(BoneTM, (FVector)Scale3D);
    }
    
    return Box;
}

void FKAggregateGeom::CalcBoxSphereBounds(FBoundingBox& Output, const FTransform& LocalToWorld) const
{
    // Calculate the AABB
    const FBoundingBox AABB = CalcAABB(LocalToWorld);

    if ((SphereElems.Num() == 0) && (SphylElems.Num() == 0) && (BoxElems.Num() == 0))
    {
        // For bounds that only consist of convex shapes (such as anything generated from a BSP model),
        // we can get nice tight bounds by considering just the points of the convex shape
        const FVector Origin = AABB.GetCenter();

        float RadiusSquared = 0.0f;
        for (int32 i = 0; i < ConvexElems.Num(); i++)
        {
            const FKConvexElem& Elem = ConvexElems[i];
            for (int32 j = 0; j < Elem.VertexData.Num(); ++j)
            {
                const FVector Point = LocalToWorld.TransformPosition(Elem.VertexData[j]);
                RadiusSquared = FMath::Max(RadiusSquared, (Point - Origin).MagnitudeSquared());
            }
        }

        // Push the resulting AABB and sphere into the output
        FVector OutOrigin;
        FVector OutExtents;
        
        AABB.GetCenterAndExtents(OutOrigin, OutExtents);
        Output.Min = OutOrigin - OutExtents;
        Output.Max = OutOrigin + OutExtents;
    }
    else if ((SphereElems.Num() == 1) && (SphylElems.Num() == 0) && (BoxElems.Num() == 0) && (ConvexElems.Num() == 0))
    {
        FVector OutOrigin;
        FVector OutExtents;
        
        AABB.GetCenterAndExtents(OutOrigin, OutExtents);
        Output.Min = OutOrigin - OutExtents;
        Output.Max = OutOrigin + OutExtents;
    }
    else
    {
        // Just use the loose sphere bounds that totally fit the AABB
        Output = FBoundingBox(AABB);
    }
}

float FKAggregateGeom::GetVolume(const FVector& Scale3D) const
{
    return GetScaledVolume(Scale3D);
}

float FKAggregateGeom::GetScaledVolume(const FVector& Scale3D) const
{
    float Volume = 0.0f;

    for (auto SphereElem : SphereElems)
    {
        Volume += SphereElem.GetVolume(Scale3D);
    }

    for (auto BoxElem : BoxElems)
    {
        Volume += BoxElem.GetVolume(Scale3D);
    }

    for (auto SphylElem : SphylElems)
    {
        Volume += SphylElem.GetVolume(Scale3D);
    }

    for (auto ConvexElem : ConvexElems)
    {
        Volume += ConvexElem.GetVolume(Scale3D);
    }

    return Volume;
}

