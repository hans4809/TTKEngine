#include "ShaderHeaders/GSamplers.hlsli"

Texture2D SceneDepthTex : register(t0);
Texture2D SceneColorTex : register(t1);

cbuffer FViewportInfo : register(b0)
{
    float2 ViewportSize;
    float2 ViewportOffset;
    float2 ScreenSize;
    float2 Pad0;
}

cbuffer DepthOfFieldParameters : register(b1)
{
    float  FocalLength;      // 렌즈 초점 거리 (mm)
    float  Aperture;            // 조리개 값
    float  FocusDistance;    // 포커스 거리 (월드 단위)
    float  SensorWidth;       // 센서 크기 (mm)
    
    float  MaxCoCRadius;     // 최대 CoC 반경 (픽셀)
    uint   SampleCount;      // 디스크 샘플 개수
    float2 UserData;
};

cbuffer FFogCameraConstant : register(b2)
{
    matrix InvProjMatrix;
    matrix InvViewMatrix;
    
    float3 CameraPos;
    float NearPlane;
    
    float3 CameraForward;
    float FarPlane;
};

float CalculateCircleOfConfusion(float Depth) // Depth는 LinearDepth (미터 단위로 가정)
{
    // 초점 허용 오차
    // "거의 초점이 맞은" 영역의 선명도를 조절하려면 이 값을 변경하세요.
    // 값이 클수록 초점 거리 주변의 더 넓은 영역이 완벽하게 선명해집니다.
    const float FocusDepthTolerance = 5.0f; // 허용 오차

    // 물체의 깊이가 (초점 거리 ± 허용 오차) 범위 내에 있다면 CoC를 0으로 강제합니다.
    if (abs(Depth - FocusDistance) < FocusDepthTolerance)
    {
        return 0.0f; // 완벽히 초점이 맞은 것으로 처리
    }
    
    if (Depth <= 0.0001f) 
    {
        return 0.0f; 
    }
    if (FocalLength <= 0.0f || Aperture <= 0.0f || SensorWidth <= 0.0f)
    {
        return 0.0f;
    }

    float focusMinusFocal = FocusDistance - FocalLength;
    if (focusMinusFocal <= 0.00001f) 
    {
        if (focusMinusFocal <= 0.0f) return MaxCoCRadius; // 초점 설정이 물리적으로 어렵거나 무한대 초점 시 예외적 블러
    }
    
    // Numerator: |D - S| / D (단위 없음)
    float Numerator = abs(Depth - FocusDistance) / Depth;

    // LensFactor: f^2 / (N * (S - f))
    float LensFactor = (FocalLength * FocalLength) / (Aperture * focusMinusFocal);
    
    // SensorFactor: 1 / SensorWidth
    float SensorFactor = 1.0 / SensorWidth;

    // RawCoC_NormalizedDiameter: 센서 폭에 대한 CoC 직경의 비율 (단위 없음)
    float RawCoC_NormalizedDiameter = Numerator * LensFactor * SensorFactor;

    // 정규화된 CoC 직경을 픽셀 반경으로 변환
    float CoC_PixelRadius = RawCoC_NormalizedDiameter * ScreenSize.x * 0.5f;

    return clamp(CoC_PixelRadius, 0.0, MaxCoCRadius);
}

struct VS_OUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float2 uv : TEXCOORD0; // UV 좌표
};

float4 mainPS(VS_OUT input) : SV_Target
{
    float Depth      = SceneDepthTex.Sample(postProcessingSampler, input.uv).r;
    float zNdc = Depth * 2.0 - 1.0f;
    float LinearDepth = 2 * NearPlane * FarPlane / (FarPlane + NearPlane - zNdc * (FarPlane - NearPlane));
    float CoCRadius  = CalculateCircleOfConfusion(LinearDepth);

    // 3) DebugMode (ShowFlag) 분기
    if (UserData.x > 0.5f)  // ShowFlag
    {
        // DepthDelta < 0 : Near  → 녹색
        // DepthDelta = 0 : Focus → 검은색
        // DepthDelta > 0 : Far   → 파란색
        float DepthDelta   = LinearDepth - FocusDistance;
        float CoCMagnitude = saturate(CoCRadius / MaxCoCRadius);

        float3 DebugColor = float3(0,0,0);
        if (DepthDelta < 0.0f)
            DebugColor.g = CoCMagnitude;
        else if (DepthDelta > 0.0f)
            DebugColor.b = CoCMagnitude;

        return float4(DebugColor, 1.0f);
    }
    
    if (CoCRadius < 1.0)
        return SceneColorTex.Sample(postProcessingSampler, input.uv);
    
    float2 PixelSize = float2(1.0 / ScreenSize.x, 1.0 / ScreenSize.y);
    float3 AccumColor = float3(0,0,0);
    float  TotalWeight = 0;

    // 원형 디스크 샘플링
    [loop]
    for (uint i = 0; i < SampleCount; ++i)
    {
        float Angle  = i * (6.2831853 / SampleCount);
        float2 Offset = float2(cos(Angle), sin(Angle)) * CoCRadius * PixelSize;
        float  Weight = 1.0; // 필요시 삼각 윈도우나 가중치 테이블 적용
        AccumColor   += SceneColorTex.Sample(postProcessingSampler, input.uv + Offset).rgb * Weight;
        TotalWeight  += Weight;
    }

    return float4(AccumColor / TotalWeight, 1.0);
}