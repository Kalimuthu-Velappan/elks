#include <arch/irq.h>
#include <linuxmt/config.h>

/*
 *	Easy way to store our kernel DS
 */
#define stashed_ds       cseg_stashed_ds
#define seg_stashed_irq0 cseg_seg_stashed_irq0
#define off_stashed_irq0 cseg_off_stashed_irq0

#define stashed_ss       cseg_stashed_ss
#define stashed_sp       cseg_stashed_sp
#define stashed_irq      cseg_stashed_irq

#ifndef S_SPLINT_S
#asm

	.globl	cseg_stashed_ds
	.globl	cseg_stashed_si
	.globl	cseg_sc_tmp
	.globl	IRQdata_offs

        .even

cseg_stashed_ds:
	.word	0

cseg_seg_stashed_irq0:
	.word	0

cseg_off_stashed_irq0:
	.word	0

cseg_stashed_ss:
	.word 	0

cseg_stashed_sp:
	.word 	0

cseg_stashed_irq:
	.word	0

; from process.c

cseg_stashed_si:
	.word 	0

cseg_sc_tmp:
	.word	0

IRQdata_offs:
        .word   0

#endasm
#endif

void irqtab_init(void)
{
#ifndef S_SPLINT_S
#asm				! disable psion hardware interrupt sources
	mov	al, #0x00
	out	0x15, al
	mov	al, #0x00
	out	0x08, al

	cli
	mov	ax,ds
	seg	cs
	mov	stashed_ds,ax	! FIXME: code is not ROM-able
	xor	ax,ax
	mov	es,ax

	cli
	push	ds
	mov	ax, #0x0000	! change data segment
	mov	ds, ax

	out	0x15, al	! memory protection

	mov	ax, cs
	mov	0x01e6, ax
	mov	ax, #_irq0
	mov	0x01e4, ax

	pop	ds
#if 0
	mov	ax, cs
	mov	0x01c2, ax
	lea	ax,_irq0
	seg	es
	mov	[0xe4],ax
	mov	ax,cs
	seg	es
	mov	[0x02],ax

	lea	ax,_irq1
	seg	es
	mov	[0x04],ax
	mov	ax,cs
	seg	es
	mov	[0x06],ax
	
	lea	ax,_irq2
	seg	es
	mov	[0x08],ax
	mov	ax,cs
	seg	es
	mov	[0x0A],ax

	lea	ax,_irq3
	seg	es
	mov	[0x0C],ax
	mov	ax,cs
	seg	es
	mov	[0x0E],ax

	lea	ax,_irq4
	seg	es
	mov	[0x10],ax
	mov	ax,cs
	seg	es
	mov	[0x12],ax
	
				! etc for the rest
#endif

				! Setup INT 0x80 (for syscall)
	lea	ax,_syscall_int
	seg	es
	mov	[512],ax
	mov	ax,cs
	seg	es
	mov	[514],ax

				! Tidy up
	mov	ax,ds
	mov	es,ax
	sti
#endasm
#endif
}	
 
/*
 *	IRQ and IRQ return paths for Linux 8086
 */

#ifndef S_SPLINT_S
#asm
!
!	Other IRQs (see IRQ 0 at the bottom for the
!	main code).
!
	.text
	.globl	_irq1
!	.globl	_irq2
!	.globl	_irq3
!	.globl	_irq4
!	.globl	_irq5
!	.globl	_irq6
!	.globl	_irq7
!	.globl	_irq8
!	.globl	_irq9
!	.globl	_irq10
!	.globl	_irq11
!	.globl	_irq12
!	.globl	_irq13
!	.globl	_irq14
!	.globl	_irq15

	.extern	_do_IRQ

	.data

	.extern	_cache_A1
	.extern	_cache_21
	.extern	_jiffies

	.text 
			
!_irq0:
!	push	ax
!	mov	ax,#0
!	br	_irqit

_irq1:
	push	ax
	mov	ax,#1
	br	_irqit

_irq2:
	push	ax
	mov	ax,#2
	br	_irqit

_irq3:
	push	ax
	mov	ax,#3
	br	_irqit

_irq4:
	push	ax
	mov	ax,#4
	br	_irqit

_irq5:
	push	ax
	mov	ax,#5
	br	_irqit

_irq6:
	push	ax
	mov	ax,#6
	br	_irqit

_irq7:
	push	ax
	mov	ax,#7
	br	_irqit

_irq8:
	push	ax
	mov	ax,#8
	br	_irqit

_irq9:
	push	ax
	mov	ax,#9
	br	_irqit

_irq10:
	push	ax
	mov	ax,#10
	br	_irqit

_irq11:
	push	ax
	mov	ax,#11
	jmp	_irqit

_irq12:
	push	ax
	mov	ax,#12
	jmp	_irqit

_irq13:
	push	ax
	mov	ax,#13
	jmp	_irqit

_irq14:
	push	ax
	mov	ax,#14
	jmp	_irqit

_irq15:
	push	ax
	mov	ax,#15
	jmp	_irqit

!	Traps (we use IRQ 16->31 for these)
!
!	Currently not used so removed for space.

#if 0

	.globl	_div0

_div0:
	push	ax
	mov	ax,#16
	jmp	_irqit

	.globl _dbugtrap

_dbugtrap:
	push	ax
	mov	ax,#17
	jmp	_irqit

	.globl _nmi

_nmi:
	push	ax
	mov	ax,#18
	jmp	_irqit

	.globl	_brkpt

_brkpt:
	push 	ax
	mov	ax,#19
	jmp	_irqit

	.globl	_oflow

_oflow:
	push	ax
	mov	ax,#20
	jmp	_irqit

	.globl	_bounds

_bounds:
	push	ax
	mov	ax,#21
	jmp	_irqit
	
	.globl	_invop

