/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/intrinsics/x86/compv_imageconv_rgba_i420_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba;
	__m256i ymmYCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_YCoeffs8);
	__m256i ymm16 = _mm256_load_si256((__m256i*)k16_i16);
	__m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kMaskstore_0_i64);
	vcomp_scalar_t i, j, maxI = ((width + 7) & -8), padY = (stride - maxI), padRGBA = padY << 2;

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 8) {
			_mm256_store_si256(&ymmRgba, _mm256_load_si256((__m256i*)rgbaPtr)); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba, _mm256_maddubs_epi16(ymmRgba, ymmYCoeffs)); // 
			_mm256_store_si256(&ymmRgba, _mm256_hadd_epi16(ymmRgba, ymmRgba)); // aaaabbbbaaaabbbb
			_mm256_store_si256(&ymmRgba, _mm256_permute4x64_epi64(ymmRgba, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // aaaaaaaabbbbbbbb
			_mm256_store_si256(&ymmRgba, _mm256_srai_epi16(ymmRgba, 7)); // >> 7
			_mm256_store_si256(&ymmRgba, _mm256_add_epi16(ymmRgba, ymm16)); // + 16
			_mm256_store_si256(&ymmRgba, _mm256_packus_epi16(ymmRgba, ymmRgba)); // Saturate(I16 -> U8)
#if 1		// best way to use AVX code *only* and avoid AVX/SSE mixing penalities
			_mm256_maskstore_epi64((int64_t*)outYPtr, ymmMaskToExtractFirst64Bits, ymmRgba);
#else
			*((int64_t*)outYPtr) = _mm_cvtsi128_si64(_mm256_castsi256_si128(ymmRgba));
#endif

			outYPtr += 8;
			rgbaPtr += 32;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
	_mm256_zeroupper();
}

void rgbaToI420Kernel41_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba0, ymmRgba1, ymmRgba2, ymmRgba3;
	__m256i ymmYCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_YCoeffs8);
	__m256i ymm16 = _mm256_load_si256((__m256i*)k16_i16);
	vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGBA = padY << 2;

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			_mm256_store_si256(&ymmRgba0, _mm256_load_si256((__m256i*)(rgbaPtr))); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba1, _mm256_load_si256((__m256i*)(rgbaPtr + 32))); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba2, _mm256_load_si256((__m256i*)(rgbaPtr + 64))); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba3, _mm256_load_si256((__m256i*)(rgbaPtr + 96))); // 8 RGBA samples
			
			_mm256_store_si256(&ymmRgba0, _mm256_maddubs_epi16(ymmRgba0, ymmYCoeffs));
			_mm256_store_si256(&ymmRgba1, _mm256_maddubs_epi16(ymmRgba1, ymmYCoeffs));
			_mm256_store_si256(&ymmRgba2, _mm256_maddubs_epi16(ymmRgba2, ymmYCoeffs));
			_mm256_store_si256(&ymmRgba3, _mm256_maddubs_epi16(ymmRgba3, ymmYCoeffs));

			_mm256_store_si256(&ymmRgba0, _mm256_hadd_epi16(ymmRgba0, ymmRgba1)); // 0000111100001111
			_mm256_store_si256(&ymmRgba2, _mm256_hadd_epi16(ymmRgba2, ymmRgba3)); // 2222333322223333

			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // 0000000011111111
			_mm256_store_si256(&ymmRgba2, _mm256_permute4x64_epi64(ymmRgba2, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // 2222222233333333
			
			_mm256_store_si256(&ymmRgba0, _mm256_srai_epi16(ymmRgba0, 7)); // >> 7
			_mm256_store_si256(&ymmRgba2, _mm256_srai_epi16(ymmRgba2, 7)); // >> 7

			_mm256_store_si256(&ymmRgba0, _mm256_add_epi16(ymmRgba0, ymm16)); // + 16
			_mm256_store_si256(&ymmRgba2, _mm256_add_epi16(ymmRgba2, ymm16)); // + 16

			_mm256_store_si256(&ymmRgba0, _mm256_packus_epi16(ymmRgba0, ymmRgba2)); // Saturate(I16 -> U8): 002200220022...
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0))); //000000022222.....
			
			_mm256_storeu_si256((__m256i*)outYPtr, ymmRgba0);

			outYPtr += 32;
			rgbaPtr += 128;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
	_mm256_zeroupper();
}

