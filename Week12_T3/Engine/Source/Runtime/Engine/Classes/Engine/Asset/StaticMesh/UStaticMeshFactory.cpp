#include "UStaticMeshFactory.h"

#include "Components/Mesh/StaticMesh.h"

UStaticMeshFactory::UStaticMeshFactory()
{
    SetSupportedExtensions({".obj"});
    SetSupportedClass(UStaticMesh::StaticClass());
    SetPriority(100);
}

UStaticMeshFactory::~UStaticMeshFactory()
{
}

UAsset* UStaticMeshFactory::ImportFromFile(const FString& InFilePath)
{
    UStaticMesh* NewSkeletalMesh = FObjectFactory::ConstructObject<UStaticMesh>(nullptr);
    if (!NewSkeletalMesh->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    return NewSkeletalMesh;
}

UAsset* UStaticMeshFactory::CreateNew(const FAssetDescriptor& desc)
{
    UStaticMesh* NewSkeletalMesh = FObjectFactory::ConstructObject<UStaticMesh>(nullptr);
    return NewSkeletalMesh;
}
