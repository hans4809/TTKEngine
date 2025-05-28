#include "Vehicle4W.h"

#include "snippets/snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleSceneQuery.h"
#include "snippets/snippetvehiclecommon/SnippetVehicleTireFriction.h"

using namespace physx;
using namespace snippetvehicle;

extern VehicleSceneQueryData*	vehicleSceneQueryData;
extern PxBatchQuery* BatchQuery;

// 전역 변수 정의 (VehicleGlobals.cpp 등)
PxF32 AutoDriveInterval = 3.0f;  // 예: 3초마다 모드 전환
PxF32 AutoDriveTimer    = 0.0f;

void ComputeWheelCenterActorOffsets4W(const PxF32 wheelFrontX, const PxF32 wheelRearX, const PxVec3& chassisDims, const PxF32 wheelWidth, const PxF32 wheelRadius, const PxU32 numWheels, PxVec3* wheelCentreOffsets)
{
    // 앞쪽 바퀴 개수 (전체 휠 2개가 좌우 한 쌍)
    const PxF32 numLeftWheels = numWheels / 2.0f;
    // X축(앞뒤) 간격
    const PxF32 deltaX = (wheelFrontX - wheelRearX) / (numLeftWheels - 1.0f);

    // Y축(좌우) 오프셋: 차체 반폭 - 휠 반폭
    const PxF32 lateralOffset = (chassisDims.y - wheelWidth) * 0.5f;
    // Z축(높이) 오프셋: 차체 반높이 + 휠 반경 (휠이 바닥에 닿도록 아래 방향)
    const PxF32 verticalOffset = -(chassisDims.z * 0.5f + wheelRadius);

    // Rear Left / Rear Right
    wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT] = PxVec3(
        wheelRearX,
        -lateralOffset,
        verticalOffset);
    wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = PxVec3(
        wheelRearX,
         lateralOffset,
        verticalOffset);

    // Front Left / Front Right
    wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = PxVec3(
        wheelRearX + (numLeftWheels - 1) * deltaX,
        -lateralOffset,
        verticalOffset);
    wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = PxVec3(
        wheelRearX + (numLeftWheels - 1) * deltaX,
         lateralOffset,
        verticalOffset);

    // 중간 바퀴 (6륜 이상)
    PxU32 idx = 4;
    for (PxU32 i = 2; i < numWheels - 2; i += 2)
    {
        const PxF32 posX = wheelRearX + i * deltaX * 0.5f;
        wheelCentreOffsets[idx++] = PxVec3(posX, -lateralOffset, verticalOffset);
        wheelCentreOffsets[idx++] = PxVec3(posX,  lateralOffset, verticalOffset);
    }
}

