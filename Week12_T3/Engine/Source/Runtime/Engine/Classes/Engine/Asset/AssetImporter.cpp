#include "AssetImporter.h"

#include "Asset.h"
#include "AssetFactory.h"
#include "AssetManager.h"
#include "Engine/FEditorStateManager.h"
#include "Serialization/Serializer.h"

UAssetImporter& UAssetImporter::Get()
{
    if (UAssetImporter* Singleton = GEngine->AssetImporter)
    {
        return *Singleton;
    }
    else
    {
        UE_LOG(LogLevel::Error, "Cannot use AssetManager if no AssetManagerClassName is defined!");
        assert(0);
        return *new UAssetImporter; // never calls this
    }
}

UAssetImporter::UAssetImporter()
{
}

UAssetImporter::~UAssetImporter()
{
}

void UAssetImporter::Import(UClass* InClass, const FString& InFilepath, const ImportCallback& InCallback)
{
    auto factory = FindFactory(InClass, InFilepath);
    if (!factory) return;

    // 디스크 읽기→Descriptor 생성
    FAssetDescriptor desc;
    std::filesystem::path filePath(InFilepath);

    // 에셋 식별자
    desc.AssetName = FName(FString(filePath.stem().string()));
    
    // ② 절대경로 (OS 구분자와 상관없이)
    desc.AbsolutePath  = FString(filePath.generic_wstring().c_str());
    desc.RelativePath  = std::filesystem::relative(filePath, std::filesystem::current_path()).string();
    
    // 확장자
    desc.AssetExtension = filePath.extension().string();

    // 메타데이터
    desc.Size       = std::filesystem::file_size(filePath);
    desc.CreateDate = std::filesystem::last_write_time(filePath);
    desc.UpdateDate = desc.CreateDate; 

    UAsset* asset;
    if (desc.AssetExtension == ".ttalkak")
    {
        // .uasset: Serializer로 직접 로드
        UObject* root = Serializer::LoadFromFile(desc.AbsolutePath);
        asset = Cast<UAsset>(root);
    }
    else
    {
        // 그 외: 팩토리 임포트
        asset = factory->ImportFromFile(desc.AbsolutePath);
    }

    if (asset)
    {
        asset->SetAssetDescriptor(desc);
        UAssetManager::Get().Store(desc.AssetName, asset);
    }
    
    if (InCallback)
    {
        InCallback(desc, asset);
    }
}

void UAssetImporter::ImportDirectory(UClass* InClass, const FString& InDir, const ImportCallback& InCallback)
{
    for (auto& p : std::filesystem::recursive_directory_iterator(InDir))
    {
        if (!p.is_regular_file()) continue;
        Import(InClass, p.path().string(), InCallback);
    }
}

UAssetFactory* UAssetImporter::FindFactory(UClass* InClass, const FString& filepath)
{
    return UAssetManager::Get().FindFactoryForFile(InClass, filepath);
}
