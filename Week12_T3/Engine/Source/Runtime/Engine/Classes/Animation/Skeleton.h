#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Skeletal/SkeletalDefine.h"

struct FRefSkeletal;
class USkeleton : public UObject
{
    DECLARE_CLASS(USkeleton, UObject)
public:
    USkeleton() = default;
    
    FRefSkeletal& GetRefSkeletal() { return RefSkeletal; }
    void SetRefSkeletal(const FRefSkeletal& InRefSkeletal) { RefSkeletal = InRefSkeletal; }
private:
    FRefSkeletal RefSkeletal;
};
