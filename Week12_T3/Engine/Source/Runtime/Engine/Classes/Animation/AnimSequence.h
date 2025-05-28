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
    bool LoadFromFile(const FString& filepath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;

private:
    UPROPERTY(EditAnywhere, EAnimInterpolationType, InterpolationType, = EAnimInterpolationType::None)
};

