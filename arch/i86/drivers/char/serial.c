#include <linuxmt/types.h>
#include <linuxmt/wait.h>
#include <linuxmt/chqueue.h>
#include <linuxmt/config.h>
#include <linuxmt/sched.h>
#include <linuxmt/errno.h>
#include <linuxmt/mm.h>
#include <linuxmt/serial_reg.h>
#include <linuxmt/ntty.h>
#include <linuxmt/debug.h>
#include <linuxmt/termios.h>

#include <arch/io.h>

extern struct tty ttys[];

struct serial_info {
    unsigned short io;
    unsigned char irq;
    unsigned char flags;
    unsigned char lcr;
    unsigned char mcr;
    struct tty *tty;

#define SERF_TYPE	15
#define SERF_EXIST	16
#define SERF_INUSE	32
#define ST_8250		0
#define ST_16450	1
#define ST_16550	2
#define ST_UNKNOWN	15

};

#define RS_MINOR_OFFSET 64

#define CONSOLE_PORT 0

/* all boxes should be able to do 9600 at least,
 * afaik 8250 works fine up to 19200
 */

#define DEFAULT_BAUD_RATE	9600
#define DEFAULT_LCR		UART_LCR_WLEN8

#define DEFAULT_MCR		\
	(unsigned char) (UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2)

#define MAX_RX_BUFFER_SIZE 16

static struct serial_info ports[4] = {
    {0x3f8, 4, 0, DEFAULT_LCR, DEFAULT_MCR, NULL},
    {0x2f8, 3, 0, DEFAULT_LCR, DEFAULT_MCR, NULL},
    {0x3e8, 5, 0, DEFAULT_LCR, DEFAULT_MCR, NULL},
    {0x2e8, 2, 0, DEFAULT_LCR, DEFAULT_MCR, NULL},
};

static unsigned int divisors[16] = {
    0,				/*  0 = B0      */
    2304,			/*  1 = B50     */
    1536,			/*  2 = B75     */
    1047,			/*  3 = B110    */
    860,			/*  4 = B134    */
    768,			/*  5 = B150    */
    576,			/*  6 = B200    */
    384,			/*  7 = B300    */
    192,			/*  8 = B600    */
    96,				/*  9 = B1200   */
    64,				/* 10 = B1800   */
    48,				/* 11 = B2400   */
    24,				/* 12 = B4800   */
    12,				/* 13 = B9600   */
    6,				/* 14 = B19200  */
    3				/* 15 = B38400  */
};

/* Flow control buffer markers */
#define	RS_IALLMOSTFULL 	(3 * INQ_SIZE / 4)
#define	RS_IALLMOSTEMPTY	(    INQ_SIZE / 4)

#ifdef CONFIG_CONSOLE_SERIAL

void console_setdefault(register struct serial_info *port)
{
    register struct tty *tty = port->tty;

    memcpy((void *) &(tty->termios), &def_vals, sizeof(struct termios));
}

#endif

void update_port(register struct serial_info *port)
{
    tcflag_t cflags;
    unsigned divisor;

    /* set baud rate divisor, first lower, then higher byte */
    cflags = port->tty->termios.c_cflag & CBAUD;
    divisor = divisors[cflags & 017];

    /* Support for this additional bauds */
    if (cflags == B57600)
	divisor = 2;
    if (cflags == B115200)
	divisor = 1;

    clr_irq();

    /* Set the divisor latch bit */
    outb_p(port->lcr | UART_LCR_DLAB, (void *) (port->io + UART_LCR));

    /* Set the divisor low and high byte */
    outb_p(divisor & 0xff, (void *) (port->io + UART_DLL));
    outb_p((divisor >> 8) & 0xff, port->io + UART_DLM);

    /* Clear the divisor latch bit */
    outb_p(port->lcr, port->io + UART_LCR);

    set_irq();
}

static char irq_port[4] = { 3, 1, 0, 2 };

void rs_release(struct tty *tty)
{
    register struct serial_info *port = &ports[tty->minor - RS_MINOR_OFFSET];

    debug("SERIAL: rs_release called\n");
    port->flags &= ~SERF_INUSE;
    outb_p(0, port->io + UART_IER);
}

static int get_serial_info(struct serial_info *info,
			   struct serial_info *ret_info)
{
    return verified_memcpy_tofs(ret_info, info, sizeof(struct serial_info));
}

