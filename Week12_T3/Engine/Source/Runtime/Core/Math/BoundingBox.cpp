#include "BoundingBox.h"

bool FBoundingBox::IntersectRay(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const
{
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
    const float epsilon = 1e-6f;

    // X축 처리
    if (fabs(rayDir.X) < epsilon)
    {
        // 레이가 X축 방향으로 거의 평행한 경우,
        // 원점의 x가 박스 [min.x, max.x] 범위 밖이면 교차 없음
        if (rayOrigin.X < Min.X || rayOrigin.X > Max.X)
            return false;
    }
    else
    {
        float t1 = (Min.X - rayOrigin.X) / rayDir.X;
        float t2 = (Max.X - rayOrigin.X) / rayDir.X;
        if (t1 > t2)  std::swap(t1, t2);

        // tmin은 "현재까지의 교차 구간 중 가장 큰 min"
        tmin = (t1 > tmin) ? t1 : tmin;
        // tmax는 "현재까지의 교차 구간 중 가장 작은 max"
        tmax = (t2 < tmax) ? t2 : tmax;
        if (tmin > tmax)
            return false;
    }

    // Y축 처리
    if (fabs(rayDir.Y) < epsilon)
    {
        if (rayOrigin.Y < Min.Y || rayOrigin.Y > Max.Y)
            return false;
    }
    else
    {
        float t1 = (Min.Y - rayOrigin.Y) / rayDir.Y;
        float t2 = (Max.Y - rayOrigin.Y) / rayDir.Y;
        if (t1 > t2)  std::swap(t1, t2);

        tmin = (t1 > tmin) ? t1 : tmin;
        tmax = (t2 < tmax) ? t2 : tmax;
        if (tmin > tmax)
            return false;
    }

    // Z축 처리
    if (fabs(rayDir.Z) < epsilon)
    {
        if (rayOrigin.Z < Min.Z || rayOrigin.Z > Max.Z)
            return false;
    }
    else
    {
        float t1 = (Min.Z - rayOrigin.Z) / rayDir.Z;
        float t2 = (Max.Z - rayOrigin.Z) / rayDir.Z;
        if (t1 > t2)  std::swap(t1, t2);

        tmin = (t1 > tmin) ? t1 : tmin;
        tmax = (t2 < tmax) ? t2 : tmax;
        if (tmin > tmax)
            return false;
    }

    // 여기까지 왔으면 교차 구간 [tmin, tmax]가 유효하다.
    // tmax < 0 이면, 레이가 박스 뒤쪽에서 교차하므로 화면상 보기엔 교차 안 한다고 볼 수 있음
    if (tmax < 0.0f)
        return false;

    // outDistance = tmin이 0보다 크면 그게 레이가 처음으로 박스를 만나는 지점
    // 만약 tmin < 0 이면, 레이의 시작점이 박스 내부에 있다는 의미이므로, 거리를 0으로 처리해도 됨.
    outDistance = (tmin >= 0.0f) ? tmin : 0.0f;

    return true;
}

bool FBoundingBox::IntersectAABB(FBoundingBox Other) const
{
    return (Min.X <= Other.Max.X && Max.X >= Other.Min.X) &&
        (Min.Y <= Other.Max.Y && Max.Y >= Other.Min.Y) &&
        (Min.Z <= Other.Max.Z && Max.Z >= Other.Min.Z);
}

FBoundingBox::FBoundingBox(const TArray<FVector>& Points) : Min(FLT_MAX, FLT_MAX, FLT_MAX), Max(FLT_MIN, FLT_MIN, FLT_MIN)
{
    if (Points.IsEmpty())
    {
        return;
    }

    Min = Points[0];
    Max = Points[0];

    for (int32 i = 1; i < Points.Num(); ++i)
    {
        Min.X = FMath::Min(Min.X, Points[i].X);
        Min.Y = FMath::Min(Min.Y, Points[i].Y);
        Min.Z = FMath::Min(Min.Z, Points[i].Z);

        Max.X = FMath::Max(Max.X, Points[i].X);
        Max.Y = FMath::Max(Max.Y, Points[i].Y);
        Max.Z = FMath::Max(Max.Z, Points[i].Z);
    }

}

void FBoundingBox::Serialize(FArchive& Ar) const
{
    Ar << Min << Max;
}

void FBoundingBox::Deserialize(FArchive& Ar)
{
    Ar >> Min >> Max;
}

FBoundingBox FBoundingBox::TransformBy(FMatrix InMatirx) const
{
    FBoundingBox NewBox;

    VectorRegister4Float VecMin = SSE::VectorLoadFloat3_W0(&Min.X);
    VectorRegister4Float VecMax = SSE::VectorLoadFloat3_W0(&Max.X);

    VectorRegister4Float m0 = SSE::VectorLoadFloat3_W0(InMatirx.M[0]);
    VectorRegister4Float m1 = SSE::VectorLoadFloat3_W0(InMatirx.M[1]);
    VectorRegister4Float m2 = SSE::VectorLoadFloat3_W0(InMatirx.M[2]);
    VectorRegister4Float m3 = SSE::VectorLoadFloat3_W0(InMatirx.M[3]);

    VectorRegister4Float Sum = SSE::VectorAdd(VecMax, VecMin);
    VectorRegister4Float Diff = SSE::VectorSubtract(VecMax, VecMin);
    VectorRegister4Float Half = _mm_set1_ps(0.5f);
    VectorRegister4Float Origin = SSE::VectorMultiply(Sum, Half);
    VectorRegister4Float Extent = SSE::VectorMultiply(Diff, Half);

    // 3) Origin 변환
    VectorRegister4Float NewOrigin = SSE::VectorMultiply(SSE::VectorReplicateTemplate<0>(Origin), m0);
    NewOrigin = SSE::VectorMultiplyAdd(SSE::VectorReplicateTemplate<1>(Origin), m1, NewOrigin);
    NewOrigin = SSE::VectorMultiplyAdd(SSE::VectorReplicateTemplate<2>(Origin), m2, NewOrigin);
    NewOrigin = SSE::VectorAdd(NewOrigin, m3);

    VectorRegister4Float NewExtent = SSE::VectorAbs(SSE::VectorMultiply(SSE::VectorReplicateTemplate<0>(Extent), m0));
    NewExtent = SSE::VectorAdd(NewExtent, SSE::VectorAbs(SSE::VectorMultiply(SSE::VectorReplicateTemplate<1>(Extent), m1)));
    NewExtent = SSE::VectorAdd(NewExtent, SSE::VectorAbs(SSE::VectorMultiply(SSE::VectorReplicateTemplate<2>(Extent), m2)));

    VectorRegister4Float NewMin = SSE::VectorSubtract(NewOrigin, NewExtent);
    VectorRegister4Float NewMax = SSE::VectorAdd(NewOrigin, NewExtent);

    SSE::VectorStoreFloat3(NewMin, &NewBox.Min.X);
    SSE::VectorStoreFloat3(NewMax, &NewBox.Max.X);

    return NewBox;
}

FBoundingBox FBoundingBox::operator+=(const FBoundingBox& Other)
{
    Min.X = FMath::Min(Min.X, Other.Min.X);
    Min.Y = FMath::Min(Min.Y, Other.Min.Y);
    Min.Z = FMath::Min(Min.Z, Other.Min.Z);

    Max.X = FMath::Max(Max.X, Other.Max.X);
    Max.Y = FMath::Max(Max.Y, Other.Max.Y);
    Max.Z = FMath::Max(Max.Z, Other.Max.Z);

    return *this;
}

FVector FBoundingBox::GetCenter() const
{
    return FVector((Min + Max) * 0.5f);
}

FVector FBoundingBox::GetSize() const
{
    return (Max - Min);
}

FBoundingBox FBoundingBox::ExpandBy(const FVector V) const
{
    return FBoundingBox(Min - V, Max + V);
}
