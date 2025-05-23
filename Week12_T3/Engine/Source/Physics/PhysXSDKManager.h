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
    FPhysXSDKManager() = default;
    ~FPhysXSDKManager();

    FPhysXSDKManager(const FPhysXSDKManager&) = delete;
    FPhysXSDKManager& operator=(const FPhysXSDKManager&) = delete;
    FPhysXSDKManager(FPhysXSDKManager&&) = delete;
    FPhysXSDKManager& operator=(FPhysXSDKManager&&) = delete;

    FPhysXSDKManager& GetInstance()
    {
        static FPhysXSDKManager Inst;
        return Inst;
    }
    void Initalize();
    void Shutdown();

public:
    physx::PxDefaultAllocator DefaultAllocatorCallback;
    physx::PxDefaultErrorCallback DefaultErrorCallback;

    physx::PxFoundation* PxFoundationInstance;
    physx::PxPhysics* PxSDKInstance;
    
};

