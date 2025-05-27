#pragma once
#include "NameTypes.h"
#include "Container/String.h"
#include "sol/sol.hpp"

class FObjectDuplicator;
class FArchive2;
class UWorld;
class UClass;
class AActor;
class UActorComponent;
class FFunctionRegistry;

enum EObjectFlags
{
    // Do not add new flags unless they truly belong here. There are alternatives.
    // if you change any the bit of any of the RF_Load flags, then you will need legacy serialization
    RF_NoFlags					= 0x00000000,	///< No flags, used to avoid a cast

    // This first group of flags mostly has to do with what kind of object it is. Other than transient, these are the persistent object flags.
    // The garbage collector also tends to look at these.
    RF_Public					=0x00000001,	///< Object is visible outside its package.

    RF_Transactional			=0x00000008,	///< Object is transactional.
    
    RF_Transient				=0x00000040,	///< Don't save object.
    
    RF_DuplicateTransient		=0x00800000,	///< Object should not be included in any type of duplication (copy/paste, binary duplication, etc.)
};

#define RF_AllFlags				(EObjectFlags)0xffffffff	///< All flags, used mainly for error checking

class FProperty;
// for sol2 typing
namespace SolTypeBinding
{
    template<typename... Types>
    struct TypeList {};

    // PushBack
    template<typename List, typename NewType>
    struct PushBack;

    // Pushback
    template<typename... Types, typename NewType>
    struct PushBack<TypeList<Types...>, NewType> {
        using type = TypeList<Types..., NewType>;
    };

    // Base 클래스를 상속하는 모든 타입을 리스트로 모은다
    template<typename Derived, typename Base = void>
    struct InheritList;

    // Base 없는 경우 (Root 클래스)
    template<typename Derived>
    struct InheritList<Derived, void> {
        using type = TypeList<Derived>;
    };

    // Base 있는 경우 (Derived -> Base)
    template<typename Derived, typename Base>
    struct InheritList {
        using base_list = typename Base::InheritTypes;
        using type = typename PushBack<base_list, Derived>::type;
    };

    // to unpack types
    template<typename TypeList>
    struct TypeListToBases;

    // to unpack types
    template<typename... Types>
    struct TypeListToBases<TypeList<Types...>> {
        static auto Get() {
            return sol::bases<Types...>();
        }
    };

    // for Register to AActor::GetComponentByClass
    template <typename T>
    constexpr bool IsCompleteType_v = requires { sizeof(T); };

    // Register to AActor::GetComponentByClass
    template <typename T>
    void RegisterGetComponentByClass(sol::state& lua, std::string className)
    {
        if constexpr (IsCompleteType_v<AActor> && IsCompleteType_v<UActorComponent> && std::derived_from<T, UActorComponent>)
        {
            // 암시적 형변환에서 AActor가 완전한 타입임을 요구해서 명시적으로 형변환.
            using FuncType = T * (AActor::*)();
            auto funcPtr = static_cast<FuncType>(&AActor::template GetComponentByClass<T>);
            AActor::GetLuaUserType(lua)["Get" + className] = funcPtr;
            std::cout << "Register AActor::Get" << className << std::endl;
        }
        else
        {
            std::cout << "Failed Register AActor::Get" << className << std::endl;
        }
    }
}

class UObject
{
private:
    UObject& operator=(const UObject&) = delete;
    UObject(UObject&&) = delete;
    UObject& operator=(UObject&&) = delete;

public:
    using InheritTypes = SolTypeBinding::InheritList<UObject>::type;
    using Super = UObject;
    using ThisClass = UObject;
    UObject(const UObject& Other)
        : UUID(0)
        , InternalIndex(Other.InternalIndex)
        , NamePrivate(Other.NamePrivate)
        , ClassPrivate(Other.ClassPrivate)
    {
    }

    static UClass* StaticClass();
    static FFunctionRegistry* FunctionRegistry();

    virtual UObject* Duplicate(UObject* InOuter);
    
    virtual void PostEditChangeProperty(const FProperty* PropertyThatChanged) {};
    // Src가 런타임에 직접 생성·소유한 자식 UObject들(예: 컴포넌트, 인라인 서브오브젝트)을 별도로 복제
    virtual void DuplicateSubObjects(const UObject* Source, UObject* InOuter, FObjectDuplicator& Duplicator) {} // 하위 클래스에서 override
    // 복제가 전부 끝난 뒤 “초기화가 필요한 상태”를 정리하고, 사용자 정의 후처리(override) 콜백을 호출
    virtual void PostDuplicate() {}
private:
    friend class FObjectFactory;
    friend class FObjectDuplicator;
    friend class FSceneMgr;
    friend class UClass;
    friend class UStruct;

    uint32 UUID;
    uint32 InternalIndex; // Index of GUObjectArray

    FName NamePrivate;
    UClass* ClassPrivate = nullptr;
    UObject* OuterPrivate = nullptr;
    
    uint32 ObjectFlags = RF_NoFlags;
public:
    UObject();
    virtual ~UObject();

    UObject* GetOuter() const;
    virtual UWorld* GetWorld() const;

    FName GetFName() const { return NamePrivate; }
    FString GetName() const { return NamePrivate.ToString(); }

    //TODO 이름 바꾸면 FNAME을 키값으로 하는 자료구조 모두 반영해야됨
    void SetFName(const FName& Name)
    {
        NamePrivate = Name;
    }

    uint32 GetUUID() const { return UUID; }
    uint32 GetInternalIndex() const { return InternalIndex; }

    UClass* GetClass() const { return ClassPrivate; }


    /** this가 SomeBase인지, SomeBase의 자식 클래스인지 확인합니다. */
    bool IsA(const UClass* SomeBase) const;

    template <typename T>
        requires std::derived_from<T, UObject>
    bool IsA() const
    {
        return IsA(T::StaticClass());
    }
    void MarkAsGarbage();

    bool HasAnyFlags(const uint32 Flags) const    { return (ObjectFlags & Flags) != 0; }
    void SetFlags   (const uint32 Flags)          { ObjectFlags |= Flags; }
    void ClearFlags (const uint32 Flags)          { ObjectFlags &= ~Flags; }

public:
    //void* operator new(size_t Size);

    //void operator delete(void* Ptr, size_t Size);
public:
    // Serialize
    /** 이 객체의 UPROPERTY들을 읽거나 쓰는 직렬화 함수 */
    virtual void Serialize(FArchive2& Ar);
};
