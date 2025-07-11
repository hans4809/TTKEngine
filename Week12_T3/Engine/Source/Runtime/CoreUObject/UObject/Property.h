#pragma once
#include <variant>
#include <optional>

#include "PropertyTypes.h"
#include "Struct.h"
#include "Container/Queue.h"
#include "Serialization/Archive2.h"
#include "Templates/TemplateUtilities.h"
#include "Templates/TypeUtilities.h"
#include "ThirdParty/include/ImGUI/imgui.h"

class UScriptStruct;
class FArchive2;

struct FProperty
{
    /**
     * 클래스의 프로퍼티를 등록합니다.
     * @param InOwnerStruct 이 프로퍼티를 가지고 있는 UStruct
     * @param InPropertyName 프로퍼티의 이름
     * @param InType 프로퍼티의 타입
     * @param InSize 프로퍼티의 크기
     * @param InOffset 프로퍼티가 클래스로부터 떨어진 거리
     * @param InFlags Reflection 관련 Flag
     */
    FProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        EPropertyType InType,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : OwnerStruct(InOwnerStruct)
        , Name(InPropertyName)
        , Type(InType)
        , Size(InSize)
        , Offset(InOffset)
        , Flags(InFlags)
    {
    }

    virtual ~FProperty() = default;
    FProperty(const FProperty&) = delete;
    FProperty& operator=(const FProperty&) = delete;
    FProperty(FProperty&&) = delete;
    FProperty& operator=(FProperty&&) = delete;

public:
    /** ImGui에 각 프로퍼티에 맞는 UI를 띄웁니다. */
    virtual void DisplayInImGui(UObject* Object) const;

    /**
     * ImGui를 통해 주어진 Raw Data를 표시합니다.
     * @param PropertyLabel 표시할 프로퍼티의 레이블
     * @param DataPtr 표시할 Raw Data의 포인터
     */
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const;

    /** 런타임에 타입 정보를 검사하고 업데이트 합니다. */
    virtual void Resolve();

    /**
     * 이 프로퍼티가 EPropertyType::Object 타입일 경우, 해당 UObject의 UClass*를 반환합니다.
     * 그렇지 않거나 아직 해결되지 않은 경우 nullptr을 반환합니다.
     * @return UClass* 또는 nullptr
     */
    FORCEINLINE UClass* GetSpecificClass() const
    {
        return GetTypeSpecificDataAs<UClass*>().value_or(nullptr);
    }

    /**
     * Property를 실제 데이터 타입으로 변환합니다.
     * @tparam T 변환할 타입
     * @param Object 변환할 값을 가지고 있는 Object
     * @return Object의 실제 값
     *
     * @warning 타입이 잘못되면 UB가 발생할 수 있습니다.
     */
    template <typename T>
    T* GetPropertyData(UObject* Object) const
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(Object) + Offset);
    }

    void* GetPropertyData(UObject* Object) const
    {
        return reinterpret_cast<std::byte*>(Object) + Offset;
    }

    /** 이 프로퍼티의 메모리 블록을 읽거나 씁니다. */
    virtual void Serialize(FArchive2& Ar, void* DataPtr) const;

    virtual void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const;
private:
    /**
     * TypeSpecificData에서 특정 타입 T의 값을 안전하게 가져옵니다.
     * 만약 현재 저장된 타입이 T가 아니거나, Type이 해당 T를 가질 수 있는
     * EPropertyType 상태가 아니라면 std::nullopt를 반환합니다.
     *
     * @tparam T 가져오고자 하는 타입 (예: UClass*, FStructInfo*, FName)
     * @return 해당 타입의 값을 담은 std::optional<T>, 또는 std::nullopt
     */
    template <typename T>
    [[nodiscard]] std::optional<T> GetTypeSpecificDataAs() const
    {
        if constexpr (std::same_as<T, UClass*>)
        {
            switch (Type)  // NOLINT(clang-diagnostic-switch-enum)
            {
                // Type이 Object와 SubclassOf 일 때만 UClass*가 유효
            case EPropertyType::Asset:
            case EPropertyType::Object:
            case EPropertyType::SubclassOf:
                break;
            default:
                return std::nullopt;
            }
        }
        // else if constexpr (std::same_as<T, FStructInfo*>) {
        //     if (Type != EPropertyType::Struct) { // Struct 타입일 때만 FStructInfo*가 유효
        //         return std::nullopt;
        //     }
        // }
        else if constexpr (std::same_as<T, FName>)
        {
            if (Type != EPropertyType::UnresolvedPointer /* && Type != EPropertyType::UnresolvedStruct 등 */)
            {
                return std::nullopt;
            }
        }

        // 실제 variant에서 값 가져오기 시도
        if (std::holds_alternative<T>(TypeSpecificData))
        {
            return std::get<T>(TypeSpecificData);
        }
        return std::nullopt;
    }

