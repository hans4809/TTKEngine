#include "ConstraintInstance.h"
#include "Physics/ConstraintProfileProperties.h"
#include "PhysicsCore/PhysScene.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysXSDKManager.h"
#include "ConstraintProfileProperties.h"
#include <PxPhysicsAPI.h>
using namespace physx;
namespace {
    physx::PxD6Motion::Enum ConvertMotion(EConstraintMotion InMotion)
    {
        switch (InMotion)
        {
        case EConstraintMotion::Free:    return physx::PxD6Motion::eFREE;
        case EConstraintMotion::Limited: return physx::PxD6Motion::eLIMITED;
        case EConstraintMotion::Locked:  return physx::PxD6Motion::eLOCKED;
        default:                       return physx::PxD6Motion::eFREE; // 안전 기본값
        }
    }
}

bool FConstraintInstance::InitConstraint(UPrimitiveComponent* InOwnerComponent, FBodyInstance* InBody1, FBodyInstance* InBody2, FPhysScene* InScene)
{
    if (!InBody1 || !InScene) // Body1과 PhysScene은 필수
    {
       return false;
    }

    physx::PxRigidActor* Actor1 = InBody1->GetPxRigidActor(); // FBodyInstance에 GetPxActor() 함수가 있다고 가정
    physx::PxRigidActor* Actor2 = InBody2 ? InBody2->GetPxRigidActor() : nullptr;

    Component1 = InOwnerComponent;
    Component2 = InBody2 ? InBody2->GetOwnerComponent() : nullptr;

    if (!Actor1) // Body1의 PxActor는 반드시 존재해야 함
    {
        return false;
    }
    // Body2는 nullptr일 수 있음 (월드에 고정)

    physx::PxPhysics* PxPhysicsSDK = FPhysXSDKManager::GetInstance().GetPhysicsSDK();
    if (!PxPhysicsSDK)
    {
        return false;
    }

    // 로컬 프레임 변환 (FTransform Pos1, Pos2를 PxTransform으로)
    // FTransform에 ToPxTransform() 같은 변환 함수가 있다고 가정
    physx::PxTransform PxLocalFrame1 = Pos1.ToPxTransform();
    physx::PxTransform PxLocalFrame2 = Pos2.ToPxTransform();
    if (!PxLocalFrame1.isValid()) {
        UE_LOG(LogLevel::Error, TEXT("PxLocalFrame1 IS INVALID! P: (%f,%f,%f), Q: (%f,%f,%f,%f)"),
            PxLocalFrame1.p.x, PxLocalFrame1.p.y, PxLocalFrame1.p.z,
            PxLocalFrame1.q.x, PxLocalFrame1.q.y, PxLocalFrame1.q.z, PxLocalFrame1.q.w);
    }
    if (!PxLocalFrame2.isValid()) {
        UE_LOG(LogLevel::Error, TEXT("PxLocalFrame2 IS INVALID! P: (%f,%f,%f), Q: (%f,%f,%f,%f)"),
            PxLocalFrame2.p.x, PxLocalFrame2.p.y, PxLocalFrame2.p.z,
            PxLocalFrame2.q.x, PxLocalFrame2.q.y, PxLocalFrame2.q.z, PxLocalFrame2.q.w);
    }
    physx::PxD6Joint* D6 = PxD6JointCreate(*PxPhysicsSDK, Actor1, PxLocalFrame1, Actor2, PxLocalFrame2);
  
    if (!D6)
    {
        return false;
    }


    PxJoint = D6;
  
    physx::PxTransform Actor1ActualWorldPose = Actor1->getGlobalPose();
    physx::PxTransform Actor2ActualWorldPose = Actor2 ? Actor2->getGlobalPose() : physx::PxTransform(physx::PxIdentity);

    physx::PxTransform JointWorldFrameFromActor1 = Actor1ActualWorldPose * PxLocalFrame1;
    physx::PxTransform JointWorldFrameFromActor2 = Actor2ActualWorldPose * PxLocalFrame2;
    
    UE_LOG(LogLevel::Error, TEXT("Joint: %s"), *JointName.ToString()); // 어떤 조인트인지 식별
    UE_LOG(LogLevel::Error, TEXT("  Actor1 (%s) World Pose: P(%f,%f,%f) Q(%f,%f,%f,%f)"), *ConstraintBone1.ToString(), Actor1ActualWorldPose.p.x, Actor1ActualWorldPose.p.y, Actor1ActualWorldPose.p.z, Actor1ActualWorldPose.q.x, Actor1ActualWorldPose.q.y, Actor1ActualWorldPose.q.z, Actor1ActualWorldPose.q.w);
    UE_LOG(LogLevel::Error, TEXT("  PxLocalFrame1: P(%f,%f,%f) Q(%f,%f,%f,%f)"), PxLocalFrame1.p.x, PxLocalFrame1.p.y, PxLocalFrame1.p.z, PxLocalFrame1.q.x, PxLocalFrame1.q.y, PxLocalFrame1.q.z, PxLocalFrame1.q.w);
    UE_LOG(LogLevel::Error, TEXT("  => JointWorldFrame (from Actor1): P(%f,%f,%f) Q(%f,%f,%f,%f)"), JointWorldFrameFromActor1.p.x, JointWorldFrameFromActor1.p.y, JointWorldFrameFromActor1.p.z, JointWorldFrameFromActor1.q.x, JointWorldFrameFromActor1.q.y, JointWorldFrameFromActor1.q.z, JointWorldFrameFromActor1.q.w);
    
    if (Actor2) 
    {
        UE_LOG(LogLevel::Error, TEXT("  Actor2 (%s) World Pose: P(%f,%f,%f) Q(%f,%f,%f,%f)"), *ConstraintBone2.ToString(), Actor2ActualWorldPose.p.x, Actor2ActualWorldPose.p.y, Actor2ActualWorldPose.p.z, Actor2ActualWorldPose.q.x, Actor2ActualWorldPose.q.y, Actor2ActualWorldPose.q.z, Actor2ActualWorldPose.q.w);
        UE_LOG(LogLevel::Error, TEXT("  PxLocalFrame2: P(%f,%f,%f) Q(%f,%f,%f,%f)"), PxLocalFrame2.p.x, PxLocalFrame2.p.y, PxLocalFrame2.p.z, PxLocalFrame2.q.x, PxLocalFrame2.q.y, PxLocalFrame2.q.z, PxLocalFrame2.q.w);
        UE_LOG(LogLevel::Error, TEXT("  => JointWorldFrame (from Actor2): P(%f,%f,%f) Q(%f,%f,%f,%f)"), JointWorldFrameFromActor2.p.x, JointWorldFrameFromActor2.p.y, JointWorldFrameFromActor2.p.z, JointWorldFrameFromActor2.q.x, JointWorldFrameFromActor2.q.y, JointWorldFrameFromActor2.q.z, JointWorldFrameFromActor2.q.w);

        physx::PxVec3 PosDiff = JointWorldFrameFromActor1.p - JointWorldFrameFromActor2.p;
        // 쿼터니언 차이 (각도 차이) 계산은 더 복잡하지만, 일단 위치만 비교
        UE_LOG(LogLevel::Error, TEXT("  World Frame Position Difference Mag: %f"), PosDiff.magnitude());
        physx::PxQuat RotDiff = JointWorldFrameFromActor1.q * JointWorldFrameFromActor2.q.getConjugate();
        UE_LOG(LogLevel::Error, TEXT("  World Frame Rotation Difference (Angle): %f degrees"),FMath::RadiansToDegrees(RotDiff.getAngle()));
    }
    ApplyFrames();         // 로컬 프레임 (이미 생성 시 적용되었지만, 필요시 추가 조정)
    ApplyLimits();         // 선형/각도 제한
    ApplyDrive();          // 모터/스프링
    ApplyBreakableLimits();
    ApplyDisableCollision();
    ApplyProjection();

    // 사용자 데이터 설정
    PxJoint->userData = this;

    // 물리 씬에 조인트 추가
    physx::PxScene* PxScenePtr = InScene->GetPxScene();
    physx::PxScene* Scene = InScene->GetPxScene();

    if (!Actor1->getScene())     
        Scene->addActor(*Actor1);
    if (Actor2 && !Actor2->getScene()) 
        Scene->addActor(*Actor2);
    
    bConstraintBroken = false; // 초기에는 파괴되지 않은 상태

    return true; // 성공적으로 조인트 생성 및 설정 완료
}

