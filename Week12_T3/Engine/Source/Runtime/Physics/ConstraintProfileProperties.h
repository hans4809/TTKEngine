#pragma once
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

struct FConstraintProfileProperties
{
    DECLARE_STRUCT(FConstraintProfileProperties)

    // Twist 제한 각도 (degree)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, TwistLimitAngle, = 45.f)
    
    // Swing 제한 각도 (degree)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, Swing1LimitAngle, = 45.f)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, Swing2LimitAngle, = 45.f)

    // 선형 Breakable 
    UPROPERTY(EPropertyFlags::EditAnywhere, bool, bLinearBreakable, = false)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, LinearBreakThreshold, = 0.0f)

    //Angular Breakable
    UPROPERTY(EPropertyFlags::EditAnywhere, bool, bAngularBreakable, = false)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, AngularBreakThreshold, = 0.0f)

    // 선형 드라이브 Stiffness / Damping
    UPROPERTY(EPropertyFlags::EditAnywhere, float, LinearStiffness, = 0.0f);
    UPROPERTY(EPropertyFlags::EditAnywhere, float, LinearDamping, = 0.0f)

    // Angular 드라이브 Stiffness / Damping
    UPROPERTY(EPropertyFlags::EditAnywhere, float, AngularStiffness, = 0.0f);
    UPROPERTY(EPropertyFlags::EditAnywhere, float, AngularDamping, = 0.0f)

    // Projection ( Error Connection)
    UPROPERTY(EPropertyFlags::EditAnywhere, bool, bEnableProjection, = false)

    // TODO : meta = (EditCondition = "bEnableProjection"))
    UPROPERTY(EPropertyFlags::EditAnywhere, float, ProjectionLinearTolerance, = 1.0f);

    // TODO : meta = (EditCondition = "bEnableProjection")
    UPROPERTY(EPropertyFlags::EditAnywhere, float, ProjectionAngularTolerance, = 1.0f);

    void CopyFrom(const FConstraintProfileProperties& Other)
    {
        TwistLimitAngle = Other.TwistLimitAngle;
        Swing1LimitAngle = Other.Swing1LimitAngle;
        Swing2LimitAngle = Other.Swing2LimitAngle;

        bLinearBreakable = Other.bLinearBreakable;
        LinearBreakThreshold = Other.LinearBreakThreshold;

        bAngularBreakable = Other.bAngularBreakable;
        AngularBreakThreshold = Other.AngularBreakThreshold;

        LinearStiffness = Other.LinearStiffness;
        LinearDamping = Other.LinearDamping;
        AngularStiffness = Other.AngularStiffness;
        AngularDamping = Other.AngularDamping;

        bEnableProjection = Other.bEnableProjection;
        ProjectionLinearTolerance = Other.ProjectionLinearTolerance;
        ProjectionAngularTolerance = Other.ProjectionAngularTolerance;
    }
};