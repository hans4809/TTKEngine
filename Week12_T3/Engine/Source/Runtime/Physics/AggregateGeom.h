#pragma once
#include "UObject/ObjectMacros.h"
#include "Physics/ShapeElems/BoxElem.h"
#include "Physics/ShapeElems/SphereElem.h"
#include "Physics/ShapeElems/SphylElem.h"
#include "Physics/ShapeElems/ConvexElem.h"

class UMaterial;

struct FKAggregateGeom
{
    DECLARE_STRUCT(FKAggregateGeom)

    UPROPERTY(EditAnywhere, TArray<FKSphereElem>, SphereElems, = {})

    UPROPERTY(EditAnywhere, TArray<FKBoxElem>, BoxElems, = {})

    UPROPERTY(EditAnywhere, TArray<FKSphylElem>, SphylElems, = {})
     
    UPROPERTY(EditAnywhere, TArray<FKConvexElem>, ConvexElems, = {})

    FKAggregateGeom()
    {}

    FKAggregateGeom(const FKAggregateGeom& Other)
    {
        CloneAgg(Other);
    }

    const FKAggregateGeom& operator=(const FKAggregateGeom& Other)
    {
        CloneAgg(Other);
        return *this;
    }

    int32 GetElementCount() const
    {
        return SphereElems.Num() + SphylElems.Num() + BoxElems.Num() + ConvexElems.Num();
    }

    int32 GetElementCount(EAggCollisionShape::Type Type) const;

    FORCEINLINE SIZE_T GetAllocatedSize() const
    {
        return SphereElems.GetAllocatedSize() + SphylElems.GetAllocatedSize() + BoxElems.GetAllocatedSize() + ConvexElems.GetAllocatedSize();
    }

    FKShapeElem* GetElement(const EAggCollisionShape::Type Type, const int32 Index);

    FKShapeElem* GetElement(const int32 InIndex);

    const FKShapeElem* GetElement(const int32 InIndex) const;

    const FKShapeElem* GetElementByName(const FName InName) const;
    
    int32 GetElementIndexByName(const FName InName) const;

    void EmptyImportedElements()
    {
        auto CleanUp = [](auto& Elems)
        {
            Elems.RemoveAll([](const FKShapeElem& Elem)
            {
                return Elem.bIsGenerated == false;
            });
        };
        CleanUp(BoxElems);
        CleanUp(ConvexElems);
        CleanUp(SphylElems);
        CleanUp(SphereElems);
    }

    void EmptyElements()
    {
        BoxElems.Empty();
        ConvexElems.Empty();
        SphylElems.Empty();
        SphereElems.Empty();
    }

    void GetAggGeom(const FTransform& Transform, const FColor Color, const UMaterial* MatInst, bool bPerHullColor, bool bDrawSolid, bool bOutputVelocity, int32 ViewIndex) const;

    FBoundingBox CalcAABB(const FTransform& Transform) const;
    
    void CalcBoxSphereBounds(FBoundingBox& Output, const FTransform& LocalToWorld) const;

    float GetVolume(const FVector& Scale3D) const;

    /** Returns the volume of this element */
    float GetScaledVolume(const FVector& Scale3D) const;
private:

    /** Helper function for safely copying instances */
    void CloneAgg(const FKAggregateGeom& Other)
    {
        SphereElems = Other.SphereElems;
        BoxElems = Other.BoxElems;
        SphylElems = Other.SphylElems;
        ConvexElems = Other.ConvexElems;
    }

    template <class T>
    const FKShapeElem* GetElementByName(TArray<T> Elements, const FName InName) const
    {
        const FKShapeElem* FoundElem = Elements.FindByPredicate(
            [InName](const T& Elem)
            {
                return InName == Elem.GetName();
            });
        return FoundElem;
    }
    
    template <class T>
    int32 GetElementIndexByName(TArray<T> Elements, const FName InName) const
    {
        int32 FoundIndex = Elements.IndexOfByPredicate(
            [InName](const T& Elem)
            {
                return InName == Elem.GetName();
            });
        return FoundIndex;
    }

};
