#include "AssetManager.h"

#include <filesystem>

#include "AssetFactory.h"
#include "AssetRegistry.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/Texture.h"
#include "Particles/ParticleSystem.h"
#include "Serialization/MemoryArchive.h"
#include "UObject/Casts.h"

extern UEngine* GEngine;

UAssetManager::~UAssetManager()
{
    for (auto factory : Factories)
    {
        UnregisterFactory(factory);
        factory->MarkAsGarbage();
    }

    for (auto asset : LoadedAssets)
    {
        asset.Value->MarkAsGarbage();
    }

    Registry->MarkAsGarbage();
}

bool UAssetManager::IsInitialized()
{
    return GEngine && GEngine->AssetManager;
}

UAssetManager& UAssetManager::Get()
{
    if (UAssetManager* Singleton = GEngine->AssetManager)
    {
        return *Singleton;
    }
    else
    {
        UE_LOG(LogLevel::Error, "Cannot use AssetManager if no AssetManagerClassName is defined!");
        assert(0);
        return *new UAssetManager; // never calls this
    }
}

UAssetManager* UAssetManager::GetIfInitialized()
{
    return GEngine ? GEngine->AssetManager : nullptr;
}

void UAssetManager::Initalize()
{
    Registry = FObjectFactory::ConstructObject<UAssetRegistry>(this);
    Registry->ScanDirectory();
    Registry->ScanDirectory(TEXT("Assets"));
    
    //LoadObjFiles();
}

void UAssetManager::RegisterFactory(UAssetFactory* InFactory)
{
    std::lock_guard<std::mutex> lock(Mutex);
    Factories.Add(InFactory);
    std::sort(Factories.begin(), Factories.end(),
        [](const auto& A, const auto& B)
        {
            return A->GetPriority() > B->GetPriority();
        }
    );
}

void UAssetManager::UnregisterFactory(UAssetFactory* InFactory)
{
    std::lock_guard<std::mutex> lock(Mutex);
    Factories.Erase(
        std::remove(Factories.begin(), Factories.end(), InFactory),
        Factories.end()
    );
}

// UAsset* UAssetManager::Load(const FString& InFilepath)
// {
//     std::filesystem::path Path = InFilepath;
//     FString Name = Path.stem().string();
//
//     // --- 여기서만 캐시 검사 ---
//     {
//         std::lock_guard<std::mutex> lock(Mutex);
//         UAsset** it = LoadedAssets.Find(Name);
//         if (it != nullptr)
//             return *it;
//     }
//     
//     // 2) 파일 존재 여부 확인
//     if (!std::filesystem::exists(Path))
//         return nullptr;
//
//     // 2) 파일 크기 검사 (최소 4바이트 만큼은 있어야 NameLen을 읽을 수 있음)
//     auto sz = std::filesystem::file_size(Path);
//     if (sz < sizeof(uint32))
//         return nullptr;
//
//     UAsset* Asset = nullptr;
//     const auto ext = Path.extension().string();
//     
//     // 3) 패키지(.uasset) 직접 로드
//     if (ext == ".uasset")
//     {
//         // Serializer를 통해 UObject 역직렬화
//         UObject* Raw = Serializer::LoadFromFile(Path);
//         if (Raw)
//         {
//             // UAsset 파생 클래스인지 검사 후 캐스팅
//             Asset = Cast<UAsset>(Raw);
//         }
//     }
//     else
//     {
//         // 4) 외부 포맷은 팩토리 임포트
//         for (UAssetFactory* Factory : Factories)
//         {
//             if (Factory->CanImport(InFilepath))
//             {
//                 Asset = Factory->ImportFromFile(InFilepath);
//                 break;
//             }
//         }
//     }
//
//     // 로드 성공 시 캐시에 저장
//     if (Asset)
//     {
//         std::lock_guard<std::mutex> lock(Mutex);
//         LoadedAssets[Name] = Asset;
//     }
//     return Asset;
// }

