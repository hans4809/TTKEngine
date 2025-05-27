#pragma once
#include "UObject/ObjectMacros.h"

struct FBoundingBox
{
    DECLARE_STRUCT(FBoundingBox);
    
    FBoundingBox(){}
    FBoundingBox(const FVector InMin, const FVector InMax) : Min(InMin), Max(InMax) {}
    UPROPERTY(EditAnywhere, FVector, Min, = FVector::ZeroVector)
    float padding0;
    UPROPERTY(EditAnywhere, FVector, Max, = FVector::ZeroVector)
    
    float padding1;
    bool IntersectRay(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const;
    bool IntersectAABB(FBoundingBox Other) const;
    FBoundingBox(const TArray<FVector>& Points);
    void Serialize(FArchive& Ar) const;
    
    void Deserialize(FArchive& Ar);

    FBoundingBox TransformBy(FMatrix InMatirx) const;
    FBoundingBox operator+=(const FBoundingBox& bounding_box);
    
    FVector GetCenter() const;

    FORCEINLINE FVector GetExtent() const
    {
        return (Max - Min) * 0.5f;
    }
    
    FORCEINLINE void GetCenterAndExtents(FVector& Center, FVector& Extents) const
    {
        Extents = GetExtent();
        Center = Min + Extents;
    }

    FVector GetSize() const;
    FBoundingBox ExpandBy(FVector V) const;
};
