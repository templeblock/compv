/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_DRAWING_DRAW_LINES_H_)
#define _COMPV_GL_DRAWING_DRAW_LINES_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/drawing/compv_gl_draw.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

enum COMPV_GL_LINE_TYPE {
	COMPV_GL_LINE_TYPE_SIMPLE,
	COMPV_GL_LINE_TYPE_MATCH,
};

COMPV_OBJECT_DECLARE_PTRS(GLDrawLines)

class CompVGLDrawLines : public CompVGLDraw
{
protected:
	CompVGLDrawLines();
public:
	virtual ~CompVGLDrawLines();
	COMPV_OBJECT_GET_ID(CompVGLDrawLines);

	COMPV_ERROR_CODE lines(const CompVGLPoint2D* lines, GLsizei count);
	COMPV_ERROR_CODE matches(const CompVGLPoint2D* lines, GLsizei count, GLsizei queryOffsetx);

	static COMPV_ERROR_CODE newObj(CompVGLDrawLinesPtrPtr drawLines);

private:
	COMPV_ERROR_CODE draw(const CompVGLPoint2D* lines, GLsizei count, COMPV_GL_LINE_TYPE type = COMPV_GL_LINE_TYPE_SIMPLE, GLsizei offset1x = 0, GLsizei offset2x = 0);

private:
	GLint m_fboWidth;
	GLint m_fboHeight;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_DRAWING_DRAW_LINES_H_ */