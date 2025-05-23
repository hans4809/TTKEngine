#pragma once
#include "Define.h"
#include "Math/Vector.h"
#include "UObject/Casts.h"

namespace physx 
{
    class PxRigidActor;
    class PxShape;
    class PxMaterial;
    struct PxVec3;
    struct PxRaycastHit;
    struct PxSweepHit;
    struct PxOverlapHit;
}

struct FHitResult 
{
    // 충돌한 액터 및 셰이프
    physx::PxRigidActor* actor = nullptr;
    physx::PxShape* shape = nullptr;
    physx::PxMaterial* material = nullptr;

    // 월드 공간에서의 충돌 지점 위치
    FVector impactPoint = { 0.0f, 0.0f, 0.0f };

    // 월드 공간에서의 충돌 지점 표면 법선 벡터
    FVector impactNormal = { 0.0f, 0.0f, 0.0f };

    // 레이/스윕 시작점에서 충돌 지점까지의 거리.
    float distance = -1.0f;

    // 충돌한 면의 인덱스 (주로 트라이앵글 메쉬용).
    uint32 faceIndex = 0xFFFFFFFF;

    // 충돌이 쿼리의 진행을 막는 '블로킹' 충돌인지 여부.
    bool bBlockingHit = false;

    bool bInitialOverlap = false;

    FHitResult() = default;

    // PxRaycastHit로부터 FHitResult를 채우는 헬퍼 함수
    static FHitResult FromPxRaycastHit(const physx::PxRaycastHit& pxHit);

    // PxSweepHit로부터 FHitResult를 채우는 헬퍼 함수
    static FHitResult FromPxSweepHit(const physx::PxSweepHit& pxHit);

    // PxOverlapHit로부터 FHi{}tResult를 채우는 헬퍼 함수 (정보가 적음)
    static FHitResult FromPxOverlapHit(const physx::PxOverlapHit& pxHit);

    UObject* GetHitUObject() const;
};