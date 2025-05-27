#include "Material.h"

bool UMaterial::LoadFromFile(const FString& filepath)
{
    return UAsset::LoadFromFile(filepath);
}

bool UMaterial::SerializeToFile(std::ostream& Out)
{
    return UAsset::SerializeToFile(Out);
}

bool UMaterial::DeserializeFromFile(std::istream& In)
{
    return UAsset::DeserializeFromFile(In);
}

void UMaterial::PostLoad()
{
    UAsset::PostLoad();
}
