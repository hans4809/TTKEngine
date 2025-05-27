#include "ObjectDuplicator.h"

#include "Serialization/Serializer.h"
#include "UObject/ObjectFactory.h"

UObject* FObjectDuplicator::DuplicateObject(UObject* Src)
{
    // 1) 이미 복제됐으면 재사용
    if (DuplicatedMap.Contains(Src) == true)
    {
        return DuplicatedMap[Src];
    }

    // 2) RF_DuplicateTransient 검사(얕은 참조 처리)
    if (Src->HasAnyFlags(RF_DuplicateTransient))
        return Src;
    
    // ——— DestOuter 결정 ———
    UObject* SrcOuter = Src->GetOuter();
    UObject* DestOuter = nullptr;
    if (SrcOuter && DuplicatedMap.Contains(SrcOuter))
    {
        // SrcOuter가 이미 복제됐다면 그 복제본을 Outer로 사용
        DestOuter = DuplicatedMap[SrcOuter];
    }
    else
    {
        // 그렇지 않으면 루트 DestOuter
        DestOuter = Params.DestOuter;
    }
    
    // ——— DestName 결정 ———
    FName DestName;
    if (Src == Params.Source)
    {
        // 루트 객체
        DestName = Params.DestName;
    }
    else
    {
        // 중첩된 객체: 원본 이름 그대로
        DestName = TEXT("CopyOf_") + Src->GetName();
    }
    

    // 3) 새 인스턴스 생성 및 맵에 등록
    UObject* NewObj = FObjectFactory::ConstructObject(Src->GetClass(), DestOuter, DestName);
    NewObj->ObjectFlags = Src->ObjectFlags;
    DuplicatedMap[Src] = NewObj;

    // 4) 프로퍼티별 복제
    for (UStruct* StructIter = Src->GetClass(); StructIter; StructIter = StructIter->GetSuperStruct())
    {
        for (FProperty* Prop : StructIter->GetProperties())
        {
            // CPF_DuplicateTransient 검사
            if (HasAnyFlags(Prop->Flags, EPropertyFlags::DuplicateTransient))
            {
                void* SrcPtr = reinterpret_cast<void*>(reinterpret_cast<std::byte*>(Src) + Prop->Offset);
                void* DstPtr = reinterpret_cast<void*>(reinterpret_cast<std::byte*>(NewObj) + Prop->Offset);
                FPlatformMemory::Memcpy(DstPtr, SrcPtr, static_cast<size_t>(Prop->Size));
                continue;
            }

            // 원본/대상 주소 계산
            void* SrcPtr = reinterpret_cast<void*>(reinterpret_cast<std::byte*>(Src) + Prop->Offset);
            void* DstPtr = reinterpret_cast<void*>(reinterpret_cast<std::byte*>(NewObj) + Prop->Offset);

            // 타입별 복제 로직 위임
            Prop->CopyData(SrcPtr, DstPtr, *this);
        }
    }

    // 5) 서브오브젝트 복제 및 후처리 훅
    NewObj->DuplicateSubObjects(Src, DestOuter, *this);
    NewObj->PostDuplicate();

    return NewObj;
}
