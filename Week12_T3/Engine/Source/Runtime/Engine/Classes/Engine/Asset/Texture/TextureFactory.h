#pragma once
#include "Engine/Asset/AssetFactory.h"

class UTextureFactory : public UAssetFactory
{
    DECLARE_CLASS(UTextureFactory, UAssetFactory)
public:
    UTextureFactory();
    ~UTextureFactory() override;
    
    UAsset* ImportFromFile(const FString& filepath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
