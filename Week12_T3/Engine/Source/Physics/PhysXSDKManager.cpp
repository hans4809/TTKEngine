#include "PhysXSDKManager.h"
#include <PxTolerancesScale.h>
#include <PxPhysicsAPI.h> 
FPhysXSDKManager::~FPhysXSDKManager()
{
    Shutdown();
}
void FPhysXSDKManager::Initalize()
{
    PxFoundationInstance = PxCreateFoundation(PX_PHYSICS_VERSION, DefaultAllocatorCallback, DefaultErrorCallback);
    PxSDKInstance = PxCreatePhysics(PX_PHYSICS_VERSION, *PxFoundationInstance, physx::PxTolerancesScale());
}

void FPhysXSDKManager::Shutdown()
{
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

}