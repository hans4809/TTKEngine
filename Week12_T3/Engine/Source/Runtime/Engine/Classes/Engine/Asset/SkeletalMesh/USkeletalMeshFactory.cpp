#include "USkeletalMeshFactory.h"

#include "Components/Mesh/SkeletalMesh.h"

USkeletalMeshFactory::USkeletalMeshFactory()
{
    SetSupportedExtensions({".fbx"});
    SetSupportedClass(USkeletalMesh::StaticClass());
    SetPriority(100);
}

USkeletalMeshFactory::~USkeletalMeshFactory()
{
}

UAsset* USkeletalMeshFactory::ImportFromFile(const FString& InFilePath)
{
    USkeletalMesh* NewSkeletalMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
    if (!NewSkeletalMesh->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    return NewSkeletalMesh;
}

UAsset* USkeletalMeshFactory::CreateNew(const FAssetDescriptor& desc)
{
    USkeletalMesh* NewSkeletalMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
    return NewSkeletalMesh;
}