void rgbaToI420Kernel11_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba;
#if 0
	__m128i xmmUV;
#endif
	__m256i ymmUV4Coeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_U4V4Coeffs8); // UV coeffs interleaved: each appear #4 times
	__m256i ymm128 = _mm256_load_si256((__m256i*)k128_i16);
	__m256i ymmMaskToExtractFirst32Bits = _mm256_load_si256((__m256i*)kMaskstore_0_i32);
	vcomp_scalar_t i, j, maxI = ((width + 7) & -8), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 8) {
			_mm256_store_si256(&ymmRgba, _mm256_load_si256((__m256i*)rgbaPtr)); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			_mm256_store_si256(&ymmRgba, _mm256_maddubs_epi16(ymmRgba, ymmUV4Coeffs)); // Ua Ub Uc Ud Va Vb Vc Vd
			_mm256_store_si256(&ymmRgba, _mm256_hadd_epi16(ymmRgba, ymmRgba));
			_mm256_store_si256(&ymmRgba, _mm256_permute4x64_epi64(ymmRgba, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
			_mm256_store_si256(&ymmRgba, _mm256_srai_epi16(ymmRgba, 8)); // >> 8
			_mm256_store_si256(&ymmRgba, _mm256_add_epi16(ymmRgba, ymm128)); // + 128 -> UUVV----
			_mm256_store_si256(&ymmRgba, _mm256_packus_epi16(ymmRgba, ymmRgba)); // Saturate(I16 -> U8)
			
#if 1 // best way to use AVX code *only* and avoid AVX/SSE mixing penalities
			_mm256_maskstore_epi32((int*)outUPtr, ymmMaskToExtractFirst32Bits, ymmRgba);
			_mm256_store_si256(&ymmRgba, _mm256_srli_si256(ymmRgba, 4)); // >> 4
			_mm256_maskstore_epi32((int*)outVPtr, ymmMaskToExtractFirst32Bits, ymmRgba);
#else
			_mm_store_si128(&xmmUV, _mm256_castsi256_si128(ymmRgba)); // UV
			*((uint*)outUPtr) = _mm_cvtsi128_si32(xmmUV);
			_mm_store_si128(&xmmUV, _mm_srli_si128(xmmUV, 4)); // >> 4
			*((uint*)outVPtr) = _mm_cvtsi128_si32(xmmUV);
#endif

			outUPtr += 4;
			outVPtr += 4;
			rgbaPtr += 32; // 4 * 8
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

void rgbaToI420Kernel41_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba0, ymmRgba1, ymmRgba2, ymmRgba3, ymm0, ymm1;
	__m256i ymmUCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_UCoeffs8);
	__m256i ymmVCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_VCoeffs8);
	__m256i ymm128 = _mm256_load_si256((__m256i*)k128_i16);
	__m256i ymmMaskToExtract128bits = _mm256_load_si256((__m256i*)kMaskstore_0_1_i64);
	vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines
	
	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 32) {			
			// Read 32 RGBA samples
			_mm256_store_si256(&ymmRgba0, _mm256_load_si256((__m256i*)(rgbaPtr))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			_mm256_store_si256(&ymmRgba1, _mm256_load_si256((__m256i*)(rgbaPtr + 32))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): exfxgxhx
			_mm256_store_si256(&ymmRgba2, _mm256_load_si256((__m256i*)(rgbaPtr + 64))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): ixjxkxlx
			_mm256_store_si256(&ymmRgba3, _mm256_load_si256((__m256i*)(rgbaPtr + 96))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): mxnxoxpx

			_mm256_store_si256(&ymm0, _mm256_unpacklo_epi32(ymmRgba0, ymmRgba1)); // aexxcgxx
			_mm256_store_si256(&ymm1, _mm256_unpackhi_epi32(ymmRgba0, ymmRgba1)); // bfxxdhxx
			_mm256_store_si256(&ymmRgba0, _mm256_unpacklo_epi32(ymm0, ymm1)); // abefcdgh
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // abcdefgh
			_mm256_store_si256(&ymmRgba1, ymmRgba0);

			_mm256_store_si256(&ymm0, _mm256_unpacklo_epi32(ymmRgba2, ymmRgba3)); // imxxkoxx
			_mm256_store_si256(&ymm1, _mm256_unpackhi_epi32(ymmRgba2, ymmRgba3)); // jnxxlpxx
			_mm256_store_si256(&ymmRgba2, _mm256_unpacklo_epi32(ymm0, ymm1)); // ijmnklop
			_mm256_store_si256(&ymmRgba2, _mm256_permute4x64_epi64(ymmRgba2, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // ijklmnop
			_mm256_store_si256(&ymmRgba3, ymmRgba2);

			// U = (ymmRgba0, ymmRgba2)
			// V = (ymmRgba1, ymmRgba3)
			_mm256_store_si256(&ymmRgba0, _mm256_maddubs_epi16(ymmRgba0, ymmUCoeffs));
			_mm256_store_si256(&ymmRgba2, _mm256_maddubs_epi16(ymmRgba2, ymmUCoeffs));
			_mm256_store_si256(&ymmRgba1, _mm256_maddubs_epi16(ymmRgba1, ymmVCoeffs));
			_mm256_store_si256(&ymmRgba3, _mm256_maddubs_epi16(ymmRgba3, ymmVCoeffs));

			// U = ymmRgba0
			// V = ymmRgba1
			_mm256_store_si256(&ymmRgba0, _mm256_hadd_epi16(ymmRgba0, ymmRgba2));
			_mm256_store_si256(&ymmRgba1, _mm256_hadd_epi16(ymmRgba1, ymmRgba3));
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
			_mm256_store_si256(&ymmRgba1, _mm256_permute4x64_epi64(ymmRgba1, COMPV_MM_SHUFFLE(3, 1, 2, 0)));

			_mm256_store_si256(&ymmRgba0, _mm256_srai_epi16(ymmRgba0, 8)); // >> 8
			_mm256_store_si256(&ymmRgba1, _mm256_srai_epi16(ymmRgba1, 8)); // >> 8

			_mm256_store_si256(&ymmRgba0, _mm256_add_epi16(ymmRgba0, ymm128)); // + 128 -> UUVV----
			_mm256_store_si256(&ymmRgba1, _mm256_add_epi16(ymmRgba1, ymm128)); // + 128 -> UUVV----
			
			// UV = ymmRgba0
			_mm256_store_si256(&ymmRgba0, _mm256_packus_epi16(ymmRgba0, ymmRgba1)); // Saturate(I16 -> U8)
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0)));			

#if 1		// Best way to have AVX code *only* and avoid SSE/AVX mixing penalities
			_mm256_maskstore_epi64((int64_t*)outUPtr, ymmMaskToExtract128bits, ymmRgba0);
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(0, 0, 3, 2)));
			_mm256_maskstore_epi64((int64_t*)outVPtr, ymmMaskToExtract128bits, ymmRgba0);
			
