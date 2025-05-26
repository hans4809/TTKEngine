#pragma once
#include "MathSSE.h"

namespace GlobalVectorConstants
{
    inline constexpr VectorRegister4Float FloatOne = SSE::MakeVectorRegisterFloatConstant(1.f, 1.f, 1.f, 1.f);
	inline constexpr VectorRegister4Float FloatZero = SSE::MakeVectorRegisterFloatConstant(0.f, 0.f, 0.f, 0.f);
	inline constexpr VectorRegister4Float FloatMinusOne = SSE::MakeVectorRegisterFloatConstant(-1.f, -1.f, -1.f, -1.f);
	inline constexpr VectorRegister4Float Float0001 = SSE::MakeVectorRegisterFloatConstant(0.f, 0.f, 0.f, 1.f);
	inline constexpr VectorRegister4Float Float1000 = SSE::MakeVectorRegisterFloatConstant(1.f, 0.f, 0.f, 0.f);
	inline constexpr VectorRegister4Float Float1110 = SSE::MakeVectorRegisterFloatConstant(1.f, 1.f, 1.f, 0.f);
	inline constexpr VectorRegister4Float SmallLengthThreshold = SSE::MakeVectorRegisterFloatConstant(1.e-8f, 1.e-8f, 1.e-8f, 1.e-8f);
	inline constexpr VectorRegister4Float FloatOneHundredth = SSE::MakeVectorRegisterFloatConstant(0.01f, 0.01f, 0.01f, 0.01f);
	inline constexpr VectorRegister4Float Float111_Minus1 = SSE::MakeVectorRegisterFloatConstant( 1.f, 1.f, 1.f, -1.f );
	inline constexpr VectorRegister4Float FloatMinus1_111= SSE::MakeVectorRegisterFloatConstant( -1.f, 1.f, 1.f, 1.f );
	inline constexpr VectorRegister4Float FloatOneHalf = SSE::MakeVectorRegisterFloatConstant( 0.5f, 0.5f, 0.5f, 0.5f );
	inline constexpr VectorRegister4Float FloatMinusOneHalf = SSE::MakeVectorRegisterFloatConstant( -0.5f, -0.5f, -0.5f, -0.5f );
	inline constexpr VectorRegister4Float SmallNumber = SSE::MakeVectorRegisterFloatConstant(SMALL_NUMBER, SMALL_NUMBER, SMALL_NUMBER, SMALL_NUMBER);
	inline constexpr VectorRegister4Float BigNumber = SSE::MakeVectorRegisterFloatConstant(BIG_NUMBER, BIG_NUMBER, BIG_NUMBER, BIG_NUMBER);

    inline constexpr VectorRegister4Float QINV_SIGN_MASK = SSE::MakeVectorRegisterFloatConstant( -1.f, -1.f, -1.f, 1.f );

    inline constexpr VectorRegister4Float QMULTI_SIGN_MASK0 = SSE::MakeVectorRegisterFloatConstant( 1.f, -1.f, 1.f, -1.f );
    inline constexpr VectorRegister4Float QMULTI_SIGN_MASK1 = SSE::MakeVectorRegisterFloatConstant( 1.f, 1.f, -1.f, -1.f );
    inline constexpr VectorRegister4Float QMULTI_SIGN_MASK2 = SSE::MakeVectorRegisterFloatConstant( -1.f, 1.f, 1.f, -1.f );

    inline constexpr VectorRegister4Float DEG_TO_RAD = SSE::MakeVectorRegisterFloatConstant(PI/(180.f), PI/(180.f), PI/(180.f), PI/(180.f));
    inline constexpr VectorRegister4Float DEG_TO_RAD_HALF = SSE::MakeVectorRegisterFloatConstant((PI/180.f) * 0.5f, (PI/180.f) * 0.5f, (PI/180.f) * 0.5f, (PI/180.f) * 0.5f);
    inline constexpr VectorRegister4Float RAD_TO_DEG = SSE::MakeVectorRegisterFloatConstant((180.f)/PI, (180.f)/PI, (180.f)/PI, (180.f)/PI);
}
