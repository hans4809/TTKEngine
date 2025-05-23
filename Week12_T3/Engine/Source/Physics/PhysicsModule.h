// PhysicsModule.h
#pragma once

class PhysXSDKManager; // 전방 선언

class PhysicsModule
{
public:
    PhysicsModule();
    ~PhysicsModule();

    // 모듈 진입점 역할의 함수들
    bool Startup();    // 엔진 시작 시 호출
    void Shutdown();   // 엔진 종료 시 호출

    // PhysX SDK 관리자 인스턴스에 접근하는 방법 제공 (예시)
    PhysXSDKManager* GetSDKManager() const;

private:
    PhysXSDKManager* SDKManagerInstance;
    bool bIsInitialized;
};