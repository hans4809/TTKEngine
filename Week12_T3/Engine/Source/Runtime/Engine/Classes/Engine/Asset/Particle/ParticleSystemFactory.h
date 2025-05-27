#pragma once
#include "Engine/Asset/AssetFactory.h"

class UParticleSystemFactory : public UAssetFactory
{
    DECLARE_CLASS(UParticleSystemFactory, UAssetFactory)
public:
    UParticleSystemFactory();

    ~UParticleSystemFactory() override;

    // 파일로부터 에셋 생성
    UAsset* ImportFromFile(const FString& InFilePath) override;

    // 빈 에셋 신규 생성
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