void SetupWheelsSimulationData(
    const PxF32 wheelMass,
    const PxF32 wheelMOI,
    const PxF32 wheelRadius,
    const PxF32 wheelWidth,
    const PxU32 numWheels,
    const PxVec3* wheelCenterActorOffsets,
    const PxVec3& chassisCMOffset,
    const PxF32 chassisMass,
    PxVehicleWheelsSimData* wheelsSimData)
{
    // 1) Wheel data
    PxVehicleWheelData wheels[PX_MAX_NB_WHEELS];
    for(PxU32 i = 0; i < numWheels; i++)
    {
        wheels[i].mMass   = wheelMass;
        wheels[i].mMOI    = wheelMOI;
        wheels[i].mRadius = wheelRadius;
        wheels[i].mWidth  = wheelWidth;
    }
    // Handbrake only on rear
    wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT ].mMaxHandBrakeTorque = 4000.0f;
    wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
    // Steering only on front
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].mMaxSteer = PxPi*0.3333f;
    wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = PxPi*0.3333f;

    // 2) Tire data
    PxVehicleTireData tires[PX_MAX_NB_WHEELS];
    for(PxU32 i = 0; i < numWheels; i++)
        tires[i].mType = TIRE_TYPE_NORMAL;

    // 3) Suspension data
    PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
    PxVehicleComputeSprungMasses(
        numWheels,
        wheelCenterActorOffsets,
        chassisCMOffset,
        chassisMass,
        1,
        suspSprungMasses);

    PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
    for(PxU32 i = 0; i < numWheels; i++)
    {
        suspensions[i].mMaxCompression     = 0.3f;
        suspensions[i].mMaxDroop           = 0.1f;
        suspensions[i].mSpringStrength     = 35000.0f;
        suspensions[i].mSpringDamperRate   = 4500.0f;
        suspensions[i].mSprungMass         = suspSprungMasses[i];
    }
    // Camber
    const PxF32 camberRest = 0.0f;
    const PxF32 camberDroop = 0.01f;
    const PxF32 camberComp = -0.01f;
    for(PxU32 i = 0; i < numWheels; i += 2)
    {
        suspensions[i+0].mCamberAtRest           =  camberRest;
        suspensions[i+1].mCamberAtRest           = -camberRest;
        suspensions[i+0].mCamberAtMaxDroop       =  camberDroop;
        suspensions[i+1].mCamberAtMaxDroop       = -camberDroop;
        suspensions[i+0].mCamberAtMaxCompression =  camberComp;
        suspensions[i+1].mCamberAtMaxCompression = -camberComp;
    }

    // 4) Geometry offsets
    PxVec3 suspTravelDirections[PX_MAX_NB_WHEELS];
    PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 suspForceAppCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
    for(PxU32 i = 0; i < numWheels; i++)
    {
        // Z-up coordinate: suspension travel downwards along negative Z
        suspTravelDirections[i] = PxVec3(0, 0, -1);
        
        // Offset from chassis CM
        wheelCentreCMOffsets[i] = wheelCenterActorOffsets[i] - chassisCMOffset;
        
        // Force application points 0.3m below chassis CM along Z
        PxF32 drop = 0.3f;
        suspForceAppCMOffsets[i] = PxVec3(
            wheelCentreCMOffsets[i].x,
            wheelCentreCMOffsets[i].y,
            wheelCentreCMOffsets[i].z - drop);
        tireForceAppCMOffsets[i] = suspForceAppCMOffsets[i];
    }

    // 5) Scene query filter
    PxFilterData qryFilterData;
    setupNonDrivableSurface(qryFilterData);

    // 6) Assign to sim data
    for(PxU32 i = 0; i < numWheels; i++)
    {
        wheelsSimData->setWheelData         (i, wheels[i]);
        wheelsSimData->setTireData          (i, tires[i]);
        wheelsSimData->setSuspensionData    (i, suspensions[i]);
        wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
        wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
        wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
        wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
        wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
        wheelsSimData->setWheelShapeMapping(i, PxI32(i));
    }

    // 7) Anti-roll bars front & rear
    PxVehicleAntiRollBarData bar;
    bar.mWheel0    = PxVehicleDrive4WWheelOrder::eFRONT_LEFT;
    bar.mWheel1    = PxVehicleDrive4WWheelOrder::eFRONT_RIGHT;
    bar.mStiffness = 10000.0f;
    wheelsSimData->addAntiRollBarData(bar);
    bar.mWheel0    = PxVehicleDrive4WWheelOrder::eREAR_LEFT;
    bar.mWheel1    = PxVehicleDrive4WWheelOrder::eREAR_RIGHT;
    wheelsSimData->addAntiRollBarData(bar);
}


FVehicle4W::FVehicle4W()
{
}

FVehicle4W::~FVehicle4W()
{
    if (Vehicle)
    {
        Vehicle->getRigidDynamicActor()->release();
        Vehicle->free();
    }
}

void FVehicle4W::Initialize(const VehicleDesc& vehicle4WDesc, PxPhysics* physics, PxCooking* cooking, const physx::PxTransform& startPose)
{
    CreateVehicle4W(vehicle4WDesc, physics, cooking);

    // Z-up 기준 높이 설정 (chassisDims.z 반 + wheelRadius + margin)
    PxF32 height = vehicle4WDesc.chassisDims.z * 0.5f + vehicle4WDesc.wheelRadius + 1.0f;
    PxTransform startTransform(PxVec3(0.f, 0.f, height), PxQuat(PxIdentity));
    Vehicle->getRigidDynamicActor()->setGlobalPose(startTransform);
}

