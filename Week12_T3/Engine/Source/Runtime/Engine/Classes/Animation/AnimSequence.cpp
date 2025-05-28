#include "AnimSequence.h"
#include "AnimData/AnimDataModel.h"


void UAnimSequence::PostDuplicate()
{
}

bool UAnimSequence::LoadFromFile(const FString& FilePath)
{
    DataModel = FObjectFactory::ConstructObject<UAnimDataModel>(this);
    if (FFBXLoader::ParseAnimationFromFBX(FilePath, DataModel) == false)
        return false;

    return true;
}

bool UAnimSequence::SerializeToFile(std::ostream& Out)
{
    return UAnimSequenceBase::SerializeToFile(Out);
}

bool UAnimSequence::DeserializeFromFile(std::istream& In)
{
    return UAnimSequenceBase::DeserializeFromFile(In);
}
