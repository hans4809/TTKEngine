#include "PhysXSDKManager.h"
#include "UserInterface/Console.h"
#include "PhysicsEngine/PhysicsMaterial.h"

#include "UObject/ObjectFactory.h"

#include <PxTolerancesScale.h>
#include <PxPhysicsAPI.h>

#include "snippets/snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleSceneQuery.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleTireFriction.h"

using namespace physx;
using namespace snippetvehicle;

PxF32 ThresholdForwardSpeedForWheelAngleIntegration = 0;
PxF32 RecipThresholdForwardSpeedForWheelAngleIntegration = 0;
PxF32 MinLatSpeedForTireModel = 0;
PxF32 StickyTireFrictionThresholdSpeed = 0;
PxF32 ToleranceScaleLength = 0;
PxF32 MinimumSlipThreshold = 0;

PxVec3 RightDefault(1.f,0,0);
PxVec3 UpDefault(0,1.f,0);
PxVec3 ForwardDefault(0,0,1.f);
PxVec3 Right;
PxVec3 Up;
PxVec3 Forw;

bool ApplyForces;
const bool ApplyForcesDefault = false;

PxF32 PointRejectAngleThreshold;
const PxF32 PointRejectAngleThresholdDefault = 0.707f;  //PxCos(PxPi*0.25f);

PxF32 NormalRejectAngleThreshold;
const PxF32 NormalRejectAngleThresholdDefault = 0.707f;  //PxCos(PxPi*0.25f);

const PxF32 MaxHitActorAccelerationDefault = PX_MAX_REAL;
PxF32 MaxHitActorAcceleration;

PxVehicleDrivableSurfaceToTireFrictionPairs* FrictionPairs = nullptr;

VehicleSceneQueryData*	vehicleSceneQueryData = nullptr;

PxBatchQuery*			BatchQuery = nullptr;

//Tire model friction for each combination of drivable surface type and tire type.
PxF32 TireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES] =
{
    //NORMAL,	WORN
    {1.00f,		0.1f}//TARMAC
};

void SetVehicleToleranceScale(const PxTolerancesScale& ts)
{
    ThresholdForwardSpeedForWheelAngleIntegration=5.0f*ts.length;
    RecipThresholdForwardSpeedForWheelAngleIntegration= 1.0f / ThresholdForwardSpeedForWheelAngleIntegration;

    MinLatSpeedForTireModel=1.0f*ts.length;

    StickyTireFrictionThresholdSpeed=0.2f*ts.length;

    ToleranceScaleLength=ts.length;

    MinimumSlipThreshold = 1e-5f;
}

void SetVehicleDefaults()
{
    Right = RightDefault;
    Up = UpDefault;
    Forw = ForwardDefault;

    ApplyForces = ApplyForcesDefault;

    PointRejectAngleThreshold = PointRejectAngleThresholdDefault;
    NormalRejectAngleThreshold = NormalRejectAngleThresholdDefault;

    MaxHitActorAcceleration = MaxHitActorAccelerationDefault;
}

void VehicleSetBasisVectors(const PxVec3& up, const PxVec3& forward)
{
    Right = up.cross(forward);
    Up = up;
    Forw = forward;
}

void VehicleSetUpdateMode(PxVehicleUpdateMode::Enum vehicleUpdateMode)
{
    switch(vehicleUpdateMode)
    {
    case PxVehicleUpdateMode::eVELOCITY_CHANGE:
        ApplyForces = false;
        break;
    case PxVehicleUpdateMode::eACCELERATION:
        ApplyForces = true;
        break;
    }
}

VehicleDesc FPhysXSDKManager::InitVehicleDesc(physx::PxMaterial* InMaterial, const FVector InChassisDims)
{
    // 1) 차체 파라미터
    const PxF32 chassisMass = 60.0f;
    const PxF32 length      = InChassisDims.X;
    const PxF32 width       = InChassisDims.Y;
    const PxF32 height      = InChassisDims.Z;

    const PxVec3 chassisDims = PxVec3(width, length, height);

    const PxVec3 chassisMOI
        ((chassisDims.y*chassisDims.y + chassisDims.z * chassisDims.z) * chassisMass / 12.0f,
         (chassisDims.x*chassisDims.x + chassisDims.z * chassisDims.z) * 0.8f * chassisMass / 12.0f,
         (chassisDims.x*chassisDims.x + chassisDims.y * chassisDims.y) * chassisMass / 12.0f);
    const PxVec3 chassisCMOffset(0.25f, 0.0f, -chassisDims.z * 0.5f + 0.65f);

    // 가운데 바퀴 빼고 4륜만 쓸 거면 이걸 4로!
    const PxU32 nbWheels = 4;
    
    // 반경: 전장 1/10
    const PxF32 wheelRadius = length * 0.1f;
    // 너비: 전폭 1/8
    const PxF32 wheelWidth  = width  * 0.125f;
    // 질량: 차체 질량 / 휠 개수 × 0.5 (임의 계수, 필요 시 튜닝)
    const PxF32 wheelMass   = (chassisMass / nbWheels) * 0.5f;
    // 관성모멘트(원통 가정)
    const PxF32 wheelMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;

    VehicleDesc vehicleDesc;

    vehicleDesc.chassisMass = chassisMass;
    vehicleDesc.chassisDims = chassisDims;
    vehicleDesc.chassisMOI = chassisMOI;
    vehicleDesc.chassisCMOffset = chassisCMOffset;
    vehicleDesc.chassisMaterial = InMaterial;
    vehicleDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

    vehicleDesc.wheelMass = wheelMass;
    vehicleDesc.wheelRadius = wheelRadius;
    vehicleDesc.wheelWidth = wheelWidth;
    vehicleDesc.wheelMOI = wheelMOI;
    vehicleDesc.numWheels = nbWheels;
    vehicleDesc.wheelMaterial = InMaterial;
    vehicleDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);

    return vehicleDesc;
}