PxVehicleDrive4W* FVehicle4W::CreateVehicle4W(const VehicleDesc& vehicle4WDesc, PxPhysics* physics, PxCooking* cooking)
{
    // X=앞뒤, Y=좌우, Z=높이
    const PxVec3 chassisDims = vehicle4WDesc.chassisDims;
	const PxF32 wheelWidth = vehicle4WDesc.wheelWidth;
	const PxF32 wheelRadius = vehicle4WDesc.wheelRadius;
	const PxU32 numWheels = vehicle4WDesc.numWheels;

	const PxFilterData& chassisSimFilterData = vehicle4WDesc.chassisSimFilterData;
	const PxFilterData& wheelSimFilterData = vehicle4WDesc.wheelSimFilterData;

	//Construct a physx actor with shapes for the chassis and wheels.
	//Set the rigid body mass, moment of inertia, and center of mass offset.
	PxRigidDynamic* veh4WActor = nullptr;
	{
		//Construct a convex mesh for a cylindrical wheel.
		PxConvexMesh* wheelMesh = createWheelMesh(wheelWidth, wheelRadius, *physics, *cooking);
		//Assume all wheels are identical for simplicity.
		PxConvexMesh* wheelConvexMeshes[PX_MAX_NB_WHEELS];
		PxMaterial* wheelMaterials[PX_MAX_NB_WHEELS];

		//Set the meshes and materials for the driven wheels.
		for(PxU32 i = PxVehicleDrive4WWheelOrder::eFRONT_LEFT; i <= PxVehicleDrive4WWheelOrder::eREAR_RIGHT; i++)
		{
			wheelConvexMeshes[i] = wheelMesh;
			wheelMaterials[i] = vehicle4WDesc.wheelMaterial;
		}
		//Set the meshes and materials for the non-driven wheels
		for(PxU32 i = PxVehicleDrive4WWheelOrder::eREAR_RIGHT + 1; i < numWheels; i++)
		{
			wheelConvexMeshes[i] = wheelMesh;
			wheelMaterials[i] = vehicle4WDesc.wheelMaterial;
		}

		//Chassis just has a single convex shape for simplicity.
		PxConvexMesh* chassisConvexMesh = createChassisMesh(chassisDims, *physics, *cooking);
		PxConvexMesh* chassisConvexMeshes[1] = {chassisConvexMesh};
		PxMaterial* chassisMaterials[1] = {vehicle4WDesc.chassisMaterial};

		//Rigid body data.
		PxVehicleChassisData rigidBodyData;
		rigidBodyData.mMOI = vehicle4WDesc.chassisMOI;
		rigidBodyData.mMass = vehicle4WDesc.chassisMass;
		rigidBodyData.mCMOffset = vehicle4WDesc.chassisCMOffset;

		veh4WActor = createVehicleActor
			(rigidBodyData,
			wheelMaterials, wheelConvexMeshes, numWheels, wheelSimFilterData,
			chassisMaterials, chassisConvexMeshes, 1, chassisSimFilterData,
			*physics);
	}

	//Set up the sim data for the wheels.
	PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(numWheels);
	{
		//Compute the wheel center offsets from the origin.
		PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
        // X축 front/back 오프셋 계산
        // 앞뒤 휠을 차체 끝에 딱 붙이려면
        const PxF32 frontX =  chassisDims.x * 0.5f - wheelRadius;
        const PxF32 rearX  = -frontX;
		ComputeWheelCenterActorOffsets4W(frontX, rearX, chassisDims, wheelWidth, wheelRadius, numWheels, wheelCenterActorOffsets);

		//Set up the simulation data for all wheels.
		SetupWheelsSimulationData
			(vehicle4WDesc.wheelMass, vehicle4WDesc.wheelMOI, wheelRadius, wheelWidth, 
			 numWheels, wheelCenterActorOffsets,
			 vehicle4WDesc.chassisCMOffset, vehicle4WDesc.chassisMass,
			 wheelsSimData);
	}

	//Set up the sim data for the vehicle drive model.
	PxVehicleDriveSimData4W driveSimData;
	{
		//Diff
		PxVehicleDifferential4WData diff;
		diff.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
		driveSimData.setDiffData(diff);

		//Engine
		PxVehicleEngineData engine;
		engine.mPeakTorque = 500.0f;
		engine.mMaxOmega = 600.0f;//approx 6000 rpm
		driveSimData.setEngineData(engine);

		//Gears
		PxVehicleGearsData gears;
		gears.mSwitchTime = 0.5f;
		driveSimData.setGearsData(gears);

		//Clutch
		PxVehicleClutchData clutch;
		clutch.mStrength = 10.0f;
		driveSimData.setClutchData(clutch);

        // Ackermann (X-front, Y-track)
        PxVehicleAckermannGeometryData ack;
        ack.mAccuracy = 1.0f;
        ack.mAxleSeparation =
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x
          - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT ).x;
        ack.mFrontWidth = PxAbs(
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).y
          - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT ).y
        );
        ack.mRearWidth = PxAbs(
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).y
          - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT ).y
        );
        driveSimData.setAckermannGeometryData(ack);
	}

	//Create a vehicle from the wheels and drive sim data.
	Vehicle = PxVehicleDrive4W::allocate(numWheels);
	Vehicle->setup(physics, veh4WActor, *wheelsSimData, driveSimData, numWheels - 4);

	//Configure the userdata
	configureUserData(Vehicle, vehicle4WDesc.actorUserData, vehicle4WDesc.shapeUserDatas);

	//Free the sim data because we don't need that any more.
	wheelsSimData->free();

	return Vehicle;
}

