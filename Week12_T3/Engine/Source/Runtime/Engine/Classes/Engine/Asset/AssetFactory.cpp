#include "AssetFactory.h"

UAssetFactory::UAssetFactory()
{
}

UAssetFactory::~UAssetFactory()
{
}

bool UAssetFactory::CanImport(UClass* InClass, const FString& filepath) const
{
    const FString ext = std::filesystem::path(filepath).extension().string();
    for (const FString& supported : SupportedExtensions)
    {
        if (supported == ext)
        {
            if (SupportedClass == InClass || InClass->IsChildOf(SupportedClass))
            {
                return true;
            }
        }
    }
    return false;
}
