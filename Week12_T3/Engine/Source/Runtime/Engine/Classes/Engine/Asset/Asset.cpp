#include "Asset.h"

UAsset::UAsset()
    : Super()
{
    SetFlags(RF_DuplicateTransient);
}
