#include "PhysXSDKManager.h"
#include "UserInterface/Console.h"
#include <PxTolerancesScale.h>
#include <PxPhysicsAPI.h> 
FPhysXSDKManager::FPhysXSDKManager()
    : PxFoundationInstance(nullptr), PxSDKInstance(nullptr)
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
    PxSDKInstance = PxCreatePhysics(PX_PHYSICS_VERSION, *PxFoundationInstance, physx::PxTolerancesScale());
    if (!PxSDKInstance)
    {
        UE_LOG(LogLevel::Error, "FPhysXSDKManager: Failed Create PxSDKInstance!");
        PxFoundationInstance->release();
        PxFoundationInstance = nullptr;
        return false;
    }
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