/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_COMMON_H_)
#define _COMPV_BASE_COMMON_H_

#include "compv/base/compv_config.h"

#include <vector>
#include <algorithm>

COMPV_NAMESPACE_BEGIN()

#define COMPV_CAT_(A, B) A ## B
#define COMPV_CAT(A, B) COMPV_CAT_(A, B)
#define COMPV_STRING_(A) #A
#define COMPV_STRING(A) COMPV_STRING_(A)

#define COMPV_VERSION_MAJOR 1
#define COMPV_VERSION_MINOR 0
#define COMPV_VERSION_MICRO 0
#if !defined(COMPV_VERSION_STRING)
#	define COMPV_VERSION_STRING COMPV_STRING(COMPV_CAT(COMPV_VERSION_MAJOR, .)) COMPV_STRING(COMPV_CAT(COMPV_VERSION_MINOR, .)) COMPV_STRING(COMPV_VERSION_MICRO)
#endif

#if !defined(COMPV_SAFE_DELETE_CPP)
#	define COMPV_SAFE_DELETE_CPP(cpp_obj) if(cpp_obj) delete (cpp_obj), (cpp_obj) = NULL;
#endif /* COMPV_SAFE_DELETE_CPP */

#if defined(NDEBUG)
#	define COMPV_ASSERT(x) do { bool __COMPV_b_ret = (x); if (!__COMPV_b_ret) { COMPV_DEBUG_FATAL("Assertion failed!"); abort(); } } while(0)
#else
#	define COMPV_ASSERT(x) do { bool __COMPV_b_ret = (x); assert(__COMPV_b_ret); } while(0)
#endif

#if COMPV_INTRINSIC
#	define COMPV_EXEC_IFDEF_INTRIN(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN(EXPR)
#endif
#if COMPV_INTRINSIC && COMPV_ARCH_X86
#	define COMPV_EXEC_IFDEF_INTRIN_X86(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN_X86(EXPR)
#endif
#if COMPV_INTRINSIC && COMPV_ARCH_ARM
#	define COMPV_EXEC_IFDEF_INTRIN_ARM(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN_ARM(EXPR)
#endif
#if COMPV_INTRINSIC && COMPV_ARCH_ARM64
#	define COMPV_EXEC_IFDEF_INTRIN_ARM64(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntrinsicsEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTRIN_ARM64(EXPR)
#endif
#if COMPV_ASM
#	define COMPV_EXEC_IFDEF_ASM(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_X64
#	define COMPV_EXEC_IFDEF_ASM_X64(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_X64(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_X86
#	define COMPV_EXEC_IFDEF_ASM_X86(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_X86(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_ARM32
#	define COMPV_EXEC_IFDEF_ASM_ARM32(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_ARM32(EXPR)
#endif
#if COMPV_ASM && COMPV_ARCH_ARM64
#	define COMPV_EXEC_IFDEF_ASM_ARM64(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isAsmEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_ASM_ARM64(EXPR)
#endif
#if COMPV_HAVE_INTEL_IPP
#	define COMPV_EXEC_IFDEF_INTEL_IPP(EXPR) do { if (COMPV_NAMESPACE::CompVCpu::isIntelIppEnabled()) { (EXPR); } } while(0)
#else
#	define COMPV_EXEC_IFDEF_INTEL_IPP(EXPR)
#endif

#define COMPV_NUM_THREADS_SINGLE	1
#define COMPV_NUM_THREADS_MULTI		-1

#if !defined(COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET)
#	define COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET 32
#endif

#if !defined(COMPV_PLANE_MAX_COUNT)
#	define COMPV_PLANE_MAX_COUNT		4
#endif /* COMPV_PLANE_MAX_COUNT */
#define COMPV_PLANE_Y		0
#define COMPV_PLANE_U		1
#define COMPV_PLANE_V		2
#define COMPV_PLANE_UV		1

