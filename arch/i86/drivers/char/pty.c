/*
 * arch/i86/drivers/char/pty.c - A simple pty driver
 * (C) 1999 Alistair Riddoch
 */


#include <linuxmt/types.h>
#include <linuxmt/config.h>
#include <linuxmt/sched.h>
#include <linuxmt/fs.h>
#include <linuxmt/fcntl.h>
#include <linuxmt/errno.h>
#include <linuxmt/mm.h>
#include <linuxmt/termios.h>
#include <linuxmt/chqueue.h>
#include <linuxmt/ntty.h>
#include <linuxmt/major.h>


int pty_open(inode, file)
struct inode * inode;
struct file * file;
{
	struct tty * otty;

	if (otty = determine_tty(inode->i_rdev)) {
		printk("pty_open() %x", otty);
		if (otty->flags & TTY_OPEN) {
			return -EBUSY;
		}
		printk(" succeeded\n");
		return 0;
	} else {
		printk(" failed\n");
		return -ENODEV;
	}
}

int pty_release(inode, file)
struct inode * inode;
struct file * file;
{
	struct tty * otty;

	if (otty = determine_tty(inode->i_rdev)) {
		kill_pg(otty->pgrp, SIGHUP, 1);
	}
	return 0;
}

int pty_ioctl(inode, file, cmd, arg)
struct inode * inode;
struct file * file;
int cmd;
char * arg;
{
	return -EINVAL;
}

int pty_select (inode, file, sel_type)
struct inode * inode;
struct file * file;
int sel_type;
{
	register struct tty *tty=determine_tty(inode->i_rdev);

	switch (sel_type) {
		case SEL_IN:
			if (chq_peekch(&tty->outq)) {
				return 1;
			}
		case SEL_EX: /* fall thru! */
			select_wait (&tty->outq.wq);
			return 0;
		case SEL_OUT: /* Hm.  We can always write to a tty?  (not really) */
		default:
			return 1;
	}
}

int pty_read(inode,file,data,len)
struct inode *inode;
struct file *file;
char *data;
int len;
{
	register struct tty * tty = determine_tty(inode->i_rdev);
	int i = 0, j, l;
	unsigned char ch;

	printk("pty_read()");
	if (tty == NULL) {
		return -ENODEV;
	}
	l = (file->f_flags & O_NONBLOCK) ? 0 : 1;
	while (i < len) {
		j = chq_getch(&tty->outq, &ch, l);
		if (j == -1) {
			if (l) {
				return -EINTR;
			} else {
				break;
			}
		}
		printk(" rc[%d,%d]", i, len);
		pokeb(current->t_regs.ds, (data + i++), ch);
	}
	printk("{%d}\n", i);
	return i;
}

int pty_write(inode, file, data, len)
struct inode *inode;
struct file *file;
char *data;
int len;
{
	register struct tty * tty = determine_tty(inode->i_rdev);
	int i = 0, l;
	char ch;

	printk("pty_write()");
	if (tty == NULL) {
		return -ENODEV;
	}
	l = (file->f_flags & O_NONBLOCK) ? 0 : 1;
	while (i < len) {
		ch = peekb(current->t_regs.ds, data + i);
		if (chq_addch(&tty->inq, ch, l) == -1) {
			if (l) {
				return -EINTR;
			} else {
				break;
			}
		}
		i++;
		printk(" wc");
	}
	printk("\n");
	return i;
}

static struct file_operations pty_fops =
{
	pipe_lseek,	/* Same behavoir, return -ESPIPE */
	pty_read,
	pty_write,
	NULL,
	pty_select,	/* Select - needs doing */
	pty_ioctl,		/* ioctl */
	pty_open,
	pty_release,
#ifdef BLOAT_FS
	NULL,
	NULL,
	NULL
#endif
};

int ttyp_write(tty)
struct tty * tty;
{
	if (tty->outq.len == tty->outq.size) {
		interruptible_sleep_on(&tty->outq.wq);
	}
}

struct tty_ops ttyp_ops = {
	ttynull_openrelease,	/* None of these really need to do anything */
	ttynull_openrelease,
	ttyp_write,
	NULL,
	NULL,
};

void pty_init()
{
	register_chrdev(PTY_MASTER_MAJOR, "pty", &pty_fops);
}
