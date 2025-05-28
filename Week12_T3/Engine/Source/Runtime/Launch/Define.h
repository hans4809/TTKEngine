#pragma once
#include "Core/Container/String.h"
#include "Core/Container/Array.h"
#include "UObject/NameTypes.h"

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "Math/Matrix.h"
#include "UObject/ObjectMacros.h"

class FArchive;
class UMaterial;

struct FVertexSimple
{
    DECLARE_STRUCT(FVertexSimple)
    
    UPROPERTY(float, x)
    UPROPERTY(float, y)
    UPROPERTY(float, z)
    UPROPERTY(float, w)

    UPROPERTY(float, r)
    UPROPERTY(float, g)
    UPROPERTY(float, b)
    UPROPERTY(float, a)

    UPROPERTY(float, nx)
    UPROPERTY(float, ny)
    UPROPERTY(float, nz)

    UPROPERTY(float, Tangentnx)
    UPROPERTY(float, Tangentny)
    UPROPERTY(float, Tangentnz)

    UPROPERTY(float, u, = 0.0f)
    UPROPERTY(float, v, = 0.0f)

    UPROPERTY(uint32, MaterialIndex)
};

// Material Subset
struct FMaterialSubset
{
    DECLARE_STRUCT(FMaterialSubset)

    UPROPERTY(uint32, IndexStart)
    UPROPERTY(uint32, IndexCount)
    UPROPERTY(uint32, MaterialIndex)
    UPROPERTY(FString, MaterialName)
    
    void Serialize(FArchive& ar) const;
    void Deserialize(FArchive& ar);
};

struct FMaterialSlot
{
    DECLARE_STRUCT(FMaterialSlot)
    ~FMaterialSlot();
    
    UPROPERTY(VisibleAnywhere | DuplicateTransient, UMaterial*, Material, = nullptr)
    UPROPERTY(VisibleAnywhere, FName, MaterialSlotName, = TEXT("None"))
    //FMeshUVChannelInfo UVChannelData;
};

// OBJ File Raw Data
struct FObjInfo
{
    FString ObjectName; // OBJ File Name
    FString PathName; // OBJ File Paths
    FString DisplayName; // Display Name
    FString MatName; // OBJ MTL File Name
    
    // Group
    uint32 NumOfGroup = 0; // token 'g' or 'o'
    TArray<FString> GroupName;
    
    // Vertex, UV, Normal List
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    
    // Faces
    TArray<int32> Faces;

    // Index
    TArray<uint32> VertexIndices;
    TArray<uint32> NormalIndices;
    TArray<uint32> TextureIndices;
    
    // Material
    TArray<FMaterialSubset> MaterialSubsets;
};

struct FObjMaterialInfo
{
    DECLARE_STRUCT(FObjMaterialInfo)

    UPROPERTY(EditAnywhere, FString, MTLName, = TEXT("None"))

    UPROPERTY(EditAnywhere, bool, bHasTexture, = false)
    UPROPERTY(EditAnywhere, bool, bTransparent, = false)

    // Diffuse (Kd) : 일반적으로 흰색, 완전 불투명한 색상
    UPROPERTY(EditAnywhere, FVector, Diffuse, = FVector(1.0f, 1.0f, 1.0f))
    
    // Specular (Ks) : 반사광 기본값, 흰색으로 표기하는 경우가 많음
    UPROPERTY(EditAnywhere, FVector, Specular, = FVector(1.0f, 1.0f, 1.0f))
    
    // Ambient (Ka) : 주변광 기본값, 너무 강하지 않은 낮은 값으로
    UPROPERTY(EditAnywhere, FVector, Ambient, = FVector(0.1f, 0.1f, 0.1f))

    // Emissive (Ke) : 자체 발광 없음
    UPROPERTY(EditAnywhere, FVector, Emissive, = FVector(0.0f, 0.0f, 0.0f))
    
    // SpecularScalar (Ns) : 스페큘러 파워 (보통 1.0 이상, 필요에 따라 조정)
    UPROPERTY(EditAnywhere, float, SpecularScalar, = 1.0f)
    
