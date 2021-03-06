/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_utils.h"
#include "compv/base/image/compv_image_conv_to_yuv444p.h"
#include "compv/base/image/compv_image_conv_to_grayscale.h"
#include "compv/base/image/compv_image_scale_bilinear.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_fileutils.h"

#define COMPV_THIS_CLASSNAME	"CompVImage"

#define COMPV_IMAGE_NEWOBJ_CASE(elmType,pixelFormat) \
		case COMPV_SUBTYPE_PIXELS_##pixelFormat: \
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<elmType, COMPV_MAT_TYPE_PIXELS, COMPV_SUBTYPE_PIXELS_##pixelFormat>(image, height, width, stride))); \
			return COMPV_ERROR_CODE_S_OK;
#define COMPV_IMAGE_NEWOBJ_SWITCH(elmType, subType) \
	switch (subType) { \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGB24); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGR24); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGBA32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGRA32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, ABGR32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, ARGB32); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGB565LE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, RGB565BE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGR565LE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, BGR565BE); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, Y); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, NV12); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, NV21); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV420P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YVU420P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV422P); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUYV422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, UYVY422); \
		COMPV_IMAGE_NEWOBJ_CASE(elmType, YUV444P); \
	default: \
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); \
		break; \
	}

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImage::newObj8u(CompVMatPtrPtr image, COMPV_SUBTYPE subType, size_t width, size_t height, size_t stride COMPV_DEFAULT(0))
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint8_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj16u(CompVMatPtrPtr image, COMPV_SUBTYPE subType, size_t width, size_t height, size_t stride COMPV_DEFAULT(0))
{
    COMPV_IMAGE_NEWOBJ_SWITCH(uint16_t, subType);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::newObj16s(CompVMatPtrPtr image, COMPV_SUBTYPE subType, size_t width, size_t height, size_t stride COMPV_DEFAULT(0))
{
	COMPV_IMAGE_NEWOBJ_SWITCH(int16_t, subType);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::readPixels(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t stride, const char* filePath, CompVMatPtrPtr image)
{
	COMPV_CHECK_EXP_RETURN(!filePath || !width || !height || stride < width || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr buffer;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &buffer));
	size_t expectedFileSize;
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::sizeForPixelFormat(ePixelFormat, stride, height, &expectedFileSize));
	if (expectedFileSize != buffer->size()) {
		// FFmpeg requires outputs with even width when converting from RGB to YUV
		if ((stride & 1) || (height & 1)) {
			COMPV_CHECK_CODE_RETURN(CompVImageUtils::sizeForPixelFormat(ePixelFormat, (stride + 1) & ~1, (height + 1) & ~1, &expectedFileSize));
			COMPV_CHECK_EXP_RETURN(expectedFileSize != buffer->size(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT, "Size mismatch");
		}
		else {
			COMPV_CHECK_EXP_RETURN(expectedFileSize != buffer->size(), COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT, "Size mismatch");
		}
	}
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(ePixelFormat, buffer->ptr(), width, height, stride, image));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::wrap(COMPV_SUBTYPE ePixelFormat, const void* dataPtr, size_t width, size_t height, size_t stride, CompVMatPtrPtr image)
{
	COMPV_CHECK_EXP_RETURN(!dataPtr || !width || !height || stride < width || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	// Compute best stride
	size_t bestStride = width;
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::bestStride(width, &bestStride));

#if 1
	bool bAllocNewImage = !(*image) ||
		(*image)->type() != COMPV_MAT_TYPE_PIXELS ||
		(*image)->subType() != ePixelFormat ||
		(*image)->cols() != width ||
		(*image)->rows() != height ||
		(*image)->stride() != bestStride;
#else // to test copy and alloc
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	bool bAllocNewImage = true;
#endif

	CompVMatPtr image_;

	if (bAllocNewImage) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVImage::newObj8u(&image_, ePixelFormat, width, height, bestStride)
			, "Failed to allocate new image");
	}
	else {
		image_ = *image;
	}

	if (dataPtr) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVImageUtils::copy(ePixelFormat,
			dataPtr, width, height, stride,
			(void*)image_->ptr(), image_->cols(), image_->rows(), image_->stride()), "Failed to copy image"); // copy data
	}
