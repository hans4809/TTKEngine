#pragma once
#include "Engine/Asset/AssetFactory.h"

class UTextureFactory : public UAssetFactory
{
    DECLARE_CLASS(UTextureFactory, UAssetFactory)
public:
    UTextureFactory()
    {
        // .uparticles 파일을 파티클 시스템 에셋으로 인식
        SetSupportedExtensions({".dds", ".png", "jpg", "jpeg"});
        SetPriority(100);
    }
    ~UTextureFactory() override;
    
    bool CanImport(const FString& filepath) const override;
    UAsset* ImportFromFile(const FString& filepath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
