/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BASE_H_)
#define _COMPV_BASE_BASE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVBase : public CompVObj
{
protected:
	CompVBase();
public:
	virtual ~CompVBase();
	static COMPV_ERROR_CODE init(int32_t numThreads = -1);
	static COMPV_ERROR_CODE deInit();
	static COMPV_ERROR_CODE setTestingModeEnabled(bool bTesting);
	static COMPV_ERROR_CODE setMathTrigFastEnabled(bool bMathTrigFast);
	static COMPV_ERROR_CODE setMathFixedPointEnabled(bool bMathFixedPoint);
	static bool isInitialized();
	static bool isInitializing();
	static bool isBigEndian();
	static bool isTestingMode();
	static bool isMathTrigFast();
	static bool isMathFixedPoint();

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static bool s_bInitialized;
	static bool s_bInitializing;
	static bool s_bBigEndian;
	static bool s_bTesting;
	static bool s_bMathTrigFast;
	static bool s_bMathFixedPoint;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BASE_H_ */
