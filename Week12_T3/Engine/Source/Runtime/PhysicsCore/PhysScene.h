#pragma once
#include "Math/Vector.h"
#include "HitResult.h"
class UPrimitiveComponent;

class FPhysScene 
{
public:
    virtual ~FPhysScene() = default;

    virtual void AddObject(UPrimitiveComponent* Component) = 0;
    virtual void Simulate(float DeltaTime) = 0;
    // virtual void FetchResults(bool bBlock) = 0; // Simulate에 통합될 수도 있음
    
    // 가장 가까운 첫 번째 교차점에 대한 정보만을 반환
    virtual bool RaycastSingle(const FVector& Origin, const FVector& Direction, float MaxDistance, FHitResult& OutHit) = 0;
    virtual void SetGravity(const FVector& NewGravity) = 0;
};