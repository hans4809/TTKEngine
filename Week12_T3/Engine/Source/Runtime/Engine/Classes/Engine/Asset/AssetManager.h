#pragma once
#include <mutex>

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Asset.h"
#include "AssetFactory.h"
#include "AssetRegistry.h"
#include "Serialization/Serializer.h"
#include "UObject/Casts.h"

class UAssetRegistry;
class UAssetFactory;

struct FAssetInfo
{
    FName AssetName;      // Asset의 이름
    FName PackagePath;    // Asset의 패키지 경로
    EAssetType AssetType; // Asset의 타입
    uint32 Size;          // Asset의 크기 (바이트 단위)
};

struct FAssetRegistry
{
    TMap<FName, FAssetInfo> PathNameToAssetInfo;
};

class UAssetManager : public UObject
{
    DECLARE_CLASS(UAssetManager, UObject)

private:
    std::unique_ptr<FAssetRegistry> AssetRegistry;

public:
    UAssetManager() = default;
    ~UAssetManager();

    static bool IsInitialized();

    /** UAssetManager를 가져옵니다. */
    static UAssetManager& Get();

    /** UAssetManager가 존재하면 가져오고, 없으면 nullptr를 반환합니다. */
    static UAssetManager* GetIfInitialized();

    void Initalize();

    // 팩토리 등록/해제
    void RegisterFactory(UAssetFactory* InFactory);
    void UnregisterFactory(UAssetFactory* InFactory);

    // ClassType을 템플릿으로 받아서 원하는 타입으로 리턴
    template<typename T>
    T* Load(const FString& Path);

    UAsset* Load(UClass* ClassType, const FString& Path);
    
    // 에셋 로드 (캐시 적용)
    //* Load(const FString& InFilepath);
    
    // 캐시 등록
    void Store(const FName& InName, UAsset* InAsset);
    
    // 에셋 조회
    template<typename T>
    T* Get(const FString& InName);

    UAsset* Get(UClass* ClassType, const FString& InName);
    
    // 에셋 언로드
    void Unload(const FString& InName);

    // 루트 오브젝트를 디스크에 .uasset/.umap 형태로 저장
    bool SaveAsset(UObject* Root, const FString& Path);

    TMap<FName, UAsset*> GetLoadedAssetsByType(UClass* InClass) const
    {
        TMap<FName, UAsset*> Result;
        for (const auto& Pair : LoadedAssets)
        {
            if (UAsset* Asset = Cast<UAsset>(Pair.Value))
            {
                if (Asset->GetClass() == InClass)
                {
                    Result.Add(Pair.Key, Asset);
                }
            }
        }
        return Result;
    }

    TArray<FAssetDescriptor> GetDescriptorsByType(UClass* InClass);

    UAssetRegistry* GetRegistry() const { return Registry; }
public:
    UAssetFactory* FindFactoryForFile(UClass* InClass, const FString& filepath);

private:
    UAssetRegistry* Registry;
    TArray<UAssetFactory*> Factories;
    TMap<FName, UAsset*> LoadedAssets;

    mutable std::mutex Mutex;  // 스레드 안전 보장
};

template <typename T>
T* UAssetManager::Load(const FString& Path)
{
    return Cast<T>(Load(T::StaticClass(), Path));
}

template <typename T>
T* UAssetManager::Get(const FString& InName)
{
    return Cast<T>(Get(T::StaticClass(), InName));
}