UAsset* UAssetManager::Load(UClass* ClassType, const FString& Path)
{
    namespace fs = std::filesystem;
    fs::path     fsPath(Path.ToWideString());
    FString      BaseName = fsPath.stem().string();

    // 1) 캐시 조회
    {
        std::lock_guard<std::mutex> lock(Mutex);
        if (UAsset** Cached = LoadedAssets.Find(BaseName))
        {
            if ((*Cached)->GetClass() == ClassType)
            {
                return *Cached;
            }
        }
    }

    // 2) 파일 존재 및 크기 확인
    if (!fs::exists(fsPath) || fs::file_size(fsPath) < sizeof(uint32))
        return nullptr;

    UAsset* Asset = nullptr;
    const auto ext = fsPath.extension().string();

    // 3) .ttalkak → Serializer 로드
    if (ext == ".ttalkak")
    {
        UObject* Raw = Serializer::LoadFromFile(fsPath);
        Asset = static_cast<UAsset*>(Raw);
        Asset->PostLoad();
    }
    // 4) 그 외 포맷 → 팩토리 임포트
    else
    {
        for (UAssetFactory* F : Factories)
        {
            if (F->CanImport(ClassType, Path))
            {
                Asset = F->ImportFromFile(Path);
                Asset->PostLoad();
                if (ClassType != UTexture::StaticClass())
                {
                    // 5) 새 파일명 + 확장자 조합
                    const FString NewFilename = BaseName + TEXT("_") + ClassType->GetName() + TEXT(".ttalkak");
                    // 6) 디렉터리 + 새 파일명 연결
                    const std::filesystem::path NewPath = (fsPath.parent_path() / NewFilename).generic_wstring();
                    Serializer::SaveToFile(Asset, NewPath);
                    Registry->RegisterNewFile(NewPath);
                }
            }
        }
    }
    
    // 7) 캐시에 저장
    if (Asset)
    {
        std::lock_guard<std::mutex> lock(Mutex);
        LoadedAssets.Add(BaseName, Asset);
        
        FAssetDescriptor desc;
        Registry->GetDescriptor(BaseName, desc);
        Asset->SetAssetDescriptor(desc);
    }
    
    return Asset;
}

void UAssetManager::Store(const FName& InName, UAsset* InAsset)
{
    if (InAsset)
    {
        std::lock_guard<std::mutex> lock(Mutex);
        LoadedAssets[InName] = InAsset;
    }
}

// UAsset* UAssetManager::Get(const FString& InName)
// {
//     // 1) Registry에서 Descriptor만 꺼내고
//     FAssetDescriptor desc;
//     if (!Registry || !Registry->GetDescriptor(InName, desc))
//         return nullptr;
//
//     // 2) Load 에 모든 캐시 검사+로드 로직 위임
//     return Load(desc.RelativePath);
// }

UAsset* UAssetManager::Get(UClass* ClassType, const FString& InName)
{
    // 1) Registry에서 Descriptor만 꺼내고
    FAssetDescriptor desc;
    const FString NewName = InName + "_" + ClassType->GetName();
    if (!Registry || !Registry->GetDescriptor(NewName, desc))
    {
        if (!Registry->GetDescriptor(InName, desc))
        {
            return nullptr;
        }
    }

    // 2) Load 에 모든 캐시 검사+로드 로직 위임
    UAsset* Asset = Load(ClassType, desc.AbsolutePath);
    
    return Asset;
}

void UAssetManager::Unload(const FString& InName)
{
    std::lock_guard<std::mutex> lock(Mutex);
    LoadedAssets.Remove(InName);
}

bool UAssetManager::SaveAsset(UObject* Root, const FString& Path)
{
    UAsset* ToSaveAsset = Cast<UAsset>(Root);
    // 1) 상위 디렉토리 확인 및 생성
    const std::filesystem::path fsPath(Path.ToWideString());
    const std::filesystem::path dir = fsPath.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }
    // 2) Serializer로 바이너리 직렬화 후 파일 쓰기
    const bool bOk = Serializer::SaveToFile(Root, Path);
    return bOk;
}

TArray<FAssetDescriptor> UAssetManager::GetDescriptorsByType(UClass* InClass)
{
    if (InClass == UTexture::StaticClass())
    {
        TArray<FAssetDescriptor> Result = Registry->GetDescriptorsByExtension(TEXT(".png"));
        Result.Append(Registry->GetDescriptorsByExtension(TEXT(".dds")));
        Result.Append(Registry->GetDescriptorsByExtension(TEXT(".tga")));
        Result.Append(Registry->GetDescriptorsByExtension(TEXT(".jpg")));
        Result.Append(Registry->GetDescriptorsByExtension(TEXT(".jpeg")));
        return Result;
    }

    return TArray<FAssetDescriptor>();
}

UAssetFactory* UAssetManager::FindFactoryForFile(UClass* InClass, const FString& filepath)
{
    std::string ext = std::filesystem::path(filepath).extension().string();
    std::lock_guard<std::mutex> lock(Mutex);
    for (const auto& factory : Factories)
    {
        // CanImport 내부에서 확장자 검사 등을 수행하도록 구현되어 있어야 함
        if (factory->CanImport(InClass, filepath))
        {
            return factory;
        }
    }
    return nullptr;
}