public:
    UStruct* OwnerStruct;

    const char* Name;
    EPropertyType Type;
    int64 Size;
    int64 Offset;
    EPropertyFlags Flags;

public:
    std::variant<
        std::monostate,  // 현재 값이 없음을 나타냄
        UClass*,         // FProperty의 Type이 Object일때 원본 Property를 가지고 있는 UClass
        UScriptStruct*,  // FProperty의 Type이 Struct일때 커스텀 구조체의 정보
        FName            // FProperty의 Type이 UnresolvedPointer일 때 런타임에 검사할 UClass 이름
    > TypeSpecificData;
};

/* Property를 Display할 떄 부모순으로 출력할 수 있도록 도와주는 Trait Class*/
class FDisplayMemberRecursiveTrait
{
protected:
    /*
     *   구조체와 연결된 멤버를 재귀적으로 디스플레이 합니다.
     *  @param CurrentStructToDisplay 현재 디스플레이할 UStruct.
     *  @param StructInstanceDataPtr 구조체 인스턴스 데이터에 대한 포인터.
     *  @param TopLevelOwnerObject 최상위 소유자 객체.
    */
    void DisplayMemberRecursive(const UStruct* CurrentStructToDisplay, void* StructInstanceDataPtr, UObject* TopLevelOwnerObject) const;

};


struct FNumericProperty : public FProperty
{
    FNumericProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        EPropertyType InType,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, InType, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
};

