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
    float2 Pad;
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

float CalculateCircleOfConfusion(float Depth)
{
    float Numerator  = abs(Depth - FocusDistance) / Depth;
    float LensFactor = (FocalLength * FocalLength) / (Aperture * (FocusDistance - FocalLength));
    float SensorNorm = 1.0 / SensorWidth;
    float RawCoC     = Numerator * LensFactor * SensorNorm;
    return clamp(RawCoC, 0.0, MaxCoCRadius);
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
    if (CoCRadius < 1.0)
        return SceneColorTex.Sample(postProcessingSampler, input.uv);
    
    float2 PixelSize = float2(1.0 / ScreenSize.x, 1.0 / ScreenSize.y);
    float3 AccumColor = float3(0,0,0);
    float  TotalWeight = 0;

    // 원형 디스크 샘플링
    [unroll(12)]
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