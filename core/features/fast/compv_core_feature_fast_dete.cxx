/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/* @description
This class implement FAST (Features from Accelerated Segment Test) algorithm.
Some literature about FAST:
- http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/AV1011/AV1FeaturefromAcceleratedSegmentTest.pdf
- https://en.wikipedia.org/wiki/Features_from_accelerated_segment_test
- http://www.edwardrosten.com/work/fast.html
- http://web.eecs.umich.edu/~silvio/teaching/EECS598_2010/slides/11_16_Hao.pdf
*/

#include "compv/core/features/fast/compv_core_feature_fast_dete.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math_utils.h"

#include "compv/core/features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_sse2.h"

#include <algorithm>

#define COMPV_THIS_CLASSNAME	"CompVCornerDeteFAST"

// Flags generated using FastFlags() in "tests/fast.cxx"
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() uint16_t kCompVFast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
COMPV_EXTERNC COMPV_CORE_API const COMPV_ALIGN_DEFAULT() uint16_t kCompVFast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

// FIXME: kCompVFast9Arcs and kCompVFast9Flags non longer used

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT			20
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP				true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES				2000 // maximum number of features to retain (<0 means all)
#define COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD		(200*250) // number of pixels
#define COMPV_FEATURE_DETE_FAST_NMS_MIN_SAMPLES_PER_THREAD	(80*80) // number of interestPoints

static int32_t COMPV_INLINE __continuousCount(int32_t fasType) {
    switch (fasType) {
    case COMPV_FAST_TYPE_9: return 9;
    case COMPV_FAST_TYPE_12: return 12;
    default:
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid fastType:%d", fasType);
        return 9;
    }
}

static void CompVFastDataRange(RangeFAST* range);
static void CompVFastNmsGatherRange(RangeFAST* range);
static void CompVFastNmsApplyRange(RangeFAST* range);
static void CompVFastDataRow1_C(const uint8_t* IP,  compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths);
static void CompVFastNmsGather_C(const uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride);
static void CompVFastNmsApply_C(uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride);
static COMPV_ERROR_CODE FastRangesAlloc(size_t nRanges, RangeFAST** ppRanges, size_t stride);
static COMPV_ERROR_CODE FastRangesFree(size_t nRanges, RangeFAST** ppRanges);

CompVCornerDeteFAST::CompVCornerDeteFAST()
    : CompVCornerDete(COMPV_FAST_ID)
    , m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
    , m_iType(COMPV_FAST_TYPE_9)
    , m_iNumContinuous(__continuousCount(COMPV_FAST_TYPE_9))
	, m_iMaxFeatures(COMPV_FEATURE_DETE_FAST_MAX_FEATURTES)
    , m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nStride(0)
    , m_pRanges(NULL)
    , m_nRanges(0)
    , m_pStrengthsMap(NULL)
	, m_pNmsMap(NULL)
{

}

CompVCornerDeteFAST::~CompVCornerDeteFAST()
{
    FastRangesFree(m_nRanges, &m_pRanges);
    CompVMem::free(reinterpret_cast<void**>(&m_pStrengthsMap));
	CompVMem::free(reinterpret_cast<void**>(&m_pNmsMap));
}

