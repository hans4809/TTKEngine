#include "USkeletonFactory.h"

#include "Animation/Skeleton.h"


USkeletonFactory::USkeletonFactory()
{
    SetSupportedExtensions({".fbx"});
    SetSupportedClass(USkeleton::StaticClass());
    SetPriority(100);
}

USkeletonFactory::~USkeletonFactory()
{
}

UAsset* USkeletonFactory::ImportFromFile(const FString& InFilePath)
{
    USkeleton* NewSkeleton = FObjectFactory::ConstructObject<USkeleton>(nullptr);
    if (!NewSkeleton->LoadFromFile(InFilePath))
    {
        return nullptr;
    }
    
    return NewSkeleton;
}

UAsset* USkeletonFactory::CreateNew(const FAssetDescriptor& desc)
{
    USkeleton* NewSkeleton = FObjectFactory::ConstructObject<USkeleton>(nullptr);
    return NewSkeleton;
}