void FVehicle4W::StartAccelerateForwardsMode()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalAccel(true);
    }
    else
    {
        VehicleInputData.setAnalogAccel(1.0f);
    }
}

void FVehicle4W::StartAccelerateReverseMode() const
{
    Vehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);

    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalAccel(true);
    }
    else
    {
        VehicleInputData.setAnalogAccel(1.0f);
    }
}

void FVehicle4W::StartBrakeMode()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalBrake(true);
    }
    else
    {
        VehicleInputData.setAnalogBrake(1.0f);
    }
}

void FVehicle4W::StartTurnHardLeftMode()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalAccel(true);
        VehicleInputData.setDigitalSteerLeft(true);
    }
    else
    {
        VehicleInputData.setAnalogAccel(true);
        VehicleInputData.setAnalogSteer(-1.0f);
    }
}

void FVehicle4W::StartTurnHardRightMode()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalAccel(true);
        VehicleInputData.setDigitalSteerRight(true);
    }
    else
    {
        VehicleInputData.setAnalogAccel(1.0f);
        VehicleInputData.setAnalogSteer(1.0f);
    }
}

void FVehicle4W::StartHandbrakeTurnLeftMode()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalSteerLeft(true);
        VehicleInputData.setDigitalHandbrake(true);
    }
    else
    {
        VehicleInputData.setAnalogSteer(-1.0f);
        VehicleInputData.setAnalogHandbrake(1.0f);
    }
}

void FVehicle4W::StartHandbrakeTurnRightMode()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalAccel(false);
        VehicleInputData.setDigitalSteerLeft(false);
        VehicleInputData.setDigitalSteerRight(false);
        VehicleInputData.setDigitalBrake(false);
        VehicleInputData.setDigitalHandbrake(false);
    }
    else
    {
        VehicleInputData.setAnalogAccel(0.0f);
        VehicleInputData.setAnalogSteer(0.0f);
        VehicleInputData.setAnalogBrake(0.0f);
        VehicleInputData.setAnalogHandbrake(0.0f);
    }
}

void FVehicle4W::IncrementDrivingMode(float deltaTime)
{
    VehicleModeTimer += deltaTime;
    if(VehicleModeTimer > VehicleModeLifetime)
    {
        //If the mode just completed was eDRIVE_MODE_ACCEL_REVERSE then switch back to forward gears.
        if(EDriveMode::eDRIVE_MODE_ACCEL_REVERSE == DriveModeOrder[VehicleOrderProgress])
        {
            Vehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
        }

        //Increment to next driving mode.
        VehicleModeTimer = 0.0f;
        VehicleOrderProgress++;
        ReleaseAllControls();

        //If we are at the end of the list of driving modes then start again.
        if(EDriveMode::eDRIVE_MODE_NONE == DriveModeOrder[VehicleOrderProgress])
        {
            VehicleOrderProgress = 0;
            VehicleOrderComplete = true;
        }

        //Start driving in the selected mode.
        EDriveMode eDriveMode = DriveModeOrder[VehicleOrderProgress];
        switch(eDriveMode)
        {
        case EDriveMode::eDRIVE_MODE_ACCEL_FORWARDS:
            StartAccelerateForwardsMode();
            break;
        case EDriveMode::eDRIVE_MODE_ACCEL_REVERSE:
            StartAccelerateReverseMode();
            break;
        case EDriveMode::eDRIVE_MODE_HARD_TURN_LEFT:
            StartTurnHardLeftMode();
            break;
        case EDriveMode::eDRIVE_MODE_HANDBRAKE_TURN_LEFT:
            StartHandbrakeTurnLeftMode();
            break;
        case EDriveMode::eDRIVE_MODE_HARD_TURN_RIGHT:
            StartTurnHardRightMode();
            break;
        case EDriveMode::eDRIVE_MODE_HANDBRAKE_TURN_RIGHT:
            StartHandbrakeTurnRightMode();
            break;
        case EDriveMode::eDRIVE_MODE_BRAKE:
            StartBrakeMode();
            break;
        case EDriveMode::eDRIVE_MODE_NONE:
            break;
        };

        //If the mode about to start is eDRIVE_MODE_ACCEL_REVERSE then switch to reverse gears.
        if(EDriveMode::eDRIVE_MODE_ACCEL_REVERSE == DriveModeOrder[VehicleOrderProgress])
        {
            Vehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
        }
    }
}

