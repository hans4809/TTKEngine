#include "Asset.h"

UAsset::UAsset()
    : Super()
{
    SetFlags(RF_DuplicateTransient);
}

void UAsset::SetAssetName(const FString& InName)
{
    Descriptor.AssetName = InName;
}

void UAsset::SetAssetDescriptor(const FAssetDescriptor& InDescriptor)
{
    Descriptor = InDescriptor;
}
