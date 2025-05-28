#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Physics/ConstraintProfileProperties.h"

class UPhysicsConstraintTemplate : public UObject
{
    DECLARE_CLASS(UPhysicsConstraintTemplate, UObject);

public:
    UPhysicsConstraintTemplate();
    
public:
    // 각 Constraint 이름
    UPROPERTY(EditAnywhere, FName, JointName, = NAME_None)

    // 연결된 두 Bone 이름
    UPROPERTY(EditAnywhere, FName, ConstraintBone1, = NAME_None)
    UPROPERTY(EditAnywhere, FName, ConstraintBone2, = NAME_None)

    UPROPERTY(EditAnywhere, float, TwistLimit, =0.f)
    UPROPERTY(EditAnywhere, float, SwingLimit1, =0.f)
    UPROPERTY(EditAnywhere, float, SwingLimit2, =0.f)



/* TODO : Vehicle, Ragdoll 같은 프로필 관련 구현 필요.*/
public:
    

    // 기본 ConstarintInstance (기본값으로 Ball and Socket Joint 세팅) 
    //UPROPERTY(EditAnywhere, Joint, DefaultInstance, =nullptr)


    // 여러 Profiel(Ragdoll, Vehilce 등) 저장 가능
    //UPROPERTY(EditAnywhere, TArray<FMyConstraintProfileHandle>, PhysicalAnimationProfiles, = {})

    // 기본 Profile (Profile이 지정되지 않았을 때 사용)
    // UPROPERTY(EditAnywhere, FConstraintProfileProperties, DefaultProfile, = nullptr)
public:
    //// 기본 ConstraintInstance (기본값으로 Ball and Socket Joint 세팅)
    //UPROPERTY(EditAnywhere, Category = Joint, meta = (ShowOnlyInnerProperties))
    //FConstraintInstance DefaultInstance;

    //// 여러 Profile (Ragdoll, Vehicle 등) 저장 가능
    //UPROPERTY()
    //TArray<FMyConstraintProfileHandle> ProfileHandles;

    //// 기본 Profile (Profile이 지정되지 않았을 때 사용)
    //UPROPERTY(transient)
    //FConstraintProfileProperties DefaultProfile;
    FConstraintProfileProperties DefaultProfile;
public:
    // 특정 Profile 존재 여부 확인
    //bool ContainsConstraintProfile(FName ProfileName) const
    //{
    //    return ProfileHandles.ContainsByPredicate([&](const FMyConstraintProfileHandle& Handle)
    //        {
    //            return Handle.ProfileName == ProfileName;
    //        });
    //}

    //// Profile 추가
    //void AddConstraintProfile(FName ProfileName)
    //{
    //    FMyConstraintProfileHandle& NewHandle = ProfileHandles.AddDefaulted_GetRef();
    //    NewHandle.ProfileName = ProfileName;
    //    NewHandle.ProfileProperties = DefaultInstance.ProfileInstance; // 현재 세팅 복사
    //}

    //// Profile 삭제
    //void RemoveConstraintProfile(FName ProfileName)
    //{
    //    ProfileHandles.RemoveAll([&](const FMyConstraintProfileHandle& Handle)
    //        {
    //            return Handle.ProfileName == ProfileName;
    //        });
    //}

    //// Profile을 ConstraintInstance에 적용
    //void ApplyConstraintProfile(FName ProfileName, FConstraintInstance& OutInstance, bool bUseDefaultIfNotFound = true) const
    //{
    //    bool bFound = false;

    //    for (const FMyConstraintProfileHandle& Handle : ProfileHandles)
    //    {
    //        if (Handle.ProfileName == ProfileName)
    //        {
    //            OutInstance.CopyProfilePropertiesFrom(Handle.ProfileProperties);
    //            bFound = true;
    //            break;
    //        }
    //    }

    //    if (!bFound && bUseDefaultIfNotFound)
    //    {
    //        OutInstance.CopyProfilePropertiesFrom(DefaultProfile);
    //    }
    //}

    //// Profile 속성 조회 (없으면 DefaultProfile 반환)
    //const FConstraintProfileProperties& GetConstraintProfilePropertiesOrDefault(FName ProfileName) const
    //{
    //    for (const FMyConstraintProfileHandle& Handle : ProfileHandles)
    //    {
    //        if (Handle.ProfileName == ProfileName)
    //        {
    //            return Handle.ProfileProperties;
    //        }
    //    }
    //    return DefaultProfile;
    //}
};