#elif 0
			_mm256_store_si256(&ymmRgba1, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(1, 0, 3, 2)));
			_mm_storeu_si128((__m128i*)outUPtr, _mm256_castsi256_si128(ymmRgba0));
			_mm_storeu_si128((__m128i*)outVPtr, _mm256_castsi256_si128(ymmRgba1));
#elif 0
			// ASM code
			vmovups xmm0,xmm2
			vextractf128 xmm1,ymm2,1
#elif 0
			// SSE / AVX mix, no way to invoke _mm256_zeroupper()
			_mm_storeu_si128((__m128i*)outUPtr, _mm256_castsi256_si128(ymmRgba0));
			_mm_storeu_si128((__m128i*)outVPtr, _mm256_extractf128_si256(ymmRgba0, 0x1));
#elif 0
			// SSE code is inserted 
			_mm256_storeu2_m128i((__m128i*)outVPtr, (__m128i*)outUPtr, ymmRgba0);
#endif

			outUPtr += 16;
			outVPtr += 16;
			rgbaPtr += 128;
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

void i420ToRGBAKernel11_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(AVX2) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();

	vcomp_scalar_t i, j, maxI = ((width + 31) & -32), rollbackUV = -((maxI + 1) >> 1), padY = (stride - maxI), padUV = ((padY + 1) >> 1), padRGBA = (padY << 2); // +stride to skip even lines
	__m256i ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymmY, ymmU, ymmV, ymm16, ymmRCoeffs, ymmGCoeffs, ymmBCoeffs, ymmZeroCoeffs, ymmAlpha, ymmMaskToExtract128bits;

	_mm256_store_si256(&ymmRCoeffs, _mm256_load_si256((__m256i*)kYUVToRGBA_RCoeffs8));
	_mm256_store_si256(&ymmGCoeffs, _mm256_load_si256((__m256i*)kYUVToRGBA_GCoeffs8));
	_mm256_store_si256(&ymmBCoeffs, _mm256_load_si256((__m256i*)kYUVToRGBA_BCoeffs8));
	_mm256_store_si256(&ymmZeroCoeffs, _mm256_setzero_si256());
	_mm256_store_si256(&ymm5, _mm256_load_si256((__m256i*)k5_i8));
	_mm256_store_si256(&ymm16, _mm256_load_si256((__m256i*)k16_i8));
	_mm256_store_si256(&ymmAlpha, _mm256_load_si256((__m256i*)k255_i16));
	_mm256_store_si256(&ymmMaskToExtract128bits, _mm256_load_si256((__m256i*)kMaskstore_0_1_i64));

	// R!u8 = (37Y' + 0U' + 51V') >> 5
	// G!u8 = (37Y' - 13U' - 26V') >> 5
	// B!u8 = (37Y' + 65U' + 0V') >> 5
	// where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
	// _mm_subs_epu8(U, 128) produce overflow -> use I16
	// R!i16 = (37Y + 0U + 51V - 7120) >> 5
	// G!i16 = (37Y - 13U - 26V + 4400) >> 5
	// B!i16 = (37Y + 65U + 0V - 8912) >> 5
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			_mm256_store_si256(&ymmY, _mm256_load_si256((__m256i*)yPtr)); // 32 Y samples
			_mm256_store_si256(&ymmU, _mm256_maskload_epi64((int64_t const*)uPtr, ymmMaskToExtract128bits)); // 16 U samples, low mem
			_mm256_store_si256(&ymmV, _mm256_maskload_epi64((int64_t const*)vPtr, ymmMaskToExtract128bits)); // 16 V samples, low mem

			/*COMV_ALIGN_DEFAULT() int32_t mask0[8] = {
				0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
				0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c,
			};*/
