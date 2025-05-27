#pragma once
#include "Engine/Asset/AssetFactory.h"

class UMaterialFactory : public UAssetFactory
{
    DECLARE_CLASS(UMaterialFactory, UAssetFactory)
public:
    UMaterialFactory();
    ~UMaterialFactory() override;
    UAsset* ImportFromFile(const FString& filepath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