// Fixed point Q value
#if COMPV_ARCH_ARM
#	define COMPV_FXPQ	15
#else
#	define COMPV_FXPQ	16
#endif

#define COMPV_IS_ALIGNED(p, a) (!((uintptr_t)(p) & ((a) - 1)))
#define COMPV_IS_ALIGNED_MMX(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_MMX)
#define COMPV_IS_ALIGNED_SSE(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_SSE)
#define COMPV_IS_ALIGNED_AVX(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_AVX)
#define COMPV_IS_ALIGNED_AVX2(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_AVX2)
#define COMPV_IS_ALIGNED_AVX512(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_AVX512)
#define COMPV_IS_ALIGNED_NEON(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_NEON)
#define COMPV_IS_ALIGNED_DEFAULT(p) COMPV_IS_ALIGNED(p, COMPV_ALIGNV_SIMD_DEFAULT)

#define COMPV_ALIGNED(x)
#define COMPV_ALIGNED_DEFAULT(x)
#define COMPV_ALIGN_DEFAULT() COMPV_ALIGN(COMPV_ALIGNV_SIMD_DEFAULT)
#define COMPV_ALIGN_AVX() COMPV_ALIGN(COMPV_ALIGNV_SIMD_AVX)
#define COMPV_ALIGN_AVX2() COMPV_ALIGN(COMPV_ALIGNV_SIMD_AVX2)
#define COMPV_ALIGN_SSE() COMPV_ALIGN(COMPV_ALIGNV_SIMD_SSE)
#define COMPV_ALIGN_MMX() COMPV_ALIGN(COMPV_ALIGNV_SIMD_MMX)
#define COMPV_ALIGN_NEON() COMPV_ALIGN(COMPV_ALIGNV_SIMD_NEON)

#define COMPV_DEFAULT_ARG(arg_, val_) arg_

#define COMPV_IS_POW2(x) (((x) != 0) && !((x) & ((x) - 1)))

#define CompVPtrDef(T)			CompVPtr<T* >

#define CompVPtrBox(T)			CompVPtrDef(CompVBox<T >)
#define CompVPtrBoxPoint(T)		CompVPtrDef(CompVBox<CompVPoint<T > >)

#define CompVPtrBoxNew(T)		CompVBox<T >::newObj
#define CompVPtrBoxPointNew(T)	CompVBox<CompVPoint<T > >::newObj

#if defined(_MSC_VER)
#	define snprintf		_snprintf
#	define vsnprintf	_vsnprintf
#	define strdup		_strdup
#	define stricmp		_stricmp
#	define strnicmp		_strnicmp
#else
#	if !HAVE_STRNICMP && !HAVE_STRICMP
#	define stricmp		strcasecmp
#	define strnicmp		strncasecmp
#	endif
#endif

// Should be defined in <inttypes.h>, include in <compv_config.h>
#if !defined(PRIu64)
#	define	PRIu64 "llu"
#endif

/*******************************************************/
/* MACRO for shuffle parameter for _mm_shuffle_ps().   */
/* Argument fp3 is a digit[0123] that represents the fp*/
/* from argument "b" of mm_shuffle_ps that will be     */
/* placed in fp3 of result. fp2 is the same for fp2 in */
/* result. fp1 is a digit[0123] that represents the fp */
/* from argument "a" of mm_shuffle_ps that will be     */
/* places in fp1 of result. fp0 is the same for fp0 of */
/* result                                              */
/*******************************************************/
#define COMPV_MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

/*
Macro to build arg32 values for _mm(256/128)_shuffle_epi8().
a,b,c,d must be <= 16 for _mm128_shuffle_epi8() and <32 for _mm256_shuffle_epi8()
*/
#define COMPV_MM_SHUFFLE_EPI8(fp3,fp2,fp1,fp0) ((fp3 << 24) | (fp2 << 16) | (fp1 << 8) | (fp0 & 0xFF))

