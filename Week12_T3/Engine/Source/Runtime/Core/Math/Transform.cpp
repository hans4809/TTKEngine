#include "Transform.h"

#include "MathSSE.h"
#include "MathVectorContants.h"

void FTransform::SetFromMatrix(const FMatrix& InMatrix)
{
    // 행렬 복사본 생성
    FMatrix M = InMatrix;

    // 스케일 추출
    Scale = M.ExtractScaling();

    // 음수 스케일링 처리
    if (InMatrix.Determinant() < 0.f)
    {
        // 음수 행렬식은 음수 스케일이 있다는 의미입니다.
        // X축을 따라 음수 스케일이 있다고 가정하고 변환을 수정합니다.
        // 어떤 축을 선택하든 '외관'은 동일합니다.
        Scale.X = -Scale.X;

        // X축 방향 반전
        M.SetAxis(0, -M.GetScaledAxis(EAxis::X));
    }

    // 스케일이 제거된 행렬에서 회전값 추출
    Rotation = FQuat(M);
    Rotation.Normalize();

    // 이동값 추출
    Location = InMatrix.GetOrigin();
}

FTransform FTransform::operator*(const FTransform& Other) const
{
    FTransform Output;
    Multiply(&Output, this, &Other);
    return Output;
}

void FTransform::operator*=(const FTransform& Other)
{
    Multiply(this, this, &Other);
}

FVector FTransform::InverseTransformPositionNoScale(const FVector Vector) const
{
    const VectorRegister4Float InputVector = SSE::VectorLoadFloat3_W0(&Vector.X);

    // (V-Translation)
    const VectorRegister4Float Translation = SSE::VectorLoadFloat3_W1(&Location.X);
    const VectorRegister4Float TranslatedVec = SSE::VectorSet_W0(SSE::VectorSubtract(InputVector, Translation));

    const VectorRegister4Float Quat = SSE::VectorLoadFloat4(&Rotation.X);
    // ( Rotation.Inverse() * (V-Translation) )
    const VectorRegister4Float VResult = SSE::VectorQuaternionInverseRotateVector(Quat, TranslatedVec);

    FVector Result;
    SSE::VectorStoreFloat3(VResult, &Result.X);
    return Result;
}

FVector FTransform::TransformPositionNoScale(const FVector Vector) const
{
    const VectorRegister4Float InputVectorW0 = SSE::VectorLoadFloat3_W0(&Vector.X);

    //Transform using QST is following
    //QST(P) = Q.Rotate(S*P) + T where Q = quaternion, S = 1.0f, T = translation

    //RotatedVec = Q.Rotate(V.X, V.Y, V.Z, 0.f)
    const VectorRegister4Float Quat = SSE::VectorLoadFloat4(&Rotation.X);
    const VectorRegister4Float RotatedVec = SSE::VectorQuaternionRotateVector(Quat, InputVectorW0);

    // (V-Translation)
    const VectorRegister4Float Translation = SSE::VectorLoadFloat3_W1(&Location.X);
    const VectorRegister4Float TranslatedVec = SSE::VectorAdd(RotatedVec, Translation);

    FVector Result;
    SSE::VectorStoreFloat3(TranslatedVec, &Result.X);
    return Result;
}

FVector FTransform::TransformVectorNoScale(const FVector Vector) const
{
    const VectorRegister4Float InputVectorW0 = SSE::VectorLoadFloat3_W0(&Vector.X);

    //RotatedVec = Q.Rotate(V.X, V.Y, V.Z, 0.f)
    //RotatedVec = Q.Rotate(V.X, V.Y, V.Z, 0.f)
    const VectorRegister4Float Quat = SSE::VectorLoadFloat4(&Rotation.X);
    const VectorRegister4Float RotatedVec = SSE::VectorQuaternionRotateVector(Quat, InputVectorW0);

    FVector Result;
    SSE::VectorStoreFloat3(RotatedVec, &Result.X);
    return Result;
}

void FTransform::ScaleTranslation(const FVector InScale3D)
{
    const VectorRegister4Float VectorInScale3D = SSE::VectorLoadFloat3_W0(&InScale3D.X);

    // (V-Translation)
    const VectorRegister4Float Translation = SSE::VectorLoadFloat3_W1(&Location.X);
    const VectorRegister4Float ScaledTranslation = SSE::VectorMultiply( Translation, VectorInScale3D );

    SSE::VectorStoreFloat3(ScaledTranslation, &Location.X);
}

