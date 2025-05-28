#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "CoreUObject/UObject/ObjectFactory.h"
#include "Engine/Asset/Asset.h"

class UAnimationAsset : public UAsset
{
    DECLARE_CLASS(UAnimationAsset, UObject)
public:
    UAnimationAsset() = default;
    
    virtual void PostDuplicate() override;
    bool LoadFromFile(const FString& filepath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;
};

struct FAnimExtractContext
{

    double CurrentTime;
    bool bLooping;
    bool bExtractRootMotion;

    FAnimExtractContext(float InCurrentTime, bool bInLooping = true, bool bInExtractRootMotion = false)
        : CurrentTime(InCurrentTime), bLooping(bInLooping), bExtractRootMotion(bInExtractRootMotion)
    {
    }
};