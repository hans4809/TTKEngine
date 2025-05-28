#pragma once
#include "Engine/Asset/AssetFactory.h"

class USkeletonFactory : public UAssetFactory
{
    DECLARE_CLASS(USkeletonFactory, UAssetFactory)
public:
    USkeletonFactory();
    ~USkeletonFactory() override;
    
    UAsset* ImportFromFile(const FString& InFilePath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