void FVehicle4W::ReleaseAllControls()
{
    if(MimicKeyInputs)
    {
        VehicleInputData.setDigitalAccel(false);
        VehicleInputData.setDigitalSteerLeft(false);
        VehicleInputData.setDigitalSteerRight(false);
        VehicleInputData.setDigitalBrake(false);
        VehicleInputData.setDigitalHandbrake(false);
    }
    else
    {
        VehicleInputData.setAnalogAccel(0.0f);
        VehicleInputData.setAnalogSteer(0.0f);
        VehicleInputData.setAnalogBrake(0.0f);
        VehicleInputData.setAnalogHandbrake(0.0f);
    }
}

void FVehicle4W::Update(float deltaTime)
{
    // IncrementDrivingMode(deltaTime);
    //
    // //Update the control inputs for the vehicle.
    // if (gMimicKeyInputs)
    // {
    //     PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs(gKeySmoothingData, gSteerVsForwardSpeedTable, VehicleInputData, deltaTime, gIsVehicleInAir, *Vehicle);
    // }
    // else
    // {
    //     PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, VehicleInputData, deltaTime, gIsVehicleInAir, *Vehicle);
    // }

    // 1) 자동 모드 타이머 증가
    AutoDriveTimer += deltaTime;
    if (AutoDriveTimer >= AutoDriveInterval)
    {
        AutoDriveTimer = 0.0f;

        // 기존 모드 완료 시 후처리 (리버스 → 포워드 기어)
        if (DriveModeOrder[VehicleOrderProgress] == EDriveMode::eDRIVE_MODE_ACCEL_REVERSE)
        {
            Vehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
        }

        // 다음 모드로 전환
        size_t DriveModeCount = sizeof(DriveModeOrder) / sizeof(DriveModeOrder[0]); 
        VehicleOrderProgress = (VehicleOrderProgress + 1) % DriveModeCount;
        ReleaseAllControls();

        // 새 모드 시작
        switch (DriveModeOrder[VehicleOrderProgress])
        {
            case EDriveMode::eDRIVE_MODE_ACCEL_FORWARDS:        StartAccelerateForwardsMode();   break;
            case EDriveMode::eDRIVE_MODE_ACCEL_REVERSE:         StartAccelerateReverseMode();    break;
            case EDriveMode::eDRIVE_MODE_HARD_TURN_LEFT:        StartTurnHardLeftMode();         break;
            case EDriveMode::eDRIVE_MODE_HANDBRAKE_TURN_LEFT:   StartHandbrakeTurnLeftMode();    break;
            case EDriveMode::eDRIVE_MODE_HARD_TURN_RIGHT:       StartTurnHardRightMode();        break;
            case EDriveMode::eDRIVE_MODE_HANDBRAKE_TURN_RIGHT:  StartHandbrakeTurnRightMode();   break;
            case EDriveMode::eDRIVE_MODE_BRAKE:                 StartBrakeMode();                break;
            default: /* none */                                                         break;
        }

        // 리버스 모드일 때 기어 변경
        if (DriveModeOrder[VehicleOrderProgress] == EDriveMode::eDRIVE_MODE_ACCEL_REVERSE)
        {
            Vehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
        }
    }

    // 2) 자동으로 세팅된 gVehicleInputData 를 기반으로 스무딩 & 아날로그 입력 설정
    PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
        PadSmoothingData,
        SteerVsForwardSpeedTable,
        VehicleInputData,
        deltaTime,
        IsVehicleInAir,
        *Vehicle
    );
}