#define COMPV_MM_SHUFFLE_EPI8(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | (a & 0xFF))
			/*COMV_ALIGN_DEFAULT() int32_t mask_shuffle_epi8_permute4x64_0_2_1_3[] = { // AVX-only
				COMPV_MM_SHUFFLE_EPI8(0, 1, 2, 3), COMPV_MM_SHUFFLE_EPI8(4, 5, 6, 7), COMPV_MM_SHUFFLE_EPI8(8, 9, 10, 11), COMPV_MM_SHUFFLE_EPI8(12, 13, 14, 15),
				COMPV_MM_SHUFFLE_EPI8(16, 17, 18, 19), COMPV_MM_SHUFFLE_EPI8(20, 21, 22, 23), COMPV_MM_SHUFFLE_EPI8(24, 25, 26, 27), COMPV_MM_SHUFFLE_EPI8(28, 29, 30, 31),
			};*/
			static COMV_ALIGN_DEFAULT() const int32_t mask_shuffle_epi8_permute4x64_0_2_1_3[] = { // AVX-only
				COMPV_MM_SHUFFLE_EPI8(0, 8, 1, 9), COMPV_MM_SHUFFLE_EPI8(2, 10, 3, 11), COMPV_MM_SHUFFLE_EPI8(4, 12, 5, 13), COMPV_MM_SHUFFLE_EPI8(6, 14, 7, 15), // 128bits-lane(can only mov 0-15)
				COMPV_MM_SHUFFLE_EPI8(16, 24, 17, 25), COMPV_MM_SHUFFLE_EPI8(18, 26, 19, 27), COMPV_MM_SHUFFLE_EPI8(20, 28, 21, 29), COMPV_MM_SHUFFLE_EPI8(22, 30, 23, 31),  // 128-lane(can only mov 16-31)
			};
			/*ymmZeroCoeffs = _mm256_set_epi8(
				32, 31, 30, 29, 28, 27, 26, 25,
				24, 23, 22, 21, 20, 19, 18, 17, 
				16, 15, 14, 13, 12, 11, 10, 9, 
				8, 7, 6, 5, 4, 3, 2, 1);
			ymmZeroCoeffs = _mm256_shuffle_epi8(ymmZeroCoeffs, _mm256_load_si256((__m256i*)mask_shuffle_epi8_permute4x64_0_2_1_3));
			int a = 0;*/
			//_mm256_shuffle_epi8(_mm256_unpacklo_epi64(_mm256_unpacklo_epi8(ymm0_, ymm1_), _mm256_unpackhi_epi8(ymm0_, ymm1_)), _mm256_load_si256((__m256i*)mask_shuffle_epi8_permute4x64_0_2_1_3)) //
			
