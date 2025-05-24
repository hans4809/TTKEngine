#pragma once
#include "UObject/ObjectMacros.h"
#include "Physics/ShapeElems/BoxElem.h"
#include "Physics/ShapeElems/SphereElem.h"
#include "Physics/ShapeElems/SphylElem.h"
#include "Physics/ShapeElems/ConvexElem.h"

struct FKAggregateGeom
{
    DECLARE_STRUCT(FKAggregateGeom)

    UPROPERTY(EditAnywhere, TArray<FKSphereElem>, SphereElems, = {})

    UPROPERTY(EditAnywhere, TArray<FKBoxElem>, BoxElems, = {})

    UPROPERTY(EditAnywhere, TArray<FKSphylElem>, SphylElems, = {})
     
    UPROPERTY(EditAnywhere, TArray<FKConvexElem>, ConvexElems, = {})
};
