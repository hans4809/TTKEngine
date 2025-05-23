#include "TextureFactory.h"

#include "Engine/Texture.h"

UTextureFactory::~UTextureFactory()
{
}

bool UTextureFactory::CanImport(const FString& filepath) const
{
    return Super::CanImport(filepath);
}

UAsset* UTextureFactory::ImportFromFile(const FString& InFilePath)
{
    UTexture* NewTexture = FObjectFactory::ConstructObject<UTexture>(nullptr);
    if (!NewTexture->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    std::filesystem::path filePath(InFilePath);
    NewTexture->GetDescriptor().AbsolutePath = FString(filePath.generic_wstring().c_str());
    NewTexture->GetDescriptor().RelativePath = std::filesystem::relative(filePath, std::filesystem::current_path()).string();

    return NewTexture;
}

UAsset* UTextureFactory::CreateNew(const FAssetDescriptor& desc)
{
    UTexture* NewTexture = FObjectFactory::ConstructObject<UTexture>(nullptr);
    return NewTexture;
}
