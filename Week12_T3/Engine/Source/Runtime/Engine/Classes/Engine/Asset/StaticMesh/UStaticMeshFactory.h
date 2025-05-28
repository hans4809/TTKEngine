#pragma once
#include "Engine/Asset/AssetFactory.h"

class UStaticMeshFactory : public UAssetFactory
{
    DECLARE_CLASS(UStaticMeshFactory, UAssetFactory)
public:
    UStaticMeshFactory();
    ~UStaticMeshFactory() override;
    
    UAsset* ImportFromFile(const FString& filepath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
