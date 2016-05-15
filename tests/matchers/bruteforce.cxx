#include <compv/compv_api.h>

#include "../common.h"

#define FAST_THRESHOLD				10
#define FAST_NONMAXIMA				true
#define ORB_MAX_FEATURES			2000
#define ORB_PYRAMID_LEVELS			8
#define ORB_PYRAMID_SCALEFACTOR		0.83f
#define ORB_PYRAMID_SCALE_TYPE		COMPV_SCALE_TYPE_BILINEAR
#define CROSS_CHECK					false
#define KNN							4 // for this test use values from 1..4
#define NORM						COMPV_BRUTEFORCE_NORM_HAMMING
#define JPEG_TRAIN_IMG				"C:/Projects/GitHub/pan360/images/mandekalou.JPG" // Mande Griots
#define JPEG_QUERY_IMAGE			"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

#define LOOP_COUNT					1

static const std::string expectedMD5_ST[KNN] = {
#if KNN > 0
	"550e2cc896ec11103593815af64226be", // KNN = 1
#endif
#if KNN > 1
	"b846befc7e349bb57bbadd1f0ee8c709", // KNN = 2
#endif
#if KNN > 2
	"ce4251684e649f8a3299120056342f40", // KNN = 3
#endif
#if KNN > 3
	"62ea24e07898727de748d36d47aa342f", // KNN = 4
#endif
};
static const std::string expectedMD5_MT[KNN] = {
#if KNN > 0
	"6fbdc5ff7279a63c922514e3033f586b", // KNN = 1
#endif
#if KNN > 1
	"097e549336f04c46b91d2070037d0efd", // KNN = 2
#endif
#if KNN > 2
	"6580f428fd94b3585bcf273b9719d4dd", // KNN = 3
#endif
#if KNN > 3
	"7ccb58da0c2518d16183e71bfd7b5fbd", // KNN = 4
#endif
};

bool TestBruteForce()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVPtr<CompVMatcher *> matcher;
	CompVPtr<CompVFeatureDete* > dete; // feature detector
	CompVPtr<CompVFeatureDesc* > desc; // feature descriptor
	CompVPtr<CompVImage *> trainImage;
	CompVPtr<CompVImage *> queryImage;
	CompVPtr<CompVBoxInterestPoint* > interestPoints;
	CompVPtr<CompVArray<uint8_t>* > queryDescriptors;
	CompVPtr<CompVArray<uint8_t>* > trainDescriptors;
	CompVPtr<CompVArray<CompVDMatch>* > matches;
	int32_t val32;
	bool valBool;
	float valFloat;
	uint64_t timeStart, timeEnd;

	COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_TRAIN_IMG, &trainImage));
	COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_QUERY_IMAGE, &queryImage));
	COMPV_CHECK_CODE_BAIL(err_ = trainImage->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &trainImage));
	COMPV_CHECK_CODE_BAIL(err_ = queryImage->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &queryImage));

	COMPV_CHECK_CODE_BAIL(err_ = CompVFeatureDete::newObj(COMPV_ORB_ID, &dete));
	COMPV_CHECK_CODE_BAIL(err_ = CompVFeatureDesc::newObj(COMPV_ORB_ID, &desc));
	COMPV_CHECK_CODE_BAIL(err_ = desc->attachDete(dete)); // attach detector to make sure we'll share context
	val32 = FAST_THRESHOLD;
	COMPV_CHECK_CODE_BAIL(err_ = dete->set(COMPV_ORB_SET_INT32_FAST_THRESHOLD, &val32, sizeof(val32)));
	valBool = FAST_NONMAXIMA;
	COMPV_CHECK_CODE_BAIL(err_ = dete->set(COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP, &valBool, sizeof(valBool)));
	val32 = ORB_PYRAMID_LEVELS;
	COMPV_CHECK_CODE_BAIL(err_ = dete->set(COMPV_ORB_SET_INT32_PYRAMID_LEVELS, &val32, sizeof(val32)));
	val32 = ORB_PYRAMID_SCALE_TYPE;
	COMPV_CHECK_CODE_BAIL(err_ = dete->set(COMPV_ORB_SET_INT32_PYRAMID_SCALE_TYPE, &val32, sizeof(val32)));
	valFloat = ORB_PYRAMID_SCALEFACTOR;
	COMPV_CHECK_CODE_BAIL(err_ = dete->set(COMPV_ORB_SET_FLOAT_PYRAMID_SCALE_FACTOR, &valFloat, sizeof(valFloat)));
	val32 = ORB_MAX_FEATURES;
	COMPV_CHECK_CODE_BAIL(err_ = dete->set(COMPV_ORB_SET_INT32_MAX_FEATURES, &val32, sizeof(val32)));

	COMPV_CHECK_CODE_BAIL(err_ = CompVMatcher::newObj(COMPV_BRUTEFORCE_ID, &matcher));
	val32 = KNN;
	COMPV_CHECK_CODE_BAIL(err_ = matcher->set(COMPV_BRUTEFORCE_SET_INT32_KNN, &val32, sizeof(val32)));
	val32 = NORM;
	COMPV_CHECK_CODE_BAIL(err_ = matcher->set(COMPV_BRUTEFORCE_SET_INT32_NORM, &val32, sizeof(val32)));
	valBool = CROSS_CHECK;
	COMPV_CHECK_CODE_BAIL(err_ = matcher->set(COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK, &valBool, sizeof(valBool)));

	COMPV_CHECK_CODE_BAIL(err_ = dete->process(trainImage, interestPoints));
	COMPV_CHECK_CODE_BAIL(err_ = desc->process(trainImage, interestPoints, &trainDescriptors));

	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_BAIL(err_ = dete->process(queryImage, interestPoints));
		COMPV_CHECK_CODE_BAIL(err_ = desc->process(queryImage, interestPoints, &queryDescriptors));
		COMPV_CHECK_CODE_BAIL(err_ = matcher->process(queryDescriptors, trainDescriptors, &matches));
	}
	timeEnd = CompVTime::getNowMills();
	
	for (size_t row = 0; row < matches->rows(); ++row) {
		for (size_t col = 0; col < matches->cols(); ++col) {
			COMPV_CHECK_EXP_BAIL(row > 0 && matches->ptr(row - 1, col)->distance > matches->ptr(row, col)->distance, (err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED));
		}
	}

	COMPV_DEBUG_INFO("Elapsed time (TestBruteForce) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	const std::string(*expectedMD5)[KNN] = CompVEngine::isMultiThreadingEnabled() ? &expectedMD5_MT : &expectedMD5_ST;
	COMPV_CHECK_EXP_BAIL(arrayMD5<CompVDMatch>(matches) != (*expectedMD5)[KNN - 1], (err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED));

bail:
	COMPV_CHECK_CODE_ASSERT(err_);
	return COMPV_ERROR_CODE_IS_OK(err_);
}