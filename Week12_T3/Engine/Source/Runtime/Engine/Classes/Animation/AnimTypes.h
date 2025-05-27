#pragma once
#include "Container/Array.h"
#include "Delegates/Delegate.h"
#include "UObject/NameTypes.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "UObject/ObjectMacros.h"

enum class EAnimInterpolationType : std::uint8_t
{
    None,
    Linear,
    Cubic,
    Constant,
    Step,
    Max
};

enum class EAnimationBlendMode : std::uint8_t
{
    None,
    Linear,
    Additive,
    Masked,
    TimeBased,
    Max
};

enum class ETangentMode : std::uint8_t
{
    None,
    NoneTangent,
    Auto,
    User,
    Break,
    Max
};

enum class ETangentWeightMode : std::uint8_t
{
    None,
    NoneWeight,
    Arrive,
    Leave,
    Both,
    Max
};

/*
*FbxTime
*FbxAnimLayer
*FbxAnimStack
 */

struct FCurve
{
    DECLARE_STRUCT(FCurve)

    UPROPERTY(VisibleAnywhere, float, Time, = 0.0f)// 키프레임의 시간

    UPROPERTY(VisibleAnywhere, float, Value, = 0.0f)// 키프레임의 값

    UPROPERTY(VisibleAnywhere, float, ArriveTangent, = 0.0f) // 들어오는 탄젠트 (이전 키프레임에서 현재 키프레임으로의 기울기)
    UPROPERTY(VisibleAnywhere, float, LeaveTangent, = 0.0f) // 나가는 탄젠트 (현재 키프레임에서 다음 키프레임으로의 기울기)
    
    // 탄젠트의 가중치 (탄젠트의 영향력을 제어)
    UPROPERTY(VisibleAnywhere, float, TangentWeightArrive, = 0.0f)
    UPROPERTY(VisibleAnywhere, float, TangentWeightLeave, = 0.0f)

    UPROPERTY(VisibleAnywhere, EAnimInterpolationType, InterpMode, = EAnimInterpolationType::None)// 이 키프레임에 사용되는 보간 모드
    UPROPERTY(VisibleAnywhere, ETangentMode, TangentMode, = ETangentMode::None)
    UPROPERTY(VisibleAnywhere, ETangentWeightMode, TangentWeightMode, = ETangentWeightMode::None)

    float Evaluate(float CurrentTime) const;

    void Serialize(FArchive& Ar) const
    {
        Ar << Time << Value << ArriveTangent << LeaveTangent << TangentWeightArrive << TangentWeightLeave;
        Ar << static_cast<uint32>(InterpMode) << static_cast<uint32>(TangentMode) << static_cast<uint32>(TangentWeightMode);
    }

    void Deserialize(FArchive& Ar)
    {
        uint32 interpMode, tangentMode, tangentWeightMode;
        Ar >> Time >> Value >> ArriveTangent >> LeaveTangent >> TangentWeightArrive >> TangentWeightLeave;
        Ar >> interpMode >> tangentMode >> tangentWeightMode;
        InterpMode = static_cast<EAnimInterpolationType>(interpMode);
        TangentMode = static_cast<ETangentMode>(tangentMode);
        TangentWeightMode = static_cast<ETangentWeightMode>(tangentWeightMode);
    }
};

struct FAnimationCurveData
{
    DECLARE_STRUCT(FAnimationCurveData)

    UPROPERTY(VisibleAnywhere, FName, CurveName, = TEXT("None"))

    UPROPERTY(VisibleAnywhere, TArray<FCurve>, CurveWeights, = {})

    void Serialize(FArchive& Ar) const
    {
        Ar << CurveName << CurveWeights;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> CurveName >> CurveWeights;
    }
};

struct FAnimNotifyEvent
{
    DECLARE_STRUCT(FAnimNotifyEvent)

    UPROPERTY(VisibleAnywhere, float, TriggerTime, = 0.0f)
    UPROPERTY(VisibleAnywhere, float, Duration, = 0.0f)
    UPROPERTY(VisibleAnywhere, int32, TrackIndex, = 0)
    UPROPERTY(VisibleAnywhere, FName, NotifyName, = TEXT("None"))

    // class UAnimNotify* Notify;
    TDelegate<void()> OnNotify;
    bool bIsTriggered = false;
    // < 연산자 오버로딩
    bool operator<(const FAnimNotifyEvent& other) const
    {
        return TriggerTime < other.TriggerTime;
    }
    friend bool operator==(const FAnimNotifyEvent& A, const FAnimNotifyEvent& B);

    bool IsState() const { return Duration > 0.f; }
    float GetEndTime() const { return TriggerTime + Duration; }
};

struct FAnimNotifyTrack
{
    DECLARE_STRUCT(FAnimNotifyTrack)

    UPROPERTY(VisibleAnywhere, FName, TrackName, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, TArray<int32>, NotifyIndices, = {})

    FAnimNotifyTrack() = default;

    FAnimNotifyTrack(const FName InName)
        : TrackName(InName)
    {
    }
};

struct FRawAnimSequenceTrack
{
    DECLARE_STRUCT(FRawAnimSequenceTrack)

    UPROPERTY(VisibleAnywhere, TArray<FVector>, PosKeys, = {}) // 위치 키들
    UPROPERTY(VisibleAnywhere, TArray<FQuat>, RotKeys, = {})  // 회전 키들 
    UPROPERTY(VisibleAnywhere, TArray<FVector>, ScaleKeys, = {}) // 스케일 키들
    UPROPERTY(VisibleAnywhere, TArray<float>, KeyTimes, = {}) // 각 키프레임의 시간값
    UPROPERTY(VisibleAnywhere, EAnimInterpolationType, InterpMode, = EAnimInterpolationType::None)// 전체 트랙의 보간 모드

    void Serialize(FArchive& Ar) const;

    void Deserialize(FArchive& Ar);
};


struct FBoneAnimationTrack
{
    DECLARE_STRUCT(FBoneAnimationTrack)
    UPROPERTY(VisibleAnywhere, FName, Name, = TEXT("Empty")) // Bone 이름
    UPROPERTY(VisibleAnywhere, FRawAnimSequenceTrack, InternalTrackData, = {}) // 실제 애니메이션 데이터

    void Serialize(FArchive& Ar) const
    {
        Ar << Name << InternalTrackData;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> Name >> InternalTrackData;
    }
};

struct FSkeletalAnimation
{
    FName Name = "Empty";
    TArray<FBoneAnimationTrack> BoneAnimTracks;
    TArray<FAnimationCurveData> AttributeCurves;
    TArray<FAnimNotifyEvent> Notifies;

    void Serialize(FArchive& Ar) const
    {
        Ar << Name << BoneAnimTracks << AttributeCurves;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> Name >> BoneAnimTracks >> AttributeCurves;
    }
};