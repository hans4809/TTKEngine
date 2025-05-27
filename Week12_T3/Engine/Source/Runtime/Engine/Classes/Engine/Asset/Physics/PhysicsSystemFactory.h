#pragma once
#include "Engine/Asset/AssetFactory.h"

class UPhysicsSystemFactory : public UAssetFactory
{
    DECLARE_CLASS(UPhysicsSystemFactory, UAssetFactory)
public:
    UPhysicsSystemFactory()
    {
        SetSupportedExtensions({ ".uphysicsasset" });
        SetPriority(50);
    }
    ~UPhysicsSystemFactory() override;

    // 파일로부터 에셋 생성
    UAsset* ImportFromFile(const FString& InFilePath) override;

    // 빈 에셋 신규 생성
    UAsset* CreateNew(const FAssetDescriptor& desc) override;
};
 