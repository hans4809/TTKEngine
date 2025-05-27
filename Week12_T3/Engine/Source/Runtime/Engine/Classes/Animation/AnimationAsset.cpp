#include "AnimationAsset.h"
#include "CoreUObject/UObject/Casts.h"

void UAnimationAsset::PostDuplicate()
{
}

bool UAnimationAsset::LoadFromFile(const FString& filepath)
{
    return UAsset::LoadFromFile(filepath);
}

bool UAnimationAsset::SerializeToFile(std::ostream& Out)
{
    return UAsset::SerializeToFile(Out);
}

bool UAnimationAsset::DeserializeFromFile(std::istream& In)
{
    return UAsset::DeserializeFromFile(In);
}