COMPV_ERROR_CODE CompVCornerDeteFAST::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
    COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_FAST_SET_INT_THRESHOLD: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int threshold = *reinterpret_cast<const int*>(valuePtr);
        m_iThreshold = COMPV_MATH_CLIP3(0, 255, threshold);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_INT_MAX_FEATURES: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_iMaxFeatures = *reinterpret_cast<const int*>(valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_INT_FAST_TYPE: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        int iType = *reinterpret_cast<const int*>(valuePtr);
        COMPV_CHECK_EXP_RETURN(iType != COMPV_FAST_TYPE_9 && iType != COMPV_FAST_TYPE_12, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_iType = iType;
        m_iNumContinuous = __continuousCount(iType);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_bNonMaximaSupp = *reinterpret_cast<const bool*>(valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
    }
}

// overrides CompVCornerDete::process
COMPV_ERROR_CODE CompVCornerDeteFAST::process(const CompVMatPtr& image, CompVBoxInterestPointPtrPtr interestPoints) /*Overrides(CompVCornerDete)*/
{
    COMPV_CHECK_EXP_RETURN(!image || image->isEmpty() || image->subType() != COMPV_SUBTYPE_PIXELS_Y || !interestPoints,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	const uint8_t* dataPtr = image->ptr<const uint8_t>();
    size_t width = image->cols();
	size_t height = image->rows();
	size_t stride = image->stride();
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
    size_t threadsCountRange = 1;

    COMPV_CHECK_EXP_RETURN(width < 4 || height < 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Invalid size (too short)");

    // Free ranges memory if stride increase
    if (m_nStride < stride) {
        FastRangesFree(m_nRanges, &m_pRanges);
        m_nRanges = 0;
    }

    // Even if newStride < oldStride, free the strenghtMap to cleanup old values.
    // FastData() function will cleanup only new matching positions.
    if (m_nStride != stride) {
        CompVMem::free(reinterpret_cast<void**>(&m_pStrengthsMap));
		CompVMem::free(reinterpret_cast<void**>(&m_pNmsMap));
    }

    // Alloc strenghts map if not already done
    if (!m_pStrengthsMap || !m_pNmsMap) {
        size_t nMapSize = CompVMem::alignForward((3 + stride + 3) * (3 + height + 3)); // +3 for the borders, alignForward() for the SIMD functions
		// Must use calloc to fill the strengths with null values
		if (!m_pStrengthsMap) {
			m_pStrengthsMap = reinterpret_cast<uint8_t*>(CompVMem::calloc(nMapSize, sizeof(uint8_t)));
			COMPV_CHECK_EXP_RETURN(!m_pStrengthsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate strengths map");
		}
		if (!m_pNmsMap) {
			m_pNmsMap = reinterpret_cast<uint8_t*>(CompVMem::calloc(nMapSize, sizeof(uint8_t)));
			COMPV_CHECK_EXP_RETURN(!m_pNmsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate nms map");
		}
    }

    // Update width and height
    m_nWidth = width;
    m_nHeight = height;
    m_nStride = stride;

    // create or reset points
	CompVBoxInterestPointPtr interestPoints_ = *interestPoints;
    if (!interestPoints_) {
        COMPV_CHECK_CODE_RETURN(CompVBoxInterestPoint::newObj(&interestPoints_));
    }
    else {
		interestPoints_->reset();
    }

	const compv_scalar_t strideScalarSigned = static_cast<compv_scalar_t>(stride);
    COMPV_ALIGN_DEFAULT() const compv_scalar_t pixels16[16] = {
        -(strideScalarSigned * 3) + 0, // 1
        -(strideScalarSigned * 3) + 1, // 2
        -(strideScalarSigned * 2) + 2, // 3
        -(strideScalarSigned * 1) + 3, // 4
        +(strideScalarSigned * 0) + 3, // 5
        +(strideScalarSigned * 1) + 3, // 6
        +(strideScalarSigned * 2) + 2, // 7
        +(strideScalarSigned * 3) + 1, // 8
        +(strideScalarSigned * 3) + 0, // 9
        +(strideScalarSigned * 3) - 1, // 10
        +(strideScalarSigned * 2) - 2, // 11
        +(strideScalarSigned * 1) - 3, // 12
        +(strideScalarSigned * 0) - 3, // 13
        -(strideScalarSigned * 1) - 3, // 14
        -(strideScalarSigned * 2) - 2, // 15
        -(strideScalarSigned * 3) - 1, // 16
    };

    // Compute number of threads
	threadsCountRange = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD)
		: 1;

    // Alloc ranges
    if (m_nRanges < threadsCountRange) {
        m_nRanges = 0;
        COMPV_CHECK_CODE_RETURN(FastRangesAlloc(threadsCountRange, &m_pRanges, m_nStride));
        m_nRanges = threadsCountRange;
    }

    if (threadsCountRange > 1) {
		size_t rowStart = 0;
		size_t heights = (height / threadsCountRange);
		size_t lastHeight = height - ((threadsCountRange - 1) * heights);
        RangeFAST* pRange;
        CompVAsyncTaskIds taskIds;
        taskIds.reserve(threadsCountRange);
        auto funcPtr = [&](RangeFAST* pRange) -> void {
            CompVFastDataRange(pRange);
        };
        for (size_t i = 0; i < threadsCountRange; ++i) {
            pRange = &m_pRanges[i];
            pRange->IP = dataPtr;
            pRange->rowStart = rowStart;
            pRange->rowEnd = (rowStart + (i == (threadsCountRange - 1) ? lastHeight : heights));
            pRange->rowCount = height;
            pRange->width = width;
            pRange->stride = stride;
            pRange->threshold = m_iThreshold;
            pRange->N = m_iNumContinuous;
            pRange->pixels16 = pixels16;
            pRange->strengths = m_pStrengthsMap;
			pRange->nms = m_pNmsMap;
            COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, pRange), taskIds));
            rowStart = pRange->rowEnd;
        }
        COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
    } 
	else {
        RangeFAST* pRange = &m_pRanges[0];
        pRange->IP = dataPtr;
        pRange->rowStart = 0;
        pRange->rowEnd = height;
        pRange->rowCount = height;
        pRange->width = width;
        pRange->stride = stride;
        pRange->threshold = m_iThreshold;
        pRange->N = m_iNumContinuous;
        pRange->pixels16 = pixels16;
        pRange->strengths = m_pStrengthsMap;
		pRange->nms = m_pNmsMap;
        CompVFastDataRange(pRange);
    }

	// Non Maximal Suppression for removing adjacent corners
	if (m_bNonMaximaSupp) {
		size_t threadsCountNMS = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
			? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(stride, height, maxThreads, COMPV_FEATURE_DETE_FAST_NMS_MIN_SAMPLES_PER_THREAD)
			: 1;
		threadsCountNMS = COMPV_MATH_MIN(threadsCountRange, threadsCountNMS); // only 'threadsCountRange' values were allocated

		if (threadsCountNMS > 1) {
			size_t threadIdx, rowStart;
			size_t heights = (height / threadsCountNMS);
			size_t lastHeight = height - ((threadsCountNMS - 1) * heights);
			RangeFAST* pRange;
			CompVAsyncTaskIds taskIds;
			taskIds.reserve(threadsCountNMS);
			auto funcPtrNmsGather = [&](RangeFAST* pRange) -> void {
				CompVFastNmsGatherRange(pRange);
			};
			auto funcPtrNmsApply = [&](RangeFAST* pRange) -> void {
				CompVFastNmsApplyRange(pRange);
			};
			// NMS gathering
			for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountNMS; ++threadIdx) {
				pRange = &m_pRanges[threadIdx];
				pRange->rowStart = rowStart;
				pRange->rowEnd = (rowStart + (threadIdx == (threadsCountNMS - 1) ? lastHeight : heights));
				pRange->rowCount = height;
				pRange->width = width;
				pRange->stride = stride;
				pRange->strengths = m_pStrengthsMap;
				pRange->nms = m_pNmsMap;
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsGather, pRange), taskIds));
				rowStart = pRange->rowEnd;
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
			// NMS-apply
			taskIds.clear();
			for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountNMS; ++threadIdx) {
				pRange = &m_pRanges[threadIdx];
				pRange->rowStart = rowStart;
				pRange->rowEnd = (rowStart + (threadIdx == (threadsCountNMS - 1) ? lastHeight : heights));
				pRange->rowCount = height;
				pRange->width = width;
				pRange->stride = stride;
				pRange->strengths = m_pStrengthsMap;
				pRange->nms = m_pNmsMap;
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsApply, pRange), taskIds));
				rowStart = pRange->rowEnd;
			}
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		}
		else {
			RangeFAST* pRange = &m_pRanges[0];
			pRange->rowStart = 0;
			pRange->rowEnd = height;
			pRange->rowCount = height;
			pRange->width = width;
			pRange->stride = stride;
			pRange->strengths = m_pStrengthsMap;
			pRange->nms = m_pNmsMap;

			CompVFastNmsGatherRange(pRange);
			CompVFastNmsApplyRange(pRange);
		}
	}

    // Build interest points
