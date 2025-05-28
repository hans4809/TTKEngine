#include "PhysicsConstraintTemplate.h"
#include "Physics/ConstraintInstance.h"
#include "Physics/ConstraintProfileProperties.h"

UPhysicsConstraintTemplate::UPhysicsConstraintTemplate()
{
    DefaultProfile.LinearXMotion = EConstraintMotion::Locked;
    DefaultProfile.LinearYMotion = EConstraintMotion::Locked;
    DefaultProfile.LinearZMotion = EConstraintMotion::Locked;
    DefaultProfile.TwistLimitAngle = 45.0f;
    DefaultProfile.AngularStiffness = 50.0f;
    DefaultProfile.RefFrame1 = FTransform::Identity; // 기본 로컬 프레임
    DefaultProfile.RefFrame2 = FTransform::Identity; // 기본 로컬 프레임
}
