/*
 *  linux/arch/i386/kernel/signal.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Modified for ELKS 1998-1999 Al Riddoch
 */

#include <linuxmt/types.h>
#include <linuxmt/config.h>
#include <linuxmt/sched.h>
#include <linuxmt/mm.h>
#include <linuxmt/kernel.h>
#include <linuxmt/signal.h>
#include <linuxmt/errno.h>
#include <linuxmt/wait.h>
#include <linuxmt/debug.h>

#include <arch/segment.h>

#define _S(nr) (1<<((nr)-1))

#define put_user(val,ptr) pokew(current->t_regs.ds,ptr,val)

#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

int sys_waitpid();
int do_signal();

/*
 * OK, we're invoking a handler
 */	

static void handle_signal(signr, sa)
unsigned signr;
struct sigaction *sa;
{
	printd_sig1("Setting up return stack for sig handler %x.\n", sa->sa_handler);
	printd_sig1("Stack at %x\n", current->t_regs.sp);
	arch_setup_sighandler_stack(current, sa->sa_handler, signr);
	printd_sig1("Stack at %x\n", current->t_regs.sp);
	sa->sa_handler = SIG_DFL;
}

int do_signal()
{
	register struct sigaction * sa;
	register __ptask currentp = current;
	unsigned signr;

	while (currentp->signal) {
		signr = find_first_non_zero_bit(&currentp->signal, NSIG);
		if (signr == NSIG) {
			panic("No signal set!\n");
		}
		printd_sig2("Process %d has signal %d.\n", currentp->pid, signr);
		sa = &currentp->sig.action[signr];
		signr++;
		if (sa->sa_handler == SIG_IGN) {
			printd_sig("Ignore\n");
			if (signr != SIGCHLD)
				continue;
			while (sys_wait4(-1,NULL,WNOHANG) > 0);
			continue;
		}
		if (sa->sa_handler == SIG_DFL) {
			printd_sig("Default\n");
			if (currentp->pid == 1)
				continue;
			switch (signr) {
			case SIGCONT: case SIGCHLD: case SIGWINCH:
				continue;
			case SIGSTOP: case SIGTSTP:
#ifndef SMALLSIG
			case SIGTTIN: case SIGTTOU:
#endif
				currentp->state = TASK_STOPPED;
			/*	currentp->exit_code =  signr; */
			/* Let the parent know */
				currentp->p_parent->child_lastend = currentp->pid;
				currentp->p_parent->lastend_status = signr;
				schedule();
				continue;
/*
			case SIGQUIT: case SIGABRT: case SIGSEGV:
#ifndef SMALLSIG
			case SIGFPE: case SIGILL: case SIGTRAP:
#endif
		/* This is where we dump the core, which we must do */
			default:
				do_exit(signr);
			}
		}
		handle_signal(signr, sa);
		return 1;
	}
	return 0;
}