#if 0
#define avx2_unpacklo_epi8(ymm0_, ymm1_)  _mm256_unpacklo_epi8(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define avx2_unpackhi_epi8(ymm0_, ymm1_) _mm256_unpackhi_epi8(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define avx2_hadd_epi16(ymm0_, ymm1_) _mm256_permute4x64_epi64(_mm256_hadd_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define avx2_packus_epi16(ymm0_, ymm1_) _mm256_permute4x64_epi64(_mm256_packus_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define avx2_unpacklo_epi16(ymm0_, ymm1_) _mm256_unpacklo_epi16(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define avx2_unpackhi_epi16(ymm0_, ymm1_) _mm256_unpackhi_epi16(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#else
#define avx2_unpacklo_epi8(ymm0_, ymm1_) _mm256_unpacklo_epi8(ymm0_, ymm1_)
#define avx2_unpackhi_epi8(ymm0_, ymm1_) _mm256_unpackhi_epi8(ymm0_, ymm1_)
#define avx2_hadd_epi16(ymm0_, ymm1_) _mm256_hadd_epi16(ymm0_, ymm1_)
#define avx2_packus_epi16(ymm0_, ymm1_) _mm256_packus_epi16(ymm0_, ymm1_)
#define avx2_unpacklo_epi16(ymm0_, ymm1_) _mm256_unpacklo_epi16(ymm0_, ymm1_)
#define avx2_unpackhi_epi16(ymm0_, ymm1_) _mm256_unpackhi_epi16(ymm0_, ymm1_)
#endif

			// AVX computations don't cross the 128bits line which means we can perform all the computations without permutations until the end of the function.
			// Important: The imput values (ymmY, ymmU, ymmV) must lay in the same 128bit-lane before we begin the computation ("Computation start" label).

#if 0
			// U = ABXX (64bits), XX being high 128bits filled with zeros
			_mm256_store_si256(&ymmU, avx2_unpacklo_epi8(ymmU, ymmU)); // inteleave(A,A)||inteleave(B,B) -> duplicate each sample -> 32 U samples
			// V = ABXX (64bits), XX being high 128bits filled with garbage
			_mm256_store_si256(&ymmV, avx2_unpacklo_epi8(ymmV, ymmV)); // inteleave(A,A)||inteleave(B,B) -> duplicate each sample -> 32 V samples
#else
			_mm256_store_si256(&ymm0, avx2_unpacklo_epi8(ymmU, ymmU));
			_mm256_store_si256(&ymm1, avx2_unpackhi_epi8(ymmU, ymmU));
			_mm256_store_si256(&ymmU, _mm256_permute2x128_si256(ymm0, ymm1, 0x0002));

			_mm256_store_si256(&ymm0, avx2_unpacklo_epi8(ymmV, ymmV));
			_mm256_store_si256(&ymm1, avx2_unpackhi_epi8(ymmV, ymmV));
			_mm256_store_si256(&ymmV, _mm256_permute2x128_si256(ymm0, ymm1, 0x0002));
