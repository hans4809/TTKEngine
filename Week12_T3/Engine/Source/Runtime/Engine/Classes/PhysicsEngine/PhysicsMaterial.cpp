#include "PhysicsMaterial.h"
#include "UserInterface/Console.h"
#include <PxPhysics.h>
#include <PxMaterial.h>

UPhysicalMaterial::UPhysicalMaterial(physx::PxPhysics* InPxPhysicsSDK, float StaticFriction, float DynamicFriction, float Restitution)
    :OwningPxPhysicsSDK(InPxPhysicsSDK)
{
    if (OwningPxPhysicsSDK)
    {
        PxMaterialInstance = OwningPxPhysicsSDK->createMaterial(StaticFriction, DynamicFriction, Restitution);
    }

    if (!PxMaterialInstance)
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create PxMaterial."));
    }
}

UPhysicalMaterial::~UPhysicalMaterial()
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->release();
        PxMaterialInstance = nullptr;
    }
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