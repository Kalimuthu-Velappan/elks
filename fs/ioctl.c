/*
 *  linux/fs/ioctl.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linuxmt/types.h>
#include <linuxmt/ioctl.h>
#include <arch/segment.h>
#include <linuxmt/sched.h>
#include <linuxmt/mm.h>
#include <linuxmt/errno.h>
#include <linuxmt/string.h>
#include <linuxmt/stat.h>
#include <linuxmt/termios.h>
#include <linuxmt/fcntl.h>	/* for f_flags values */

static int file_ioctl(register struct file *filp, unsigned int cmd,
		      unsigned int arg)
{
    loff_t val;

    register struct file_operations *fop = filp->f_op;

    if (cmd == FIONREAD) {
/*      switch (cmd) { */
/*      case FIONREAD: */
	val = filp->f_inode->i_size - filp->f_pos;
	return verified_memcpy_tofs((char *) arg, (char *) &val,
				    sizeof(loff_t));
    }
    if (fop && fop->ioctl)
	return fop->ioctl(filp->f_inode, filp, cmd, arg);
    return -EINVAL;
}

int sys_ioctl(int fd, unsigned int cmd, unsigned int arg)
{
    register struct file *filp;
    register struct file_operations *fop;
    int on;

    if (fd >= NR_OPEN || !(filp = current->files.fd[fd]))
	return -EBADF;
    fop = filp->f_op;
    filp->f_inode = filp->f_inode;
    switch (cmd) {
    case FIOCLEX:
	FD_SET(fd, &current->files.close_on_exec);
	return 0;

    case FIONCLEX:
	FD_CLR(fd, &current->files.close_on_exec);
	return 0;

    case FIONBIO:
	memcpy_fromfs(&on, (unsigned int *) arg, 2);
#if 0
	on = get_fs_word((unsigned int *) arg);
#endif
	filp->f_flags = (on)
	    ? filp->f_flags | O_NONBLOCK
	    : filp->f_flags & ~O_NONBLOCK;
	return 0;

    default:
	if (filp->f_inode && S_ISREG(filp->f_inode->i_mode))
	    return file_ioctl(filp, cmd, arg);

	if (fop && fop->ioctl)
	    return fop->ioctl(filp->f_inode, filp, cmd, arg);

	return -EINVAL;
    }
}
