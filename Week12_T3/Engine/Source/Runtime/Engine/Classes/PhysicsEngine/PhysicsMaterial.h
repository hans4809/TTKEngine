#pragma once
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
namespace physx 
{
    class PxMaterial;
    class PxPhysics;
};

class UPhysicalMaterial : public UObject
{
    DECLARE_CLASS(UPhysicalMaterial, UObject)
public:
    UPhysicalMaterial() {};
    void Initialize(physx::PxPhysics* InPxPhysicsSDK, float StaticFriction, float DynamicFriction, float Restitution);
    ~UPhysicalMaterial();

    physx::PxMaterial* GetPxMaterial();

    // 재질 속성 접근자 (선택적, 필요시 추가)
    float GetStaticFriction() const;
    void SetStaticFriction(float NewValue);

    float GetDynamicFriction() const;
    void SetDynamicFriction(float NewValue);

    float GetRestitution() const;
    void SetRestitution(float NewValue);
public:
    UPROPERTY(EditAnywhere,float, StaticFriction,=0);
    UPROPERTY(EditAnywhere,float, DynamicFriction,=0);
    UPROPERTY(EditAnywhere,float, Restitution,=0);
  
private:
    physx::PxPhysics* OwningPxPhysicsSDK = nullptr;
    physx::PxMaterial* PxMaterialInstance = nullptr;
};

