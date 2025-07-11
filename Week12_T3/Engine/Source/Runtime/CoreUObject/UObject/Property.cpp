#include "Property.h"
#include "Class.h"
#include "ScriptStruct.h"
#include "Engine/Asset/Asset.h"
#include "Serialization/Archive2.h"
#include "UserInterface/Console.h"


void FProperty::Resolve()
{
}

void FProperty::Serialize(FArchive2& Ar, void* DataPtr) const
{
    Ar.SerializeRaw(DataPtr, Size);
}

void FProperty::CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const
{
    FPlatformMemory::Memcpy(DstPtr, SrcPtr, Size);
}

void FUnresolvedPtrProperty::Resolve()
{
    FProperty::Resolve();

    if (Type == EPropertyType::Object)
    {
        return;
    }

    if (Type == EPropertyType::UnresolvedPointer)
    {
        if (std::holds_alternative<FName>(TypeSpecificData))
        {
            const FName& TypeName = std::get<FName>(TypeSpecificData);

            // ClassMap에서 먼저 검사
            if (UClass* FoundClass = UClass::FindClass(TypeName))
            {
                if (FoundClass->IsChildOf(UAsset::StaticClass()))
                {
                    Type = EPropertyType::Asset;
                    TypeSpecificData = FoundClass;
                    ResolvedProperty = new FAssetProperty{ OwnerStruct, Name, Size, Offset, Flags };
                }
                else
                {
                    Type = EPropertyType::Object;
                    TypeSpecificData = FoundClass;
                    ResolvedProperty = new FObjectProperty{ OwnerStruct, Name, Size, Offset, Flags };
                }
                return;
            }

            // StructMap에서 검사
            if (UScriptStruct* FoundStruct = UScriptStruct::FindScriptStruct(TypeName))
            {
                Type = EPropertyType::Struct;
                TypeSpecificData = FoundStruct;
                ResolvedProperty = new FStructProperty{ OwnerStruct, Name, Size, Offset, Flags };
                return;
            }
        }
    }

    Type = EPropertyType::Unknown;
    TypeSpecificData = std::monostate{};
    UE_LOG(LogLevel::Error, "Unknown Property Type: %s", Name);
}