#define COMPV_PUSH1() if (*begin1) { *begin1 += thresholdMinus1; interestPoints_->push(CompVInterestPoint(static_cast<compv_float32_t>(begin1 - strengths), static_cast<compv_float32_t>(j), static_cast<compv_float32_t>(*begin1))); } ++begin1;
#define COMPV_PUSH4() COMPV_PUSH1() COMPV_PUSH1() COMPV_PUSH1() COMPV_PUSH1()
#define COMPV_PUSH8() COMPV_PUSH4() COMPV_PUSH4()
    uint8_t *strengths = m_pStrengthsMap + (3 * stride), *begin1;
    if (COMPV_IS_ALIGNED(stride, 64) && COMPV_IS_ALIGNED(CompVCpu::cache1LineSize(), 64)) {
        uint64_t *begin8, *end8;
        size_t width_div8 = width >> 3;
        const uint8_t thresholdMinus1 = static_cast<uint8_t>(m_iThreshold - 1);
        for (size_t j = 3; j < height - 3; ++j) {
            begin8 = reinterpret_cast<uint64_t*>(strengths + 0); // i can start at +3 but we prefer +0 because strengths[0] is cacheline-aligned
            end8 = (begin8 + width_div8);
            do {
                if (*begin8) {
                    begin1 = reinterpret_cast<uint8_t*>(begin8);
                    COMPV_PUSH8();
                }
            }
            while (begin8++ < end8);
            strengths += stride;
        }
    }
    else {
        uint32_t *begin4, *end4;
		size_t width_div4 = width >> 2;
        const uint8_t thresholdMinus1 = static_cast<uint8_t>(m_iThreshold - 1);
        for (size_t j = 3; j < height - 3; ++j) {
            begin4 = reinterpret_cast<uint32_t*>(strengths + 0); // i can start at +3 but we prefer +0 because strengths[0] is cacheline-aligned
            end4 = (begin4 + width_div4);
            do {
                if (*begin4) {
                    begin1 = reinterpret_cast<uint8_t*>(begin4);
                    COMPV_PUSH4();
                }
            }
            while (begin4++ < end4);
            strengths += stride;
        }
    }

    // Retain best "m_iMaxFeatures" features
    if (m_iMaxFeatures > 0 && static_cast<int32_t>(interestPoints_->size()) > m_iMaxFeatures) {
        interestPoints_->retainBest(m_iMaxFeatures);
    }

	*interestPoints = interestPoints_;
    return err_;
}