bail:
	if (COMPV_ERROR_CODE_IS_OK(err_)) {
		*image = image_;
	}
	return err_;
}

COMPV_ERROR_CODE CompVImage::clone(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty() || !imageOut || imageIn->type() != COMPV_MAT_TYPE_PIXELS, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(imageIn->subType(), imageIn->ptr(), imageIn->cols(), imageIn->rows(), imageIn->stride(), imageOut));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::convert(const CompVMatPtr& imageIn, COMPV_SUBTYPE pixelFormatOut, CompVMatPtrPtr imageOut)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || imageIn->isEmpty() || !imageOut || imageIn->type() != COMPV_MAT_TYPE_PIXELS, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (pixelFormatOut) {
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToYUV444P::process(imageIn, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_Y:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToGrayscale::process(imageIn, imageOut));
		return COMPV_ERROR_CODE_S_OK;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Chroma conversion not supported: %s -> %s", CompVGetSubtypeString(imageIn->subType()), CompVGetSubtypeString(pixelFormatOut));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

COMPV_ERROR_CODE CompVImage::convertGrayscale(const CompVMatPtr& imageIn, CompVMatPtrPtr imageGray)
{
	// Input parameters will be checked in 'convert'
	COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageIn, COMPV_SUBTYPE_PIXELS_Y, imageGray));
	return COMPV_ERROR_CODE_S_OK;
}

// This function is faster when the input data is planar (or semi-planar) YUV as we'll just reshape the data.
// It requires the input to be equal to the output to avoid copying, this is whay we require a single parameter
COMPV_ERROR_CODE CompVImage::convertGrayscaleFast(CompVMatPtr& imageInOut)
{
	// Input parameters will be checked in 'convert'
	COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageInOut, COMPV_SUBTYPE_PIXELS_Y, &imageInOut));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImage::scale(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, size_t widthOut, size_t heightOut, COMPV_SCALE_TYPE scaleType COMPV_DEFAULT(COMPV_SCALE_TYPE_BILINEAR))
{
	COMPV_CHECK_EXP_RETURN(!imageIn || !imageOut || imageIn->isEmpty() || !widthOut || !heightOut, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	bool bSelfTransfer = imageIn == (*imageOut);
	bool bScaleFactor1 = widthOut == imageIn->cols() && heightOut == imageIn->rows();

	if (bSelfTransfer && bScaleFactor1) {
		// No scaling and output is equal to input
		return COMPV_ERROR_CODE_S_OK;
	}

	CompVMatPtr imageOut_ = (imageIn == *imageOut) ? nullptr : *imageOut; // When (imageIn == imageOut) we have to save imageIn
	size_t strideOut = widthOut;
	COMPV_CHECK_CODE_RETURN(CompVImageUtils::bestStride(strideOut, &strideOut));
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut_, imageIn->subType(), widthOut, heightOut, strideOut));

	if (bScaleFactor1 & !CompVBase::isTestingMode()) { // In testing mode we may want to encode the same image several times to check CPU, Memory, Latency...
		if (bSelfTransfer) {
			// *outImage = This is enought
			return COMPV_ERROR_CODE_S_OK;
		}
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
			imageIn->subType(), imageIn->ptr(), imageIn->cols(), imageIn->rows(), imageIn->stride(),
			imageOut_->ptr<void>(), imageOut_->cols(), imageOut_->rows(), imageOut_->stride()
		));
		*imageOut = imageOut_;
		return COMPV_ERROR_CODE_S_OK;
	}

	switch (scaleType) {
	case COMPV_SCALE_TYPE_BILINEAR:
		COMPV_CHECK_CODE_RETURN(CompVImageScaleBilinear::process(imageIn, imageOut_));
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%d not supported as scaling type", scaleType);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Invalid scaling type");
		break;
	}
	*imageOut = imageOut_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