void FConstraintInstance::TermConstraint(FPhysScene* InScene)
{
    if (PxJoint)
    {
        PxJoint->release();
        PxJoint = nullptr;
        bConstraintBroken = true;
    }
}

void FConstraintInstance::ApplyBreakableLimits()
{
    if (!PxJoint) return;
    // 파괴 가능성은 조인트 생성 시 또는 setBreakForce를 통해 설정
    physx::PxReal LinBreak = ProfileInstance.bLinearBreakable ? ProfileInstance.LinearBreakThreshold : PX_MAX_REAL;
    physx::PxReal AngBreak = ProfileInstance.bAngularBreakable ? ProfileInstance.AngularBreakThreshold : PX_MAX_REAL;
    PxJoint->setBreakForce(LinBreak, AngBreak);
}

void FConstraintInstance::ApplyDisableCollision()
{
    if (!PxJoint) return;
    // 연결된 액터 간의 충돌 활성화/비활성화
    PxJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, false);
}

void FConstraintInstance::ApplyFrames()
{
    if (!PxJoint) return;

}


void FConstraintInstance::ApplyLimits()
{
    if (!PxJoint) // PxJoint가 null이면 아무것도 하지 않음
    {
        return;
    }

    physx::PxD6Joint* JointD6 = static_cast<physx::PxD6Joint*>(PxJoint);
    if (!JointD6) // 안전을 위해 캐스팅 확인
    {
        return;
    }

    // TolerancesScale은 PhysX SDK에서 가져오는 것이 좋음 (또는 고정값 사용)
    // physx::PxTolerancesScale PxScale = PxGetPhysics().getTolerancesScale(); // PxGetPhysics() 접근 방법 필요
    // 여기서는 기본 스케일이라고 가정하고 직접 사용하지 않거나, 필요시 PxJointLinearLimitPair 등에 전달

    // --- 선형 제한 (Linear Limits) ---
    // PxD6Joint의 선형 제한은 각 축을 Locked/Limited/Free로 설정하는 것이 기본.
    // "Limited"로 설정하고 실제 제한 범위를 주는 것은 PxDistanceJoint의 setDistanceLimit과 유사하게 동작하거나,
    // 드라이브를 사용하여 구현하는 경우가 많음.
    // 여기서는 ProfileInstance.LinearLimitValue가 "거리 제한" 값이라고 가정하고,
    // 모든 선형 축이 Limited일 때만 PxJointLinearLimit을 설정 시도. (이 API는 D6에 직접 없을 수 있음)
    // 가장 일반적인 래그돌 설정은 모든 선형 축을 Locked로 하는 것.

    //JointD6->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);
    //JointD6->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eFREE);
    //JointD6->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eFREE);
    JointD6->setMotion(physx::PxD6Axis::eX, ConvertMotion(ProfileInstance.LinearXMotion));
    JointD6->setMotion(physx::PxD6Axis::eY, ConvertMotion(ProfileInstance.LinearYMotion));
    JointD6->setMotion(physx::PxD6Axis::eZ, ConvertMotion(ProfileInstance.LinearZMotion));

    // 만약 모든 선형 축이 Limited이고, ProfileInstance.LinearLimitValue로 "거리 제한"을 표현하고 싶다면,
    // PxD6Joint에는 직접적인 setDistanceLimit 같은 API가 없을 수 있습니다.
    // 이 경우, 각 축을 Locked로 두거나, Drive를 사용하여 스프링처럼 동작하게 하는 것이 일반적입니다.
    // 아래 코드는 PxDistanceJoint의 API를 PxD6Joint에 적용하려는 시도로 보이며,
    // PxD6Joint의 setLinearLimit은 PxJointLinearLimitPair를 받습니다.
    // 여기서는 "모든 선형 축이 Locked"인 래그돌 기본 설정을 따른다고 가정하고,
    // 만약 특정 축을 Limited로 하고 싶다면 해당 축에 대한 Drive를 설정하는 방식으로 접근하는 것을 권장.
    // 아래 PxJointLinearLimit 사용 부분은 PxD6Joint API와 맞지 않을 가능성이 높으므로 주석 처리 또는 수정 필요.

    /*
    // 이 부분은 PxD6Joint API와 맞는지 확인 필요.
    // PxD6Joint는 setLinearLimit(PxJointLinearLimitPair)를 사용하거나, 각 축을 개별적으로 제한하지 않음.
    if (ProfileInstance.LinearXMotion == EConstraintMotion::Limited ||
        ProfileInstance.LinearYMotion == EConstraintMotion::Limited ||
        ProfileInstance.LinearZMotion == EConstraintMotion::Limited)
    {
        if (ProfileInstance.bLinearSoftConstraint && (ProfileInstance.LinearStiffness > 0.f || ProfileInstance.LinearDamping > 0.f))
        {
            // contactDist는 PxJointLinearLimit 생성자에 직접 들어가지 않고, restitution, bounceThreshold와 함께 설정됨.
            // PxJointLinearLimit은 스프링을 직접 받지 않음. PxJointLinearLimitPair가 스프링을 받음.
            // PxReal contactDist = 0.05f; // 예시 값
            physx::PxSpring LinSpring(ProfileInstance.LinearStiffness, ProfileInstance.LinearDamping);
            physx::PxJointLinearLimitPair SoftLinLimit(
                PxGetPhysics().getTolerancesScale(), // PxPhysics 객체에서 가져와야 함
                -ProfileInstance.LinearLimitValue, // Lower
                ProfileInstance.LinearLimitValue,  // Upper
                LinSpring
            );
            SoftLinLimit.restitution = 0.0f; // 필요시 ProfileInstance에서
            SoftLinLimit.bounceThreshold = 0.0f; // 필요시 ProfileInstance에서
            // JointD6->setLinearLimit(SoftLinLimit); // 이 API는 PxD6Joint에 없음.
                                                  // PxD6Joint는 각 축별로 setMotion 후,
                                                  // 만약 모든 축이 동일한 제한이면 setDistanceLimit (없음)
                                                  // 또는 각 축별로 drive를 설정하여 제한된 움직임 흉내
        }
        else
        {
            // 하드 제한 (스프링 없음)
            physx::PxJointLinearLimitPair HardLinLimit(
                PxGetPhysics().getTolerancesScale(),
                -ProfileInstance.LinearLimitValue,
                ProfileInstance.LinearLimitValue
            );
            // JointD6->setLinearLimit(HardLinLimit); // 위와 동일한 문제
        }
    }
    */
    // 래그돌의 경우 보통 선형은 Locked이므로, 위 LinearLimit 설정은 주석 처리하거나 삭제하고
    // setMotion(..., PxD6Motion::eLOCKED) 만으로 충분할 수 있습니다.

    // --- 각도 제한 (Angular Limits) ---

    // Twist Limit (X-axis)
    // --- 각도 제한 (테스트를 위해 모두 eFREE로 설정) ---
    JointD6->setMotion(physx::PxD6Axis::eTWIST, physx::PxD6Motion::eFREE);
    JointD6->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eFREE);
    JointD6->setMotion(physx::PxD6Axis::eSWING2, physx::PxD6Motion::eFREE);

    //////JointD6->setMotion(physx::PxD6Axis::eTWIST, ConvertMotion(ProfileInstance.AngularTwistMotion));
    if (ProfileInstance.AngularTwistMotion == EConstraintMotion::Limited)
    {
        physx::PxReal TwistRad = physx::PxPi * ProfileInstance.TwistLimitAngle / 180.0f;
        if (ProfileInstance.bTwistSoftConstraint && (ProfileInstance.AngularStiffness > 0.f || ProfileInstance.AngularDamping > 0.f))
        {
            physx::PxSpring Spring(ProfileInstance.AngularStiffness, ProfileInstance.AngularDamping);
            JointD6->setTwistLimit(physx::PxJointAngularLimitPair(-TwistRad, TwistRad, Spring));
        }
        else
        {
            // 하드 제한 (스프링 없음)
            JointD6->setTwistLimit(physx::PxJointAngularLimitPair(-TwistRad, TwistRad));
        }
    }

    //// Swing Limits (Y and Z axes, forming a cone or pyramid)
    //// Swing1은 보통 Y축, Swing2는 Z축에 대한 제한을 의미 (PhysX 기준)
    //JointD6->setMotion(physx::PxD6Axis::eSWING1, ConvertMotion(ProfileInstance.AngularSwing1Motion));
    //JointD6->setMotion(physx::PxD6Axis::eSWING2, ConvertMotion(ProfileInstance.AngularSwing2Motion));

    // 두 스윙 축 중 하나라도 Limited이면 SwingLimit 설정 시도
    if (ProfileInstance.AngularSwing1Motion == EConstraintMotion::Limited ||
        ProfileInstance.AngularSwing2Motion == EConstraintMotion::Limited)
    {
        physx::PxReal Swing1Rad = physx::PxPi * ProfileInstance.Swing1LimitAngle / 180.0f;
        physx::PxReal Swing2Rad = physx::PxPi * ProfileInstance.Swing2LimitAngle / 180.0f;

        // 두 스윙 축이 모두 Limited일 때만 PxJointLimitCone을 사용하는 것이 일반적.
        // 만약 한 축만 Limited이고 다른 축은 Free라면, PxJointLimitCone은 부적절할 수 있음.
        // 이 경우, 해당 축만 Limited로 두고 다른 축은 Free로 두는 것이 맞음.
        // (위에서 setMotion으로 이미 각 축별 모드를 설정했으므로, 여기서는 Limit 값만 설정)

        // bSwing1SoftConstraint 또는 bSwing2SoftConstraint 중 하나라도 true이고, 유효한 스프링 값이 있다면 소프트 제한 사용
        bool bUseSoftSwing = (ProfileInstance.bSwing1SoftConstraint || ProfileInstance.bSwing2SoftConstraint) &&
            (ProfileInstance.AngularStiffness > 0.f || ProfileInstance.AngularDamping > 0.f);

        if (bUseSoftSwing)
        {
            physx::PxSpring Spring(ProfileInstance.AngularStiffness, ProfileInstance.AngularDamping);
            JointD6->setSwingLimit(physx::PxJointLimitCone(Swing1Rad, Swing2Rad, Spring));
        }
        else
        {
            // 하드 제한 (스프링 없음)
            JointD6->setSwingLimit(physx::PxJointLimitCone(Swing1Rad, Swing2Rad));
        }
    }
}


void FConstraintInstance::ApplyDrive()
{
}

void FConstraintInstance::ApplyProjection()
{

}

float FConstraintInstance::GetCurrentSwing1() const
{
    return 0.0f;
}

float FConstraintInstance::GetCurrentSwing2() const
{
    return 0.0f;
}

float FConstraintInstance::GetCurrentTwist() const
{
    return 0.0f;
}

bool FConstraintInstance::IsBroken() const
{
    return false;
}

void FConstraintInstance::ClearBroken()
{
}

FBodyInstance* FConstraintInstance::GetBody1() const
{
    return nullptr;
}

FBodyInstance* FConstraintInstance::GetBody2() const
{
    return nullptr;
}

FName FConstraintInstance::GetRefBoneName(int32 BoneIndex) const
{
    return FName();
}

FTransform FConstraintInstance::GetRefFrame(int32 BoneIndex) const
{
    return FTransform();
}

void FConstraintInstance::SetLinearLimits_AssumesLocked()
{
}

void FConstraintInstance::SetAngularLimits_AssumesLocked()
{
}
