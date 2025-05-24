#include "MathSSE.h"

#include "MathVectorContants.h"

VectorRegister4Float SSE::VectorQuaternionInverse(const VectorRegister4Float& NormalizedQuat)
{
    return SSE::VectorMultiply(GlobalVectorConstants::QINV_SIGN_MASK, NormalizedQuat);
}

VectorRegister4Float SSE::VectorMultiplyAdd(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2, const VectorRegister4Float& Vec3)
{
    return VectorAdd(VectorMultiply(Vec1, Vec2), Vec3);
}

void SSE::VectorMatrixMultiply(FMatrix* Result, const FMatrix* Matrix1, const FMatrix* Matrix2)
{
    // 레지스터에 값 로드
    const VectorRegister4Float* Matrix1Ptr = reinterpret_cast<const VectorRegister4Float*>(Matrix1);
    const VectorRegister4Float* Matrix2Ptr = reinterpret_cast<const VectorRegister4Float*>(Matrix2);
    VectorRegister4Float* Ret = reinterpret_cast<VectorRegister4Float*>(Result);
    VectorRegister4Float Temp, R0, R1, R2;

    // 첫번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[0], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 2), Matrix2Ptr[2], Temp);
    R0 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 3), Matrix2Ptr[3], Temp);

    // 두번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[1], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 2), Matrix2Ptr[2], Temp);
    R1 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 3), Matrix2Ptr[3], Temp);

    // 세번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[2], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 2), Matrix2Ptr[2], Temp);
    R2 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 3), Matrix2Ptr[3], Temp);

    // 네번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[3], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 2), Matrix2Ptr[2], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 3), Matrix2Ptr[3], Temp);

    // 결과 저장
    Ret[0] = R0;
    Ret[1] = R1;
    Ret[2] = R2;
    Ret[3] = Temp;
}

VectorRegister4Float SSE::VectorQuaternionMultiply2( const VectorRegister4Float& Quat1, const VectorRegister4Float& Quat2)
{
    VectorRegister4Float Result = SSE::VectorMultiply(VectorReplicate(Quat1, 3), Quat2);
    Result = SSE::VectorMultiplyAdd( VectorMultiply(VectorReplicate(Quat1, 0), VectorSwizzle(Quat2, 3, 2, 1, 0)), GlobalVectorConstants::QMULTI_SIGN_MASK0, Result);
    Result = SSE::VectorMultiplyAdd( VectorMultiply(VectorReplicate(Quat1, 1), VectorSwizzle(Quat2, 2, 3, 0, 1)), GlobalVectorConstants::QMULTI_SIGN_MASK1, Result);
    Result = SSE::VectorMultiplyAdd( VectorMultiply(VectorReplicate(Quat1, 2), VectorSwizzle(Quat2, 1, 0, 3, 2)), GlobalVectorConstants::QMULTI_SIGN_MASK1, Result);

    return Result;
}

VectorRegister4Float SSE::VectorQuaternionRotateVector(const VectorRegister4Float& Quat, const VectorRegister4Float& VectorW0)
{
    // Q * V * Q.Inverse
    //const VectorRegister InverseRotation = VectorQuaternionInverse(Quat);
    //const VectorRegister Temp = VectorQuaternionMultiply2(Quat, VectorW0);
    //const VectorRegister Rotated = VectorQuaternionMultiply2(Temp, InverseRotation);

    // Equivalence of above can be shown to be:
    // http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
    // V' = V + 2w(Q x V) + (2Q x (Q x V))
    // refactor:
    // V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
    // T = 2(Q x V);
    // V' = V + w*(T) + (Q x T)

    const VectorRegister4Float QW = VectorReplicate(Quat, 3);
    VectorRegister4Float T = SSE::VectorCross(Quat, VectorW0);
    T = SSE::VectorAdd(T, T);
    const VectorRegister4Float VTemp0 = SSE::VectorMultiplyAdd(QW, T, VectorW0);
    const VectorRegister4Float VTemp1 = SSE::VectorCross(Quat, T);
    const VectorRegister4Float Rotated = SSE::VectorAdd(VTemp0, VTemp1);
    return Rotated;
}


int SSE::VectorAnyGreaterThan(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    return VectorMaskBits(VectorCompareGT(Vec1, Vec2));
}

float SSE::VectorAnyLesserThan(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    return VectorAnyGreaterThan(Vec2, Vec1);
}

VectorRegister4Float SSE::VectorQuaternionInverseRotateVector(const VectorRegister4Float& Quat, const VectorRegister4Float& VectorW0)
{
    // Q.Inverse * V * Q
    //const VectorRegister InverseRotation = VectorQuaternionInverse(Quat);
    //const VectorRegister Temp = VectorQuaternionMultiply2(InverseRotation, VectorW0);
    //const VectorRegister Rotated = VectorQuaternionMultiply2(Temp, Quat);

    const VectorRegister4Float QInv = SSE::VectorQuaternionInverse(Quat);
    return SSE::VectorQuaternionRotateVector(QInv, VectorW0);
}


VectorRegister4Float SSE::VectorCross(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    // YZX
    VectorRegister4Float A = VectorSwizzle(Vec2, 1, 2, 0, 3);
    VectorRegister4Float B = VectorSwizzle(Vec1, 1, 2, 0, 3);
    // XY, YZ, ZX
    A = SSE::VectorMultiply(A, Vec1);
    // XY-YX, YZ-ZY, ZX-XZ
    A = SSE::VectorNegateMultiplyAdd(B, Vec2, A);
    // YZ-ZY, ZX-XZ, XY-YX
    return VectorSwizzle(A, 1, 2, 0, 3);
}

VectorRegister4Float SSE::VectorNegateMultiplyAdd(const VectorRegister4Float& A, const VectorRegister4Float& B, const VectorRegister4Float& C)
{
    return VectorSubtract(C, VectorMultiply(A, B));
}

VectorRegister4Float SSE::VectorSign(const VectorRegister4Float& X)
{
    const VectorRegister4Float Mask = SSE::VectorCompareGE(X, GlobalVectorConstants::FloatZero);
    return SSE::VectorSelect(Mask, GlobalVectorConstants::FloatOne, GlobalVectorConstants::FloatMinusOne);
}
