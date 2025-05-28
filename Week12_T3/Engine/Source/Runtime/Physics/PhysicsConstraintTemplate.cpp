#include "PhysicsConstraintTemplate.h"
#include "Physics/ConstraintInstance.h"
#include "Physics/ConstraintProfileProperties.h"

UPhysicsConstraintTemplate::UPhysicsConstraintTemplate()
{
    DefaultProfile.LinearXMotion = EConstraintMotion::Locked;
    DefaultProfile.LinearYMotion = EConstraintMotion::Locked;
    DefaultProfile.LinearZMotion = EConstraintMotion::Locked;

    //DefaultProfile.TwistLimitAngle = 10.0f;
    //DefaultProfile.AngularStiffness = 100.0f;
    //DefaultProfile.LinearStiffness = 1.0f;
    //DefaultProfile.LinearDamping = 30.0f;
    //DefaultProfile.AngularDamping = 30.0f;

    DefaultProfile.AngularTwistMotion= EConstraintMotion::Limited;
    DefaultProfile.AngularSwing1Motion = EConstraintMotion::Limited;
    DefaultProfile.AngularSwing2Motion = EConstraintMotion::Limited;
    DefaultProfile.TwistLimitAngle = 100.0f;
    DefaultProfile.AngularStiffness = 500.0f;
    DefaultProfile.LinearStiffness = 500.0f;
    DefaultProfile.LinearDamping = 500.0f;
    DefaultProfile.AngularDamping = 500.0f;

    DefaultProfile.RefFrame1 = FTransform::Identity; // 기본 로컬 프레임
    DefaultProfile.RefFrame2 = FTransform::Identity; // 기본 로컬 프레임
}
