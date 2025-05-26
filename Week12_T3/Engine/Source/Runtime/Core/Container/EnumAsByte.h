// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include <type_traits>
#include "HAL/PlatformType.h"
#include "Math/IsPODType.h"

/**
 * 바이트 단위로 열거형 값을 타입 안전하게 저장하기 위한 템플릿입니다.
 * 블루프린트 열거형은 enum class (권장)로 정의해야 합니다:
 *     enum class EMyEnum : uint8 { One, Two }; // 이 경우 템플릿으로 래핑할 필요가 없습니다.
 * 또는 네임스페이스 안의 enum으로 정의해야 합니다:
 *     namespace EMyEnum
 *     {
 *         enum Type // 실제로 'Type'이라고 명명해야 합니다.
 *         { One, Two };
 *     }
 */
template <class InEnumType>
class TEnumAsByte
{
    static_assert(std::is_enum_v<InEnumType> && std::is_convertible_v<InEnumType, int>,
                  "TEnumAsByte는 enum class와 함께 사용하도록 의도된 것이 아닙니다 - enum class를 uint8 기반으로 정의하세요.");

public:
    using EnumType = InEnumType;

    TEnumAsByte() = default;
    TEnumAsByte(const TEnumAsByte&) = default;
    TEnumAsByte& operator=(const TEnumAsByte&) = default;

    /**
     * 열거형 값으로 초기화하는 생성자입니다.
     *
     * @param InValue 초기화할 열거형 값입니다.
     */
    FORCEINLINE TEnumAsByte(EnumType InValue)
        : Value(static_cast<uint8>(InValue))
    { }

    /**
     * 정수(int32) 값으로 초기화하는 생성자입니다.
     *
     * @param InValue 초기화할 정수 값입니다.
     */
    explicit FORCEINLINE TEnumAsByte(int32 InValue)
        : Value(static_cast<uint8>(InValue))
    { }

    /**
     * 정수(uint8) 값으로 초기화하는 생성자입니다.
     *
     * @param InValue 초기화할 정수 값입니다.
     */
    explicit FORCEINLINE TEnumAsByte(uint8 InValue)
        : Value(InValue)
    { }

public:
    /**
     * 두 열거형 값이 같은지 비교합니다.
     *
     * @param InValue 비교할 열거형 값입니다.
     * @return 두 값이 같으면 true, 그렇지 않으면 false입니다.
     */
    bool operator==(EnumType InValue) const
    {
        return static_cast<EnumType>(Value) == InValue;
    }

    /**
     * 두 열거형 값이 같은지 비교합니다.
     *
     * @param InValue 비교할 다른 TEnumAsByte 값입니다.
     * @return 두 값이 같으면 true, 그렇지 않으면 false입니다.
     */
    bool operator==(TEnumAsByte InValue) const
    {
        return Value == InValue.Value;
    }

    /** 암시적 EnumType 변환 연산자입니다. */
    operator EnumType() const
    {
        return (EnumType)Value;
    }

public:
    /**
     * 열거형 값을 반환합니다.
     *
     * @return 현재 저장된 열거형 값입니다.
     */
    EnumType GetValue() const
    {
        return (EnumType)Value;
    }

    /**
     * 정수형 열거형 값을 반환합니다.
     *
     * @return 현재 저장된 정수형 값입니다.
     */
    uint8 GetIntValue() const
    {
        return Value;
    }

private:
    /** 값을 바이트로 저장합니다. **/
    uint8 Value;
};

template<class T>
FORCEINLINE uint32 GetTypeHash(const TEnumAsByte<T>& Enum)
{
	return GetTypeHash((uint8)Enum.GetValue());
}

template<class T> struct TIsPODType<TEnumAsByte<T>> { enum { Value = true }; };

template <typename T>
struct TIsTEnumAsByte
{
	static constexpr bool Value = false;
};

template <typename T> struct TIsTEnumAsByte<               TEnumAsByte<T>> { static constexpr bool Value = true; };
template <typename T> struct TIsTEnumAsByte<const          TEnumAsByte<T>> { static constexpr bool Value = true; };
template <typename T> struct TIsTEnumAsByte<      volatile TEnumAsByte<T>> { static constexpr bool Value = true; };
template <typename T> struct TIsTEnumAsByte<const volatile TEnumAsByte<T>> { static constexpr bool Value = true; };

