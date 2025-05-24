#pragma once
#include <immintrin.h>

#include "MathUtility.h"
#include "HAL/PlatformType.h"

/**
 * @param A0	Selects which element (0-3) from 'A' into 1st slot in the result
 * @param A1	Selects which element (0-3) from 'A' into 2nd slot in the result
 * @param B2	Selects which element (0-3) from 'B' into 3rd slot in the result
 * @param B3	Selects which element (0-3) from 'B' into 4th slot in the result
 */
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )

#define VectorReplicate(Vec, Index) VectorReplicateTemplate<Index>(Vec)
struct FMatrix;

// 4 floats
typedef __m128	VectorRegister4Float;

namespace SSEPermuteHelpers
{
    /**
     * 4개의 컴포넌트를 X,Y,Z,W 순서로 섞어 새로운 벡터를 만듭니다.
     * @tparam X Vec의 어느 인덱스(0~3)를 결과의 X로 쓸지
     * @tparam Y Vec의 어느 인덱스(0~3)를 결과의 Y로 쓸지
     * @tparam Z Vec의 어느 인덱스(0~3)를 결과의 Z로 쓸지
     * @tparam W Vec의 어느 인덱스(0~3)를 결과의 W로 쓸지
     * @param Vec 원본 벡터
     * @return 섞인(new swizzled) 벡터
     */
    template<int X, int Y, int Z, int W>
    FORCEINLINE VectorRegister4Float VectorSwizzleTemplate(const VectorRegister4Float& Vec)
    {
        // _mm_shuffle_ps(a, b, mask)는
        //   결과.xyzw = { a[mask&0x3], a[(mask>>2)&0x3], b[(mask>>4)&0x3], b[(mask>>6)&0x3] }
        // 여기서는 a=b=Vec, SHUFFLEMASK으로 X,Y,Z,W를 모두 Vec에서 가져오게 함
        return _mm_shuffle_ps(Vec, Vec, SHUFFLEMASK(X, Y, Z, W));
    }
}

// 사용 매크로 정의 (이미 MathSSE.h 어딘가에 있다면 중복 방지)
#ifndef VectorSwizzle
#define VectorSwizzle(Vec, X, Y, Z, W) SSEPermuteHelpers::VectorSwizzleTemplate<X, Y, Z, W>(Vec)
#endif

namespace SSE
{
    FORCEINLINE constexpr VectorRegister4Float MakeVectorRegisterFloatConstant(const float X, const float Y, const float Z, const float W)
    {
        return VectorRegister4Float { X, Y, Z, W };
    }

    FORCEINLINE VectorRegister4Float VectorCompareGT(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        return _mm_cmpgt_ps(Vec1, Vec2);
    }

    /**
     * Returns an integer bit-mask (0x00 - 0x0f) based on the sign-bit for each component in a vector.
     *
     * @param VecMask		Vector
     * @return				Bit 0 = sign(VecMask.x), Bit 1 = sign(VecMask.y), Bit 2 = sign(VecMask.z), Bit 3 = sign(VecMask.w)
     */
    FORCEINLINE int VectorMaskBits(const VectorRegister4Float& VecMask)
    {
        return _mm_movemask_ps(VecMask);
    }
        
    FORCEINLINE VectorRegister4Float VectorLoadFloat3_W0(const float* Ptr)
    {
        // Ptr[0]=X, Ptr[1]=Y, Ptr[2]=Z, W=0.0f
        return _mm_setr_ps(Ptr[0], Ptr[1], Ptr[2], 0.0f);
    }

    FORCEINLINE VectorRegister4Float VectorSet_W0(const VectorRegister4Float& Vec)
    {
        return _mm_insert_ps(Vec, Vec, 0x08); // Copies lane 0 to lane 0 and zero-masks lane 3
    }

    /**
     * __m128 레지스터에서 앞의 3개 성분만 메모리에 저장
     * @param Vec  저장할 레지스터
     * @param Ptr  float 배열 포인터(최소 3개 공간)
     */
    FORCEINLINE void VectorStoreFloat3(const VectorRegister4Float& Vec, float* Ptr)
    {
        alignas(16) float Tmp[4];
        _mm_store_ps(Tmp, Vec);
        Ptr[0] = Tmp[0];
        Ptr[1] = Tmp[1];
        Ptr[2] = Tmp[2];
    }

    FORCEINLINE void VectorStoreFloat4(const VectorRegister4Float& Vec, float* Ptr)
    {
        alignas(16) float Tmp[4];
        _mm_store_ps(Tmp, Vec);
        Ptr[0] = Tmp[0];
        Ptr[1] = Tmp[1];
        Ptr[2] = Tmp[2];
        Ptr[3] = Tmp[3];
    }
    
    /**
     * Vector의 특정 인덱스를 복제합니다.
     * @tparam Index 복제할 Index (0 ~ 3)
     * @param Vector 복제할 대상
     * @return 복제된 레지스터
     */
    template <int Index>
    FORCEINLINE VectorRegister4Float VectorReplicateTemplate(const VectorRegister4Float& Vector)
    {
        return _mm_shuffle_ps(Vector, Vector, SHUFFLEMASK(Index, Index, Index, Index));
    }


