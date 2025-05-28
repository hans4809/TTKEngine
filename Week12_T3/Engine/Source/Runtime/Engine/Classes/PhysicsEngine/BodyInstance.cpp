#include <cassert>

#include "BodyInstance.h"
#include "PhysicsMaterial.h"
#include "PhysScene_PhysX.h"
#include "Components/PrimitiveComponents/PrimitiveComponent.h"
#include <PxPhysicsAPI.h> 
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>

#include <geometry/PxBoxGeometry.h>
#include <geometry/PxSphereGeometry.h>
#include <geometry/PxCapsuleGeometry.h>
#include <extensions/PxRigidBodyExt.h>

FBodyInstance::FBodyInstance()
    : OwnerComponent(nullptr)
    , PxPhysicsSDK(nullptr)
    , PxActor(nullptr)
    , DefaultPhysicalMaterial(nullptr)
    , CurrentBodyType(EPhysBodyType::Static)
{
}
FBodyInstance::~FBodyInstance()
{
}
void FBodyInstance::Initialize(UPrimitiveComponent* InOwnerComponent, physx::PxPhysics* InPxPhysicsSDK)
{
    OwnerComponent = InOwnerComponent;
    PxPhysicsSDK = InPxPhysicsSDK;
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

    return true;
}

void FBodyInstance::InitConstraint(FBodyInstance InBody1, FBodyInstance InBody2)
{
}

void FBodyInstance::ReleasePhysicsState()
{
    if (PxActor)
    {
        PxActor->release();
        PxActor = nullptr;
    }
}

void FBodyInstance::AddObject(FPhysScene* PhysScene)
{
    PhysScene->AddObject(this);
}
physx::PxShape* FBodyInstance::AddBoxGeometry(const FVector& HalfExtents, UPhysicalMaterial* Material, const FTransform& Transform)
{
    if (!PxActor || !PxPhysicsSDK || !Material || !Material->GetPxMaterial())
    {
        if (!PxActor)
            UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: Invalid PxActor."));
        if (!PxPhysicsSDK)
            UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: Invalid SDK."));
        if (!Material)
            UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: Invalid Material"));
        if (!Material->GetPxMaterial())
            UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: Invalid PxMaterial."));
        return nullptr;
    }

    physx::PxBoxGeometry BoxGeom(HalfExtents.ToPxVec3());
    physx::PxShapeFlags shapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE);
    physx::PxShape* NewShape = PxPhysicsSDK->createShape(BoxGeom, *Material->GetPxMaterial(), true, shapeFlags);

    if (NewShape)
    {
        NewShape->setLocalPose(Transform.ToPxTransform());
        physx::PxFilterData filterData;
        filterData.word0 = 1;   // 예: 그룹 1
        filterData.word1 = 0xFFFFFFFF; // 모든 그룹과 충돌
        NewShape->setSimulationFilterData(filterData);
        PxActor->attachShape(*NewShape);
        NewShape->release();

        UpdateMassAndInertia(10);
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddBoxGeometry failed: PxShape creation failed."));
    }
    return NewShape;
}

physx::PxShape* FBodyInstance::AddSphereGeometry(float Radius, UPhysicalMaterial* Material, const FTransform& Transform)
{
    if (!PxActor || !PxPhysicsSDK || !Material || !Material->GetPxMaterial())
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddSphereGeometry failed: Invalid PxActor, SDK, Material, or PxMaterial."));
        return nullptr;
    }

    physx::PxSphereGeometry SphereGeom(Radius);
    physx::PxShapeFlags shapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE);
    physx::PxShape* NewShape = PxPhysicsSDK->createShape(SphereGeom, *Material->GetPxMaterial(), true, shapeFlags);

    if (NewShape)
    {
        NewShape->setLocalPose(Transform.ToPxTransform());
        physx::PxFilterData filterData;
        filterData.word0 = 1;   // 예: 그룹 1
        filterData.word1 = 0xFFFFFFFF; // 모든 그룹과 충돌
        NewShape->setSimulationFilterData(filterData);
        PxActor->attachShape(*NewShape);
        NewShape->release();

        UpdateMassAndInertia(10);
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddSphereGeometry failed: PxShape creation failed."));
    }
    return NewShape;
}