_invop:
	push	ax
	mov	ax,#22
	jmp	_irqit
	
	.globl _devnp

_devnp:
	push	ax
	mov	ax,#23
	jmp	_irqit

	.globl	_dfault

_dfault:
	push	ax
	mov	ax,#24
	jmp	_irqit

!
!	trap 9 is reserved
!
	.globl _itss
_itss:

	push	ax
	mov	ax,#26
	jmp	_irqit

	.globl _nseg

_nseg:
	push	ax
	mov	ax,#27
	jmp	_irqit
	
	.globl _stkfault

_stkfault:
	push 	ax
	mov	ax,#28
	jmp	_irqit

	.globl	_segovr

_segovr:
	push	ax
	mov	ax,#29
	jmp	_irqit
	
	.globl _pfault

_pfault:
	push	ax
	mov	ax,#30
	jmp	_irqit

!
!	trap 15 is reserved
!
	.globl	_fpetrap

_fpetrap:
	push	ax
	mov	ax,#32
	jmp	_irqit

	.globl	_algn

_algn:
	push	ax
	mov	ax,#33
	jmp	_irqit


#endif

!
!	On entry CS:IP is all we can trust
!
!	There are three possible cases to cope with
!
!	SS = kernel DS. 
!		Interrupted kernel mode code.
!		No task switch allowed
!		Running on a kernel process stack anyway.
!
!	SS = current->user_ss
!		Interrupted user mode code
!		Switch to kernel stack for process (will be free)
!		Task switch allowed
!
!	Other
!		BIOS or other 'strange' code.
!		Must be called from kernel space, but kernel stack is in use
!		Switch to int_stack
!		No task switch allowed.
!
!	We do all of this to avoid per process interrupt stacks and 
!	related nonsense. This way we need only one dedicted int stack
!
!
	.globl	_irq0

_irq0:
!
!	Save AX and load it with the IRQ number
!
	push	ax
	xor	ax,ax
	
_irqit:
!
!	Save all registers
!
	cli		! Might not be disabled on an exception
	push	ds
	push	es
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
!
!	Recover segments
!
	seg	cs
	mov	stashed_irq,ax	! Save IRQ number
	mov	ax,ss		! Get current SS
	mov	bx,ax		! Save for later
	seg	cs
	mov	stashed_ss, ax	! Save SS:SP
	mov	ax,sp
	seg	cs
	mov	stashed_sp, ax
	seg	cs		! Recover the data segment
	mov	ax,stashed_ds
!
!	Switch segments
!
	mov	ds,ax
	mov	es,ax
!
!	Set up task switch controller
!
	xor	ch,ch		! Assume we are not allowed to switch
!
!	See where we were (BX holds the SS on entry)
!
	cmp	ax,bx		! SS = kernel SS ?
	je	ktask		! Kernel - no work
!
!	User or BIOS etc
!
	mov	ax,bx
	mov	bx,_current
	cmp	ax,4[bx]	! entry ss = current->t_regs.ss?
	je	utask		! Switch to kernel
!
!	Bios etc - switch to interrupt stack
!
	mov	sp,#_intstack
!	lea	sp, _intstack
	j	switched
!
!	User task. Extract kernel SP. (BX already holds current)
!

utask:
	mov	ax,[bx]		! kernel stack ptr
	mov	sp,ax		! switch to kernel stack
	inc	ch		! Switch allowable
	j	switched

ktask:
!
!	In ktask state we have a suitable stack. It might be 
!	better to use the intstack..
!

switched:
	mov	ax,ds
	mov	ss,ax		! Set SS: right
!
!	Put the old SS;SP on the top of the stack. We can not
!	leave them in stashed_ss/sp as we could re-enter the
!	routine on a reschedule.
!
	seg 	cs
	push	stashed_sp
	seg	cs
	push	stashed_ss
!
!	We are on a suitable stack and cx says whether we can	
!	switch afterwards. The C code will want to eat CX so
!	we have to hide it
!
!	The registers are now stored. Remember where
!
	mov	bp,sp
	mov	_can_tswitch, ch
	push	cx		! Save ch
	seg	cs		! Recover the IRQ we saved
	mov	ax,stashed_irq
	push	ax		! IRQ for later
	push	bp		! Register base
	push	ax		! IRQ number
!
!	Call the C code
!
	call	_do_IRQ		! Do the work
!
!	Return path
!
	pop	ax		! We want the ax value back
	pop	bx		! Drop arguments
	pop	ax		! Saved IRQ
	pop	cx		! Recover switch allowed flag
!
!	Restore any chips
!
! The individual IRQ now reset as the Psion has a weird structure
!
!	Now look at rescheduling
!
	cmp	ch,#0			! Schedule allowed ?
	je	nosched			! No
!	mov	bx,_need_resched	! Schedule needed
!	cmp	bx,#0			! 
!	je	nosched			! No
	call	_schedule		! Task switch
!
! Fix current->ksp (_schedule messes it up).
!
	pop	ax	! stacked SS
	pop	cx	! stacked SP
	mov	bx,_current
	mov	[bx],sp
	j	noschedpop
	
nosched:
!
!	Now we have to rescue our stack pointer/segment.
!
	pop	ax	! SS
	pop	cx	! SP
!
!	Switch stacks to the interrupting stack
!

noschedpop:
	mov	ss,ax
	mov	sp,cx
!
!	Restore registers and return
!
	pop	bp
	pop 	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	es
	pop	ds
	pop	ax
!
!	Iret restores CS:IP and F (thus including the interrupt bit)
!
	iret
!
!	Data.
!
	.data

	.globl	_can_tswitch

_can_tswitch:
	.byte 0
	.zerow	256		! (was) 128 byte interrupt stack

_intstack:

#endasm
#endif
