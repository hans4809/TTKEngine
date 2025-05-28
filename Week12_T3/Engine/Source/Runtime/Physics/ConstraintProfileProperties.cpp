#include "ConstraintProfileProperties.h"

void FConstraintProfileProperties::CopyFrom(const FConstraintProfileProperties& Other)
{
    LinearXMotion = Other.LinearXMotion;
    LinearYMotion = Other.LinearYMotion;
    LinearZMotion = Other.LinearZMotion;
    LinearLimitValue = Other.LinearLimitValue;
    bLinearSoftConstraint = Other.bLinearSoftConstraint;
    LinearStiffness = Other.LinearStiffness;
    LinearDamping = Other.LinearDamping;

    AngularTwistMotion = Other.AngularTwistMotion;
    TwistLimitAngle = Other.TwistLimitAngle;
    bTwistSoftConstraint = Other.bTwistSoftConstraint;

    AngularSwing1Motion = Other.AngularSwing1Motion;
    Swing1LimitAngle = Other.Swing1LimitAngle;
    bSwing1SoftConstraint = Other.bSwing1SoftConstraint;

    AngularSwing2Motion = Other.AngularSwing2Motion;
    Swing2LimitAngle = Other.Swing2LimitAngle;
    bSwing2SoftConstraint = Other.bSwing2SoftConstraint;
    AngularStiffness = Other.AngularStiffness;
    AngularDamping = Other.AngularDamping;

    RefFrame1 = Other.RefFrame1;
    RefFrame2 = Other.RefFrame2;

    bEnableLinearPositionDrive = Other.bEnableLinearPositionDrive;
    LinearDriveMaxForce = Other.LinearDriveMaxForce;
    LinearPositionTarget = Other.LinearPositionTarget;

    bEnableAngularOrientationDrive = Other.bEnableAngularOrientationDrive;
    AngularDriveMaxForce = Other.AngularDriveMaxForce;
    AngularOrientationTarget = Other.AngularOrientationTarget;

    bLinearBreakable = Other.bLinearBreakable;
    LinearBreakThreshold = Other.LinearBreakThreshold;
    bAngularBreakable = Other.bAngularBreakable;
    AngularBreakThreshold = Other.AngularBreakThreshold;

    bDisableCollision = Other.bDisableCollision;
    bEnableProjection = Other.bEnableProjection;
    ProjectionLinearTolerance = Other.ProjectionLinearTolerance;
    ProjectionAngularTolerance = Other.ProjectionAngularTolerance;
}