int rs_open(struct tty *tty)
{
    register struct serial_info *port = &ports[tty->minor - RS_MINOR_OFFSET];
    register char *countp;

    debug("SERIAL: rs_open called\n");

    if (!(port->flags & SERF_EXIST))
	return -ENODEV;

    /* is port already in use ? */
    if (port->flags & SERF_INUSE)
	return -EBUSY;

    /* no, mark it in use */
    port->flags |= SERF_INUSE;

    /* clear RX buffer */
    (void) inb_p(port->io + UART_LSR);

    countp = (int) MAX_RX_BUFFER_SIZE;
    do
	(void) inb_p((void *) (port->io + UART_RX));
    while (--countp && (inb_p((void *) (port->io + UART_LSR)) & UART_LSR_DR));

    (void) inb_p((void *) (port->io + UART_IIR));
    (void) inb_p((void *) (port->io + UART_MSR));

    /* set serial port parameters to match ports[rs_minor] */
    update_port(port);

    /* enable reciever data interrupt; FIXME: update code to utilize full interrupt interface */
    outb_p(UART_IER_RDI, port->io + UART_IER);

    outb_p(port->mcr, port->io + UART_MCR);

    /* clear Line/Modem Status, Intr ID and RX register */
    (void) inb_p((void *) (port->io + UART_LSR));
    (void) inb_p((void *) (port->io + UART_RX));
    (void) inb_p((void *) (port->io + UART_IIR));
    (void) inb_p((void *) (port->io + UART_MSR));

    return 0;
}

static int set_serial_info(struct serial_info *info,
			   struct serial_info *new_info)
{
    register struct inode *inode;
    register char *errp;

    errp = (char *) verified_memcpy_fromfs(info, new_info,
					   sizeof(struct serial_info));
    if (!errp) {
	/* shutdown serial port and restart UART with new settings */

	/* witty cheat :) - either we do this (and waste some DS) or duplicate
	 * almost whole rs_release and rs_open (and waste much more CS)
	 */
	inode->i_rdev = info->tty->minor;
	rs_release(inode, NULL);
	errp = (char *) rs_open(inode, NULL);
    }
    return (int) errp;
}

int rs_write(register struct tty *tty)
{
    register struct serial_info *port = &ports[tty->minor - RS_MINOR_OFFSET];
    unsigned char ch;

    while (chq_getch(&tty->outq, &ch, 0) != -1) {
	while (!(inb_p(port->io + UART_LSR) & UART_LSR_TEMT))
	    /* Do nothing */ ;
	outb(ch, (void *) (port->io + UART_TX));
    }
}

int rs_ioctl(struct tty *tty, int cmd, char *arg)
{
    register struct serial_info *port = &ports[tty->minor - RS_MINOR_OFFSET];
    register char *retvalp = 0;

    /* few sanity checks should be here */
#if 0
    printk("rs_ioctl: sp = %d, cmd = %d\n", tty->minor - RS_MINOR_OFFSET, cmd);
#endif
    switch (cmd) {
	/* Unlike Linux we use verified_memcpy*fs() which calls verify_area() for us */
    case TCSETS:
    case TCSETSW:
    case TCSETSF:		/* For information, return value is ignored */
	update_port(port);
	break;
    case TIOCSSERIAL:
	retvalp = (char *) set_serial_info(port, arg);
	break;

    case TIOCGSERIAL:
	retvalp = (char *) get_serial_info(port, arg);
	break;
    }

    return (int) retvalp;
}

void receive_chars(register struct serial_info *sp)
{
    register struct ch_queue *q;
    int size;
    unsigned char ch;

    q = &sp->tty->inq;
    size = q->size - 1;
    do {
	ch = inb_p((void *) (sp->io + UART_RX));
	if (!tty_intcheck(sp->tty, ch)) {
	    if (q->len == size)
		break;
	    q->buf[(q->tail + q->len) & size] = (char) ch;
	    q->len++;
	}
    } while (inb_p(sp->io + UART_LSR) & UART_LSR_DR);
    wake_up(&q->wq);
}

int rs_irq(int irq, struct pt_regs *regs, void *dev_id)
{
    register struct serial_info *sp;
    register char *statusp;


    debug1("SERIAL: Interrupt %d recieved.\n", irq);
    sp = &ports[irq_port[irq - 2]];
    do {
	statusp = (int) inb_p(sp->io + UART_LSR);
	if (((int) statusp) & UART_LSR_DR)
	    receive_chars(sp);
#if 0
	if (((int) statusp) & UART_LSR_THRE)
	    transmit_chars(sp);
#endif
    } while (!(inb_p(sp->io + UART_IIR) & UART_IIR_NO_INT));
}

