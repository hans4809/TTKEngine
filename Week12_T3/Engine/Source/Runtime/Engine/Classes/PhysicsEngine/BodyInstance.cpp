#include <cassert>

#include "BodyInstance.h"
#include "PhysicsMaterial.h"
#include "Components/PrimitiveComponents/PrimitiveComponent.h"
#include <PxPhysicsAPI.h> 
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>

#include <geometry/PxBoxGeometry.h>
#include <geometry/PxSphereGeometry.h>
#include <geometry/PxCapsuleGeometry.h>
#include <extensions/PxRigidBodyExt.h>

FBodyInstance::FBodyInstance(UPrimitiveComponent* InOwnerComponent, physx::PxPhysics* InPxPhysicsSDK)
    : OwnerComponent(InOwnerComponent)
    , PxPhysicsSDK(InPxPhysicsSDK)
    , PxActor(nullptr)
    , CurrentBodyType(EPhysBodyType::Static)
{
    // OwnerComponent와 PxPhysicsSDK는 유효해야 함
    assert(OwnerComponent != nullptr && "OwnerComponent cannot be null in FBodyInstance constructor");
    assert(PxPhysicsSDK != nullptr && "PxPhysicsSDK cannot be null in FBodyInstance constructor");
}
bool FBodyInstance::CreatePhysicsState(const FTransform& InitialTransform, EPhysBodyType BodyType)
{
    if (PxActor)
    {
        ReleasePhysicsState();
    }
    if (!OwnerComponent || !PxPhysicsSDK)
    {
        UE_LOG(LogLevel::Error, TEXT("FBodyInstance::CreatePhysicsState failed: OwnerComponent or PxPhysicsSDK is null."));
        return false;
    }
    CurrentBodyType = BodyType;

    physx::PxTransform InitialPxTransform = InitialTransform.ToPxTransform();

    if (CurrentBodyType == EPhysBodyType::Static)
    {
        PxActor = PxPhysicsSDK->createRigidStatic(InitialPxTransform);
    }
    else
    {
        // Dynamic 또는 Kinematic
        PxActor = PxPhysicsSDK->createRigidDynamic(InitialPxTransform);
        if (PxActor && CurrentBodyType == EPhysBodyType::Kinematic)
        {
            static_cast<physx::PxRigidDynamic*>(PxActor)->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
        }
    }

    if (!PxActor)
    {
        UE_LOG(LogLevel::Error, TEXT("FBodyInstance::CreatePhysicsState failed: PxActor creation failed."));
        return false;
    }

    // userData 설정: PhysX 콜백 등에서 이 FBodyInstance 또는 OwnerComponent를 다시 찾아올 수 있도록 함
    PxActor->userData = this;

    return false;
}

void FBodyInstance::ReleasePhysicsState()
{
    if (PxActor)
    {
        PxActor->release();
        PxActor = nullptr;
    }
}

physx::PxShape* FBodyInstance::AddBoxGeometry(const FVector& HalfExtents, FPhysicsMaterial* Material, const FTransform& LocalPose)
{
    if (!PxActor || !PxPhysicsSDK || !Material || !Material->GetPxMaterial())
    {

        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: Invalid PxActor, SDK, Material, or PxMaterial."));
        return nullptr;
    }

    physx::PxBoxGeometry BoxGeom(HalfExtents.ToPxVec3());

    // 셰이프 생성 시 PxShapeFlag::eEXCLUSIVE_SHAPE (true)를 사용하면 액터 해제 시 같이 해제됨
    physx::PxShape* NewShape = PxPhysicsSDK->createShape(BoxGeom, *(Material->GetPxMaterial()), true);

    if (NewShape)
    {
        NewShape->setLocalPose(LocalPose.ToPxTransform());
        PxActor->attachShape(*NewShape);
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: PxShape creation failed."));
    }
    return NewShape;
}

physx::PxShape* FBodyInstance::AddSphereGeometry(float Radius, FPhysicsMaterial* Material, const FTransform& LocalPose)
{
    return nullptr;
}

physx::PxShape* FBodyInstance::AddCapsuleGeometry(float Radius, float HalfHeight, FPhysicsMaterial* Material, const FTransform& LocalPose)
{
    return nullptr;
}

void FBodyInstance::SetBodyType(EPhysBodyType NewType)
{
}

void FBodyInstance::SetMass(float Mass)
{
}

void FBodyInstance::UpdateMassAndInertia(float DensityOrMass, const FVector* MassLocalPose)
{
}

void FBodyInstance::SetLinearVelocity(const FVector& Velocity)
{
}

FVector FBodyInstance::GetLinearVelocity() const
{
    return FVector();
}

void FBodyInstance::SetAngularVelocity(const FVector& AngVelocity)
{
}

FVector FBodyInstance::GetAngularVelocity() const
{
    return FVector();
}

void FBodyInstance::AddForce(const FVector& Force, bool bWakeUp)
{
}

void FBodyInstance::AddTorque(const FVector& Torque, bool bWakeUp)
{
}

void FBodyInstance::AddImpulse(const FVector& Impulse, bool bWakeUp)
{
}

void FBodyInstance::AddAngularImpulse(const FVector& AngularImpulse, bool bWakeUp)
{
}

void FBodyInstance::SetKinematicTarget(const FTransform& TargetTransform)
{
}

void FBodyInstance::SetSimulationFilterData(const physx::PxFilterData& FilterData)
{
}

physx::PxFilterData FBodyInstance::GetSimulationFilterData() const
{
    return physx::PxFilterData();
}

FTransform FBodyInstance::GetGlobalPose() const
{
    return FTransform();
}

void FBodyInstance::SetGlobalPose(const FTransform& NewTransform, bool bAutoWake)
{
}

void FBodyInstance::ReleaseShapes()
{
}
