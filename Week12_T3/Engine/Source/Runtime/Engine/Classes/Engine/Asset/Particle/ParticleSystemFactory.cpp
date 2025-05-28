#include "ParticleSystemFactory.h"

#include "Particles/ParticleSystem.h"

UParticleSystemFactory::UParticleSystemFactory()
{
    // .uparticles 파일을 파티클 시스템 에셋으로 인식
    SetSupportedExtensions({".uparticles"});
    SetSupportedClass(UParticleSystem::StaticClass());
    SetPriority(50);
}

UParticleSystemFactory::~UParticleSystemFactory()
{
}

UAsset* UParticleSystemFactory::ImportFromFile(const FString& InFilePath)
{
    UParticleSystem* NewParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    if (!NewParticleSystem->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    return NewParticleSystem;
}

UAsset* UParticleSystemFactory::CreateNew(const FAssetDescriptor& desc)
{
    UParticleSystem* NewParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    return NewParticleSystem;
}