typedef int32_t compv_core_id_t;
typedef intptr_t compv_scalar_t;  /* This type *must* have the width of a general-purpose register on the target CPU. 64bits or 32bits. */
typedef uintptr_t compv_uscalar_t;  /* This type *must* have the width of a general-purpose register on the target CPU. 64bits or 32bits. */
typedef float compv_float32_t;
typedef double compv_float64_t;
typedef compv_float32_t compv_float32x2_t[2];
typedef compv_float32_t compv_float32x4_t[4];

enum COMPV_DEBUG_LEVEL {
    COMPV_DEBUG_LEVEL_INFO = 4,
    COMPV_DEBUG_LEVEL_WARN = 3,
    COMPV_DEBUG_LEVEL_ERROR = 2,
    COMPV_DEBUG_LEVEL_FATAL = 1,
};

#define kErrorCodeSuccessStart		0
#define kErrorCodeWarnStart			10000
#define kErrorCodeErrorStart		(kErrorCodeWarnStart << 1)
#define kErrorCodeFatalStart		(kErrorCodeErrorStart << 1)

// TODO(dmi) complete COMPVGetErrorString(code) with all the newly added codes
enum COMPV_ERROR_CODE {
    COMPV_ERROR_CODE_S_OK = kErrorCodeSuccessStart,

    COMPV_ERROR_CODE_W = kErrorCodeWarnStart,
    COMPV_ERROR_CODE_W_WINDOW_CLOSED,

    COMPV_ERROR_CODE_E = kErrorCodeErrorStart,
    COMPV_ERROR_CODE_E_NOT_IMPLEMENTED,
    COMPV_ERROR_CODE_E_NOT_INITIALIZED,
    COMPV_ERROR_CODE_E_NOT_FOUND,
    COMPV_ERROR_CODE_E_INVALID_CALL,
    COMPV_ERROR_CODE_E_INVALID_STATE,
    COMPV_ERROR_CODE_E_INVALID_PARAMETER,
    COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT,
    COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT,
	COMPV_ERROR_CODE_E_INVALID_SUBTYPE,
    COMPV_ERROR_CODE_E_FAILED_TO_OPEN_FILE,
    COMPV_ERROR_CODE_E_FAILED_TO_READ_FILE,
    COMPV_ERROR_CODE_E_OUT_OF_MEMORY,
    COMPV_ERROR_CODE_E_OUT_OF_BOUND,
    COMPV_ERROR_CODE_E_DECODER_NOT_FOUND,
    COMPV_ERROR_CODE_E_FILE_NOT_FOUND,
    COMPV_ERROR_CODE_E_TIMEDOUT,
    COMPV_ERROR_CODE_E_UNITTEST_FAILED,
    COMPV_ERROR_CODE_E_SYSTEM,
	COMPV_ERROR_CODE_E_MEMORY_LEAK,
	COMPV_ERROR_CODE_E_MEMORY_NOT_ALIGNED,
    COMPV_ERROR_CODE_E_THIRD_PARTY_LIB,
    COMPV_ERROR_CODE_E_PTHREAD,
	COMPV_ERROR_CODE_E_JNI,
    COMPV_ERROR_CODE_E_DIRECTSHOW,
    COMPV_ERROR_CODE_E_MFOUNDATION,
    COMPV_ERROR_CODE_E_EGL,
    COMPV_ERROR_CODE_E_GL,
    COMPV_ERROR_CODE_E_GL_NO_CONTEXT,
    COMPV_ERROR_CODE_E_GLEW,
    COMPV_ERROR_CODE_E_GLFW,
	COMPV_ERROR_CODE_E_OPENCL,
	COMPV_ERROR_CODE_E_CUDA,
    COMPV_ERROR_CODE_E_SDL,
	COMPV_ERROR_CODE_E_SKIA,
	COMPV_ERROR_CODE_E_INTEL_IPP,
	COMPV_ERROR_CODE_E_INTEL_TBB,

    COMPV_ERROR_CODE_F = kErrorCodeFatalStart,
};
extern COMPV_BASE_API const char* CompVGetErrorString(COMPV_NAMESPACE::COMPV_ERROR_CODE code);

