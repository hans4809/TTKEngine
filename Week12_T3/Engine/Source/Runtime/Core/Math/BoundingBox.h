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

    void Serialize(FArchive& Ar) const;
    
    void Deserialize(FArchive& Ar);

    FBoundingBox TransformBy(FMatrix InMatirx) const;
};
