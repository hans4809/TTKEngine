#pragma once
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "PhysicsEnumDefinitions.h"
#include "Math/Transform.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

struct FConstraintProfileProperties
{
    DECLARE_STRUCT(FConstraintProfileProperties)


    // === 선형 제한 (Linear Limits) ===
    UPROPERTY(EditAnywhere, EConstraintMotion, LinearXMotion, = EConstraintMotion::Limited)
    UPROPERTY(EditAnywhere, EConstraintMotion, LinearYMotion, = EConstraintMotion::Limited)
    UPROPERTY(EditAnywhere, EConstraintMotion, LinearZMotion, = EConstraintMotion::Limited)

    UPROPERTY(EditAnywhere, float, LinearLimitValue, = 0.0f)
    UPROPERTY(EditAnywhere, bool, bLinearSoftConstraint, = false)
    // LinearStiffness, LinearDamping: 기존 멤버 (소프트 제한 및/또는 드라이브에 사용)
    UPROPERTY(EditAnywhere, float, LinearStiffness, = 0.0f)
    UPROPERTY(EditAnywhere, float, LinearDamping, = 0.0f)

    // === 각도 제한 (Angular Limits) ===
    UPROPERTY(EditAnywhere, EConstraintMotion, AngularTwistMotion, = EConstraintMotion::Limited)
    UPROPERTY(EditAnywhere, float, TwistLimitAngle, = 45.f)
    UPROPERTY(EditAnywhere, bool, bTwistSoftConstraint, = true)

    UPROPERTY(EditAnywhere, EConstraintMotion, AngularSwing1Motion, = EConstraintMotion::Limited)
    UPROPERTY(EditAnywhere, float, Swing1LimitAngle, = 45.f)
    UPROPERTY(EditAnywhere, bool, bSwing1SoftConstraint, = true)

    UPROPERTY(EditAnywhere, EConstraintMotion, AngularSwing2Motion, = EConstraintMotion::Limited)
    UPROPERTY(EditAnywhere, float, Swing2LimitAngle, = 45.f)
    UPROPERTY(EditAnywhere, bool, bSwing2SoftConstraint, = true)

    // AngularStiffness, AngularDamping: (소프트 제한 및/또는 드라이브에 사용)
    UPROPERTY(EditAnywhere, float, AngularStiffness, = 0.0f)
    UPROPERTY(EditAnywhere, float, AngularDamping, = 0.0f)

    // === 조인트 로컬 프레임 ===
    FTransform RefFrame1;
    FTransform RefFrame2;

    // === 드라이브 설정 ===
    // 선형 드라이브
    UPROPERTY(EditAnywhere, bool, bEnableLinearPositionDrive, = false)
    FVector LinearPositionTarget;
    UPROPERTY(EditAnywhere, float, LinearDriveMaxForce, = 0.0f)

    // 각도 드라이브
    UPROPERTY(EditAnywhere, bool, bEnableAngularOrientationDrive, = false)
    FQuat AngularOrientationTarget;
    UPROPERTY(EditAnywhere, float, AngularDriveMaxForce, = 0.0f)
    // (AngularStiffness, AngularDamping은 이미 위에 정의됨)

    // === 파괴 가능성 (기존 멤버) ===
    UPROPERTY(EditAnywhere, bool, bLinearBreakable, = false)
    UPROPERTY(EditAnywhere, float, LinearBreakThreshold, = 300.0f)
    UPROPERTY(EditAnywhere, bool, bAngularBreakable, = false)
    UPROPERTY(EditAnywhere, float, AngularBreakThreshold, = 500.0f)

    // === 프로젝션 (기존 멤버) ===
    UPROPERTY(EditAnywhere, bool, bEnableProjection, = true)
    UPROPERTY(EditAnywhere, float, ProjectionLinearTolerance, = 1.0f)
    UPROPERTY(EditAnywhere, float, ProjectionAngularTolerance, = 1.0f)

    // === 기타 ===
    UPROPERTY(EditAnywhere, bool, bDisableCollision, = true)

    void CopyFrom(const FConstraintProfileProperties& Other);
};