#ifndef __LINUX_KERNEL_H__
#define __LINUX_KERNEL_H__

/*
 * 'kernel.h' contains some often-used function prototypes etc
 */

#ifdef __KERNEL__

#define INT_MAX		((int)(~0U>>1))
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define ULONG_MAX	(~0UL)

extern void panic();
void do_exit();

extern int kill_proc();
extern int kill_pg();
extern int kill_sl();

extern int printk();

extern int wait_for_keypress();

/*
 * This is defined as a macro, but at some point this might become a
 * real subroutine that sets a flag if it returns true (to do
 * BSD-style accounting where the process is flagged if it uses root
 * privs).  The implication of this is that you should do normal
 * permissions checks first, and check suser() last.
 *
 * "suser()" checks against the effective user id.
 */

#define suser() (current->euid == 0)

#endif /* __KERNEL__ */

#endif
