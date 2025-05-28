#include "UStaticMeshFactory.h"

#include "Components/Mesh/StaticMesh.h"

#include "Physics/BodySetup/BodySetup.h"

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
    UStaticMesh* NewStaticMesh = FObjectFactory::ConstructObject<UStaticMesh>(nullptr); 
    if (!NewStaticMesh->LoadFromFile(InFilePath))
    {
        return nullptr;
    }
    return NewStaticMesh;
}

UAsset* UStaticMeshFactory::CreateNew(const FAssetDescriptor& desc)
{
    UStaticMesh* NewSkeletalMesh = FObjectFactory::ConstructObject<UStaticMesh>(nullptr);
    return NewSkeletalMesh;
}