#endif
			// Label: "Computation start"

			/////// 16Y - LOW ///////

			// YUV0 = (ymm2 || ymm3)
			_mm256_store_si256(&ymm0, avx2_unpacklo_epi8(ymmY, ymmV)); // YVYVYVYVYVYVYV....
			_mm256_store_si256(&ymm1, avx2_unpacklo_epi8(ymmU, ymmZeroCoeffs)); //U0U0U0U0U0U0U0U0....
			_mm256_store_si256(&ymm2, avx2_unpacklo_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0
			_mm256_store_si256(&ymm3, avx2_unpackhi_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0

			// ymm0 = R
			_mm256_store_si256(&ymm0, _mm256_maddubs_epi16(ymm2, ymmRCoeffs));
			_mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm3, ymmRCoeffs));
			_mm256_store_si256(&ymm0, avx2_hadd_epi16(ymm0, ymm1));
			_mm256_store_si256(&ymm0, _mm256_sub_epi16(ymm0, _mm256_set1_epi16(7120)));
			_mm256_store_si256(&ymm0, _mm256_srai_epi16(ymm0, 5)); // >> 5
			// ymm1 = B
			_mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm2, ymmBCoeffs));
			_mm256_store_si256(&ymm4, _mm256_maddubs_epi16(ymm3, ymmBCoeffs));
			_mm256_store_si256(&ymm1, avx2_hadd_epi16(ymm1, ymm4));
			_mm256_store_si256(&ymm1, _mm256_sub_epi16(ymm1, _mm256_set1_epi16(8912)));
			_mm256_store_si256(&ymm1, _mm256_srai_epi16(ymm1, 5)); // >> 5
			// ymm4 = RBRBRBRBRBRB
			_mm256_store_si256(&ymm4, avx2_unpacklo_epi16(ymm0, ymm1)); // low16(RBRBRBRBRBRB)
			_mm256_store_si256(&ymm5, avx2_unpackhi_epi16(ymm0, ymm1)); // high16(RBRBRBRBRBRB)
			_mm256_store_si256(&ymm4, avx2_packus_epi16(ymm4, ymm5)); // u8(RBRBRBRBRBRB)

			// ymm2 = G
			_mm256_store_si256(&ymm2, _mm256_maddubs_epi16(ymm2, ymmGCoeffs));
			_mm256_store_si256(&ymm3, _mm256_maddubs_epi16(ymm3, ymmGCoeffs));
			_mm256_store_si256(&ymm2, avx2_hadd_epi16(ymm2, ymm3));
			_mm256_store_si256(&ymm2, _mm256_add_epi16(ymm2, _mm256_set1_epi16(4400)));
			_mm256_store_si256(&ymm2, _mm256_srai_epi16(ymm2, 5)); // >> 5
			// ymm3 = GAGAGAGAGAGAGA
			_mm256_store_si256(&ymm3, avx2_unpacklo_epi16(ymm2, ymmAlpha)); // low16(GAGAGAGAGAGAGA)
			_mm256_store_si256(&ymm2, avx2_unpackhi_epi16(ymm2, ymmAlpha)); // high16(GAGAGAGAGAGAGA)
			_mm256_store_si256(&ymm3, avx2_packus_epi16(ymm3, ymm2)); // u8(GAGAGAGAGAGAGA)

			// outRgbaPtr[0-64] = RGBARGBARGBARGBA
#if 0
			_mm256_store_si256((__m256i*)(outRgbaPtr + 0), avx2_unpacklo_epi8(ymm4, ymm3)); // low8(RGBARGBARGBARGBA)
			_mm256_store_si256((__m256i*)(outRgbaPtr + 32), avx2_unpackhi_epi8(ymm4, ymm3)); // high8(RGBARGBARGBARGBA)
#else
			_mm256_store_si256((__m256i*)(outRgbaPtr + 0), _mm256_permute4x64_epi64(avx2_unpacklo_epi8(ymm4, ymm3), COMPV_MM_SHUFFLE(3, 1, 2, 0)));
			_mm256_store_si256((__m256i*)(outRgbaPtr + 32), _mm256_permute4x64_epi64(avx2_unpackhi_epi8(ymm4, ymm3), COMPV_MM_SHUFFLE(3, 1, 2, 0)));

			/*_mm256_store_si256(&ymm0, avx2_unpacklo_epi8(ymm3, ymm3));
			_mm256_store_si256(&ymm1, avx2_unpackhi_epi8(ymm3, ymm3));
			_mm256_store_si256(&ymm0, _mm256_permute2x128_si256(ymm0, ymm1, 0x0002));
			_mm256_store_si256((__m256i*)(outRgbaPtr + 32), ymm0);*/