#define COMPV_ERROR_CODE_IS_SUCCESS(code_) ((code_) < kErrorCodeWarnStart)
#define COMPV_ERROR_CODE_IS_OK(code_) COMPV_ERROR_CODE_IS_SUCCESS((code_))
#define COMPV_ERROR_CODE_IS_FAILURE(code_) (!COMPV_ERROR_CODE_IS_SUCCESS((code_)))
#define COMPV_ERROR_CODE_IS_NOK(code_) COMPV_ERROR_CODE_IS_FAILURE((code_))
#define COMPV_ERROR_CODE_IS_WARN(code_) ((code_) >= kErrorCodeWarnStart && (code_) < kErrorCodeErrorStart)
#define COMPV_ERROR_CODE_IS_ERROR(code_) ((code_) >= kErrorCodeErrorStart && (code_) < kErrorCodeFatalStart)
#define COMPV_ERROR_CODE_IS_FATAL(code_) ((code_) >= kErrorCodeFatalStart)

// In COMPV_CHECK_HR(errcode) When (errcode) is a function it will be executed twice when used in "COMPV_DEBUG_ERROR(errcode)" and "If(errcode)"
#define COMPV_CHECK_CODE_NOP(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); } } while(0)
#define COMPV_CHECK_CODE_BAIL(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); goto bail; } } while(0)
#define COMPV_CHECK_CODE_RETURN(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); return __code__; } } while(0)
#define COMPV_CHECK_CODE_ASSERT(errcode, ...) do { COMPV_NAMESPACE::COMPV_ERROR_CODE __code__ = (errcode); if (COMPV_ERROR_CODE_IS_NOK(__code__)) { COMPV_DEBUG_ERROR("Operation Failed (%s) -> "  __VA_ARGS__, CompVGetErrorString(__code__)); COMPV_ASSERT(false); } } while(0)
#define COMPV_CHECK_EXP_NOP(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_NOP(errcode,  __VA_ARGS__); } while(0)
#define COMPV_CHECK_EXP_BAIL(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_BAIL(errcode,  __VA_ARGS__); } while(0)
#define COMPV_CHECK_EXP_RETURN(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_RETURN(errcode,  __VA_ARGS__); } while(0)
#define COMPV_CHECK_EXP_ASSERT(exp, errcode, ...) do { if ((exp)) COMPV_CHECK_CODE_ASSERT(errcode,  __VA_ARGS__); } while(0)

enum COMPV_SORT_TYPE {
    COMPV_SORT_TYPE_BUBBLE, /**< https://en.wikipedia.org/wiki/Bubble_sort */
    COMPV_SORT_TYPE_QUICK, /**< https://en.wikipedia.org/wiki/Quicksort */
};

enum COMPV_MODELEST_TYPE {
	COMPV_MODELEST_TYPE_NONE,
	COMPV_MODELEST_TYPE_RANSAC
};

enum COMPV_SCALE_TYPE {
	COMPV_SCALE_TYPE_BILINEAR
};

enum COMPV_MAT_TYPE {
    COMPV_MAT_TYPE_RAW,
	COMPV_MAT_TYPE_STRUCT,
    COMPV_MAT_TYPE_PIXELS
};

enum COMPV_SUBTYPE {
	COMPV_SUBTYPE_NONE,

    COMPV_SUBTYPE_RAW_OPAQUE,
	COMPV_SUBTYPE_RAW_INT8,
	COMPV_SUBTYPE_RAW_UINT8,
	COMPV_SUBTYPE_RAW_INT16,
	COMPV_SUBTYPE_RAW_UINT16,
	COMPV_SUBTYPE_RAW_INT32,
	COMPV_SUBTYPE_RAW_UINT32,
	COMPV_SUBTYPE_RAW_SIZE,
	COMPV_SUBTYPE_RAW_FLOAT32,
	COMPV_SUBTYPE_RAW_FLOAT64,