    // DensityScalar (Ni) : 광학적 밀도(굴절률 등), 기본적으로 1.0
    UPROPERTY(EditAnywhere, float, DensityScalar, = 1.0f)
    
    // TransparencyScalar : 투명도, 1.0이면 불투명, 0.0이면 완전 투명
    UPROPERTY(EditAnywhere, float, TransparencyScalar, = 1.0f)

    UPROPERTY(EditAnywhere, uint32, IlluminanceModel, = 0)

    /* Texture */
    UPROPERTY(EditAnywhere, FString, DiffuseTextureName, = TEXT("None"))
    UPROPERTY(EditAnywhere, FString, DiffuseTexturePath, = TEXT("None"))
    
    // map_Ka : Ambient texture
    UPROPERTY(EditAnywhere, FString, AmbientTextureName, = TEXT("None"))
    UPROPERTY(EditAnywhere, FString, AmbientTexturePath, = TEXT("None"))

    // map_Ks : Specular texture
    UPROPERTY(EditAnywhere, FString, SpecularTextureName, = TEXT("None"))
    UPROPERTY(EditAnywhere, FString, SpecularTexturePath, = TEXT("None"))

    // map_Bump : Bump texture
    UPROPERTY(EditAnywhere, FString, BumpTextureName, = TEXT("None"))
    UPROPERTY(EditAnywhere, FString, BumpTexturePath, = TEXT("None"))

    // map_d : Alpha texture
    UPROPERTY(EditAnywhere, FString, AlphaTextureName, = TEXT("None"))
    UPROPERTY(EditAnywhere, FString, AlphaTexturePath, = TEXT("None"))

    // map_Ns : Normal Texture
    UPROPERTY(EditAnywhere, FString, NormalTextureName, = TEXT("None"))
    UPROPERTY(EditAnywhere, FString, NormalTexturePath, = TEXT("None"))

    UPROPERTY(EditAnywhere, float, NormalScale, = 1.0f)
    
    void Serialize(FArchive& Ar) const;
    void Deserialize(FArchive& Ar);
};

struct FStaticMeshRenderData
{
    DECLARE_STRUCT(FStaticMeshRenderData)
    
    UPROPERTY(VisibleAnywhere, FString, ObjectName, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, FString, PathName, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, FString, DisplayName, = TEXT("None"))

    UPROPERTY(VisibleAnywhere, TArray<FVertexSimple>, Vertices, = {})
    UPROPERTY(VisibleAnywhere, TArray<uint32>, Indices, = {})

    UPROPERTY(VisibleAnywhere, FString, VBName, = TEXT("None"))
    UPROPERTY(VisibleAnywhere, FString, IBName, = TEXT("None"))
    // ID3D11Buffer* VertexBuffer;
    // ID3D11Buffer* IndexBuffer;

    UPROPERTY(VisibleAnywhere, TArray<FObjMaterialInfo>, Materials, = {})
    UPROPERTY(VisibleAnywhere, TArray<FMaterialSubset>, MaterialSubsets, = {})

    UPROPERTY(VisibleAnywhere, FVector, BoundingBoxMin, = FVector::ZeroVector)
    UPROPERTY(VisibleAnywhere, FVector, BoundingBoxMax, = FVector::ZeroVector)
};

enum class EShaderStage
{
    VS,		// Vertex Shader
    HS,		// Hull Shader
    DS,		// Domain Shader
    GS,		// Geometry Shader
    PS,		// Pixel Shader
    CS,		// Compute Shader
    All,
    End,
};

// ShaderType과 Constant 이름을 결합한 키 구조체
struct FShaderConstantKey
{
    EShaderStage ShaderType;  // 예: Vertex, Pixel 등
    FName ConstantName;    // 상수 버퍼 내 상수 이름

    // 동등 비교 연산자: 두 키가 동일하면 true
    bool operator==(const FShaderConstantKey& Other) const
    {
        return ShaderType == Other.ShaderType && ConstantName == Other.ConstantName;
    }
};