FVector FTransform::GetScaledAxis(EAxis::Type InAxis)
{
    if ( InAxis == EAxis::X )
    {
        return TransformVector(FVector(1.f, 0.f, 0.f));
    }
    else if ( InAxis == EAxis::Y )
    {
        return TransformVector(FVector(0.f, 1.f, 0.f));
    }

    return TransformVector(FVector(0.f, 0.f, 1.f));
}

void FTransform::RemoveScaling()
{
    Scale = FVector::ZeroVector;    
    Rotation.Normalize();
}

FTransform FTransform::Inverse() const
{
    // 1) 회전의 역(역사원수)
    FQuat InvRot = Rotation.Inverse();
    InvRot.Normalize();

    // 2) 스케일의 역(0 방지)
    FVector InvScale(
        Scale.X != 0.f ? 1.f / Scale.X : 0.f,
        Scale.Y != 0.f ? 1.f / Scale.Y : 0.f,
        Scale.Z != 0.f ? 1.f / Scale.Z : 0.f
    );

    // 3) 위치 역변환: -T, 스케일 적용, 회전 적용
    FVector InvTrans = InvRot.RotateVector(-Location * InvScale);

    return FTransform(InvRot, InvTrans, InvScale);
}

FString FTransform::ToString() const
{
    return FString::Printf(TEXT("Translation=%s Rotation=%s Scale3D=%s"), 
                      *Location.ToString(), 
                      *Rotation.ToString(), 
                      *Scale.ToString());
}

FTransform FTransform::GetRelativeTransform(const FTransform& Base) const
{
    return (*this) * Base.Inverse();
}

FTransform FTransform::Blend(const FTransform& Atom1, const FTransform& Atom2, float Alpha)
{
    FTransform Result;

    Result.Rotation = FQuat::Slerp(Atom1.Rotation, Atom2.Rotation, Alpha);
    Result.Location = FMath::Lerp(Atom1.Location, Atom2.Location, Alpha);
    Result.Scale = FMath::Lerp(Atom1.Scale, Atom2.Scale, Alpha);

    return Result;
}

FMatrix FTransform::ToMatrixWithScale() const
{
    FMatrix M;
    
    // Quaternion을 회전 행렬로 변환 (열 우선)
    float x2 = Rotation.X * Rotation.X;
    float y2 = Rotation.Y * Rotation.Y;
    float z2 = Rotation.Z * Rotation.Z;
    float xy = Rotation.X * Rotation.Y;
    float xz = Rotation.X * Rotation.Z;
    float yz = Rotation.Y * Rotation.Z;
    float wx = Rotation.W * Rotation.X;
    float wy = Rotation.W * Rotation.Y;
    float wz = Rotation.W * Rotation.Z;
    
    // 첫 번째 열 (X축)
    M.M[0][0] = (1.0f - 2.0f * (y2 + z2)) * Scale.X;
    M.M[1][0] = (2.0f * (xy + wz)) * Scale.X;
    M.M[2][0] = (2.0f * (xz - wy)) * Scale.X;
    M.M[3][0] = 0.0f;
    
    // 두 번째 열 (Y축)
    M.M[0][1] = (2.0f * (xy - wz)) * Scale.Y;
    M.M[1][1] = (1.0f - 2.0f * (x2 + z2)) * Scale.Y;
    M.M[2][1] = (2.0f * (yz + wx)) * Scale.Y;
    M.M[3][1] = 0.0f;
    
    // 세 번째 열 (Z축)
    M.M[0][2] = (2.0f * (xz + wy)) * Scale.Z;
    M.M[1][2] = (2.0f * (yz - wx)) * Scale.Z;
    M.M[2][2] = (1.0f - 2.0f * (x2 + y2)) * Scale.Z;
    M.M[3][2] = 0.0f;
    
    // 네 번째 열 (위치)
    M.M[0][3] = Location.X;
    M.M[1][3] = Location.Y;
    M.M[2][3] = Location.Z;
    M.M[3][3] = 1.0f;
    
    return M;
}