    COMPV_SUBTYPE_PIXELS_RGB24,			// RGB24
    COMPV_SUBTYPE_PIXELS_BGR24,			// BGR8
    COMPV_SUBTYPE_PIXELS_RGBA32,		// RGB32
    COMPV_SUBTYPE_PIXELS_BGRA32,		// BGRA32
    COMPV_SUBTYPE_PIXELS_ABGR32,		// ABGR32
    COMPV_SUBTYPE_PIXELS_ARGB32,		// ARGB32
	COMPV_SUBTYPE_PIXELS_RGB565LE,		// RGB565LE: Microsoft: DirectDraw with dwFlags = DDPF_RGB and dwRGBBitCount = 16, use by android bitmaps and wince consumers
	COMPV_SUBTYPE_PIXELS_RGB565BE,		// RGB565BE
	COMPV_SUBTYPE_PIXELS_BGR565LE,		// BGR565LE
	COMPV_SUBTYPE_PIXELS_BGR565BE,		// BGR565BE
    COMPV_SUBTYPE_PIXELS_Y,				// Y-only: Grayscale
	COMPV_SUBTYPE_PIXELS_NV12,			// Microsoft: NV12, iOS camera, Android camera, Semi-Planar, Info: https://www.fourcc.org/pixel-format/yuv-nv12/
	COMPV_SUBTYPE_PIXELS_NV21,			// Microsoft: NV21, Android camera, Semi-Planar, Info: https://www.fourcc.org/pixel-format/yuv-nv21/
    COMPV_SUBTYPE_PIXELS_YUV420P,		// Microsoft: I420, Planar, Info: https://www.fourcc.org/pixel-format/yuv-i420/
	COMPV_SUBTYPE_PIXELS_YVU420P,		// Microsoft YV12, Planar, Info: https://www.fourcc.org/pixel-format/yuv-yv12/
	COMPV_SUBTYPE_PIXELS_YUV422P,		// Microsoft: YUV422, Planar
	COMPV_SUBTYPE_PIXELS_YUYV422,		// Microsoft: YUY2, V4L2/DirectShow preferred format, Packed, Info: https://www.fourcc.org/pixel-format/yuv-yuy2/
	COMPV_SUBTYPE_PIXELS_UYVY422,		// Microsoft: UYVY, iOS camera, Info: https://www.fourcc.org/pixel-format/yuv-uyvy/
	COMPV_SUBTYPE_PIXELS_YUV444P,		// Microsoft: YUV444, Planar

	// Aliases
	COMPV_SUBTYPE_PIXELS_RGB565 = COMPV_SUBTYPE_PIXELS_RGB565LE,
	COMPV_SUBTYPE_PIXELS_BGR565 = COMPV_SUBTYPE_PIXELS_BGR565LE,
    COMPV_SUBTYPE_PIXELS_I420 = COMPV_SUBTYPE_PIXELS_YUV420P,
	COMPV_SUBTYPE_PIXELS_IYUV = COMPV_SUBTYPE_PIXELS_YUV420P,
	COMPV_SUBTYPE_PIXELS_YUV422 = COMPV_SUBTYPE_PIXELS_UYVY422,
	COMPV_SUBTYPE_PIXELS_YUV444 = COMPV_SUBTYPE_PIXELS_YUV444P,
	COMPV_SUBTYPE_PIXELS_YUY2 = COMPV_SUBTYPE_PIXELS_YUYV422,
	COMPV_SUBTYPE_PIXELS_UYVY = COMPV_SUBTYPE_PIXELS_UYVY422,
	COMPV_SUBTYPE_PIXELS_Y422 = COMPV_SUBTYPE_PIXELS_UYVY,
	COMPV_SUBTYPE_PIXELS_Y420SP = COMPV_SUBTYPE_PIXELS_NV21,
};
extern COMPV_BASE_API const char* CompVGetSubtypeString(COMPV_NAMESPACE::COMPV_SUBTYPE subtype);