COMPV_ERROR_CODE CompVCornerDeteFAST::newObj(CompVCornerDetePtrPtr fast)
{
    COMPV_CHECK_EXP_RETURN(fast == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVCornerDeteFASTPtr _fast = new CompVCornerDeteFAST();
	COMPV_CHECK_EXP_RETURN(!_fast, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *fast = *_fast;
    return COMPV_ERROR_CODE_S_OK;
}

static void CompVFastDataRange(RangeFAST* range)
{
    const uint8_t* IP;
    int32_t j, kalign, kextra, align = 1, minj, maxj, rowstart, k;
    uint8_t *strengths, *extra;
    void(*FastDataRow)(const uint8_t* IP, compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths) 
		= CompVFastDataRow1_C;

    if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(range->pixels16)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((FastDataRow = CompVFastDataRow16_Intrin_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
        //COMPV_EXEC_IFDEF_ASM_X86((FastDataRow = FastData16Row_Asm_X86_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
        //COMPV_EXEC_IFDEF_ASM_X64((FastDataRow = FastData16Row_Asm_X64_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
    }
    if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
        /*COMPV_EXEC_IFDEF_INTRIN_X86((FastDataRow = FastData32Row_Intrin_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
        COMPV_EXEC_IFDEF_ASM_X86((FastDataRow = FastData32Row_Asm_X86_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
        COMPV_EXEC_IFDEF_ASM_X64((FastDataRow = FastData32Row_Asm_X64_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));*/
    }

    // Number of pixels to process (multiple of align)
    kalign = static_cast<int32_t>(CompVMem::alignForward((-3 + range->width - 3), align));
    if (kalign > static_cast<int32_t>(range->stride - 3)) { // must never happen as the image always contains a border(default 7) aligned on 64B
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Unexpected code called. k16=%d, stride=%zu", kalign, range->stride);
        COMPV_ASSERT(false);
        return;
    }
    // Number of pixels to ignore
    kextra = kalign - (-3 + static_cast<int32_t>(range->width) - 3);

    rowstart = static_cast<int32_t>(range->rowStart);
    minj = (rowstart == 0 ? 3 : 0);
    maxj = static_cast<int32_t>((range->rowEnd - rowstart) - ((range->rowCount - range->rowEnd) <= 3 ? 3 - (range->rowCount - range->rowEnd) : 0));
    IP = range->IP + ((rowstart + minj) * range->stride) + 3;
    strengths = range->strengths + ((rowstart + minj) * range->stride) + 3;

    // For testing with image "equirectangular", the first (i,j) to produce an interesting point is (1620, 279)
    // We should have 64586 non-zero results for SSE and 66958 for AVX2

    for (j = minj; j < maxj; ++j) {
        FastDataRow(IP, kalign, range->pixels16, range->N, range->threshold, strengths);
        // remove extra samples
        extra = &strengths[kalign - 1];
        for (k = 0; k < kextra; ++k) {
            *extra-- = 0;
        }
        IP += range->stride;
        strengths += range->stride;
    } // for (j)
}

void CompVFastNmsGatherRange(RangeFAST* range)
{
	void(*CompVFastNmsGather)(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
		= CompVFastNmsGather_C;

	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(range->stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVFastNmsGather = CompVFastNmsGather_Intrin_SSE2));
	}

	size_t rowStart = range->rowStart > 3 ? range->rowStart - 3 : range->rowStart;
	size_t rowEnd = COMPV_MATH_CLIP3(0, range->rowCount, (range->rowEnd + 3));
	CompVFastNmsGather(
		range->strengths + (range->stride * rowStart),
		range->nms + (range->stride * rowStart),
		range->width,
		(rowEnd - rowStart),
		range->stride
	);
}

void CompVFastNmsApplyRange(RangeFAST* range)
{
	void(*CompVFastNmsApply)(uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
		= CompVFastNmsApply_C;

	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(range->stride) && COMPV_IS_ALIGNED_SSE(range->strengths) && COMPV_IS_ALIGNED_SSE(range->nms)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVFastNmsApply = CompVFastNmsApply_Intrin_SSE2));
	}

	size_t rowStart = range->rowStart > 3 ? range->rowStart - 3 : range->rowStart;
	size_t rowEnd = COMPV_MATH_CLIP3(0, range->rowCount, (range->rowEnd + 3));
	CompVFastNmsApply(
		range->strengths + (range->stride * rowStart),
		range->nms + (range->stride * rowStart),
		range->width,
		(rowEnd - rowStart),
		range->stride
	);
}

