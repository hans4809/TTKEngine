#pragma once
#include "PxVehicleDrive4W.h"
#include "PxVehicleUtilControl.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleCreate.h"

namespace snippetvehicle
{
    class VehicleSceneQueryData;
}

namespace physx
{
    struct PxVehicleWheelConcurrentUpdateData;
    struct PxVehicleConcurrentUpdateData;
    class PxVehicleDriveSimData4W;
    class PxVehicleWheelsSimData;
    class PxVehicleDrive4W;
    class PxMaterial;
    class PxScene;
    class PxBatchQuery;
    class PxPhysics;
    class PxCooking;
}

inline physx::PxF32 SteerVsForwardSpeedData[2 * 8]=
{
    0.0f,		0.75f,
    5.0f,		0.75f,
    30.0f,		0.125f,
    120.0f,		0.1f,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32
};

inline physx::PxFixedSizeLookupTable<8> SteerVsForwardSpeedTable(SteerVsForwardSpeedData,4);

inline physx::PxVehicleKeySmoothingData KeySmoothingData =
{
    {
        6.0f,	//rise rate eANALOG_INPUT_ACCEL
        6.0f,	//rise rate eANALOG_INPUT_BRAKE		
        6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
        2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        10.0f,	//fall rate eANALOG_INPUT_ACCEL
        10.0f,	//fall rate eANALOG_INPUT_BRAKE		
        10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
        5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
        5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

inline physx::PxVehiclePadSmoothingData PadSmoothingData =
{
    {
        6.0f,	//rise rate eANALOG_INPUT_ACCEL
        6.0f,	//rise rate eANALOG_INPUT_BRAKE		
        6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
        2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        10.0f,	//fall rate eANALOG_INPUT_ACCEL
        10.0f,	//fall rate eANALOG_INPUT_BRAKE		
        10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
        5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
        5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

inline physx::PxVehicleDrive4WRawInputData VehicleInputData;

enum class EDriveMode
{
    eDRIVE_MODE_ACCEL_FORWARDS = 0,
    eDRIVE_MODE_ACCEL_REVERSE,
    eDRIVE_MODE_HARD_TURN_LEFT,
    eDRIVE_MODE_HANDBRAKE_TURN_LEFT,
    eDRIVE_MODE_HARD_TURN_RIGHT,
    eDRIVE_MODE_HANDBRAKE_TURN_RIGHT,
    eDRIVE_MODE_BRAKE,
    eDRIVE_MODE_NONE
};

inline EDriveMode DriveModeOrder[] =
{
    EDriveMode::eDRIVE_MODE_BRAKE,
    EDriveMode::eDRIVE_MODE_ACCEL_FORWARDS,
    EDriveMode::eDRIVE_MODE_BRAKE,
    EDriveMode::eDRIVE_MODE_ACCEL_REVERSE,
    EDriveMode::eDRIVE_MODE_BRAKE,
    EDriveMode::eDRIVE_MODE_HARD_TURN_LEFT,
    EDriveMode::eDRIVE_MODE_BRAKE,
    EDriveMode::eDRIVE_MODE_HARD_TURN_RIGHT,
    EDriveMode::eDRIVE_MODE_ACCEL_FORWARDS,
    EDriveMode::eDRIVE_MODE_HANDBRAKE_TURN_LEFT,
    EDriveMode::eDRIVE_MODE_ACCEL_FORWARDS,
    EDriveMode::eDRIVE_MODE_HANDBRAKE_TURN_RIGHT,
    EDriveMode::eDRIVE_MODE_NONE
};

inline physx::PxF32					VehicleModeLifetime = 4.0f;
inline physx::PxF32					VehicleModeTimer = 0.0f;
inline physx::PxU32					VehicleOrderProgress = 0;
inline bool					VehicleOrderComplete = false;
inline bool					MimicKeyInputs = false;

inline bool					IsVehicleInAir = true;

class FVehicle4W
{
public:
    FVehicle4W();
    ~FVehicle4W();

    // 호출 순서: Initialize -> 매 프레임 Update -> Release
    void Initialize(const snippetvehicle::VehicleDesc& vehicle4WDesc, physx::PxPhysics* physics, physx::PxCooking* cooking, const physx::
                    PxTransform& startPose = physx::PxTransform(physx::PxIdentity));

    static void StartAccelerateForwardsMode();
    void StartAccelerateReverseMode() const;
    static void StartBrakeMode();
    static void StartTurnHardLeftMode();
    static void StartTurnHardRightMode();
    static void StartHandbrakeTurnLeftMode();
    static void StartHandbrakeTurnRightMode();
    void IncrementDrivingMode(float deltaTime);

    void ReleaseAllControls();
    void Update(float deltaTime);

    // 디버그 렌더링 등 필요 시
    physx::PxVehicleDrive4W* GetVehicle() const { return Vehicle; }

private:
    physx::PxVehicleDrive4W* CreateVehicle4W(const snippetvehicle::VehicleDesc& vehDesc, physx::PxPhysics* physics, physx::PxCooking* cooking);
    
private:
    // Vehicle Core
    physx::PxVehicleDrive4W*         Vehicle              = nullptr;
    physx::PxVehicleWheelsSimData*   WheelsSimData        = nullptr;
    physx::PxVehicleDriveSimData4W*  DriveSimData         = nullptr;

    // 마찰 테이블
    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* mFrictionPairs = nullptr;
};