enum COMPV_IMAGE_FORMAT {
    COMPV_IMAGE_FORMAT_NONE,
    COMPV_IMAGE_FORMAT_RAW,
    COMPV_IMAGE_FORMAT_JPEG,
    COMPV_IMAGE_FORMAT_JPG = COMPV_IMAGE_FORMAT_JPEG,
    COMPV_IMAGE_FORMAT_BMP,
    COMPV_IMAGE_FORMAT_BITMAP = COMPV_IMAGE_FORMAT_BMP,
    COMPV_IMAGE_FORMAT_PNG
};

enum COMPV_DRAWING_COLOR_TYPE {
	COMPV_DRAWING_COLOR_TYPE_STATIC,
	COMPV_DRAWING_COLOR_TYPE_RANDOM
};

enum COMPV_DRAWING_LINE_TYPE {
	COMPV_DRAWING_LINE_TYPE_SIMPLE,
	COMPV_DRAWING_LINE_TYPE_MATCH,
};

struct CompVImageInfo {
    COMPV_IMAGE_FORMAT format;
    COMPV_SUBTYPE pixelFormat; // COMPV_SUBTYPE_PIXELS_XXX
    int32_t width;
    int32_t stride;
    int32_t height;
public:
    CompVImageInfo() : format(COMPV_IMAGE_FORMAT_RAW), pixelFormat(COMPV_SUBTYPE_NONE), width(0), stride(0), height(0) { }
};

struct CompVVec3f {
public:
    union {
        struct {
			compv_float32_t x, y, z;
        };
        struct {
			compv_float32_t r, g, b;
        };
        struct {
			compv_float32_t s, t, p;
        };
    };
    CompVVec3f(compv_float32_t x_, compv_float32_t y_, compv_float32_t z_) : x(x_), y(y_), z(z_) { }
};

struct CompVRect {
public:
    int left;
    int top;
    int right;
    int bottom;
    CompVRect(int left_ = 0, int top_ = 0, int right_ = 0, int bottom_ = 0) : left(left_), top(top_), right(right_), bottom(bottom_) {  }
    static CompVRect makeFromWidthHeight(int x, int y, int width, int height) {
        return CompVRect(x, y, x + width, y + height);
    }
};

struct CompVRatio {
public:
    int numerator;
    int denominator;
    CompVRatio(int numerator_ = 1, int denominator_ = 1) : numerator(numerator_), denominator(denominator_) { }
};

template <typename T>
struct CompVPoint {
	COMPV_INLINE void init(T x_ = 0, T y_ = 0, T z_ = 1) {
		x = x_, y = y_, z = z_;
	}
public:
	CompVPoint() {
		init();
	}
	CompVPoint(T x_, T y_, T z_ = 1) {
		init(x_, y_, z_);
	}
	T x, y, z;
};
typedef CompVPoint<compv_float32_t> CompVPointFloat32;
typedef CompVPoint<compv_float64_t> CompVPointFloat64;
typedef CompVPoint<int32_t> CompVPointInt32;
typedef CompVPoint<int> CompVPointInt;


