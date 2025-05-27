#pragma once
#include "AnimSequenceBase.h"

struct FRefSkeletal;
class FTransform;

class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)
public:
    UAnimSequence() = default;
    
    virtual void PostDuplicate() override;
    
private:
    EAnimInterpolationType InterpolationType;
};

