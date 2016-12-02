/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_renderer.h"

COMPV_NAMESPACE_BEGIN()

compv_renderer_id_t CompVRenderer::s_nRendererId = 0;

CompVRenderer::CompVRenderer(COMPV_PIXEL_FORMAT ePixelFormat)
    : CompVObj()
    , m_nId(compv_atomic_inc(&CompVRenderer::s_nRendererId))
    , m_ePixelFormat(ePixelFormat)
{
}

CompVRenderer::~CompVRenderer()
{

}

COMPV_NAMESPACE_END()