static void CompVFastDataRow1_C(const uint8_t* IP, compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths)
{
	// Code not intended to be fast but just readable, real code is implemented in SSE, AVX and NEON.
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Must be reviewed: see your remainders");
	int32_t sumb, sumd, sb, sd;
	int16_t threshold_ = static_cast<int16_t>(threshold); // using int16_t to avoid clipping (useless for SIMD with support for saturated sub and add)
	uint8_t ddarkers16[16], dbrighters16[16], strength, t0, t1;
	const uint16_t(&FastXFlags)[16] = N == 9 ? kCompVFast9Flags : kCompVFast12Flags;
	compv_uscalar_t fbrighters1, fdarkers1;
	const int32_t minsum = (N == 12 ? 3 : 2);
	compv_uscalar_t i, j, k, arcStart;

	for (i = 0; i < width; ++i, ++IP, ++strengths) {
		uint8_t brighter = CompVMathUtils::clampPixel8(IP[0] + threshold_); // SSE: paddusb
		uint8_t darker = CompVMathUtils::clampPixel8(IP[0] - threshold_); // SSE: psubusb

		// reset strength to zero
		*strengths = 0;

		/***** Cross: 1, 9, 5, 13 *****/
		{
			// compare I1 and I9 aka 0 and 8
			t0 = IP[pixels16[0]];
			t1 = IP[pixels16[8]];
			ddarkers16[0] = (darker > t0) ? (darker - t0) : 0; // SSE: psubusb
			ddarkers16[8] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[0] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[8] = (t1 > brighter) ? (t1 - brighter) : 0;
			sumd = (ddarkers16[0] ? 1 : 0) + (ddarkers16[8] ? 1 : 0);
			sumb = (dbrighters16[0] ? 1 : 0) + (dbrighters16[8] ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}
			// compare I5 and I13 aka 4 and 12
			t0 = IP[pixels16[4]];
			t1 = IP[pixels16[12]];
			ddarkers16[4] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[12] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[4] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[12] = (t1 > brighter) ? (t1 - brighter) : 0;
			sd = (ddarkers16[4] ? 1 : 0) + (ddarkers16[12] ? 1 : 0);
			sb = (dbrighters16[4] ? 1 : 0) + (dbrighters16[12] ? 1 : 0);
			if (!sb && !sd) {
				continue;
			}
			sumb += sb;
			sumd += sd;
			if (sumb < minsum && sumd < minsum) {
				continue;
			}
		}

		/***** Cross: 2, 10, 6, 14 *****/
		{
			// I2 and I10 aka 1 and 9
			t0 = IP[pixels16[1]];
			t1 = IP[pixels16[9]];
			ddarkers16[1] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[9] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[1] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[9] = (t1 > brighter) ? (t1 - brighter) : 0;
			sumd = (ddarkers16[1] ? 1 : 0) + (ddarkers16[9] ? 1 : 0);
			sumb = (dbrighters16[1] ? 1 : 0) + (dbrighters16[9] ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}

			// I6 and I14 aka 5 and 13
			t0 = IP[pixels16[5]];
			t1 = IP[pixels16[13]];
			ddarkers16[5] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[13] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[5] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[13] = (t1 > brighter) ? (t1 - brighter) : 0;
			sd = (ddarkers16[5] ? 1 : 0) + (ddarkers16[13] ? 1 : 0);
			sb = (dbrighters16[5] ? 1 : 0) + (dbrighters16[13] ? 1 : 0);
			if (!sb && !sd) {
				continue;
			}
			sumb += sb;
			sumd += sd;
			if (sumb < minsum && sumd < minsum) {
				continue;
			}
		}

		/***** Cross: 3, 11, 7, 15 *****/
		{
			// I3 and I11 aka 2 and 10
			t0 = IP[pixels16[2]];
			t1 = IP[pixels16[10]];
			ddarkers16[2] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[10] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[2] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[10] = (t1 > brighter) ? (t1 - brighter) : 0;
			sumd = (ddarkers16[2] ? 1 : 0) + (ddarkers16[10] ? 1 : 0);
			sumb = (dbrighters16[2] ? 1 : 0) + (dbrighters16[10] ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}
			// I7 and I15 aka 6 and 14
			t0 = IP[pixels16[6]];
			t1 = IP[pixels16[14]];
			ddarkers16[6] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[14] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[6] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[14] = (t1 > brighter) ? (t1 - brighter) : 0;
			sd = (ddarkers16[6] ? 1 : 0) + (ddarkers16[14] ? 1 : 0);
			sb = (dbrighters16[6] ? 1 : 0) + (dbrighters16[14] ? 1 : 0);
			if (!sb && !sd) {
				continue;
			}
			sumb += sb;
			sumd += sd;
			if (sumb < minsum && sumd < minsum) {
				continue;
			}
		}

		/***** Cross: 4, 12, 8, 16 *****/
		{
			// I4 and I12 aka 3 and 11
			t0 = IP[pixels16[3]];
			t1 = IP[pixels16[11]];
			ddarkers16[3] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[11] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[3] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[11] = (t1 > brighter) ? (t1 - brighter) : 0;
			sumd = (ddarkers16[3] ? 1 : 0) + (ddarkers16[11] ? 1 : 0);
			sumb = (dbrighters16[3] ? 1 : 0) + (dbrighters16[11] ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}
			// I8 and I16 aka 7 and 15
			t0 = IP[pixels16[7]];
			t1 = IP[pixels16[15]];
			ddarkers16[7] = (darker > t0) ? (darker - t0) : 0;
			ddarkers16[15] = (darker > t1) ? (darker - t1) : 0;
			dbrighters16[7] = (t0 > brighter) ? (t0 - brighter) : 0;
			dbrighters16[15] = (t1 > brighter) ? (t1 - brighter) : 0;
			sd = (ddarkers16[7] ? 1 : 0) + (ddarkers16[15] ? 1 : 0);
			sb = (dbrighters16[7] ? 1 : 0) + (dbrighters16[15] ? 1 : 0);
			if (!sb && !sd) {
				continue;
			}
			sumd += sd;
			sumb += sb;
			if (sumb < minsum && sumd < minsum) {
				continue;
			}
		}

		//COMPV_DEBUG_INFO("Passed:%d", ++FIXME);

		// SSE: pmovmskb
		fdarkers1 =
			(ddarkers16[0] ? (1 << 0) : 0)
			| (ddarkers16[1] ? (1 << 1) : 0)
			| (ddarkers16[2] ? (1 << 2) : 0)
			| (ddarkers16[3] ? (1 << 3) : 0)
			| (ddarkers16[4] ? (1 << 4) : 0)
			| (ddarkers16[5] ? (1 << 5) : 0)
			| (ddarkers16[6] ? (1 << 6) : 0)
			| (ddarkers16[7] ? (1 << 7) : 0)
			| (ddarkers16[8] ? (1 << 8) : 0)
			| (ddarkers16[9] ? (1 << 9) : 0)
			| (ddarkers16[10] ? (1 << 10) : 0)
			| (ddarkers16[11] ? (1 << 11) : 0)
			| (ddarkers16[12] ? (1 << 12) : 0)
			| (ddarkers16[13] ? (1 << 13) : 0)
			| (ddarkers16[14] ? (1 << 14) : 0)
			| (ddarkers16[15] ? (1 << 15) : 0);
		fbrighters1 =
			(dbrighters16[0] ? (1 << 0) : 0)
			| (dbrighters16[1] ? (1 << 1) : 0)
			| (dbrighters16[2] ? (1 << 2) : 0)
			| (dbrighters16[3] ? (1 << 3) : 0)
			| (dbrighters16[4] ? (1 << 4) : 0)
			| (dbrighters16[5] ? (1 << 5) : 0)
			| (dbrighters16[6] ? (1 << 6) : 0)
			| (dbrighters16[7] ? (1 << 7) : 0)
			| (dbrighters16[8] ? (1 << 8) : 0)
			| (dbrighters16[9] ? (1 << 9) : 0)
			| (dbrighters16[10] ? (1 << 10) : 0)
			| (dbrighters16[11] ? (1 << 11) : 0)
			| (dbrighters16[12] ? (1 << 12) : 0)
			| (dbrighters16[13] ? (1 << 13) : 0)
			| (dbrighters16[14] ? (1 << 14) : 0)
			| (dbrighters16[15] ? (1 << 15) : 0);

		/* Compute strength */
		{
			strength = 0;
			// SIMD: vector rotations to map arcs
			for (arcStart = 0; arcStart < 16; ++arcStart) {
				t0 = t1 = 0xff;
				if ((fbrighters1 & FastXFlags[arcStart]) == FastXFlags[arcStart]) {
					for (j = arcStart, k = 0; k < N; ++j, ++k) { // SIMD: vector rotation
						t1 = std::min(dbrighters16[j & 15], t1); // hz(pairwise) min (lowest diff)
					}
				}
				if ((fdarkers1 & FastXFlags[arcStart]) == FastXFlags[arcStart]) {
					for (j = arcStart, k = 0; k < N; ++j, ++k) { // SIMD: vector rotation
						t0 = std::min(ddarkers16[j & 15], t0); // hz(pairwise) min (lowest diff)
					}
				}
				if (t1 != 0xff || t0 != 0xff) {
					strength = std::max(strength, std::min(t0, t1));
				}
			}
			*strengths = strength;
		}
	} // for (i ....width)	
}

