#pragma once
#include "Math/MathUtility.h"
#include "UObject/ObjectMacros.h"

/**
 * CollisionDisableTable의 TMap에서 키로 사용되는 두 강체 인덱스 쌍을 저장하는 엔디안 안전 저장 구조체입니다.
 */
struct FRigidBodyIndexPair
{
    DECLARE_STRUCT(FRigidBodyIndexPair)
    /** 인덱스 쌍 */
    UPROPERTY(EditAnywhere, TArray<int32>, Indices, = {})
	
    /** TMap에서 사용되기 위한 기본 생성자 */
    FRigidBodyIndexPair()
    {
        Indices.SetNum(2);
    }

    /**
     * 정렬되지 않은 인덱스 쌍을 받아 키를 생성하는 생성자입니다.
     *
     * @param	Index1	첫 번째 정렬되지 않은 인덱스
     * @param	Index2	두 번째 정렬되지 않은 인덱스
     */
    FRigidBodyIndexPair(int32 Index1, int32 Index2)
    {
        Indices[0] = FMath::Min(Index1, Index2);
        Indices[1] = FMath::Max(Index1, Index2);
    }

    /**
     * TMap에서 사용되기 위한 == 연산자 오버로드
     *
     * @param	Other	비교할 FRigidBodyIndexPair 객체
     * @return	전달된 FRigidBodyIndexPair가 이 객체와 동일하면 true, 그렇지 않으면 false
     */
    bool operator==(const FRigidBodyIndexPair& Other) const
    {
        return (Indices[0] == Other.Indices[0]) && (Indices[1] == Other.Indices[1]);
    }
};

namespace std {
    template <>
    struct hash<FRigidBodyIndexPair>
    {
        std::size_t operator()(const FRigidBodyIndexPair& Pair) const
        {
            // 간단한 해시 함수: 두 인덱스를 섞어 하나의 해시 생성
            return std::hash<int32_t>()(Pair.Indices[0]) ^ (std::hash<int32_t>()(Pair.Indices[1]) << 1);
        }
    };
}