struct FInt8Property : public FNumericProperty
{
    FInt8Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::Int8, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FInt16Property : public FNumericProperty
{
    FInt16Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::Int16, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FInt32Property : public FNumericProperty
{
    FInt32Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::Int32, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FInt64Property : public FNumericProperty
{
    FInt64Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::Int64, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FUInt8Property : public FNumericProperty
{
    FUInt8Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::UInt8, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FUInt16Property : public FNumericProperty
{
    FUInt16Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::UInt16, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FUInt32Property : public FNumericProperty
{
    FUInt32Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::UInt32, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FUInt64Property : public FNumericProperty
{
    FUInt64Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::UInt64, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FFloatProperty : public FNumericProperty
{
    FFloatProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::Float, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FDoubleProperty : public FNumericProperty
{
    FDoubleProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FNumericProperty(InOwnerStruct, InPropertyName, EPropertyType::Double, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FBoolProperty : public FProperty
{
    FBoolProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Bool, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
};

struct FStrProperty : public FProperty
{
    FStrProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::String, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

struct FNameProperty : public FProperty
{
    FNameProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Name, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FVector2DProperty : public FProperty
{
    FVector2DProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Vector2D, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FVectorProperty : public FProperty
{
    FVectorProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Vector, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FVector4Property : public FProperty
{
    FVector4Property(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Vector4, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FRotatorProperty : public FProperty
{
    FRotatorProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Rotator, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FQuatProperty : public FProperty
{
    FQuatProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Quat, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FTransformProperty : public FProperty
{
    FTransformProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Transform, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FMatrixProperty : public FProperty
{
    FMatrixProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Matrix, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FColorProperty : public FProperty
{
    FColorProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Color, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FLinearColorProperty : public FProperty
{
    FLinearColorProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::LinearColor, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

template <typename InArrayType>
struct TArrayProperty : public FProperty
{
    using ElementType = typename InArrayType::ElementType;

    FProperty* ElementProperty = nullptr;

    TArrayProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Array, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override
    {
        FProperty::DisplayRawDataInImGui(PropertyLabel, DataPtr);

        enum class EArrayElementOption : uint8
        {
            Insert,
            Remove,
            Duplicate,
        };

        if (ImGui::TreeNode(PropertyLabel))
        {
            ImGui::BeginDisabled(HasAnyFlags(Flags, EPropertyFlags::VisibleAnywhere));
            {
                TArray<ElementType>* Data = static_cast<TArray<ElementType>*>(DataPtr);

                const ImGuiIO& IO = ImGui::GetIO();
                ImFont* IconFont = IO.Fonts->Fonts[1]; // FEATHER_FONT = 1

                ImGui::Text("Num of Elements: %d", Data->Num());

                ImGui::SameLine();
                ImGui::PushFont(IconFont);
                if (ImGui::Button("\ue9c8"))
                {
                    Data->AddDefaulted();
                }
                ImGui::PopFont();
                ImGui::SetItemTooltip("Add Element");

                ImGui::SameLine();
                ImGui::PushFont(IconFont);
                if (ImGui::Button("\ue9f6"))
                {
                    Data->Empty();
                }
                ImGui::PopFont();
                ImGui::SetItemTooltip("Remove All Elements");

                TQueue<TPair<EArrayElementOption, int32>> OptionQueue;
                for (int32 Index = 0; Index < Data->Num(); ++Index)
                {
                    ElementProperty->DisplayRawDataInImGui(std::format("Idx [{}]", Index).c_str(), &((*Data)[Index]));

                    std::string PopupLabel = std::format("ArrayElementOption##{}", Index);
                    ImGui::SameLine();
                    if (ImGui::Button(std::format("...##{}", Index).c_str()))
                    {
                        ImGui::OpenPopup(PopupLabel.c_str());
                    }

                    if (ImGui::BeginPopup(PopupLabel.c_str()))
                    {
                        if (ImGui::Selectable("Insert"))
                        {
                            OptionQueue.Enqueue(MakePair(EArrayElementOption::Insert, Index));
                        }
                        else if (ImGui::Selectable("Remove"))
                        {
                            OptionQueue.Enqueue(MakePair(EArrayElementOption::Remove, Index));
                        }
                        else if (ImGui::Selectable("Duplicate"))
                        {
                            OptionQueue.Enqueue(MakePair(EArrayElementOption::Duplicate, Index));
                        }
                        ImGui::EndPopup();
                    }
                }

                TPair<EArrayElementOption, int32> Option;
                while (OptionQueue.Dequeue(Option))
                {
                    switch (Option.Key)
                    {
                    case EArrayElementOption::Insert:
                    {
                        Data->Insert(ElementType{}, Option.Value);
                        break;
                    }
                    case EArrayElementOption::Remove:
                    {
                        Data->RemoveAt(Option.Value);
                        break;
                    }
                    case EArrayElementOption::Duplicate:
                    {
                        Data->Insert((*Data)[Option.Value], Option.Value);
                    }
                    default:
                        break;
                    }
                }
            }
            ImGui::EndDisabled();
            ImGui::TreePop();
        }
    }

    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

template <typename InArrayType>
void TArrayProperty<InArrayType>::Serialize(FArchive2& Ar, void* DataPtr) const
{
    InArrayType& Arr = *reinterpret_cast<InArrayType*>(reinterpret_cast<char*>(DataPtr));

    if (Ar.IsSaving())
    {
        int32 Count = Arr.Num();
        Ar.SerializeRaw(&Count, sizeof(Count));

        for (int i = 0; i < Count; ++i)
        {
            void* ElemPtr = &Arr[i];
            ElementProperty->Serialize(Ar, ElemPtr);
        }
    }
    else
    {
        int32 Count = 0;
        Ar.SerializeRaw(&Count, sizeof(Count));
        Arr.Empty();
        Arr.Reserve(Count);

        for (int i = 0; i < Count; ++i)
        {
            Arr.AddDefaulted();
            void* ElemPtr = &Arr[i];
            ElementProperty->Serialize(Ar, ElemPtr);
        }
    }
}

template <typename InArrayType>
void TArrayProperty<InArrayType>::CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const
{
    // 원본·대상 배열 참조 얻기
    const InArrayType& SrcArr = *reinterpret_cast<const InArrayType*>(SrcPtr);
    InArrayType&       DstArr = *reinterpret_cast<InArrayType*>(DstPtr);

    // 대상 배열 초기화 및 예약
    DstArr.Empty();
    DstArr.Reserve(SrcArr.Num());

    // 요소별 복제
    for (int32 i = 0; i < SrcArr.Num(); ++i)
    {
        // 새 요소 추가 (기본값 생성)
        DstArr.AddDefaulted();

        // 원본·대상 요소 포인터
        const void* ElemSrcPtr = &SrcArr[i];
        void*       ElemDstPtr = &DstArr[i];

        // ElementProperty에 위임하여 복제
        ElementProperty->CopyData(ElemSrcPtr, ElemDstPtr, Duplicator);
    }
}

template <typename InMapType>
struct TMapProperty : public FProperty
{
    using KeyType = typename InMapType::KeyType;
    using ValueType = typename InMapType::ValueType;

    FProperty* KeyProperty = nullptr;
    FProperty* ValueProperty = nullptr;

    TMapProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Map, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override
    {
        FProperty::DisplayRawDataInImGui(PropertyLabel, DataPtr);

        enum class EMapElementOption : uint8
        {
            Remove,
        };

        TMap<KeyType, ValueType>* Data = static_cast<TMap<KeyType, ValueType>*>(DataPtr);

        if (ImGui::TreeNode(PropertyLabel))
        {
            ImGui::BeginDisabled(HasAnyFlags(Flags, EPropertyFlags::VisibleAnywhere));
            {
                const ImGuiIO& IO = ImGui::GetIO();
                ImFont* IconFont = IO.Fonts->Fonts[1]; // FEATHER_FONT = 1

                ImGui::Text("Num of Elements: %d", Data->Num());

                ImGui::SameLine();
                ImGui::PushFont(IconFont);
                if (ImGui::Button("\ue9c8"))
                {
                    const KeyType DefaultKeyToAdd = KeyType{};
                    if (Data->Contains(DefaultKeyToAdd))
                    {
                        ImGui::OpenPopup("Duplicate Key Warning");
                    }
                    else
                    {
                        Data->Add(DefaultKeyToAdd, ValueType{});
                    }
                }
                ImGui::PopFont();
                ImGui::SetItemTooltip("Add Element");

                // --- 중복 키 경고 팝업 정의 ---
                // "Duplicate Key Warning" ID는 위 ImGui::OpenPopup 호출과 일치해야 함
                if (ImGui::BeginPopupModal("Duplicate Key Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("The key you are trying to add already exists in the map.\nPlease use a unique key.");
                    ImGui::Separator();

                    // 팝업을 닫는 OK 버튼
                    if (ImGui::Button("OK", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus(); // OK 버튼에 기본 포커스
                    ImGui::EndPopup();
                }
                // --- 중복 키 경고 팝업 정의 ---

                ImGui::SameLine();
                ImGui::PushFont(IconFont);
                if (ImGui::Button("\ue9f6"))
                {
                    Data->Empty();
                }
                ImGui::PopFont();
                ImGui::SetItemTooltip("Remove All Elements");

                TQueue<TPair<EMapElementOption, KeyType>> OptionQueue;
                for (int32 Idx = 0; auto & Pair : *Data)
                {
                    ImGui::PushID(&Pair);

                    const bool bIsOpen = ImGui::TreeNode(std::format("Elem [{}]", Idx).c_str());

                    std::string PopupLabel = std::format("MapElementOption##{}", Idx);
                    ImGui::SameLine();
                    if (ImGui::Button(std::format("...##{}", Idx).c_str()))
                    {
                        ImGui::OpenPopup(PopupLabel.c_str());
                    }

                    if (ImGui::BeginPopup(PopupLabel.c_str()))
                    {
                        if (ImGui::Selectable("Remove"))
                        {
                            OptionQueue.Enqueue(MakePair(EMapElementOption::Remove, Pair.Key));
                        }
                        ImGui::EndPopup();
                    }

                    if (bIsOpen)
                    {
                        constexpr std::string_view KeyTypeNameView = GetTypeNameString<KeyType>();
                        std::string KeyTypeName = std::string(KeyTypeNameView);
                        if (ImGui::TreeNode(std::format("Key ({})", KeyTypeName).c_str()))
                        {
                            // TODO: 겹치는 Key에 대해서는 나중에 수정해야함
                            KeyProperty->DisplayRawDataInImGui("", &const_cast<KeyType&>(Pair.Key));
                            ImGui::TreePop();
                        }

                        constexpr std::string_view ValueTypeNameView = GetTypeNameString<ValueType>();
                        std::string ValueTypeName = std::string(ValueTypeNameView);
                        if (ImGui::TreeNode(std::format("Value ({})", ValueTypeName).c_str()))
                        {
                            ValueProperty->DisplayRawDataInImGui("##Value", &Pair.Value);
                            ImGui::TreePop();
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                    ++Idx;
                }

                TPair<EMapElementOption, KeyType> Option;
                while (OptionQueue.Dequeue(Option))
                {
                    switch (Option.Key)
                    {
                    case EMapElementOption::Remove:
                    {
                        Data->Remove(Option.Value);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            ImGui::EndDisabled();
            ImGui::TreePop();
        }
    }

    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

template <typename InMapType>
void TMapProperty<InMapType>::Serialize(FArchive2& Ar, void* DataPtr) const
{
    InMapType& Map = *reinterpret_cast<InMapType*>(reinterpret_cast<std::byte*>(DataPtr));
    if (Ar.IsSaving())
    {
        int32 Count = Map.Num();
        Ar.SerializeRaw(&Count, sizeof(Count));
        for (auto& Pair : Map)
        {
            KeyProperty->Serialize(Ar, &const_cast<typename InMapType::KeyType&>(Pair.Key));
            ValueProperty->Serialize(Ar, &Pair.Value);
        }
    }
    else
    {
        int32 Count = 0;
        Ar.SerializeRaw(&Count, sizeof(Count));
        Map.Empty();
        for (int i = 0; i < Count; ++i)
        {
            typename InMapType::KeyType Key;
            typename InMapType::ValueType Val;
            KeyProperty->Serialize(Ar, &Key);
            ValueProperty->Serialize(Ar, &Val);
            Map.Add(Key, Val);
        }
    }
}

template <typename InMapType>
void TMapProperty<InMapType>::CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const
{
    const InMapType& SrcMap = *reinterpret_cast<const InMapType*>(SrcPtr);
    InMapType&       DstMap = *reinterpret_cast<InMapType*>(DstPtr);

    // 대상 맵 초기화 및 용량 확보
    DstMap.Empty();
    if constexpr (requires { DstMap.Reserve(SrcMap.Num()); })
    {
        DstMap.Reserve(SrcMap.Num());
    }

    // 각 키·값 쌍 복제
    for (const auto& Pair : SrcMap)
    {
        // 키 복제
        KeyType KeyDst{};
        KeyProperty->CopyData(&Pair.Key, &KeyDst, Duplicator);

        // 값 복제
        ValueType ValDst{};
        ValueProperty->CopyData(&Pair.Value, &ValDst, Duplicator);

        // 맵에 추가
        DstMap.Add(KeyDst, ValDst);
    }
}

template <typename InSetType>
struct TSetProperty : public FProperty
{
    using ElementType = typename InSetType::ElementType;

    FProperty* ElementProperty = nullptr;

    TSetProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Set, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override
    {
        FProperty::DisplayRawDataInImGui(PropertyLabel, DataPtr);

        enum class ESetElementOption : uint8
        {
            Remove,
        };

        TSet<ElementType>* Data = static_cast<TSet<ElementType>*>(DataPtr);

        if (ImGui::TreeNode(PropertyLabel))
        {
            ImGui::BeginDisabled(HasAnyFlags(Flags, EPropertyFlags::VisibleAnywhere));
            {
                const ImGuiIO& IO = ImGui::GetIO();
                ImFont* IconFont = IO.Fonts->Fonts[1]; // FEATHER_FONT = 1

                ImGui::Text("Num of Elements: %d", Data->Num());

                ImGui::SameLine();
                ImGui::PushFont(IconFont);
                if (ImGui::Button("\ue9c8"))
                {
                    const ElementType DefaultElementToAdd = ElementType{};
                    if (Data->Contains(DefaultElementToAdd))
                    {
                        ImGui::OpenPopup("Duplicate Element Warning");
                    }
                    else
                    {
                        Data->Add(DefaultElementToAdd);
                    }
                }
                ImGui::PopFont();
                ImGui::SetItemTooltip("Add Element");

                if (ImGui::BeginPopupModal("Duplicate Element Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("The element you are trying to add already exists in the set.\nPlease use a unique element.");
                    ImGui::Separator();

                    if (ImGui::Button("OK", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                ImGui::PushFont(IconFont);
                if (ImGui::Button("\ue9f6"))
                {
                    Data->Empty();
                }
                ImGui::PopFont();
                ImGui::SetItemTooltip("Remove All Elements");

                TQueue<TPair<ESetElementOption, ElementType>> OptionQueue;
                for (int32 Idx = 0; auto & Element : *Data)
                {
                    ImGui::PushID(&Element);

                    // TODO: 겹치는 Key에 대해서는 나중에 수정해야함
                    ElementProperty->DisplayRawDataInImGui(std::format("Elem [{}]", Idx).c_str(), &const_cast<ElementType&>(Element));

                    std::string PopupLabel = std::format("SetElementOption##{}", Idx);
                    ImGui::SameLine();
                    if (ImGui::Button(std::format("...##{}", Idx).c_str()))
                    {
                        ImGui::OpenPopup(PopupLabel.c_str());
                    }

                    if (ImGui::BeginPopup(PopupLabel.c_str()))
                    {
                        if (ImGui::Selectable("Remove"))
                        {
                            OptionQueue.Enqueue(MakePair(ESetElementOption::Remove, Element));
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                    ++Idx;
                }

                TPair<ESetElementOption, ElementType> Option;
                while (OptionQueue.Dequeue(Option))
                {
                    switch (Option.Key)
                    {
                    case ESetElementOption::Remove:
                    {
                        Data->Remove(Option.Value);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            ImGui::EndDisabled();
            ImGui::TreePop();
        }
    }

    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

template <typename InSetType>
void TSetProperty<InSetType>::Serialize(FArchive2& Ar, void* DataPtr) const
{
    InSetType& Set = *reinterpret_cast<InSetType*>(reinterpret_cast<std::byte*>(DataPtr));
    if (Ar.IsSaving())
    {
        int32 Count = Set.Num();
        Ar.SerializeRaw(&Count, sizeof(Count));
        for (auto& Elem : Set)
        {
            ElementProperty->Serialize(Ar, &const_cast<typename InSetType::ElementType&>(Elem));
        }
    }
    else
    {
        int32 Count = 0;
        Ar.SerializeRaw(&Count, sizeof(Count));
        Set.Empty();
        for (int i = 0; i < Count; ++i)
        {
            typename InSetType::ElementType Elem;
            ElementProperty->Serialize(Ar, &Elem);
            Set.Add(Elem);
        }
    }
}

template <typename InSetType>
void TSetProperty<InSetType>::CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const
{
    // 원본·대상 Set 참조 얻기
    const InSetType& SrcSet = *reinterpret_cast<const InSetType*>(SrcPtr);
    InSetType&       DstSet = *reinterpret_cast<InSetType*>(DstPtr);

    // 대상 Set 초기화 및 용량 확보(가능 시)
    DstSet.Empty();
    if constexpr (requires { DstSet.Reserve(SrcSet.Num()); })
    {
        DstSet.Reserve(SrcSet.Num());
    }

    // 각 원소 복제
    for (const auto& ElemSrc : SrcSet)
    {
        // 요소 타입별 복제
        using ElemType = typename InSetType::ElementType;
        ElemType ElemDst{};

        // ElementProperty에 위임하여 복제 로직 수행
        ElementProperty->CopyData(&ElemSrc, &ElemDst, Duplicator);

        // 복제된 요소를 대상 Set에 추가
        DstSet.Add(ElemDst);
    }
}

template <typename InEnumType>
struct TEnumProperty : public FProperty
{
    using EnumType = InEnumType;

    TEnumProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Enum, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override
    {
        ImGui::BeginDisabled(HasAnyFlags(Flags, EPropertyFlags::VisibleAnywhere));
        {
            void* Data = GetPropertyData(Object);
            if (DisplayAndCheckEditRawDataInImGui(Object, Name, Data)) 
            { 
                if (Object) 
                {
                    Object->PostEditChangeProperty(this);
                }
            }
        }
        ImGui::EndDisabled();
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override
    {
        FProperty::DisplayRawDataInImGui(PropertyLabel, DataPtr);

        EnumType* Data = static_cast<EnumType*>(DataPtr);
        constexpr auto EnumEntries = magic_enum::enum_entries<EnumType>();

        const std::string_view CurrentNameView = magic_enum::enum_name(*Data);
        const std::string CurrentName = std::string(CurrentNameView);

        ImGui::Text("%s", PropertyLabel);
        ImGui::SameLine();
        if (ImGui::BeginCombo(std::format("##{}", PropertyLabel).c_str(), CurrentName.c_str()))
        {
            for (const auto& [Enum, NameView] : EnumEntries)
            {
                const std::string EnumName = std::string(NameView);
                const bool bIsSelected = (*Data == Enum);
                if (ImGui::Selectable(EnumName.c_str(), bIsSelected))
                {
                    *Data = Enum;
                }
                if (bIsSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
  
    bool DisplayAndCheckEditRawDataInImGui(UObject* Object, const char* PropertyLabel, void* DataPtr) const;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

template <typename InEnumType>
bool TEnumProperty<InEnumType>::DisplayAndCheckEditRawDataInImGui(UObject* Object, const char* PropertyLabel, void* DataPtr) const
{
    // FProperty::DisplayRawDataInImGui(PropertyLabel, DataPtr); // 기본 클래스 호출은 이제 필요 없음

    EnumType* Data = static_cast<EnumType*>(DataPtr);
    constexpr auto EnumEntries = magic_enum::enum_entries<EnumType>(); // magic_enum 사용

    const std::string_view CurrentNameView = magic_enum::enum_name(*Data);
    const std::string CurrentName = std::string(CurrentNameView);

    ImGui::Text("%s", PropertyLabel);
    ImGui::SameLine();

    bool valueChanged = false; // 값 변경 여부 플래그

    if (ImGui::BeginCombo(std::format("##{}", PropertyLabel).c_str(), CurrentName.c_str()))
    {
        for (const auto& [EnumValue, NameView] : EnumEntries) // C++17 구조적 바인딩
        {
            const std::string EnumDisplayName = std::string(NameView);
            const bool bIsSelected = (*Data == EnumValue);

            if (ImGui::Selectable(EnumDisplayName.c_str(), bIsSelected))
            {
                if (*Data != EnumValue) // 실제 값이 변경되었을 때만
                {
                    *Data = EnumValue;
                    valueChanged = true; // 값 변경됨 표시
                }
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    return valueChanged; // 값 변경 여부 반환
}

template <typename InEnumType>
void TEnumProperty<InEnumType>::Serialize(FArchive2& Ar, void* DataPtr) const
{
    InEnumType& Val = *reinterpret_cast<InEnumType*>(reinterpret_cast<std::byte*>(DataPtr));
    using Under = std::underlying_type_t<InEnumType>;
    Ar.SerializeRaw(&Val, sizeof(Under));
}

template <typename InEnumType>
void TEnumProperty<InEnumType>::CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const
{
    // SrcPtr/DstPtr는 InEnumType 값을 저장하는 메모리 주소입니다.
    const InEnumType* SrcVal = reinterpret_cast<const InEnumType*>(SrcPtr);
    InEnumType*       DstVal = reinterpret_cast<InEnumType*>(DstPtr);

    // 단순 값 복사
    *DstVal = *SrcVal;
}

struct FSubclassOfProperty : public FProperty
{
    FSubclassOfProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::SubclassOf, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FAssetProperty : public FProperty
{
    FAssetProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Asset, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

struct FObjectProperty : public FProperty
{
    FObjectProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Object, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

struct FStructProperty : public FProperty
{
    FStructProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::Struct, InSize, InOffset, InFlags)
    {
    }

    virtual void DisplayRawDataInImGui(const char* PropertyLabel, void* DataPtr) const override;
    
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
};

struct FUnresolvedPtrProperty : public FProperty
{
    FProperty* ResolvedProperty = nullptr;

    FUnresolvedPtrProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int64 InSize,
        int64 InOffset,
        EPropertyFlags InFlags
    )
        : FProperty(InOwnerStruct, InPropertyName, EPropertyType::UnresolvedPointer, InSize, InOffset, InFlags)
    {
    }

    virtual ~FUnresolvedPtrProperty() override
    {
        delete ResolvedProperty;
    }

    FUnresolvedPtrProperty(const FUnresolvedPtrProperty&) = delete;
    FUnresolvedPtrProperty& operator=(const FUnresolvedPtrProperty&) = delete;
    FUnresolvedPtrProperty(FUnresolvedPtrProperty&&) = delete;
    FUnresolvedPtrProperty& operator=(FUnresolvedPtrProperty&&) = delete;

    virtual void DisplayInImGui(UObject* Object) const override;
    virtual void Resolve() override;
    
    void Serialize(FArchive2& Ar, void* DataPtr) const override;
    void CopyData(const void* SrcPtr, void* DstPtr, FObjectDuplicator& Duplicator) const override;
};

// struct FDelegateProperty : public FProperty {};  // TODO: 나중에 Delegate Property 만들기

// struct FMulticastDelegateProperty : public FProperty {};

namespace PropertyFactory::Private
{
    template <typename T, EPropertyFlags InFlags>
    FProperty* CreatePropertyForContainerType();

    template <typename T, EPropertyFlags InFlags>
    FProperty* MakeProperty(
        UStruct* InOwnerStruct,
        const char* InPropertyName,
        int32 InOffset
    )
    {
        // Flags 검사
        if constexpr (HasAllFlags<InFlags>(EPropertyFlags::EditAnywhere | EPropertyFlags::VisibleAnywhere))
        {
            // EditAnywhere와 VisibleAnywhere는 서로 같이 사용할 수 없음!!
            static_assert(TAlwaysFalse<T>, "EditAnywhere and VisibleAnywhere cannot be set at the same time.");
        }
        else if constexpr (HasAllFlags<InFlags>(EPropertyFlags::LuaReadOnly | EPropertyFlags::LuaReadWrite))
        {
            // LuaReadOnly와 LuaReadWrite는 서로 같이 사용할 수 없음!!
            static_assert(TAlwaysFalse<T>, "LuaReadOnly and LuaReadWrite cannot be set at the same time.");
        }

        // 각 타입에 맞는 Property 생성
        constexpr EPropertyType TypeEnum = GetPropertyType<T>();

        if constexpr (TypeEnum == EPropertyType::Int8) { return new FInt8Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Int16) { return new FInt16Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Int32) { return new FInt32Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Int64) { return new FInt64Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::UInt8) { return new FUInt8Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::UInt16) { return new FUInt16Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::UInt32) { return new FUInt32Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::UInt64) { return new FUInt64Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Float) { return new FFloatProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Double) { return new FDoubleProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Bool) { return new FBoolProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }

        else if constexpr (TypeEnum == EPropertyType::String) { return new FStrProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Name) { return new FNameProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Vector2D) { return new FVector2DProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Vector) { return new FVectorProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Vector4) { return new FVector4Property{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Rotator) { return new FRotatorProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Quat) { return new FQuatProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Transform) { return new FTransformProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Matrix) { return new FMatrixProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::Color) { return new FColorProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }
        else if constexpr (TypeEnum == EPropertyType::LinearColor) { return new FLinearColorProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags }; }

        else if constexpr (TypeEnum == EPropertyType::Array)
        {
            TArrayProperty<T>* Property = new TArrayProperty<T>{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->ElementProperty = CreatePropertyForContainerType<typename T::ElementType, InFlags>();
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::Map)
        {
            TMapProperty<T>* Property = new TMapProperty<T>{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->KeyProperty = CreatePropertyForContainerType<typename T::KeyType, InFlags>();
            Property->ValueProperty = CreatePropertyForContainerType<typename T::ValueType, InFlags>();
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::Set)
        {
            TSetProperty<T>* Property = new TSetProperty<T>{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->ElementProperty = CreatePropertyForContainerType<typename T::ElementType, InFlags>();
            return Property;
        }

        else if constexpr (TypeEnum == EPropertyType::Enum)
        {
            return new TEnumProperty<T>{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
        }
        else if constexpr (TypeEnum == EPropertyType::Struct)
        {
            FProperty* Property = new FStructProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->TypeSpecificData = T::StaticStruct();
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::StructPointer)
        {
            using PointerType = std::remove_cvref_t<std::remove_pointer_t<T>>;
            FUnresolvedPtrProperty* Property = new FUnresolvedPtrProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };

            Property->Type = EPropertyType::Struct;
            Property->TypeSpecificData = PointerType::StaticStruct();
            Property->ResolvedProperty = new FStructProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::SubclassOf)
        {
            if constexpr (std::derived_from<typename T::ElementType, UObject>)
            {
                FProperty* Property = new FSubclassOfProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
                Property->TypeSpecificData = T::ElementType::StaticClass();
                return Property;
            }
            else
            {
                // TSubclassOf에 UObject를 상속받은 클래스가 아닌 타입이 들어오면, 여기서 컴파일 에러가 날 수 있음
                static_assert(TAlwaysFalse<T>, "TSubclassOf template parameter must inherit from UObject");
            }
        }
        else if constexpr (TypeEnum == EPropertyType::Asset)
        {
            using PointerType = std::remove_cvref_t<std::remove_pointer_t<T>>;
            FProperty* Property = new FAssetProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->TypeSpecificData = PointerType::StaticClass();
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::Object)
        {
            using PointerType = std::remove_cvref_t<std::remove_pointer_t<T>>;
            FProperty* Property = new FObjectProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->TypeSpecificData = PointerType::StaticClass();
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::UnresolvedPointer)
        {
            constexpr std::string_view TypeName = GetTypeNameString<T>();
            FProperty* Property = new FUnresolvedPtrProperty{ InOwnerStruct, InPropertyName, sizeof(T), InOffset, InFlags };
            Property->TypeSpecificData = FName(TypeName.data(), TypeName.size());
            
            // UnresolvedPointer 프로퍼티를 전역 리스트에 추가
            UStruct::RegisterUnresolvedProperty(Property);
            return Property;
        }
        else if constexpr (TypeEnum == EPropertyType::Unknown)
        {
            static_assert(TAlwaysFalse<T>, "Unsupported Property Type"); // 지원되지 않는 타입!!
        }
        else
        {
            // 모든 Enum값에 대해서 처리하지 않으면 컴파일 에러
            static_assert(TAlwaysFalse<T>, "Not all property types are handled in MakeProperty function. Please add missing property type handling.");
        }

        std::unreachable(); // 이론상 도달할 수 없는 코드, (static_assert 지우면 호출됨)
    }

    template <typename T, EPropertyFlags InFlags>
    FProperty* CreatePropertyForContainerType()
    {
        constexpr EPropertyType TypeEnum = GetPropertyType<T>();

        if constexpr (
            TypeEnum == EPropertyType::Array
            || TypeEnum == EPropertyType::Set
            || TypeEnum == EPropertyType::Map
            || TypeEnum == EPropertyType::SubclassOf
            )
        {
            // 다차원 컨테이너는 UPROPERTY로 사용할 수 없음!!
            static_assert(TAlwaysFalse<T>, "Nested container types (e.g. TArray<TArray<T>>, TArray<TSet<T>>) cannot be used as UPROPERTY type.");
        }

        return MakeProperty<T, InFlags>(
            nullptr,
            "InnerProperty", // 실제 컨테이너 항목은 이 이름을 사용하지 않음
            0
        );
    }
}
