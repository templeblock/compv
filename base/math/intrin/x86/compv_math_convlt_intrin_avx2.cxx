/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_convlt_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
// no arithmetic overflow check
void CompVMathConvlt1VtHz_8u32f8u_Intrin_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	__m256i vecInPtr, vec0i, vec1i, vec2i, vec3i;
	__m256 vecCoeff, vecSum0, vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	const __m256i vecZero = _mm256_setzero_si256();
	const __m256i vecMaskToExtractFirst64Bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXMaskstore_0_u64));
	__m128 vecSum0n, vecCoeffn, vec0fn;

	for (j = 0; j < height; ++j) {
		/* Per #32 samples */
		for (i = 0; i < width - 31; i += 32) {
			vecSum0 = _mm256_setzero_ps();
			vecSum1 = _mm256_setzero_ps();
			vecSum2 = _mm256_setzero_ps();
			vecSum3 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec2i = _mm256_unpacklo_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec3i = _mm256_unpackhi_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec0i = _mm256_unpacklo_epi16(vec2i, vecZero); // epi16 -> epi32
				vec1i = _mm256_unpackhi_epi16(vec2i, vecZero); // epi16 -> epi32
				vec2i = _mm256_unpacklo_epi16(vec3i, vecZero); // epi16 -> epi32
				vec3i = _mm256_unpackhi_epi16(vec3i, vecZero); // epi16 -> epi32
				vec0f = _mm256_cvtepi32_ps(vec0i);
				vec1f = _mm256_cvtepi32_ps(vec1i);
				vec2f = _mm256_cvtepi32_ps(vec2i);
				vec3f = _mm256_cvtepi32_ps(vec3i);
				vec0f = _mm256_mul_ps(vec0f, vecCoeff);
				vec1f = _mm256_mul_ps(vec1f, vecCoeff);
				vec2f = _mm256_mul_ps(vec2f, vecCoeff);
				vec3f = _mm256_mul_ps(vec3f, vecCoeff);
				vecSum0 = _mm256_add_ps(vecSum0, vec0f);
				vecSum1 = _mm256_add_ps(vecSum1, vec1f);
				vecSum2 = _mm256_add_ps(vecSum2, vec2f);
				vecSum3 = _mm256_add_ps(vecSum3, vec3f);
			}
			vec0i = _mm256_cvttps_epi32(vecSum0);
			vec1i = _mm256_cvttps_epi32(vecSum1);
			vec2i = _mm256_cvttps_epi32(vecSum2);
			vec3i = _mm256_cvttps_epi32(vecSum3);
			vec0i = _mm256_packs_epi32(vec0i, vec1i); // _mm256_packus_epi32 is SSE4.1
			vec2i = _mm256_packs_epi32(vec2i, vec3i);
			vec0i = _mm256_packus_epi16(vec0i, vec2i);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0i);
		}
		
		/* Per #8 samples */
		for (; i < width - 7; i += 8) {
			vecSum0 = _mm256_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k])));
				vecCoeff = _mm256_set1_ps(vthzKernPtr[row]);
				vec0f = _mm256_cvtepi32_ps(vecInPtr);
				vec0f = _mm256_mul_ps(vec0f, vecCoeff);
				vecSum0 = _mm256_add_ps(vecSum0, vec0f);
			}
			vec0i = _mm256_cvttps_epi32(vecSum0);
			vec0i = _mm256_packs_epi32(vec0i, vec0i);
			vec0i = _mm256_permute4x64_epi64(vec0i, 0xD8);
			vec0i = _mm256_packus_epi16(vec0i, vec0i);
			_mm256_maskstore_epi64(reinterpret_cast<int64_t*>(&outPtr[i]), vecMaskToExtractFirst64Bits, vec0i); // ASM code: movq [mem], xmm0
		}

		/* Per #1 samples */
		for (; i < width; i += 1) {
			vecSum0n = _mm_setzero_ps();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecCoeffn = _mm_load_ss(&vthzKernPtr[row]);
				vec0fn = _mm_cvtsi32_ss(vec0fn, static_cast<int>(inPtr[i + k]));
				vec0fn = _mm_mul_ss(vec0fn, vecCoeffn);
				vecSum0n = _mm_add_ss(vecSum0n, vec0fn);
			}
			outPtr[i] = static_cast<uint8_t>(_mm_cvtt_ss2si(vecSum0n) & 0xff);
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
// no arithmetic overflow check
void CompVMathConvlt1VtHz_8u16s16s_Intrin_AVX2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	__m256i vecInPtr, vec0, vec1, vecSum0, vecSum1, vecCoeff;
	__m128i vecInPtrn, vec0n, vec1n, vecSum0n, vecSum1n, vecCoeffn;
	const __m256i vecZero = _mm256_setzero_si256();
	int sum;

	for (j = 0; j < height; ++j) {

		/* Per #32 samples */
		for (i = 0; i < width - 31; i += 32) {
			vecSum0 = _mm256_setzero_si256();
			vecSum1 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vecCoeff = _mm256_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0 = _mm256_unpacklo_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec1 = _mm256_unpackhi_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vecSum0 = _mm256_add_epi16(vecSum0, _mm256_mullo_epi16(vec0, vecCoeff));
				vecSum1 = _mm256_add_epi16(vecSum1, _mm256_mullo_epi16(vec1, vecCoeff));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), _mm256_permute2x128_si256(vecSum0, vecSum1, 0x20));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i + 16]), _mm256_permute2x128_si256(vecSum0, vecSum1, 0x31));
		}

		/* Per #16 samples */
		if (i < width - 15) {
			vecSum0n = _mm_setzero_si128();
			vecSum1n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtrn = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0n = _mm_unpacklo_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vec1n = _mm_unpackhi_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vecSum0n = _mm_add_epi16(vecSum0n, _mm_mullo_epi16(vec0n, vecCoeffn));
				vecSum1n = _mm_add_epi16(vecSum1n, _mm_mullo_epi16(vec1n, vecCoeffn));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vecSum0n);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i + 8]), vecSum1n);
			i += 16;
		}

		/* Per #8 samples */
		if (i < width - 7) {
			vecSum0n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtrn = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0n = _mm_unpacklo_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vecSum0n = _mm_add_epi16(vecSum0n, _mm_mullo_epi16(vec0n, vecCoeffn));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vecSum0n);
			i += 8;
		}

		/* Per #4 samples */
		if (i < width - 3) {
			vecSum0n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtrn = _mm_cvtsi32_si128(*reinterpret_cast<const uint32_t*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0n = _mm_unpacklo_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vecSum0n = _mm_add_epi16(vecSum0n, _mm_mullo_epi16(vec0n, vecCoeffn));
			}
			_mm_storel_epi64(reinterpret_cast<__m128i*>(&outPtr[i]), vecSum0n);
			i += 4;
		}

		/* Per #1 samples */
		for (; i < width; ++i) {
			sum = static_cast<int>(inPtr[i] * vthzKernPtr[0]);
			for (row = 1, k = step; row < kernSize; ++row, k += step) {
				sum += static_cast<int>(inPtr[i + k] * vthzKernPtr[row]);
			}
			outPtr[i] = static_cast<int16_t>(sum);
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
// no arithmetic overflow check
void CompVMathConvlt1VtHz_16s16s16s_Intrin_AVX2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	__m256i vec0, vec1, vecSum0, vecSum1, vecCoeff;
	__m128i vec0n, vecSum0n, vecCoeffn;
	const __m256i vecZero = _mm256_setzero_si256();
	int sum;

	for (j = 0; j < height; ++j) {

		/* Per #32 samples */
		for (i = 0; i < width - 31; i += 32) {
			vecSum0 = _mm256_setzero_si256();
			vecSum1 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vec1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k + 16]));
				vecCoeff = _mm256_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vecSum0 = _mm256_add_epi16(vecSum0, _mm256_mullo_epi16(vec0, vecCoeff));
				vecSum1 = _mm256_add_epi16(vecSum1, _mm256_mullo_epi16(vec1, vecCoeff));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vecSum0);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i + 16]), vecSum1);
		}

		/* Per #16 samples */
		if (i < width - 15) {
			vecSum0 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vecCoeff = _mm256_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vecSum0 = _mm256_add_epi16(vecSum0, _mm256_mullo_epi16(vec0, vecCoeff));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vecSum0);
			i += 16;
		}

		/* Per #8 samples */
		if (i < width - 7) {
			vecSum0n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0n = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vecSum0n = _mm_add_epi16(vecSum0n, _mm_mullo_epi16(vec0n, vecCoeffn));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vecSum0n);
			i += 8;
		}

		/* Per #4 samples */
		if (i < width - 3) {
			vecSum0n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0n = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vecSum0n = _mm_add_epi16(vecSum0n, _mm_mullo_epi16(vec0n, vecCoeffn));
			}
			_mm_storel_epi64(reinterpret_cast<__m128i*>(&outPtr[i]), vecSum0n);
			i += 4;
		}

		/* Per #1 samples */
		for (; i < width; ++i) {
			sum = static_cast<int>(inPtr[i] * vthzKernPtr[0]);
			for (row = 1, k = step; row < kernSize; ++row, k += step) {
				sum += static_cast<int>(inPtr[i + k] * vthzKernPtr[row]);
			}
			outPtr[i] = static_cast<int16_t>(sum);
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
// yes arithmetic overflow check
void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	__m256i vecInPtr, vec0, vec1, vecSum0, vecSum1, vecCoeff;
	__m128i vecInPtrn, vec0n, vec1n, vecSum0n, vecSum1n, vecCoeffn;
	const __m256i vecZero = _mm256_setzero_si256();
	unsigned int sum;

	for (j = 0; j < height; ++j) {

		/* Per #32 samples */
		for (i = 0; i < width - 31; i += 32) {
			vecSum0 = _mm256_setzero_si256();
			vecSum1 = _mm256_setzero_si256();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&inPtr[i + k]));
				vecCoeff = _mm256_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0 = _mm256_unpacklo_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vec1 = _mm256_unpackhi_epi8(vecInPtr, vecZero); // epi8 -> epi16
				vecSum0 = _mm256_adds_epu16(vecSum0, _mm256_mulhi_epu16(vec0, vecCoeff));
				vecSum1 = _mm256_adds_epu16(vecSum1, _mm256_mulhi_epu16(vec1, vecCoeff));
			}
			vec0 = _mm256_packus_epi16(vecSum0, vecSum1);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&outPtr[i]), vec0);
		}

		/* Per #16 samples */
		if (i < width - 15) {
			vecSum0n = _mm_setzero_si128();
			vecSum1n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtrn = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0n = _mm_unpacklo_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vec1n = _mm_unpackhi_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vecSum0n = _mm_adds_epu16(vecSum0n, _mm_mulhi_epu16(vec0n, vecCoeffn));
				vecSum1n = _mm_adds_epu16(vecSum1n, _mm_mulhi_epu16(vec1n, vecCoeffn));
			}
			vec0n = _mm_packus_epi16(vecSum0n, vecSum1n);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vec0n);
			i += 16;
		}

		/* Per #8 samples */
		if (i < width - 7) {
			vecSum0n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtrn = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0n = _mm_unpacklo_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vecSum0n = _mm_adds_epu16(vecSum0n, _mm_mulhi_epu16(vec0n, vecCoeffn));
			}
			vec0n = _mm_packus_epi16(vecSum0n, vecSum0n);
			_mm_storel_epi64(reinterpret_cast<__m128i*>(&outPtr[i]), vec0n);
			i += 8;
		}

		/* Per #4 samples */
		if (i < width - 3) {
			vecSum0n = _mm_setzero_si128();
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtrn = _mm_cvtsi32_si128(*reinterpret_cast<const uint32_t*>(&inPtr[i + k]));
				vecCoeffn = _mm_set1_epi16(static_cast<short>(vthzKernPtr[row]));
				vec0n = _mm_unpacklo_epi8(vecInPtrn, _mm256_castsi256_si128(vecZero)); // epi8 -> epi16
				vecSum0n = _mm_adds_epu16(vecSum0n, _mm_mulhi_epu16(vec0n, vecCoeffn));
			}
			vec0n = _mm_packus_epi16(vecSum0n, vecSum0n);
			*reinterpret_cast<uint32_t*>(&outPtr[i]) = _mm_cvtsi128_si32(vec0n);
			i += 4;
		}

		/* Per #1 samples */
		for (; i < width; ++i) {
			sum = static_cast<unsigned int>(inPtr[i] * vthzKernPtr[0]) >> 16;
			for (row = 1, k = step; row < kernSize; ++row, k += step) {
				sum += static_cast<unsigned int>(inPtr[i + k] * vthzKernPtr[row]) >> 16;
			}
			outPtr[i] = static_cast<uint8_t>(sum);
		}

		inPtr += stride;
		outPtr += stride;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