FMatrix FTransform::ToMatrixNoScale() const
{
    FMatrix M;
    
    // Quaternion to matrix conversion
    float x2 = Rotation.X * Rotation.X;
    float y2 = Rotation.Y * Rotation.Y;
    float z2 = Rotation.Z * Rotation.Z;
    float xy = Rotation.X * Rotation.Y;
    float xz = Rotation.X * Rotation.Z;
    float yz = Rotation.Y * Rotation.Z;
    float wx = Rotation.W * Rotation.X;
    float wy = Rotation.W * Rotation.Y;
    float wz = Rotation.W * Rotation.Z;
    
    // 첫 번째 열 (X축)
    M.M[0][0] = 1.0f - 2.0f * (y2 + z2);
    M.M[1][0] = 2.0f * (xy + wz);
    M.M[2][0] = 2.0f * (xz - wy);
    M.M[3][0] = 0.0f;
    
    // 두 번째 열 (Y축)
    M.M[0][1] = 2.0f * (xy - wz);
    M.M[1][1] = 1.0f - 2.0f * (x2 + z2);
    M.M[2][1] = 2.0f * (yz + wx);
    M.M[3][1] = 0.0f;
    
    // 세 번째 열 (Z축)
    M.M[0][2] = 2.0f * (xz + wy);
    M.M[1][2] = 2.0f * (yz - wx);
    M.M[2][2] = 1.0f - 2.0f * (x2 + y2);
    M.M[3][2] = 0.0f;
    
    // 네 번째 열 (위치)
    M.M[0][3] = Location.X;
    M.M[1][3] = Location.Y;
    M.M[2][3] = Location.Z;
    M.M[3][3] = 1.0f;
    
    return M;
}

FVector FTransform::GetUnitAxis(EAxis::Type Axis) const
{
    {
        // 로컬 축 벡터
        static const FVector LocalAxes[3] = {
            {1,0,0}, // X
            {0,1,0}, // Y
            {0,0,1}  // Z
        };

        FVector Local = LocalAxes[static_cast<int>(Axis)-1];
        // 회전만 적용한 후 정규화
        return Rotation.RotateVector(Local).GetSafeNormal();
    }
}

bool FTransform::Private_AnyHasNegativeScale(FVector InScale3D, FVector InOtherScale3D)
{
    VectorRegister4Float InScale = SSE::VectorLoadFloat3_W0(&InScale3D.X);
    VectorRegister4Float InOtherScale = SSE::VectorLoadFloat3_W0(&InOtherScale3D.X);
    
    return !!SSE::VectorAnyLesserThan(SSE::VectorMin(InScale, InOtherScale), GlobalVectorConstants::FloatZero);
}

void FTransform::ConstructTransformFromMatrixWithDesiredScale(const FMatrix& AMatrix, const FMatrix& BMatrix, VectorRegister4Float DesiredScale,
    FTransform& OutTransform)
{
    // the goal of using M is to get the correct orientation
    // but for translation, we still need scale
    FMatrix M = AMatrix * BMatrix;
    M.RemoveScaling();

    // apply negative scale back to axes
    FVector SignedScale;
    SSE::VectorStoreFloat3(SSE::VectorSign(DesiredScale), &SignedScale.X);

    M.SetAxis(0, M.GetScaledAxis(EAxis::X) * SignedScale.X);
    M.SetAxis(1, M.GetScaledAxis(EAxis::Y) * SignedScale.Y);
    M.SetAxis(2, M.GetScaledAxis(EAxis::Z) * SignedScale.Z);

    // @note: if you have negative with 0 scale, this will return rotation that is identity
    // since matrix loses that axes
    FQuat Rotation = FQuat(M);
    Rotation.Normalize();

    // set values back to output
    SSE::VectorStoreFloat4(DesiredScale, &OutTransform.Scale.X);
    OutTransform.Rotation = Rotation;

    // technically I could calculate this using TTransform<T> but then it does more quat multiplication 
    // instead of using Scale in matrix multiplication
    // it's a question of between RemoveScaling vs using TTransform<T> to move translation
    FVector Translation = M.GetOrigin();
    OutTransform.Location = Translation;
}

