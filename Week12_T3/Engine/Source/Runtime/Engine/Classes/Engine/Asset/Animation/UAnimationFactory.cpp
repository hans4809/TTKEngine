#include "UAnimationFactory.h"
#include "Animation/AnimSequence.h"

UAnimationFactory::UAnimationFactory()
{
    SetSupportedExtensions({".fbx"});
    SetSupportedClass(UAnimSequence::StaticClass());
    SetPriority(100);
}

UAnimationFactory::~UAnimationFactory()
{
}

UAsset* UAnimationFactory::ImportFromFile(const FString& InFilePath)
{
    UAnimSequence* NewAnimSequence = FObjectFactory::ConstructObject<UAnimSequence>(nullptr);
    if (!NewAnimSequence->LoadFromFile(InFilePath))
    {
        return nullptr;
    }

    return NewAnimSequence;
}

UAsset* UAnimationFactory::CreateNew(const FAssetDescriptor& desc)
{
    UAnimSequence* NewAnimSequence = FObjectFactory::ConstructObject<UAnimSequence>(nullptr);
    return NewAnimSequence;
}
