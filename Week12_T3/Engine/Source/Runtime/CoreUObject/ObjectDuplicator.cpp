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

    // 3) 새 인스턴스 생성 및 맵에 등록
    UObject* NewObj = FObjectFactory::ConstructObject(Src->GetClass(), Params.DestOuter, Params.DestName);
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
    NewObj->DuplicateSubObjects(Src, Params.DestOuter, *this);
    NewObj->PostDuplicate();

    return NewObj;
}