void FTransform::MultiplyUsingMatrixWithScale(FTransform* OutTransform, const FTransform* A, const FTransform* B)
{
    const VectorRegister4Float ScaleA = SSE::VectorLoadFloat3_W0(&A->Scale.X);
    const VectorRegister4Float ScaleB = SSE::VectorLoadFloat3_W0(&B->Scale.X);
    
	ConstructTransformFromMatrixWithDesiredScale(A->ToMatrixWithScale(), B->ToMatrixWithScale(), SSE::VectorMultiply(ScaleA, ScaleB), *OutTransform);
}

void FTransform::Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B)
{
    if (Private_AnyHasNegativeScale(A->Scale, B->Scale))
    {
        MultiplyUsingMatrixWithScale(OutTransform, A, B);
    }
    else
    {
        VectorRegister4Float QuatA = SSE::VectorLoadFloat4(&A->Rotation.X);
        VectorRegister4Float QuatB = SSE::VectorLoadFloat4(&B->Rotation.X);

        VectorRegister4Float TranslateA = SSE::VectorLoadFloat3_W1(&A->Location.X);
        VectorRegister4Float TranslateB = SSE::VectorLoadFloat3_W1(&B->Location.X);

        VectorRegister4Float ScaleA = SSE::VectorLoadFloat3_W0(&A->Scale.X);
        VectorRegister4Float ScaleB = SSE::VectorLoadFloat3_W0(&B->Scale.X);

        // Rotation
        VectorRegister4Float ResultRotation = SSE::VectorQuaternionMultiply2(QuatB, QuatA);
        SSE::VectorStoreFloat3(ResultRotation, &OutTransform->Rotation.X);

        // Translation (3채널 저장)
        VectorRegister4Float ScaledTransA = SSE::VectorMultiply(TranslateA, ScaleB);
        VectorRegister4Float RotatedTranslate = SSE::VectorQuaternionRotateVector(QuatB, ScaledTransA);
        VectorRegister4Float ResultTranslation = SSE::VectorAdd(RotatedTranslate, TranslateB);
        SSE::VectorStoreFloat3(ResultTranslation, &OutTransform->Location.X);

        // Scale    (3채널 저장)
        VectorRegister4Float ResultScale = SSE::VectorMultiply(ScaleA, ScaleB);
        SSE::VectorStoreFloat3(ResultScale, &OutTransform->Scale.X);
    }
}

FVector FTransform::TransformVector(const FVector InVector) const
{
    const VectorRegister4Float InputVectorW0 = SSE::VectorLoadFloat3_W0(&InVector.X);

    //RotatedVec = Q.Rotate(Scale*V.X, Scale*V.Y, Scale*V.Z, 0.f)
    const VectorRegister4Float CurScale = SSE::VectorLoadFloat3_W0(&Scale.X);
    const VectorRegister4Float ScaledVec = SSE::VectorMultiply(CurScale, InputVectorW0);

    const VectorRegister4Float Quat = SSE::VectorLoadFloat4(&Rotation.X);
    const VectorRegister4Float RotatedVec = SSE::VectorQuaternionRotateVector(Quat, ScaledVec);

    FVector Result;
    SSE::VectorStoreFloat3(RotatedVec, &Result.X);
    return Result;
}

FVector FTransform::TransformPosition(const FVector InVector) const
{
    const VectorRegister4Float InputVectorW0 = SSE::VectorLoadFloat3_W0(&InVector.X);

    //Transform using QST is following
    //QST(P) = Q.Rotate(S*P) + T where Q = quaternion, S = scale, T = translation
	
    //RotatedVec = Q.Rotate(Scale*V.X, Scale*V.Y, Scale*V.Z, 0.f)
    const VectorRegister4Float CurScale = SSE::VectorLoadFloat3_W0(&Scale.X);
    const VectorRegister4Float ScaledVec = SSE::VectorMultiply(CurScale, InputVectorW0);

    const VectorRegister4Float Quat = SSE::VectorLoadFloat4(&Rotation.X);
    const VectorRegister4Float RotatedVec = SSE::VectorQuaternionRotateVector(Quat, ScaledVec);

    const VectorRegister4Float Translation = SSE::VectorLoadFloat3_W1(&Location.X);
    const VectorRegister4Float TranslatedVec = SSE::VectorAdd(RotatedVec, Translation);

    FVector Result;
    SSE::VectorStoreFloat3(TranslatedVec, &Result.X);
    return Result;
}

