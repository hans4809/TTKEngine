#include "PhysXSDKManager.h"
#include "UserInterface/Console.h"
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
    if (bIsInitalized)
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
    

    Pvd = PxCreatePvd(*PxFoundationInstance);
    Transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);

    bool bPvdConnected = Pvd->connect(*Transport, physx::PxPvdInstrumentationFlag::eDEBUG);

    PxSDKInstance = PxCreatePhysics(PX_PHYSICS_VERSION, *PxFoundationInstance, physx::PxTolerancesScale(), true, Pvd);
    if (!PxSDKInstance)
    {
        UE_LOG(LogLevel::Error, "FPhysXSDKManager: Failed Create PxSDKInstance!");
        PxFoundationInstance->release();
        PxFoundationInstance = nullptr;
        return false;
    }

    PxInitExtensions(*PxSDKInstance, Pvd);
    bIsInitalized = true;
    return true;

}

void FPhysXSDKManager::Shutdown()
{
    if (!bIsInitalized)
    {
        return;
    }

    if (PxSDKInstance)
    {
        PxSDKInstance->release();
    }
    
    if (PxFoundationInstance)
    {
        PxFoundationInstance->release();
    }
    PxSDKInstance = nullptr;
    PxFoundationInstance = nullptr;
    bIsInitalized = false;

}

physx::PxPhysics* FPhysXSDKManager::GetPhysicsSDK()
{
    return PxSDKInstance;
}