#endif


			/////// 16Y - HIGH ///////

			// YUV0 = (ymm2 || ymm3)
			_mm256_store_si256(&ymm0, avx2_unpackhi_epi8(ymmY, ymmV)); // YVYVYVYVYVYVYV....
			_mm256_store_si256(&ymm1, avx2_unpackhi_epi8(ymmU, ymmZeroCoeffs)); //U0U0U0U0U0U0U0U0....
			_mm256_store_si256(&ymm2, avx2_unpacklo_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0
			_mm256_store_si256(&ymm3, avx2_unpackhi_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0

			// ymm0 = R
			_mm256_store_si256(&ymm0, _mm256_maddubs_epi16(ymm2, ymmRCoeffs));
			_mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm3, ymmRCoeffs));
			_mm256_store_si256(&ymm0, avx2_hadd_epi16(ymm0, ymm1));
			_mm256_store_si256(&ymm0, _mm256_sub_epi16(ymm0, _mm256_set1_epi16(7120)));
			_mm256_store_si256(&ymm0, _mm256_srai_epi16(ymm0, 5)); // >> 5
			// ymm1 = B
			_mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm2, ymmBCoeffs));
			_mm256_store_si256(&ymm4, _mm256_maddubs_epi16(ymm3, ymmBCoeffs));
			_mm256_store_si256(&ymm1, avx2_hadd_epi16(ymm1, ymm4));
			_mm256_store_si256(&ymm1, _mm256_sub_epi16(ymm1, _mm256_set1_epi16(8912)));
			_mm256_store_si256(&ymm1, _mm256_srai_epi16(ymm1, 5)); // >> 5
			// ymm4 = RBRBRBRBRBRB
			_mm256_store_si256(&ymm4, avx2_unpacklo_epi16(ymm0, ymm1)); // low16(RBRBRBRBRBRB)
			_mm256_store_si256(&ymm5, avx2_unpackhi_epi16(ymm0, ymm1)); // high16(RBRBRBRBRBRB)
			_mm256_store_si256(&ymm4, avx2_packus_epi16(ymm4, ymm5)); // u8(RBRBRBRBRBRB)

			// ymm2 = G
			_mm256_store_si256(&ymm2, _mm256_maddubs_epi16(ymm2, ymmGCoeffs));
			_mm256_store_si256(&ymm3, _mm256_maddubs_epi16(ymm3, ymmGCoeffs));
			_mm256_store_si256(&ymm2, avx2_hadd_epi16(ymm2, ymm3));
			_mm256_store_si256(&ymm2, _mm256_add_epi16(ymm2, _mm256_set1_epi16(4400)));
			_mm256_store_si256(&ymm2, _mm256_srai_epi16(ymm2, 5)); // >> 5
			// ymm3 = GAGAGAGAGAGAGA
			_mm256_store_si256(&ymm3, avx2_unpacklo_epi16(ymm2, ymmAlpha)); // low16(GAGAGAGAGAGAGA)
			_mm256_store_si256(&ymm2, avx2_unpackhi_epi16(ymm2, ymmAlpha)); // high16(GAGAGAGAGAGAGA)
			_mm256_store_si256(&ymm3, avx2_packus_epi16(ymm3, ymm2)); // u8(GAGAGAGAGAGAGA)
			
			// outRgbaPtr[64-128] = RGBARGBARGBARGBA
#if 0
			_mm256_store_si256((__m256i*)(outRgbaPtr + 64), avx2_unpacklo_epi8(ymm4, ymm3)); // low8(RGBARGBARGBARGBA)
			_mm256_store_si256((__m256i*)(outRgbaPtr + 96), avx2_unpackhi_epi8(ymm4, ymm3)); // high8(RGBARGBARGBARGBA)
#else
			_mm256_store_si256((__m256i*)(outRgbaPtr + 64), _mm256_permute4x64_epi64(avx2_unpacklo_epi8(ymm4, ymm3), COMPV_MM_SHUFFLE(3, 1, 2, 0)));
			_mm256_store_si256((__m256i*)(outRgbaPtr + 96), _mm256_permute4x64_epi64(avx2_unpackhi_epi8(ymm4, ymm3), COMPV_MM_SHUFFLE(3, 1, 2, 0)));
#endif

			yPtr += 32;
			uPtr += 16;
			vPtr += 16;
			outRgbaPtr += 128;
		}
		yPtr += padY;
#if 1
		uPtr += (j & 1) ? padUV : rollbackUV;
		vPtr += (j & 1) ? padUV : rollbackUV;
#else
		uPtr += ((j & 1) * padUV) + (((j + 1) & 1) * rollbackUV);
		vPtr += ((j & 1) * padUV) + (((j + 1) & 1) * rollbackUV);
#endif
		outRgbaPtr += padRGBA;
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */