/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_RENDERER_H_)
#define _COMPV_DRAWING_RENDERER_H_

#include "compv/drawing/compv_config.h"
#include "compv/drawing/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/drawing/compv_canvas.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

typedef long compv_renderer_id_t;

class CompVSurface;

class CompVRenderer;
typedef CompVPtr<CompVRenderer* > CompVRendererPtr;
typedef CompVRendererPtr* CompVRendererPtrPtr;

class COMPV_DRAWING_API CompVRenderer : public CompVObj, public CompVCanvas
{
protected:
	CompVRenderer(COMPV_PIXEL_FORMAT ePixelFormat);
public:
	virtual ~CompVRenderer();
	
	COMPV_INLINE COMPV_PIXEL_FORMAT pixelFormat()const { return m_ePixelFormat; }
	COMPV_INLINE compv_renderer_id_t id()const { return m_nId; }

	virtual bool isGLEnabled()const = 0;

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_PIXEL_FORMAT m_ePixelFormat;
	static compv_renderer_id_t s_nRendererId;
	compv_renderer_id_t m_nId;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_RENDERER_H_ */