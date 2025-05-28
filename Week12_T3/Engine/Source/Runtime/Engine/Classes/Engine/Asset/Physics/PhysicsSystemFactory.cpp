#include "PhysicsSystemFactory.h"
#include <Physics/PhysicsAsset.h>

UPhysicsSystemFactory::UPhysicsSystemFactory()
{
    SetSupportedExtensions({ ".uphysicsasset" });
    SetSupportedClass(UPhysicsAsset::StaticClass());
    SetPriority(50);
}

UPhysicsSystemFactory::~UPhysicsSystemFactory()
{
}

UAsset* UPhysicsSystemFactory::ImportFromFile(const FString& InFilePath)
{   
    UPhysicsAsset* NewParticleSystem = FObjectFactory::ConstructObject<UPhysicsAsset>(nullptr);
    if (!NewParticleSystem->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    return NewParticleSystem;
}

UAsset* UPhysicsSystemFactory::CreateNew(const FAssetDescriptor& desc)
{
    UPhysicsAsset* NewPhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(nullptr);
    return NewPhysicsAsset;
}
