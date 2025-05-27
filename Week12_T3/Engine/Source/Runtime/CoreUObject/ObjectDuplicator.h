#pragma once
#include "Container/Map.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

struct FDuplicateParams
{
    UObject*    Source = nullptr;
    UObject*    DestOuter = nullptr;
    FName       DestName = NAME_None;
    uint32      FlagMask = RF_AllFlags;        // RF_DuplicateTransient 등 플래그 제어
    bool        bSkipOuterDuplication = true;  // Outer 객체는 얕은 참조
};

class FObjectDuplicator
{
    FDuplicateParams Params;
    TMap<UObject*, UObject*> DuplicatedMap;
public:
    FObjectDuplicator(const FDuplicateParams& In) : Params(In) {}

    UObject* DuplicateObject(UObject* Src);
};
