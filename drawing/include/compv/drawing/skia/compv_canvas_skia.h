/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_CANVAS_SKIA_H_)
#define _COMPV_DRAWING_CANVAS_SKIA_H_

#include "compv/drawing/compv_config.h"
#if HAVE_SKIA
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/drawing/compv_canvas.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include <string>

COMPV_NAMESPACE_BEGIN()

class CompVCanvasImplSkia;
typedef CompVPtr<CompVCanvasImplSkia* > CompVCanvasImplSkiaPtr;
typedef CompVCanvasImplSkiaPtr* CompVCanvasImplSkiaPtrPtr;

class CompVCanvasImplSkia : public CompVCanvasImpl
{
protected:
	CompVCanvasImplSkia();
public:
	virtual ~CompVCanvasImplSkia();
	COMPV_GET_OBJECT_ID(CompVCanvasImplSkia);

	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) override;
	virtual COMPV_ERROR_CODE drawLine(int x0, int y0, int x1, int y1) override;

	static COMPV_ERROR_CODE newObj(CompVCanvasImplSkiaPtrPtr skiaCanvas);

protected:

private:

};

COMPV_NAMESPACE_END()

#endif /* HAVE_SKIA */

#endif /* _COMPV_DRAWING_CANVAS_SKIA_H_ */