#pragma once
#include "UObject/ObjectMacros.h"
#include "ConstraintProfileProperties.h"

class PxD6Joint;
class FConstraintProfileProperties;
struct FConstraintInstance
{
    DECLARE_STRUCT(FConstraintInstance)

    // Constraint 이름
    UPROPERTY(EPropertyFlags::EditAnywhere, FName, JointName, = NAME_None);

    // 부모 본 이름
    UPROPERTY(EPropertyFlags::EditAnywhere, FName, ConstraintBone1, = NAME_None);
    UPROPERTY(EPropertyFlags::EditAnywhere, FName, ConstraintBone2, = NAME_None);

    // FIXME : 필요 시 UPROPERTY 등록 하기
    FConstraintProfileProperties ProfileInstance;   // Twist/Swing 제한 등
    PxD6Joint* PxJoint;         // PhysX 조인트 포인터 (런타임에만!)

    //UPROPERTY(EPropertyFlags::EditAnywhere, FConstraintProfileProperties, ProfileInstance,  = FConstraintProfileProperties());
};
