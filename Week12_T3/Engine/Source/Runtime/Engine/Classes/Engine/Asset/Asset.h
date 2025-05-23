#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

enum class EAssetType : uint8
{
    StaticMesh,
    SkeletalMesh,
    Texture2D,
    Material,
    Curve,
};

struct FAssetDescriptor 
{
    DECLARE_STRUCT(FAssetDescriptor)

    UPROPERTY(EditAnywhere, FName, AssetName, = TEXT("")) // 에셋 식별자
    UPROPERTY(EditAnywhere, FString, RelativePath, = TEXT("")) // 상대 경로
    UPROPERTY(EditAnywhere, FString, AbsolutePath, = TEXT("")) // 절대 경로
    UPROPERTY(EditAnywhere, FString, AssetExtension, = TEXT("")) // 에셋 타입명(예: "Texture", "Mesh")
    UPROPERTY(EditAnywhere, size_t, Size, = 0) // 파일 크기 또는 메모리 사용량 (바이트 단위)
    
    std::filesystem::file_time_type CreateDate;                // 생성 타임스탬프 (Unix epoch)
    std::filesystem::file_time_type UpdateDate;                // 수정 타임스탬프
};

class UAsset : public UObject
{
    DECLARE_CLASS(UAsset, UObject)
public:
    UAsset();
    virtual ~UAsset() = default;
    FAssetDescriptor& GetDescriptor() { return Descriptor; }
    virtual bool LoadFromFile(const FString& filepath) { return true; }

    virtual bool SerializeToFile(std::ostream& Out) { return true; }
    virtual bool DeserializeFromFile(std::istream& In) { return true; }

protected:
    UPROPERTY(EditAnywhere, FAssetDescriptor, Descriptor, = {})
};