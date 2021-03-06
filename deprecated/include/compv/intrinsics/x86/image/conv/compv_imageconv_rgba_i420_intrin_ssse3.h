/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_INTRIN_SSSE3_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_INTRIN_SSSE3_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
void rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
void rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);
void rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);

void rgbToI420Kernel31_CompY_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
void rgbToI420Kernel31_CompUV_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);

void i420ToRGBAKernel11_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
void i420ToRGBAKernel41_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC*/

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_INTRIN_SSSE3_H_ */