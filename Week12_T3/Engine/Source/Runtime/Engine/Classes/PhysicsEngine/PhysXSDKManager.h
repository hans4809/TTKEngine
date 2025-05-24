// FPhysXSDKManager.h
#pragma once

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxAllocatorCallback.h>
#include <PxErrorCallback.h>

#include <PxDefaultAllocator.h>
#include <PxDefaultErrorCallback.h>

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
        Inst.Initalize();
        return Inst;
    }
    bool Initalize();
    void Shutdown();
    physx::PxPhysics* GetPhysicsSDK();
public:
    physx::PxDefaultAllocator DefaultAllocatorCallback;
    physx::PxDefaultErrorCallback DefaultErrorCallback;

    physx::PxFoundation* PxFoundationInstance;
    physx::PxPhysics* PxSDKInstance;
private:
    bool bIsInitalized = false;
};