struct CompVInterestPoint {
	compv_float32_t x; /**< Point.x */
	compv_float32_t y; /**< Point.y */
	compv_float32_t strength; /**< Corner/edge strength/response (e.g. FAST response or Harris response) */
	compv_float32_t orient; /**< angle in degree ([0-360]) */
	int level; /**< pyramid level (when image is scaled, level0 is the first one) */
	compv_float32_t size; /**< patch size (e.g. BRIEF patch size-circle diameter-) */

protected:
	COMPV_INLINE void init(compv_float32_t x_, compv_float32_t y_, compv_float32_t strength_ = -1.f, compv_float32_t orient_ = -1.f, int32_t level_ = 0, compv_float32_t size_ = 0.f) {
		x = x_, y = y_, strength = strength_, orient = orient_, level = level_, size = size_;
	}
public:
	CompVInterestPoint() {
		init(0, 0);
	}
	CompVInterestPoint(compv_float32_t x_, compv_float32_t y_, compv_float32_t strength_ = -1.f, compv_float32_t orient_ = -1.f, int32_t level_ = 0, compv_float32_t size_ = 0.f) {
		init(x_, y_, strength_, orient_, level_, size_);
	}
	static void selectBest(std::vector<CompVInterestPoint>& interestPoints, size_t max) {
		if (max > 1) {
#if 0
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("Slow and worst matches");
			std::sort(interestPoints.begin(), interestPoints.end(), InterestPointStrengthGreater());
			interestPoints.resize(static_cast<size_t>(max));
#else
			// This code gives better matches (tested with object recognition sample)
			std::nth_element(interestPoints.begin(), interestPoints.begin() + max, interestPoints.end(),
				[](const CompVInterestPoint& i, const CompVInterestPoint& j) {return i.strength > j.strength; });
			const float pivot = interestPoints.at(max - 1).strength;
			interestPoints.resize(std::partition(interestPoints.begin() + max, interestPoints.end(),
				[pivot](CompVInterestPoint i) { return i.strength >= pivot; }) - interestPoints.begin());
#endif
		}
	}
	static void eraseTooCloseToBorder(std::vector<CompVInterestPoint>& interestPoints, size_t img_width, size_t img_height, int border_size) {
		float w = static_cast<compv_float32_t>(img_width), h = static_cast<compv_float32_t>(img_height), b = static_cast<compv_float32_t>(border_size);
		auto new_end = std::remove_if(interestPoints.begin(), interestPoints.end(), [&w, &h, &b](const CompVInterestPoint& p) { 
			return ((p.x < b || (p.x + b) >= w || (p.y < b) || (p.y + b) >= h));
		});
		interestPoints.erase(new_end, interestPoints.end());
	}
};

typedef std::vector<CompVInterestPoint> CompVInterestPointVector;

struct CompVMatIndex {
	size_t row;
	size_t col;
public:
	CompVMatIndex() : row(0), col(0) {}
	CompVMatIndex(size_t row_, size_t col_): row(row_), col(col_) {}
};

struct CompVDMatch {
	int queryIdx;
	int trainIdx;
	int imageIdx;
	int distance;
protected:
	COMPV_INLINE void init(int queryIdx_, int trainIdx_, int distance_, int imageIdx_ = 0) {
		queryIdx = queryIdx_, trainIdx = trainIdx_, distance = distance_, imageIdx = imageIdx_;
	}
public:
	CompVDMatch() {
		init(0, 0, 0, 0);
	}
	CompVDMatch(int queryIdx_, int trainIdx_, int distance_, int imageIdx_ = 0) {
		init(queryIdx_, trainIdx_, distance_, imageIdx_);
	}
};

struct CompVHoughLine {
	compv_float32_t rho;
	compv_float32_t theta;
	size_t strength;
public:
	CompVHoughLine(compv_float32_t rho_, compv_float32_t theta_, size_t strength_): rho(rho_), theta(theta_), strength(strength_) { }
	CompVHoughLine(): rho(0), theta(0), strength(0) {}
};
typedef std::vector<CompVHoughLine> CompVHoughLineVector;

struct CompVDrawingOptions {
	COMPV_DRAWING_COLOR_TYPE colorType;
	compv_float32x4_t color;
	compv_float32_t pointSize;
	compv_float32_t lineWidth;
	COMPV_DRAWING_LINE_TYPE lineType;
public:
	CompVDrawingOptions() {
		colorType = COMPV_DRAWING_COLOR_TYPE_RANDOM;
		color[0] = color[1] = color[2] = 0.f, color[3] = 1.f;
		pointSize = 7.f;
		lineWidth = 2.f;
		lineType = COMPV_DRAWING_LINE_TYPE_SIMPLE;
	}
	static CompVDrawingOptions clone(const CompVDrawingOptions* options) {
		return options ? *options : CompVDrawingOptions();
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_COMMON_H_ */
