#include "PhysXSDKManager.h"
#include "UserInterface/Console.h"
#include "PhysicsEngine/PhysicsMaterial.h"

#include "UObject/ObjectFactory.h"

#include <PxTolerancesScale.h>
#include <PxPhysicsAPI.h> 

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

physx::PxPhysics* FPhysXSDKManager::GetPhysicsSDK()
{
    return PxSDKInstance;
}