static void CompVFastNmsGather_C(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	compv_uscalar_t i, j;
	uint8_t strength;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 3; i < width - 3; ++i) {
			if ((strength = pcStrengthsMap[i])) { 
				// If-Else faster than a single if(|||||||)
				if (pcStrengthsMap[i - 1] >= strength) { // left
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + 1] >= strength) { // right
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i - stride - 1] >= strength) { // left-top
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i - stride] >= strength) { // top
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i - stride + 1] >= strength) { // right-top
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + stride - 1] >= strength) { // left-bottom
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + stride] >= strength) { // bottom
					pNMS[i] = 0xff; continue;
				}
				if (pcStrengthsMap[i + stride + 1] >= strength) { // right-bottom
					pNMS[i] = 0xff; continue;
				}
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

static void CompVFastNmsApply_C(uint8_t* pcStrengthsMap, uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	compv_uscalar_t i, j;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 3; i < width - 3; ++i) { // SIMD: start at #zero index to have aligned memory
			if (pNMS[i]) {
				pNMS[i] = 0; // must, for next frame
				pcStrengthsMap[i] = 0; // suppress
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

static COMPV_ERROR_CODE FastRangesAlloc(size_t nRanges, RangeFAST** ppRanges, size_t stride)
{
    /* COMPV_DEBUG_INFO("FAST: alloc %d ranges", nRanges); */

    COMPV_CHECK_EXP_RETURN(nRanges <= 0 || !ppRanges, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_CHECK_CODE_RETURN(FastRangesFree(nRanges, ppRanges));

    *ppRanges = reinterpret_cast<RangeFAST*>(CompVMem::calloc(nRanges, sizeof(RangeFAST)));
    COMPV_CHECK_EXP_RETURN(!*ppRanges, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
#if 0
    RangeFAST* pRanges = *ppRanges;
    for (int32_t i = 0; i < nRanges; ++i) {

        pRanges[i].me = (compv_uscalar_t*)CompVMem::malloc(stride * 1 * sizeof(compv_uscalar_t));
        COMPV_CHECK_EXP_RETURN(!pRanges[i].me, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
#endif
    return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE FastRangesFree(size_t nRanges, RangeFAST** ppRanges)
{
    if (ppRanges && *ppRanges) {
#if 0
        RangeFAST* pRanges = *ppRanges;
        for (int32_t i = 0; i < nRanges; ++i) {
            CompVMem::free((void**)&pRanges[i].me);
        }
#endif
        CompVMem::free(reinterpret_cast<void**>(ppRanges));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