FPhysXSDKManager::FPhysXSDKManager()
    : PxFoundationInstance(nullptr), PxSDKInstance(nullptr), Pvd(nullptr)
{
}

FPhysXSDKManager::~FPhysXSDKManager()
{
    Shutdown();
}
bool FPhysXSDKManager::Initalize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogLevel::Display, "FPhysXSDKManager: Already Initalize!");
        return true;
    }
    PxFoundationInstance = PxCreateFoundation(PX_PHYSICS_VERSION, DefaultAllocatorCallback, DefaultErrorCallback);
    if (!PxFoundationInstance)
    {
        UE_LOG(LogLevel::Error, "FPhysXSDKManager: Failed Create PxFoundation!");
        return false;
    }
    
    physx::PxTolerancesScale Scale;
    Pvd = PxCreatePvd(*PxFoundationInstance);
    Transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);

    ConnectPVD();

    PxSDKInstance = PxCreatePhysics(PX_PHYSICS_VERSION, *PxFoundationInstance, Scale, true, Pvd);
    if (!PxSDKInstance)
    {
        UE_LOG(LogLevel::Error, "FPhysXSDKManager: Failed Create PxSDKInstance!");
        PxFoundationInstance->release();
        PxFoundationInstance = nullptr;
        return false;
    }

    PxInitExtensions(*PxSDKInstance, Pvd);

    physx::PxCookingParams CookParams(Scale);
    PxCookingInstance = PxCreateCooking(PX_PHYSICS_VERSION, *PxFoundationInstance, CookParams);

    PxInitVehicleSDK(*PxSDKInstance);
    VehicleSetBasisVectors(PxVec3(0, 0, 1), PxVec3(1, 0, 0));
    VehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

    //Create the batched scene queries for the suspension raycasts.
    vehicleSceneQueryData = VehicleSceneQueryData::allocate(1, PX_MAX_NB_WHEELS, 1, 1, WheelSceneQueryPreFilterBlocking, NULL, DefaultAllocatorCallback);
    
    FrictionPairs = createFrictionPairs(GetDefaultMaterial()->GetPxMaterial());
    
    bIsInitialized = true;
    return true;
}

void FPhysXSDKManager::ConnectPVD()
{
    bool bPvdConnected = Pvd->connect(*Transport, physx::PxPvdInstrumentationFlag::eDEBUG);

}

void FPhysXSDKManager::Shutdown()
{
    if (!bIsInitialized) return;
    
    if (BatchQuery)
    {
        BatchQuery->release();
        BatchQuery = nullptr;
    }

    if (vehicleSceneQueryData)
    {
        vehicleSceneQueryData->free(DefaultAllocatorCallback);
        vehicleSceneQueryData = nullptr;
    }

    if (FrictionPairs)
    {
        FrictionPairs->release();
        FrictionPairs = nullptr;
    }

    if (PxCookingInstance)
    {
        PxCookingInstance->release();
        PxCookingInstance = nullptr;
    }

    if (PxSDKInstance)
    {
        PxSDKInstance->release();
        PxSDKInstance = nullptr;
    }

    if (PxFoundationInstance)
    {
        PxFoundationInstance->release();
        PxFoundationInstance = nullptr;
    }

    if (Transport)
    {
        Transport->release();
        Transport = nullptr;
    }

    Pvd = nullptr;
    
    bIsInitialized = false;
}

UPhysicalMaterial* FPhysXSDKManager::GetDefaultMaterial()
{
    if (DefaultPhysicalMaterial == nullptr)
    {
        DefaultPhysicalMaterial = FObjectFactory::ConstructObject<UPhysicalMaterial>(nullptr);
        DefaultPhysicalMaterial->Initialize(PxSDKInstance, 0.5f, 1.0f, 0.5f);
        // DefaultPhysicalMaterial->SetStaticFriction(0.5f);
        // DefaultPhysicalMaterial->SetRestitution(0.5f);
        // DefaultPhysicalMaterial->SetDynamicFriction(1.0f);
    }
    return DefaultPhysicalMaterial;
}

physx::PxPhysics* FPhysXSDKManager::GetPhysicsSDK()
{
    return PxSDKInstance;
}
