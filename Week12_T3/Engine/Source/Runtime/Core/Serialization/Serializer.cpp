#include "Serializer.h"

#include "Engine/Asset/Asset.h"
#include "Serialization/MemoryArchive.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

namespace Serializer
{
    // 객체를 캐시 (ID → 인스턴스)
    static TMap<int32, UObject*> ObjectCache;
}

void Serializer::Save(FArchive2& Ar, const UObject* Obj)
{
    uint32 ObjUUID = Obj->GetUUID();
    Ar.SerializeRaw(&ObjUUID, sizeof(ObjUUID));

    bool bFirstTime = !ObjectCache.Contains(ObjUUID);
    Ar.SerializeRaw(&bFirstTime, sizeof(bFirstTime));
    
    if (bFirstTime)
    {
        ObjectCache.Add(ObjUUID, const_cast<UObject*>(Obj));
        // 3-1) 클래스명 저장
        FString CName = Obj->GetClass()->GetName();
        int32 Len = CName.Len();
        Ar.SerializeRaw(&Len, sizeof(Len));
        Ar.SerializeRaw(GetData(CName), (Len) * sizeof(TCHAR));
        // 3-2) 실제 데이터 저장
        const_cast<UObject*>(Obj)->Serialize(Ar);
    }
}

UObject* Serializer::Load(FArchive2& Ar)
{
    // 1) ID 읽기
    uint32 ObjUUID;
    Ar.SerializeRaw(&ObjUUID, sizeof(ObjUUID));

    // 2) 최초 생성 여부 플래그 읽기
    bool bFirstTime = false;
    Ar.SerializeRaw(&bFirstTime, sizeof(bFirstTime));

    if (!bFirstTime)
    {
        return ObjectCache[ObjUUID];
    }
    
    // 1) 이름 길이 읽기
    int32 NameLen = 0;
    Ar.SerializeRaw(&NameLen, sizeof(NameLen));

    // 3) 클래스명 읽고 Construct
    FString NameBuf;
    NameBuf.Resize(NameLen);
    Ar.SerializeRaw(GetData(NameBuf), (NameLen) * sizeof(TCHAR));
    
    UClass* C = UClass::FindClass(NameBuf);
    if (!C)
    {
        return nullptr;
    }
    
    UObject* Obj = FObjectFactory::ConstructObject(C, nullptr);
    // 4) 캐시에 저장
    ObjectCache.Add(ObjUUID, Obj);
    // 5) 실제 데이터 역직렬화
    Obj->Serialize(Ar);
    return Obj;
}

UObject* Serializer::Duplicate(const UObject* Obj)
{
    TArray<uint8> Buf;
    FMemoryWriter2 Ar(Buf);
    Ar.Seek(0);
    Save(Ar, Obj);
    return Load(Ar);
}

bool Serializer::SaveToFile(const UObject* Obj, const std::filesystem::path& FilePath)
{
    ClearSerializationCache();
    
    TArray<uint8> Buf;
    FMemoryWriter2 Ar(Buf);
    Ar.Seek(0);
    
    Save(Ar, Obj);

    std::ofstream Out(FilePath, std::ios::binary);
    if (!Out.is_open()) return false;
    Out.write(reinterpret_cast<const char*>(Buf.GetData()), Buf.Num());
    return true;
}

UObject* Serializer::LoadFromFile(const std::filesystem::path& FilePath)
{
    ClearSerializationCache();
    
    std::ifstream In(FilePath, std::ios::binary | std::ios::ate);
    if (!In.is_open()) return nullptr;

    size_t Size = In.tellg();
    In.seekg(0);

    TArray<uint8> Buffer;
    Buffer.SetNum(Size);
    In.read(reinterpret_cast<char*>(Buffer.GetData()), Size);

    FMemoryReader2 Reader(Buffer);
    
    return Load(Reader);
}

void Serializer::ClearSerializationCache()
{
    ObjectCache.Empty();
}
