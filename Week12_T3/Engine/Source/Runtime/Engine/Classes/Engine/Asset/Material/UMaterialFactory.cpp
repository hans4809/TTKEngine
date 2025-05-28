#include "UMaterialFactory.h"

#include "Components/Material/Material.h"

UMaterialFactory::UMaterialFactory()
{
    SetSupportedExtensions({".mtl", ".fbx"});
    SetSupportedClass(UMaterial::StaticClass());
    SetPriority(100);
}

UMaterialFactory::~UMaterialFactory()
{
}

UAsset* UMaterialFactory::ImportFromFile(const FString& InFilePath)
{
    return nullptr;
}

UAsset* UMaterialFactory::CreateNew(const FAssetDescriptor& desc)
{
    return nullptr;
}
