#include "Skeleton.h"

#include "Engine/FBXLoader.h"

bool USkeleton::LoadFromFile(const FString& FilePath)
{
    if (FFBXLoader::ParseSkeletonFromFBX(FilePath, RefSkeletal) == false)
        return false;

    return true;
}

bool USkeleton::SerializeToFile(std::ostream& Out)
{
    return UAsset::SerializeToFile(Out);
}

bool USkeleton::DeserializeFromFile(std::istream& In)
{
    return UAsset::DeserializeFromFile(In);
}
