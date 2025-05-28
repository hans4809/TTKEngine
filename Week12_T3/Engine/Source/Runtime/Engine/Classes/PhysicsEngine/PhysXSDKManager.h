// FPhysXSDKManager.h
#pragma once

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxAllocatorCallback.h>
#include <PxErrorCallback.h>

#include <PxDefaultAllocator.h>
#include <PxDefaultErrorCallback.h>

#include "snippets/snippetvehiclecommon/SnippetVehicleCreate.h"

namespace snippetvehicle
{
    class VehicleSceneQueryData;
}

namespace physx
{
    class PxVehicleDrivableSurfaceToTireFrictionPairs;
    class PxPvdTransport;
    class PxCooking;
};
class UPhysicalMaterial;
class FPhysXSDKManager
{
public:
    FPhysXSDKManager();
    ~FPhysXSDKManager();

    FPhysXSDKManager(const FPhysXSDKManager&) = delete;
    FPhysXSDKManager& operator=(const FPhysXSDKManager&) = delete;
    FPhysXSDKManager(FPhysXSDKManager&&) = delete;
    FPhysXSDKManager& operator=(FPhysXSDKManager&&) = delete;

    static FPhysXSDKManager& GetInstance()
    {
        static FPhysXSDKManager Inst;
        return Inst;
    }
    UPhysicalMaterial* GetDefaultMaterial();
    static snippetvehicle::VehicleDesc InitVehicleDesc(physx::PxMaterial* InMaterial);
    bool Initalize();
    void Shutdown();
    physx::PxCooking* GetCooking() const { return PxCookingInstance; }
    physx::PxPhysics* GetPhysicsSDK();
public:
    physx::PxDefaultAllocator DefaultAllocatorCallback;
    physx::PxDefaultErrorCallback DefaultErrorCallback;
    physx::PxPvd* Pvd;
    physx::PxPvdTransport* Transport;
    physx::PxFoundation* PxFoundationInstance;
    physx::PxPhysics* PxSDKInstance;

    physx::PxCooking* PxCookingInstance;
private:

    bool bIsInitialized = false;
    UPhysicalMaterial* DefaultPhysicalMaterial = nullptr;
};

// 전역 변수 extern 선언
extern physx::PxF32 ThresholdForwardSpeedForWheelAngleIntegration;
extern physx::PxF32 RecipThresholdForwardSpeedForWheelAngleIntegration;
extern physx::PxF32 MinLatSpeedForTireModel;
extern physx::PxF32 StickyTireFrictionThresholdSpeed;
extern physx::PxF32 ToleranceScaleLength;
extern physx::PxF32 MinimumSlipThreshold;

extern physx::PxVec3 RightDefault;
extern physx::PxVec3 UpDefault;
extern physx::PxVec3 ForwardDefault;
extern physx::PxVec3 Right;
extern physx::PxVec3 Up;
extern physx::PxVec3 Forw;

extern bool    ApplyForces;
extern const bool ApplyForcesDefault;

extern physx::PxF32 PointRejectAngleThreshold;
extern const physx::PxF32 PointRejectAngleThresholdDefault;

extern physx::PxF32 NormalRejectAngleThreshold;
extern const physx::PxF32 NormalRejectAngleThresholdDefault;

extern const physx::PxF32 MaxHitActorAccelerationDefault;
extern physx::PxF32 MaxHitActorAcceleration;

extern physx::PxVehicleDrivableSurfaceToTireFrictionPairs* FrictionPairs;

extern snippetvehicle::VehicleSceneQueryData*	vehicleSceneQueryData;
extern physx::PxBatchQuery*			BatchQuery;