    /** Vector4 곱연산 */
    FORCEINLINE VectorRegister4Float VectorMultiply(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        return _mm_mul_ps(Vec1, Vec2);
    }

    FORCEINLINE VectorRegister4Float VectorAdd(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        return _mm_add_ps(Vec1, Vec2);
    }

    /** Vector 빼기: Vec1 – Vec2 */
    FORCEINLINE VectorRegister4Float VectorSubtract(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        return _mm_sub_ps(Vec1, Vec2);
    }

    /** Vector 절댓값: 각 성분의 부호 비트를 제거 */
    FORCEINLINE VectorRegister4Float VectorAbs(const VectorRegister4Float& Vec)
    {
        // 0x7FFFFFFF 마스크 = 부호 비트만 0, 나머는 1
        const __m128 SignMask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
        return _mm_and_ps(Vec, SignMask);
    }

    FORCEINLINE VectorRegister4Float VectorLoadFloat3_W1(const float* Ptr)
    {
        // Ptr[0]=X, Ptr[1]=Y, Ptr[2]=Z, W=0.0f
        return _mm_setr_ps(Ptr[0], Ptr[1], Ptr[2], 1.0f);
    }

    FORCEINLINE VectorRegister4Float VectorLoadFloat4(const float* Ptr)
    {
        return _mm_setr_ps(Ptr[0], Ptr[1], Ptr[2], Ptr[3]);
    }

    FORCEINLINE VectorRegister4Float VectorLoadAligned(const float* Ptr)
    {
        return _mm_load_ps((const float*)(Ptr));
    }

    FORCEINLINE VectorRegister4Float VectorMin(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        return _mm_min_ps(Vec1, Vec2);
    }

    FORCEINLINE VectorRegister4Float VectorCompareGE(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        return _mm_cmpge_ps(Vec1, Vec2);
    }

    FORCEINLINE VectorRegister4Float VectorSelect(const VectorRegister4Float& Mask, const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
    {
        // Can't (in general) use BLENDVPS despite our SSE4.1 minimum requirement since
        // this is defined to be bitwise, not element-wise with MSB as toggle
        return _mm_or_ps(_mm_and_ps(Mask, Vec1), _mm_andnot_ps(Mask, Vec2));
    }
    
    /**
     * Returns non-zero if any element in Vec1 is greater than the corresponding element in Vec2, otherwise 0.
     *
     * @param Vec1			1st source vector
     * @param Vec2			2nd source vector
     * @return				Non-zero integer if (Vec1.x > Vec2.x) || (Vec1.y > Vec2.y) || (Vec1.z > Vec2.z) || (Vec1.w > Vec2.w)
     */
    int VectorAnyGreaterThan(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2);
    
    float VectorAnyLesserThan(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2);

    VectorRegister4Float VectorQuaternionInverseRotateVector(const VectorRegister4Float& Quat, const VectorRegister4Float& VectorW0);
    
    /** 
     * Inverse quaternion ( -X, -Y, -Z, W) 
     */
    VectorRegister4Float VectorQuaternionInverse(const VectorRegister4Float& NormalizedQuat);

    VectorRegister4Float VectorMultiplyAdd(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2, const VectorRegister4Float& Vec3);

    void VectorMatrixMultiply(FMatrix* Result, const FMatrix* Matrix1, const FMatrix* Matrix2);
    
    /**
    * Multiplies two quaternions; the order matters.
    *
    * Order matters when composing quaternions: C = VectorQuaternionMultiply2(A, B) will yield a quaternion C = A * B
    * that logically first applies B then A to any subsequent transformation (right first, then left).
    *
    * @param Quat1	Pointer to the first quaternion
    * @param Quat2	Pointer to the second quaternion
    * @return Quat1 * Quat2
    */
    VectorRegister4Float VectorQuaternionMultiply2( const VectorRegister4Float& Quat1, const VectorRegister4Float& Quat2);

    /**
     * Rotate a vector using a unit Quaternion.
     *
     * @param Quat Unit Quaternion to use for rotation.
     * @param VectorW0 Vector to rotate. W component must be zero.
     * @return Vector after rotation by Quat.
     */
    VectorRegister4Float VectorQuaternionRotateVector(const VectorRegister4Float& Quat, const VectorRegister4Float& VectorW0);

    VectorRegister4Float VectorCross( const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2 );

    /**
     * Multiplies two vectors (component-wise), negates the results and adds it to the third vector i.e. (-A*B + C) = (C - A*B)
     *
     * @param A		1st vector
     * @param B		2nd vector
     * @param C		3rd vector
     * @return		VectorRegister( C.x - A.x*B.x, C.y - A.y*B.y, C.z - A.z*B.z, C.w - A.w*B.w )
     */
    VectorRegister4Float VectorNegateMultiplyAdd(const VectorRegister4Float& A, const VectorRegister4Float& B, const VectorRegister4Float& C);

    VectorRegister4Float VectorSign(const VectorRegister4Float& X);
}