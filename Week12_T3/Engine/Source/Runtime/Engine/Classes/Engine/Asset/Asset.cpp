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
