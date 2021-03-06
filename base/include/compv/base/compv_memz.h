/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEMZ_H_)
#define _COMPV_BASE_MEMZ_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Helper class to avoid calling malloc + memset which is slooow
// https://vorpus.org/blog/why-does-calloc-exist/
template<class T>
class CompVMemZero : public CompVObj
{
protected:
	CompVMemZero(size_t rows, size_t cols) : m_nCols(cols), m_nRows(rows) {
		size_t strideInBytes = CompVMem::alignForward(cols * sizeof(T));
		m_pMem = static_cast<uint8_t*>(::calloc(((strideInBytes * rows)) + CompVMem::bestAlignment(), sizeof(uint8_t)));
		m_pPtr = reinterpret_cast<T*>(CompVMem::alignForward(reinterpret_cast<uintptr_t>(m_pMem)));
		m_nStride = strideInBytes / sizeof(T);
	}
public:
	virtual ~CompVMemZero() {
		if (m_pMem) {
			free(m_pMem);
		}
	}
	COMPV_OBJECT_GET_ID(CompVMemZero);
	COMPV_INLINE T* ptr(size_t row = 0, size_t col = 0) {
		return (m_pPtr + (row * m_nStride)) + col;
	}
	COMPV_INLINE size_t cols() {
		return m_nCols;
	}
	COMPV_INLINE size_t rows() {
		return m_nRows;
	}
	COMPV_INLINE size_t stride() {
		return m_nStride;
	}
	static COMPV_ERROR_CODE newObj(CompVPtr<CompVMemZero<T> *>* memz, size_t rows, size_t cols) {
		COMPV_CHECK_EXP_RETURN(!memz || !rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVPtr<CompVMemZero<T> *> memz_ = new CompVMemZero<T>(rows, cols);
		COMPV_CHECK_EXP_RETURN(!memz_, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_EXP_RETURN(!memz_->m_pPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		*memz = memz_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	T* m_pPtr;
	size_t m_nCols;
	size_t m_nRows;
	size_t m_nStride;
	uint8_t* m_pMem;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MEMZ_H_ */
