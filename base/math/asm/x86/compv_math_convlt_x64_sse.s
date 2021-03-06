;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 9
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define vecZero				xmm8
	%define vecSum0				xmm9
	%define vecSum1				xmm5
	%define vecSum2				xmm6
	%define vecSum3				xmm7
	pxor vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define width			rbx
	%define step			rcx
	%define j				rsi
	%define i				rdi
	%define inPtr			r8
	%define outPtr			r9
	%define vthzKernPtr		r10
	%define stride			r11
	%define widthMinus15	r12
	%define widthMinus3		r13
	%define octet			r14
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus15, [width - 15]
	lea widthMinus3, [width - 3]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2 
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu xmm0, [rax] ; xmm0 = vecInPtr
				add rax, step
				movss xmm4, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				shufps xmm4, xmm4, 0x0 ; xmm4 = vecCoeff
				movdqa xmm3, xmm0
				punpcklbw xmm0, vecZero
				punpckhbw xmm3, vecZero
				movdqa xmm1, xmm0				
				punpcklwd xmm0, vecZero
				punpckhwd xmm1, vecZero
				movdqa xmm2, xmm3
				punpcklwd xmm2, vecZero
				punpckhwd xmm3, vecZero
				cvtdq2ps xmm0, xmm0
				cvtdq2ps xmm1, xmm1
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				mulps xmm0, xmm4
				mulps xmm1, xmm4
				mulps xmm2, xmm4
				mulps xmm3, xmm4
				cmp rdx, kernSize
				addps vecSum0, xmm0
				addps vecSum1, xmm1
				addps vecSum2, xmm2
				addps vecSum3, xmm3
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			cvttps2dq vecSum0, vecSum0
			cvttps2dq vecSum1, vecSum1
			cvttps2dq vecSum2, vecSum2
			cvttps2dq vecSum3, vecSum3
			packssdw vecSum0, vecSum1
			packssdw vecSum2, vecSum3
			packuswb vecSum0, vecSum2
			movdqu [outPtr + i], vecSum0
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width - 3; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_LoopWidth_Per4Samples
		.LoopWidth_Per4Samples:
			xorps vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movss xmm2, [rax] ; xmm2 = vecInPtr
				lea rax, [rax + step]
				movss xmm3, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				punpcklbw xmm2, vecZero
				punpcklwd xmm2, vecZero
				cvtdq2ps xmm2, xmm2
				shufps xmm3, xmm3, 0x0 ; xmm3 = vecCoeff
				mulps xmm2, xmm3
				cmp rdx, kernSize
				addps vecSum0, xmm2
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;
			
			cvttps2dq vecSum0, vecSum0
			lea i, [i + 4]
			packssdw vecSum0, vecSum0
			cmp i, widthMinus3
			packuswb vecSum0, vecSum0
			movd [outPtr + i - 4], vecSum0
			jl .LoopWidth_Per4Samples
			.EndOf_LoopWidth_Per4Samples
			; EndOf_LoopWidth_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth_Per1Samples
		.LoopWidth_Per1Samples:
			xorps vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movzx octet, byte [rax]
				movss xmm3, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES] ; xmm3 = vecCoeff
				cvtsi2ss xmm2, octet
				mulss xmm2, xmm3
				inc rdx
				lea rax, [rax + step]
				cmp rdx, kernSize
				addss vecSum0, xmm2	
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			inc i
			cvttss2si rax, vecSum0
			cmp i, width
			mov [outPtr + i - 1], byte al
			jl .LoopWidth_Per1Samples
			.EndOf_LoopWidth_Per1Samples
			; EndOf_LoopWidth_Per1Samples ;
		
		dec j
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero
	%undef vecSum0
	%undef vecSum1
	%undef vecSum2
	%undef vecSum3

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef widthMinus15
	%undef widthMinus3
	%undef octet
	%undef kernSize

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> int16_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const int16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecCoeff			xmm3
	%define vec0				xmm4
	%define vec1				xmm5

	pxor vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define width			rbx
	%define step			rcx
	%define j				rsi
	%define i				rdi
	%define inPtr			r8
	%define outPtr			r9
	%define vthzKernPtr		r10
	%define stride			r11
	%define widthMinus15	r12
	%define widthMinus3		r13
	%define octet			r14
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus15, [width - 15]
	lea widthMinus3, [width - 3]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			pxor vecSum0, vecSum0
			pxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				add rax, step
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				movdqa vec1, vec0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				punpckhbw vec1, vecZero
				pmullw vec1, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				paddw vecSum1, vec1	
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			movdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			movdqu [outPtr + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;
						
			movdqu [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movd vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			movq [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movd vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;
						
			movq rax, vecSum0
			%assign index 0
			%rep 4
				mov [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 16
				%assign index index+1
			%endrep	
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;

		
		dec j
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero
	%undef vecSum0
	%undef vecSum1
	%undef vecCoeff
	%undef vec0
	%undef vec1

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef widthMinus15
	%undef widthMinus3
	%undef octet
	%undef kernSize

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const int16_t* inPtr
; arg(1) -> int16_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const int16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define vecSum0				xmm0
	%define vecSum1				xmm1
	%define vecCoeff			xmm2
	%define vec0				xmm3
	%define vec1				xmm4

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define width			rbx
	%define step			rcx
	%define j				rsi
	%define i				rdi
	%define inPtr			r8
	%define outPtr			r9
	%define vthzKernPtr		r10
	%define stride			r11
	%define widthMinus15	r12
	%define widthMinus3		r13
	%define octet			r14
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	lea step, [step * COMPV_YASM_INT16_SZ_BYTES] ; from samples to bytes
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus15, [width - 15]
	lea widthMinus3, [width - 3]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			pxor vecSum0, vecSum0
			pxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movdqu vec1, [rax + 8*COMPV_YASM_INT16_SZ_BYTES]
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				add rax, step
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				pmullw vec1, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				paddw vecSum1, vec1	
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			movdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			movdqu [outPtr + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movdqu vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;
						
			movdqu [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			movq [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;
						
			movq rax, vecSum0
			%assign index 0
			%rep 4
				mov [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 16
				%assign index index+1
			%endrep	
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;

		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecSum0
	%undef vecSum1
	%undef vecCoeff
	%undef vec0
	%undef vec1

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef widthMinus15
	%undef widthMinus3
	%undef octet
	%undef kernSize

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const uint16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecCoeff			xmm3
	%define vec0				xmm4
	%define vec1				xmm5

	pxor vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define width			rbx
	%define step			rcx
	%define j				rsi
	%define i				rdi
	%define inPtr			r8
	%define outPtr			r9
	%define vthzKernPtr		r10
	%define stride			r11
	%define widthMinus15	r12
	%define widthMinus3		r13
	%define octet			r14
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus15, [width - 15]
	lea widthMinus3, [width - 3]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			pxor vecSum0, vecSum0
			pxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				add rax, step
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				movdqa vec1, vec0
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				punpckhbw vec1, vecZero
				pmulhuw vec1, vecCoeff
				cmp rdx, kernSize
				paddusw vecSum0, vec0
				paddusw vecSum1, vec1	
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			packuswb vecSum0, vecSum1
			movdqu [outPtr + i], vecSum0
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				cmp rdx, kernSize
				paddusw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			packuswb vecSum0, vecSum0
			movq [outPtr + i], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movd vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				cmp rdx, kernSize
				paddusw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			packuswb vecSum0, vecSum0
			movd [outPtr + i], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movd vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				cmp rdx, kernSize
				paddusw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			packuswb vecSum0, vecSum0
			movd rax, vecSum0
			%assign index 0
			%rep 4
				mov [outPtr + i], byte al
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 8
				%assign index index+1
			%endrep	
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;

		
		dec j
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero
	%undef vecSum0
	%undef vecSum1
	%undef vecCoeff
	%undef vec0
	%undef vec1

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef widthMinus15
	%undef widthMinus3
	%undef octet
	%undef kernSize

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT
