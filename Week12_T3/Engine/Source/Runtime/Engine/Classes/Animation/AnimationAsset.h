#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "CoreUObject/UObject/ObjectFactory.h"

class UAnimationAsset : public UObject
{
    DECLARE_CLASS(UAnimationAsset, UObject)
public:
    UAnimationAsset() = default;
    
    virtual void PostDuplicate() override;
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