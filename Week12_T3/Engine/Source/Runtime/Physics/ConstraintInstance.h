#pragma once

#include "UObject/ObjectMacros.h"
#include "ConstraintProfileProperties.h"
#include "PhysicsEnumDefinitions.h"
namespace physx
{
    class PxJoint;
};
class UPrimitiveComponent;

class FPhysScene;
class FBodyInstance;

struct FConstraintInstance
{
    // DECLARE_STRUCT(FConstraintInstance) // 프로젝트의 매크로 시스템에 따라 필요

    // --- 기본 식별 정보 ---

    // Constraint의 고유 이름 (에디터에서 식별용)
    // UPROPERTY(EditAnywhere, Category="Constraint")
    FName JointName = NAME_None; // 언리얼의 ConstraintInstance.JointName

    // --- 연결 대상 ---
    // 조인트로 연결될 첫 번째 액터/컴포넌트의 본(Bone) 또는 액터 이름
    // UPROPERTY(EditAnywhere, Category="Constraint")
    FName ConstraintBone1 = NAME_None; // 언리얼의 ConstraintInstance.ConstraintBone1

    // 조인트로 연결될 두 번째 액터/컴포넌트의 본(Bone) 또는 액터 이름
    // ConstraintBone2가 NAME_None이면, ConstraintBone1을 월드에 고정하는 것으로 간주될 수 있음.
    // UPROPERTY(EditAnywhere, Category="Constraint")
    FName ConstraintBone2 = NAME_None; // 언리얼의 ConstraintInstance.ConstraintBone2

    // 런타임에 실제 연결되는 컴포넌트 (WeakPtr 등으로 관리 가능)
    // transient
    UPrimitiveComponent* Component1 = nullptr;
    // transient
    UPrimitiveComponent* Component2 = nullptr;

    // --- PhysX 조인트 (런타임 전용) ---

    physx::PxJoint* PxJoint = nullptr;

    // --- 조인트 위치 및 방향 설정 (로컬 스페이스 기준) ---

    // 각 연결된 바디(ConstraintBone1, ConstraintBone2)의 로컬 공간에서의 조인트 프레임(위치 및 방향)

    // PxD6Joint의 setLocalPose()에 사용됨.

    // ConstraintBone1의 로컬 공간에서의 조인트 프레임
    FTransform Pos1 = FTransform::Identity; // 언리얼의 ConstraintInstance.Pos1
    FQuat PriAxis1 = FQuat::Identity; // 언리얼의 ConstraintInstance.PriAxis1 (D6에서는 Pos1에 포함 가능)

    FQuat SecAxis1 = FQuat::Identity; // 언리얼의 ConstraintInstance.SecAxis1 (D6에서는 Pos1에 포함 가능)

    // ConstraintBone2의 로컬 공간에서의 조인트 프레임
    FTransform Pos2 = FTransform::Identity; // 언리얼의 ConstraintInstance.Pos2
    FQuat PriAxis2 = FQuat::Identity; // 언리얼의 ConstraintInstance.PriAxis2
    FQuat SecAxis2 = FQuat::Identity; // 언리얼의 ConstraintInstance.SecAxis2

    // --- 조인트 프로파일 및 제한 (FConstraintProfileProperties 에서 주로 관리) ---
    // 다양한 조인트 설정 (선형/각도 제한, 스프링, 모터 등)을 그룹화한 프로파일.
    // 언리얼에서는 FConstraintProfileProperties 내에 상세 설정이 있음.
    FConstraintProfileProperties ProfileInstance;

    // --- 추가적인 조인트 제어 플래그 (언리얼의 FConstraintInstance 멤버들 참고) ---

    bool bLinearBreakable = false;

    float LinearBreakThreshold = 300.0f;

    bool bAngularBreakable = false;

    float AngularBreakThreshold = 500.0f;

    bool bDisableCollision = false; // 연결된 두 바디 간의 충돌을 비활성화할지 여부

    bool bParentDominates = false; // (레거시 또는 특정 상황)

    bool bEnableProjection = false; // 조인트 오차 보정 기능 활성화

    float ProjectionLinearTolerance = 1.0f;

    float ProjectionAngularTolerance = 1.0f; // 라디안 단위

    // --- 런타임 함수 (실제 PhysX 조인트 생성 및 관리에 필요) ---
public:
    FConstraintInstance() = default;

    // 조인트 초기화 및 PhysX 조인트 생성
    // InOwnerComponent는 이 ConstraintInstance를 소유하는 컴포넌트 (예: UPhysicsConstraintComponent)
    // InBody1, InBody2는 실제 연결될 FBodyInstance
    bool InitConstraint(UPrimitiveComponent* InOwnerComponent, FBodyInstance* InBody1, FBodyInstance* InBody2, FPhysScene* InScene);

    // PhysX 조인트 해제
    void TermConstraint(FPhysScene* InScene);

    // 조인트의 파괴 가능성 설정 적용
    void ApplyBreakableLimits();

    // 연결된 바디 간 충돌 비활성화 설정 적용
    void ApplyDisableCollision();

    // 프로파일에 설정된 제한 값들을 실제 PxJoint에 적용
    void ApplyFrames();         // 로컬 프레임 설정
    void ApplyLimits();         // 선형/각도 제한 설정
    void ApplyDrive();          // 모터/스프링 설정
    void ApplyProjection();     // 프로젝션 설정

    // 현재 조인트의 힘 또는 오차 등을 가져오는 함수 (디버깅/정보용)
    float GetCurrentSwing1() const;
    float GetCurrentSwing2() const;
    float GetCurrentTwist() const;

    bool IsBroken() const;
    void ClearBroken(); // 파괴 상태 리셋

    // 연결된 FBodyInstance 가져오기 (PxJoint에서 직접 가져올 수도 있음)
    FBodyInstance* GetBody1() const;
    FBodyInstance* GetBody2() const;

    // 에디터에서 사용될 수 있는 유틸리티 함수
    FName GetRefBoneName(int32 BoneIndex) const;
    FTransform GetRefFrame(int32 BoneIndex) const;

private:
    // 내부 상태 플래그
    bool bConstraintBroken = false;

    // 조인트 설정을 PxJoint에 실제로 적용하는 헬퍼 함수들
    void SetLinearLimits_AssumesLocked();
    void SetAngularLimits_AssumesLocked();
};
