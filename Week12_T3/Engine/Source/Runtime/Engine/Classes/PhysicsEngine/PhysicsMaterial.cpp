#include "PhysicsMaterial.h"
#include "UserInterface/Console.h"
#include <PxPhysics.h>
#include <PxMaterial.h>

FPhysicsMaterial::FPhysicsMaterial(physx::PxPhysics* InPxPhysicsSDK, float StaticFriction, float DynamicFriction, float Restitution)
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

FPhysicsMaterial::~FPhysicsMaterial()
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->release();
        PxMaterialInstance = nullptr;
    }
}

float FPhysicsMaterial::GetStaticFriction() const
{
    return PxMaterialInstance ? PxMaterialInstance->getStaticFriction() : 0.0f;
}

void FPhysicsMaterial::SetStaticFriction(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setStaticFriction(NewValue);
    }
}

float FPhysicsMaterial::GetDynamicFriction() const
{
    return PxMaterialInstance ? PxMaterialInstance->getDynamicFriction() : 0.0f;
}

void FPhysicsMaterial::SetDynamicFriction(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setDynamicFriction(NewValue);
    }
}

float FPhysicsMaterial::GetRestitution() const
{
    return PxMaterialInstance ? PxMaterialInstance->getRestitution() : 0.0f;
}

void FPhysicsMaterial::SetRestitution(float NewValue)
{
    if (PxMaterialInstance)
    {
        PxMaterialInstance->setRestitution(NewValue);
    }
}