// std::hash 특수화를 통해 FShaderConstantKey를 해시 기반 컨테이너에서 사용할 수 있게 함
namespace std
{
    template<>
    struct hash<FShaderConstantKey>
    {
        std::size_t operator()(const FShaderConstantKey& Key) const noexcept
        {
            // EShaderType은 enum class이므로 int로 캐스팅하여 해시를 계산
            std::size_t h1 = std::hash<uint32>()(static_cast<int>(Key.ShaderType));
            std::size_t h2 = std::hash<FName>()(Key.ConstantName);
            // 간단한 해시 결합: XOR과 쉬프트 사용 (더 복잡한 해시 결합도 가능)
            return h1 ^ (h2 << 1);
        }
    };
}

enum class ESamplerType
{
    Point,
    Linear,
    Anisotropic,
    PostProcess,
    ComparisonSampler, // Compare
    End,
};

enum class ERenderingMode
{
    Opaque,
    CutOut,
    Transparent,
    PostProcess,
    End,
};

enum class ETextureType
{
    Albedo,
    Normal,
    Specular,
    Smoothness,
    Metallic,
    Sprite,
    End,
};

enum class ERasterizerState
{
    SolidBack,
    SolidFront,
    SolidNone,
    WireFrame,
    End,
};

enum class EBlendState
{
    AlphaBlend,
    OneOne,
    ParticleBlend,
    End,
};

enum class EDepthStencilState
{
    DepthNone,
    LessEqual,
    End,
};

enum EShadowFilterMode
{
    PCF,
    VSM
};

enum ESkinningType
{
    CPU,
    GPU,
};

struct FVertexTexture
{
	float x, y, z;    // Position
	float u, v; // Texture
};

struct FSimpleVertex
{
	float dummy; // 내용은 사용되지 않음
    float padding[11];
};

struct FOBB
{
    FVector corners[8];
};

struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    FVector4 Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];
};
struct FSphere
{
    FVector Center; 
    float Radius;
    FVector4 Color;
};
struct FLine
{
    FVector Start;
    float Length;
    FVector Direction;
    float pad;
    FVector4 Color;
};

struct FCapsule
{
    FVector Center;       // 12
    FVector UpVector;     // 12
    FVector4 Color;       // 16
    float Radius;         // 4
    float HalfHeight;     // 4
   /* FVector Center;
    float Radius;
    FVector UpVector;
    float HalfHeight;
    FVector4 Color;*/
};

struct FPlane
{
    // 평면 방정식: Ax + By + Cz + D = 0
    float A, B, C, D;

    // 기본 생성자
    FPlane() : A(0), B(0), C(0), D(0) {}

    // 값들을 인자로 받아 생성하는 생성자
    FPlane(float InA, float InB, float InC, float InD)
        : A(InA), B(InB), C(InC), D(InD) {}

    // 평면을 정규화 하는 함수: 평면의 법선(A, B, C)의 길이를 1로 만듭니다.
    void Normalize()
    {
        float Magnitude = std::sqrt(A * A + B * B + C * C);
        if (Magnitude != 0.0f)
        {
            A /= Magnitude;
            B /= Magnitude;
            C /= Magnitude;
            D /= Magnitude;
        }
    }

    // 주어진 점(p)와 평면 사이의 signed distance를 계산합니다.
    // 양수 값이면 점이 평면의 법선 방향 쪽에 있고,
    // 음수 값이면 반대 방향에 있음을 나타냅니다.
    float PlaneDot(const FVector& p) const
    {
        return A * p.X + B * p.Y + C * p.Z + D;
    }
};
struct FFrustum
{
    // 프러스텀 평면이 6개 있다고 가정 (Left, Right, Bottom, Top, Near, Far)
    FPlane Planes[6];

