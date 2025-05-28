#pragma once
#include "Engine/Asset/AssetFactory.h"

class USkeletalMeshFactory : public UAssetFactory
{
    DECLARE_CLASS(USkeletalMeshFactory, UAssetFactory)
public:
    USkeletalMeshFactory();
    ~USkeletalMeshFactory() override;
    
    UAsset* ImportFromFile(const FString& filepath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
}; 
