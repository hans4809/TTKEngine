#pragma once

#include "Engine/Asset/Asset.h"
#include "Skeletal/SkeletalDefine.h"

struct FRefSkeletal;

class USkeleton : public UAsset
{
    DECLARE_CLASS(USkeleton, UObject)
public:
    USkeleton() = default;
    
    FRefSkeletal& GetRefSkeletal() { return RefSkeletal; }
    void SetRefSkeletal(const FRefSkeletal& InRefSkeletal) { RefSkeletal = InRefSkeletal; }
    
    bool LoadFromFile(const FString& filepath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;

private:
    UPROPERTY(EditAnywhere, FRefSkeletal, RefSkeletal, = {})
};
