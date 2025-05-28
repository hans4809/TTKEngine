#include "TextureFactory.h"

#include "Engine/Texture.h"

UTextureFactory::UTextureFactory()
{
    SetSupportedExtensions({".dds", ".png", "jpg", "jpeg"});
    SetSupportedClass(UTexture::StaticClass());
    SetPriority(100);
}

UTextureFactory::~UTextureFactory()
{
}

UAsset* UTextureFactory::ImportFromFile(const FString& InFilePath)
{
    UTexture* NewTexture = FObjectFactory::ConstructObject<UTexture>(nullptr);
    if (!NewTexture->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    return NewTexture;
}

UAsset* UTextureFactory::CreateNew(const FAssetDescriptor& desc)
{
    UTexture* NewTexture = FObjectFactory::ConstructObject<UTexture>(nullptr);
    return NewTexture;
}
