/*
 *	Fetch segment registers
 */ 

#include <arch/segment.h>

#ifndef S_SPLINT_S
#asm

	.globl  _get_bp
	.globl	_get_cs
	.globl	_get_ds
	.globl	_get_es
	.globl  _get_sp
	.globl	_get_ss

_get_bp:
	mov 	ax, bp
	ret

_get_cs:
	mov	ax, cs
	ret

_get_ds:
	mov 	ax, ds
	ret

_get_es:
	mov	ax, es
	ret

_get_sp:
	mov	ax, sp
	ret

_get_ss:
	mov	ax, ss
	ret

#endasm
#endif
