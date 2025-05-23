#include "PhysicsModule.h"

PhysicsModule::PhysicsModule() 
    : SDKManagerInstance(nullptr), bIsInitialized(false) 
{
}

PhysicsModule::~PhysicsModule()
{
}
bool PhysicsModule::Startup()
{
    if (bIsInitialized)
    {
        return true;
    }

    //SDKManagerInstance = new PhysXSDKManager(); // SDK 관리자 생성
    //if (!SDKManagerInstance->Initialize()) 
    //{ 
    //    delete SDKManagerInstance;
    //    SDKManagerInstance = nullptr;
    //    return false;
    //}

    // ... 기타 물리 모듈 초기화 작업 ...

    bIsInitialized = true;
    return true;
}
void PhysicsModule::Shutdown()
{
    if (!bIsInitialized) 
    {
        return;
    }

   
    // ... 기타 물리 모듈 정리 작업 ...

    //if (SDKManagerInstance)
    //{
    //    SDKManagerInstance->Shutdown(); // SDK 종료
    //    delete SDKManagerInstance;
    //    SDKManagerInstance = nullptr;
    //}

    bIsInitialized = false;
}

PhysXSDKManager* PhysicsModule::GetSDKManager() const {
    return SDKManagerInstance;
}

PhysXSDKManager* PhysicsModule::GetSDKManager() const
{
    return nullptr;
}
