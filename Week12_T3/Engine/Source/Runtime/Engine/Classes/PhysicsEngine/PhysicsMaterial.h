#pragma once
namespace physx 
{
    class PxMaterial;
    class PxPhysics;
};

class FPhysicsMaterial
{
public:
    FPhysicsMaterial(physx::PxPhysics* InPxPhysicsSDK, float StaticFriction, float DynamicFriction, float Restitution);
    ~FPhysicsMaterial();

    physx::PxMaterial* GetPxMaterial() const { return PxMaterialInstance; }

    // 재질 속성 접근자 (선택적, 필요시 추가)
    float GetStaticFriction() const;
    void SetStaticFriction(float NewValue);

    float GetDynamicFriction() const;
    void SetDynamicFriction(float NewValue);

    float GetRestitution() const;
    void SetRestitution(float NewValue);

private:
    physx::PxPhysics* OwningPxPhysicsSDK;
    physx::PxMaterial* PxMaterialInstance;
};

