#include "PhysicsSystemFactory.h"
#include <Physics/PhysicsAsset.h>

UPhysicsSystemFactory::~UPhysicsSystemFactory()
{
}

UAsset* UPhysicsSystemFactory::ImportFromFile(const FString& InFilePath)
{   
    // TODO
    /*UParticleSystem* NewParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    if (!NewParticleSystem->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    std::filesystem::path filePath(InFilePath);
    NewParticleSystem->GetDescriptor().AbsolutePath = FString(filePath.generic_wstring().c_str());
    NewParticleSystem->GetDescriptor().RelativePath = std::filesystem::relative(filePath, std::filesystem::current_path()).string();

    return NewParticleSystem;*/
    return nullptr;
}

UAsset* UPhysicsSystemFactory::CreateNew(const FAssetDescriptor& desc)
{
    UPhysicsAsset* NewPhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(nullptr);
    return NewPhysicsAsset;
}
