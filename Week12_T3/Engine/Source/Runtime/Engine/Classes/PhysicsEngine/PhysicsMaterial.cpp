#include "PhysicsMaterial.h"
#include "UserInterface/Console.h"
#include <PxPhysics.h>
#include <PxMaterial.h>

UPhysicalMaterial::UPhysicalMaterial(physx::PxPhysics* InPxPhysicsSDK, float StaticFriction, float DynamicFriction, float Restitution)
    :OwningPxPhysicsSDK(InPxPhysicsSDK), PxMaterialInstance(nullptr)
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
    return PxMaterialInstance ? PxMaterialInstance->getStaticFriction() : 0.0f;
}

void UPhysicalMaterial::SetStaticFriction(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setStaticFriction(NewValue);
    }
}

float UPhysicalMaterial::GetDynamicFriction() const
{
    return PxMaterialInstance ? PxMaterialInstance->getDynamicFriction() : 0.0f;
}

void UPhysicalMaterial::SetDynamicFriction(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setDynamicFriction(NewValue);
    }
}

float UPhysicalMaterial::GetRestitution() const
{
    return PxMaterialInstance ? PxMaterialInstance->getRestitution() : 0.0f;
}

void UPhysicalMaterial::SetRestitution(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setRestitution(NewValue);
    }
}