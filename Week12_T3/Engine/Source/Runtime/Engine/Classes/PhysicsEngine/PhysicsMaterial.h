#pragma once
namespace physx 
{
    class PxMaterial;
    class PxPhysics;
};

class UPhysicalMaterial
{
public:
    UPhysicalMaterial(physx::PxPhysics* InPxPhysicsSDK, float StaticFriction, float DynamicFriction, float Restitution);
    ~UPhysicalMaterial();

    physx::PxMaterial* GetPxMaterial() const { return PxMaterialInstance; }

    // 재질 속성 접근자 (선택적, 필요시 추가)
    float GetStaticFriction() const;
    void SetStaticFriction(float NewValue);

    float GetDynamicFriction() const;
    void SetDynamicFriction(float NewValue);

    float GetRestitution() const;
    void SetRestitution(float NewValue);

private:
    physx::PxPhysics* OwningPxPhysicsSDK = nullptr;
    physx::PxMaterial* PxMaterialInstance = nullptr;
};