physx::PxShape* FBodyInstance::AddCapsuleGeometry(float Radius, float HalfHeight, UPhysicalMaterial* Material, const FTransform& Transform)
{
    if (!PxActor || !PxPhysicsSDK || !Material || !Material->GetPxMaterial())
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddCapsuleGeometry failed: Invalid PxActor, SDK, Material, or PxMaterial."));
        return nullptr;
    }

    physx::PxCapsuleGeometry CapsuleGeom(Radius, HalfHeight);
    physx::PxShapeFlags shapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE);
    physx::PxShape* NewShape = PxPhysicsSDK->createShape(CapsuleGeom, *Material->GetPxMaterial(), true, shapeFlags);

    if (NewShape)
    {
        FQuat PhysXCapsuleAxisAlignmentFix = FQuat(FVector::YAxisVector, FMath::DegreesToRadians(-90.0f));
        FTransform FinalShapeLocalPxPose = Transform;
        FinalShapeLocalPxPose.SetRotation(Transform.GetRotation() * PhysXCapsuleAxisAlignmentFix);
        physx::PxTransform ShapeGlobal = FinalShapeLocalPxPose.ToPxTransform();


        NewShape->setLocalPose(ShapeGlobal);
        physx::PxFilterData filterData;
        filterData.word0 = 1;   // 예: 그룹 1
        filterData.word1 = 2;//0xFFFFFFFF; // 모든 그룹과 충돌
        NewShape->setSimulationFilterData(filterData);
        PxActor->attachShape(*NewShape);
        NewShape->release();
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddCapsuleGeometry failed: PxShape creation failed."));
    }
    return NewShape;
}

physx::PxShape* FBodyInstance::AddConvexGeometry(physx::PxConvexMesh* CookedMesh, UPhysicalMaterial* Material, const FTransform& Transform, const FVector& Scale)
{
    if (!PxActor || !PxPhysicsSDK || !CookedMesh)
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddConvexGeometry failed: Invalid PxActor, SDK, or CookedMesh."));
        return nullptr;
    }

    // 컨벡스 메시에 대한 스케일링은 PxMeshScale을 사용
    physx::PxMeshScale MeshScale(Scale.ToPxVec3());
    physx::PxConvexMeshGeometry ConvexGeom(CookedMesh, MeshScale);

    physx::PxShapeFlags shapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE);
    physx::PxShape* NewShape = PxPhysicsSDK->createShape(ConvexGeom, *Material->GetPxMaterial(), true, shapeFlags);

    if (NewShape)
    {
        NewShape->setLocalPose(Transform.ToPxTransform());
        PxActor->attachShape(*NewShape);
        NewShape->release();
    }
    else
    {
        UE_LOG(LogLevel::Warning, TEXT("FBodyInstance::AddConvexGeometry failed: PxShape creation failed."));
    }
    return NewShape;
}


void FBodyInstance::SetBodyType(EPhysBodyType NewType)
{
}

void FBodyInstance::SetMass(float Mass)
{
}

void FBodyInstance::UpdateMassAndInertia(float DensityOrMass, const FVector* MassLocalPose)
{
    if (PxActor && CurrentBodyType == EPhysBodyType::Dynamic)
    {
        physx::PxRigidDynamic* DynActor = static_cast<physx::PxRigidDynamic*>(PxActor);
        if (DynActor)
        {
            // 밀도를 사용하여 질량과 관성 업데이트
            physx::PxRigidBodyExt::updateMassAndInertia(*DynActor, DensityOrMass);

            // 또는 고정 질량을 사용한다면 (그리고 관성 텐서를 수동으로 계산하거나 기본값 사용)
            // DynActor->setMass(DensityOrMass); 
            // PxRigidBodyExt::setMassAndUpdateInertia(*DynActor, DensityOrMass); // 질량을 설정하고 관성도 업데이트
        }
    }
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
    if (PxActor)
    {
        physx::PxTransform PxPose = PxActor->getGlobalPose();
        
        FQuat Rotation = FQuat::PToFQuat(PxPose.q); 
        
        Rotation.Normalize();
        
        return FTransform(Rotation, FVector::PToFVector(PxPose.p), FVector::OneVector);
    }

    return FTransform::Identity; // PxActor가 없으면 단위 행렬 반환

}

void FBodyInstance::SetGlobalPose(const FTransform& NewTransform, bool bAutoWake)
{
}

void FBodyInstance::ReleaseShapes()
{
}
