#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Math/Transform.h"
#include "Math/Quat.h"
#include "Math/Vector.h"

namespace physx 
{
    class PxRigidActor;
    class PxShape;
    class PxPhysics;
    class PxMaterial;
    class PxFilterData;
    class PxConvexMesh;
}
class UPrimitiveComponent;
class UPhysicalMaterial;

enum class EPhysBodyType 
{
    Static,
    Dynamic,
    Kinematic
};
class FPhysScene;
class FBodyInstance
{
public:
    FBodyInstance();
    ~FBodyInstance();
public:
    void Initialize(UPrimitiveComponent* InOwnerComponent, physx::PxPhysics* InPxPhysicsSDK);
   //UPROPERTY(VisibleAnywhere, USceneComponent*, RootComponent, = nullptr)
    bool CreatePhysicsState(const FTransform& InitialTransform, EPhysBodyType BodyType);
    void InitConstraint(FBodyInstance InBody1, FBodyInstance InBody2);
    void ReleasePhysicsState();
    void AddObject(FPhysScene* PhysScene);
    physx::PxShape* AddBoxGeometry(const FVector& HalfExtents, UPhysicalMaterial* Material, const FTransform& Transform = FTransform::Identity);
    physx::PxShape* AddSphereGeometry(float Radius, UPhysicalMaterial* Material, const FTransform& Transform = FTransform::Identity);
    physx::PxShape* AddCapsuleGeometry(float Radius, float HalfHeight, UPhysicalMaterial* Material, const FTransform& Transform = FTransform::Identity);
    physx::PxShape* AddConvexGeometry(physx::PxConvexMesh* CookedMesh, UPhysicalMaterial* Material, const FTransform& Transform, const FVector& Scale);
    // 물리 속성 설정 및 조회
    void SetBodyType(EPhysBodyType NewType);
    EPhysBodyType GetBodyType() const { return CurrentBodyType; }

    void SetMass(float Mass);
    void UpdateMassAndInertia(float DensityOrMass, const FVector* MassLocalPose = nullptr);

    void SetLinearVelocity(const FVector& Velocity);
    
    FVector GetLinearVelocity() const;
    
    void SetAngularVelocity(const FVector& AngVelocity);
    
    FVector GetAngularVelocity() const;

    void AddForce(const FVector& Force, bool bWakeUp = true);
    void AddTorque(const FVector& Torque, bool bWakeUp = true);
    void AddImpulse(const FVector& Impulse, bool bWakeUp = true);
    void AddAngularImpulse(const FVector& AngularImpulse, bool bWakeUp = true);

    void SetKinematicTarget(const FTransform& TargetTransform);

    void SetSimulationFilterData(const physx::PxFilterData& FilterData);
    physx::PxFilterData GetSimulationFilterData() const;

    // 트랜스폼
    FTransform GetGlobalPose() const;
    void SetGlobalPose(const FTransform& NewTransform, bool bAutoWake = true);

    // 내부 PhysX 객체 접근자
    physx::PxRigidActor* GetPxRigidActor() const { return PxActor; }

    // 소유자 컴포넌트 접근자
    UPrimitiveComponent* GetOwnerComponent() const { return OwnerComponent; }

    bool IsPhysicsStateCreated() const { return PxActor != nullptr; }
    FName AssociatedBoneName;
private:
    UPrimitiveComponent* OwnerComponent;   // 이 FBodyInstance를 소유하는 UPrimitiveComponent
    physx::PxPhysics* PxPhysicsSDK;
    physx::PxRigidActor* PxActor;
    EPhysBodyType CurrentBodyType;
    physx::PxMaterial* DefaultPhysicalMaterial;
    void ReleaseShapes();
};