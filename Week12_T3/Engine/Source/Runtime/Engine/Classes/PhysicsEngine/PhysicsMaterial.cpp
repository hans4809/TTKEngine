#include "PhysicsMaterial.h"
#include "UserInterface/Console.h"
#include <PxPhysics.h>
#include <PxMaterial.h>

void UPhysicalMaterial::Initialize(physx::PxPhysics* InPxPhysicsSDK, float InStaticFriction, float InDynamicFriction, float InRestitution)
{
    OwningPxPhysicsSDK = InPxPhysicsSDK;
    StaticFriction = InStaticFriction;
    DynamicFriction = InDynamicFriction;
    Restitution = InRestitution;
    PxMaterialInstance = nullptr;
}

UPhysicalMaterial::~UPhysicalMaterial()
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->release();
        PxMaterialInstance = nullptr;
    }
}

physx::PxMaterial* UPhysicalMaterial::GetPxMaterial()
{
    PxMaterialInstance = OwningPxPhysicsSDK->createMaterial(StaticFriction, DynamicFriction, Restitution);
    return PxMaterialInstance;

}
float UPhysicalMaterial::GetStaticFriction() const
{
    return StaticFriction;
}

void UPhysicalMaterial::SetStaticFriction(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setStaticFriction(NewValue);
    }
    StaticFriction = NewValue;
}

float UPhysicalMaterial::GetDynamicFriction() const
{
    return DynamicFriction;
}

void UPhysicalMaterial::SetDynamicFriction(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setDynamicFriction(NewValue);
    }
    DynamicFriction = NewValue;
}

float UPhysicalMaterial::GetRestitution() const
{
    return Restitution;
}

void UPhysicalMaterial::SetRestitution(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setRestitution(NewValue);
    }
    Restitution = NewValue;

}