    // 구의 (center, radius)를 기준으로 프러스텀과 교차하는지 검사하는 함수
    bool IntersectsSphere(const FVector& Center, float Radius) const
    {
        // 각 평면에 대해 구의 중심과의 거리가 -Radius보다 작으면 완전히 외부에 있다고 봅니다.
        for (int i = 0; i < 6; ++i)
        {
            if (Planes[i].PlaneDot(Center) < -Radius)
            {
                return false;
            }
        }
        return true;
    }
    bool IntersectsPoint(const FVector& Point) const
    {
        // 6개의 평면 모두에 대해 점이 평면의 '앞쪽'에 있어야 프러스텀 내부로 판단
        for (int i = 0; i < 6; ++i)
        {
            if (Planes[i].PlaneDot(Point) < 0)
            {
                return false;
            }
        }
        return true;
    }
    static FFrustum ExtractFrustum(const FMatrix& Mat) {
        FFrustum frustum;
    
        // Left Plane = M[3] + M[0]
        frustum.Planes[0] = FPlane(
            Mat.M[0][3] + Mat.M[0][0],
            Mat.M[1][3] + Mat.M[1][0],
            Mat.M[2][3] + Mat.M[2][0],
            Mat.M[3][3] + Mat.M[3][0]
        );
    
        // Right Plane = M[3] - M[0]
        frustum.Planes[1] = FPlane(
            Mat.M[0][3] - Mat.M[0][0],
            Mat.M[1][3] - Mat.M[1][0],
            Mat.M[2][3] - Mat.M[2][0],
            Mat.M[3][3] - Mat.M[3][0]
        );
    
        // Bottom Plane = M[3] + M[1]
        frustum.Planes[2] = FPlane(
            Mat.M[0][3] + Mat.M[0][1],
            Mat.M[1][3] + Mat.M[1][1],
            Mat.M[2][3] + Mat.M[2][1],
            Mat.M[3][3] + Mat.M[3][1]
        );
    
        // Top Plane = M[3] - M[1]
        frustum.Planes[3] = FPlane(
            Mat.M[0][3] - Mat.M[0][1],
            Mat.M[1][3] - Mat.M[1][1],
            Mat.M[2][3] - Mat.M[2][1],
            Mat.M[3][3] - Mat.M[3][1]
        );
    
        // Near Plane = M[3] + M[2]
        frustum.Planes[4] = FPlane(
            Mat.M[0][3] + Mat.M[0][2],
            Mat.M[1][3] + Mat.M[1][2],
            Mat.M[2][3] + Mat.M[2][2],
            Mat.M[3][3] + Mat.M[3][2]
        );
    
        // Far Plane = M[3] - M[2]
        frustum.Planes[5] = FPlane(
            Mat.M[0][3] - Mat.M[0][2],
            Mat.M[1][3] - Mat.M[1][2],
            Mat.M[2][3] - Mat.M[2][2],
            Mat.M[3][3] - Mat.M[3][2]
        );
    
        // 각 평면 정규화
        for (auto& plane : frustum.Planes) {
            plane.Normalize();
        }
    
        return frustum;
    }
};

// !NOTE : 최대 광원 수 제한
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
constexpr UINT CASCADE_COUNT = 4;

enum class ELightType
{
    None,
    DirectionalLight,
    PointLight,
    SpotLight,
    Max,
};

struct FDirectionalLight
{
    FVector Direction;
    float Intensity;

    FVector4 Color;

    float CascadeSplit[CASCADE_COUNT];

    FMatrix View[CASCADE_COUNT];
    FMatrix Projection[CASCADE_COUNT];

    uint32 CastShadow;
    FVector Pad;
};

struct FPointLight
{
    FVector Position;
    float Radius;

    FVector4 Color;

    float Intensity;
    float AttenuationFalloff;
    uint32 CastShadow;
    float Padd;

    FMatrix PointLightView[6];
    FMatrix PointLightProj;
};

struct FComputeConstants{
    FMatrix InverseView;
    FMatrix InverseProj;
    float screenWidth;
    float screenHeight;
    int tileCountX;
    int tileCountY;
};
struct FSpotLight
{
    FVector Position;
    float Intensity;

    FVector4 Color;

    FVector Direction;
    float InnerAngle;

    float OuterAngle;
    uint32 CastShadow;
    FVector2D pad;

    FMatrix View;
    FMatrix Proj;
    FVector4 AtlasUVTransform;
};

struct FIntRect
{
    int32 X;
    int32 Y;
    int32 Width;
    int32 Height;
    FIntRect() : X(0), Y(0), Width(0), Height(0) {}
    FIntRect(int32 InX, int32 InY, int32 InWidth, int32 InHeight)
        : X(InX), Y(InY), Width(InWidth), Height(InHeight) {
    }
};
