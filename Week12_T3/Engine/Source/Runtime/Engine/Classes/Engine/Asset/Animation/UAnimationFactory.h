#pragma once
#include "Engine/Asset/AssetFactory.h"

class UAnimationFactory : public UAssetFactory
{
    DECLARE_CLASS(UAnimationFactory, UAssetFactory)
public:
    UAnimationFactory();
    ~UAnimationFactory() override;
    
    UAsset* ImportFromFile(const FString& filepath) override;
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
