#include "HitResult.h"
#include <PxPhysicsAPI.h>
#include <PxQueryReport.h>

FHitResult FHitResult::FromPxRaycastHit(const physx::PxRaycastHit& pxHit)
{
    FHitResult result;
    result.actor = pxHit.actor;
    result.shape = pxHit.shape;
    if (result.shape && pxHit.shape->getNbMaterials() > 0)
    {
        pxHit.shape->getMaterials(&result.material, 1);
    }
    result.impactPoint = FVector(pxHit.position.x, pxHit.position.y, pxHit.position.z);
    result.impactNormal = FVector(pxHit.normal.x, pxHit.normal.y, pxHit.normal.z);
    result.distance = pxHit.distance;
    result.faceIndex = pxHit.faceIndex;
    result.bBlockingHit = true; 
    result.bInitialOverlap = false;
    return result;
}

FHitResult FHitResult::FromPxSweepHit(const physx::PxSweepHit& pxHit)
{
    FHitResult result;
    result.actor = pxHit.actor;
    result.shape = pxHit.shape;
    if (result.shape && pxHit.shape->getNbMaterials() > 0) 
    {
        pxHit.shape->getMaterials(&result.material, 1);
    }
    result.impactPoint = FVector(pxHit.position.x, pxHit.position.y, pxHit.position.z);
    result.impactNormal = FVector(pxHit.normal.x, pxHit.normal.y, pxHit.normal.z);
    result.distance = pxHit.distance;
    result.faceIndex = pxHit.faceIndex;
    result.bBlockingHit = true; 
    
   
    if (pxHit.flags & physx::PxHitFlag::ePRECISE_SWEEP)
    {
        result.bInitialOverlap = true;
    }
    else 
    {
        result.bInitialOverlap = false;
    }
    return result;
}

FHitResult FHitResult::FromPxOverlapHit(const physx::PxOverlapHit& pxHit)
{
    FHitResult result;
    result.actor = pxHit.actor;
    result.shape = pxHit.shape;
    if (result.shape && pxHit.shape->getNbMaterials() > 0)
    {
        pxHit.shape->getMaterials(&result.material, 1);
    }

    result.distance = 0.0f;

    result.bBlockingHit = false; 
    
    result.bInitialOverlap = true;
    return result;
}

UObject* FHitResult::GetHitUObject() const
{
    if (actor && actor->userData)
    {
        return Cast<UObject>(actor->userData);
    }
    return nullptr;
}