int rs_probe(register struct serial_info *sp)
{
    int status1, status2;
    unsigned char scratch;

    scratch = inb(sp->io + UART_IER);
    outb_p(0, sp->io + UART_IER);
    scratch = inb_p(sp->io + UART_IER);
    outb_p(scratch, sp->io + UART_IER);
    if (scratch)
	return -1;

    /* this code is weird, IMO */
    scratch = inb_p(sp->io + UART_LCR);
    outb_p(scratch | UART_LCR_DLAB, sp->io + UART_LCR);
    outb_p(0, sp->io + UART_EFR);
    outb_p(scratch, sp->io + UART_LCR);

    outb_p(UART_FCR_ENABLE_FIFO, sp->io + UART_FCR);

    /* upper two bits of IIR define UART type, but according to both RB's
     * intlist and HelpPC this code is wrong, see comments marked with [*]
     */
    scratch = inb_p(sp->io + UART_IIR) >> 6;
    switch (scratch) {
    case 0:
	sp->flags = (unsigned char) (SERF_EXIST | ST_16450);
	break;
    case 1:			/* [*] this denotes broken 16550 UART, 
				 * not 16550A or any newer type */
	sp->flags = (unsigned char) (ST_UNKNOWN);
	break;
    case 2:			/* invalid combination */
    case 3:			/* Could be a 16650.. we dont care */
	/* [*] 16550A or newer with enabled FIFO buffers
	 */
	sp->flags = (unsigned char) (SERF_EXIST | ST_16550);
	break;
    }

    /* 8250 UART if scratch register isn't present */
    if (!scratch) {
	scratch = inb_p(sp->io + UART_SCR);
	outb_p(0xA5, sp->io + UART_SCR);
	status1 = inb_p(sp->io + UART_SCR);
	outb_p(0x5A, sp->io + UART_SCR);
	status2 = inb_p(sp->io + UART_SCR);
	outb_p(scratch, sp->io + UART_SCR);
	if ((status1 != 0xA5) || (status2 != 0x5A))
	    sp->flags = (unsigned char) (SERF_EXIST | ST_8250);
    }

    /*
     *      Reset the chip
     */

    outb_p(0x00, sp->io + UART_MCR);

    /* clear RX and TX FIFOs */
    outb_p((unsigned char) (UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT),
			sp->io + UART_FCR);

    /* clear RX register */
    {
	register char *countp = (char *) MAX_RX_BUFFER_SIZE;
	do {
	    (void) inb_p((void *) (sp->io + UART_RX));
	} while (--countp && (inb_p(sp->io + UART_LSR) & UART_LSR_DR));
    }

    return 0;
}

static void print_serial_type(int st)
{
    switch (st) {
    case ST_8250:
	printk(" is an 8250");
	break;
    case ST_16450:
	printk(" is a 16450");
	break;
    case ST_16550:
	printk(" is a 16550");
/*  	break; */
    }
}

int rs_init(void)
{
    register struct serial_info *sp = ports;
    register char *pi;
    int ttyno = 4;

    printk("Serial driver version 0.02\n");
    pi = (char *) 4;
    do {
	--pi;

	if (sp->tty != NULL) {
	    /*
	     * if rs_init is called twice, because of serial console
	     */
	    printk("ttyS%d at 0x%x (irq = %d)", ttyno - 4, sp->io, sp->irq);
	    print_serial_type(sp->flags & SERF_TYPE);
	    printk(", fetched\n");
	    ttyno++;
	} else {
	    if ((rs_probe(sp) == 0) && (!request_irq(sp->irq, rs_irq, NULL))) {
		printk("ttys%d at 0x%x (irq = %d)",
		       ttyno - 4, sp->io, sp->irq);
		print_serial_type(sp->flags & SERF_TYPE);
		printk("\n");
		sp->tty = &ttys[ttyno++];
		update_port(sp);
#if 0
		outb_p(? ? ? ?, sp->io + UART_MCR);
#endif
	    }
	}
	sp++;
    } while (pi);
    return 0;
}

#ifdef CONFIG_CONSOLE_SERIAL

static int con_init = 0;

void init_console(void)
{
    rs_init();
    console_setdefault(&ports[CONSOLE_PORT]);
    update_port(&ports[CONSOLE_PORT]);
    con_init = 1;
    printk("Console: Serial\n");
}

void con_charout(char Ch)
{
    if (con_init) {
	while (!(inb_p(ports[CONSOLE_PORT].io + UART_LSR) & UART_LSR_TEMT));
	outb(Ch, ports[CONSOLE_PORT].io + UART_TX);
    }
}

int wait_for_keypress(void)
{
    /* Do something */
}

#endif

struct tty_ops rs_ops = {
    rs_open,
    rs_release,
    rs_write,
    NULL,
    rs_ioctl
};

