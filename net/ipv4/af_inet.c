/*
 * net/ipv4/af_inet.c
 *
 * (C) 2001 Harry Kalogiroy (harkal@rainbow.cs.unipi.gr)
 *
 * The kernel side part of the ELKS TCP/IP stack. It uses tcpdev.c
 * to communicate with the actual TCP/IP stack that resides in
 * user space (ktcpd)..
 */

#include <linuxmt/errno.h>
#include <linuxmt/config.h>
#include <linuxmt/socket.h>
#include <linuxmt/fs.h>
#include <linuxmt/mm.h>
#include <linuxmt/stat.h>
#include <linuxmt/debug.h>
#include <linuxmt/fcntl.h>
#include <linuxmt/net.h>
#include <linuxmt/tcpdev.h>
#include "af_inet.h"


#ifdef CONFIG_INET

extern char td_buf[];

static char tcpdev_readlock = 0;
static int tcpdev_availdata = 0;

extern int tcpdev_inetwrite();

int inet_process_tcpdev(buf, len)
char *buf;
int len;
{
	struct tdb_return_data *r;
	struct socket *sock;
	int i;
	
	r = buf;
	
/*	printk("tcpdev : got %d bytes\n",len);
	for(i = 0 ; i < len ; i++){
		printk("%x ",buf[i]);
	}
	printk("\n");
*/		

	sock = (struct socket *)r->sock;
	
	switch (r->type) {
		case TDT_CHG_STATE:
			sock->state = r->ret_value;
			break;
		case TDT_AVAIL_DATA:
			sock->avail_data = r->ret_value;
		default: 	
			wake_up(sock->wait);
	}
	
	return 1;
}

static int inet_create(sock, protocol)
register struct socket *sock;
int protocol;
{
    register struct inet_proto_data *upd;
        
	printd_inet("inet_create()\n");
	if(protocol != 0)
		return -EINVAL;

    return 0;
}


static int inet_dup(newsock, oldsock)
struct socket *newsock;
struct socket *oldsock;
{
	printd_inet("inet_dup()\n");	
    return(inet_create(newsock, 0));
}

static int inet_release(sock, peer)
struct socket *sock;
struct socket *peer;
{
	struct tdb_release cmd;
	struct tdb_return_data	*ret_data;
	int ret;

	printd_inet("inet_release()\n");	
	cmd.cmd  = TDC_RELEASE;
	cmd.sock = sock;

	ret = tcpdev_inetwrite(&cmd, sizeof(struct tdb_release));
	if(ret < 0)return ret;
	
	/* Sleep until tcpdev has news */
	interruptible_sleep_on(sock->wait);
	
	ret_data = td_buf;
	ret = ret_data->ret_value;
	tcpdev_clear_data_avail();

	return ret;
}

static int inet_bind(sock, addr, sockaddr_len)
struct socket *sock;
struct sockaddr *addr;
int sockaddr_len;
{
	struct inet_proto_data *upd = sock->data;
	struct tdb_bind cmd;
	unsigned char *tdret;
	int	len;
	int ret;
	struct tdb_return_data	*ret_data;

	printd_inet("inet_bind()\n");
	if(sockaddr_len <= 0 || sockaddr_len > sizeof(struct sockaddr_in))
		return -EINVAL;
		
	/* TODO : Check if the user has permision to bind the port */
		
	cmd.cmd  = TDC_BIND;
	cmd.sock = sock;
	memcpy(&cmd.addr, addr, sockaddr_len);
	
	ret = tcpdev_inetwrite(&cmd, sizeof(struct tdb_bind));
	if(ret < 0)return ret;
		
	/* Sleep until tcpdev has news */
	interruptible_sleep_on(sock->wait);
	
	ret_data = td_buf;
	ret = ret_data->ret_value;
	tcpdev_clear_data_avail();
	if(ret < 0)
		return ret;

	return 0;
}

static int inet_connect(sock, uservaddr, sockaddr_len, flags)
struct socket *sock;
struct sockaddr *uservaddr;
int sockaddr_len;
int flags;
{
	struct sockaddr_in	*sockin;
	struct tdb_return_data *r;
	struct tdb_connect cmd;
	int ret;

	printd_inet("inet_connect()\n");	
	if (sockaddr_len <= 0 || sockaddr_len > sizeof(struct sockaddr_in)) {
		return(-EINVAL);
	}
	sockin = uservaddr;
	if (sockin->sin_family != AF_INET)
		return(-EINVAL);
	
	if (sock->state == SS_CONNECTING)
		return(-EINPROGRESS);
	if (sock->state == SS_CONNECTED)
		return(-EISCONN);

	cmd.cmd = TDC_CONNECT;
	cmd.sock = sock;
	memcpy(&cmd.addr, uservaddr, sockaddr_len);
	
	tcpdev_inetwrite(&cmd, sizeof(struct tdb_connect));
	
	/* Sleep until tcpdev has news */
	interruptible_sleep_on(sock->wait);
	
	r = td_buf;
	ret = r->ret_value;
	tcpdev_clear_data_avail();
	
	if(ret < 0){
		return ret;
	}
	
	sock->state = SS_CONNECTED;
	return 0;
}

inet_socketpair()
{
	printk("inet_sockpair\n");
}

static int inet_accept(sock, newsock, flags)
struct socket *sock;
struct socket *newsock;
int flags;
{
	struct tdb_accept cmd;
	struct tdb_accept_ret *ret_data;
	int ret;

	printd_inet("inet_accept()\n");	
	cmd.cmd  = TDC_ACCEPT;
	cmd.sock = sock;
	cmd.newsock = newsock;
	cmd.nonblock = flags & O_NONBLOCK;
	sock->flags |= SO_WAITDATA;
	ret = tcpdev_inetwrite(&cmd, sizeof(struct tdb_accept));
	if(ret < 0)return ret;
		
	/* Sleep until tcpdev has news */
	interruptible_sleep_on(sock->wait);
	
	sock->flags &= ~SO_WAITDATA;

	ret_data = td_buf;
	ret = ret_data->ret_value;
	tcpdev_clear_data_avail();
	if(ret < 0)
		return ret;

	newsock->state = SS_CONNECTED;
	
	return 0;
}

inet_getname()
{
	printk("inet_getname\n");
}

static int inet_read(sock, ubuf, size, nonblock)
struct socket *sock;
char *ubuf;
int size;
int nonblock;
{
	struct inet_proto_data *upd;
	struct tdb_return_data *r;
	struct tdb_read	cmd;
	int ret;

	printd_inet("inet_read()\n");	
	cmd.cmd	= TDC_READ;
	cmd.sock = sock;
	cmd.size = size;
	cmd.nonblock = nonblock;
	
	tcpdev_inetwrite(&cmd, sizeof(struct tdb_read));
	
	/* Sleep until tcpdev has news */
	interruptible_sleep_on(sock->wait);

	r = td_buf;
	ret = r->ret_value;

	if(ret > 0){
		memcpy_tofs(ubuf, &r->data, ret);
		sock->avail_data = 0;
	}
	tcpdev_clear_data_avail();
	
	return ret;
	
}

static int inet_write(sock, ubuf, size, nonblock)
struct socket *sock;
char *ubuf;
int size;
int nonblock;
{
	struct inet_proto_data *upd;
	struct tdb_return_data *r;
	struct tdb_write cmd;
	int ret,todo;

	printd_inet("inet_write()\n");	
	if(size <= 0)
		return 0;
		
	if(sock->state != SS_CONNECTED){
		return -EINVAL;
	}
	
	cmd.cmd	= TDC_WRITE;
	cmd.sock = sock;
	cmd.size = size;
	cmd.nonblock = nonblock;

	todo = size;
	while(todo){
		cmd.size = todo > TDB_WRITE_MAX ? TDB_WRITE_MAX : todo;
		
		memcpy_fromfs(cmd.data, ubuf + size - todo, cmd.size);
		tcpdev_inetwrite(&cmd, sizeof(struct tdb_write));
		todo -= cmd.size;
	
		/* Sleep until tcpdev has news */
		interruptible_sleep_on(sock->wait);

		r = td_buf;
		ret = r->ret_value;
		tcpdev_clear_data_avail();
		if(ret < 0){
			if(ret == -ERESTARTSYS){
				schedule();
				todo += cmd.size;
			} else {
				return ret;
			}
		}
	}
	
	return size;
}


static int inet_select(sock, sel_type, wait)
struct socket *sock;
int sel_type;
select_table * wait;
{
	int ret;
	
	printd_inet("inet_select\n");
	if (sel_type == SEL_IN) {
		if (sock->avail_data) {
			return(1);
		} else if (sock->state != SS_CONNECTED) {
			return(1);
		}
		select_wait(sock->wait);
		return(0);
	} else 
	if (sel_type == SEL_OUT){
		return(1);
	}
}

inet_ioctl()
{
	printk("inet_ioctl\n");
}

static int inet_listen(sock, backlog)
struct socket *sock;
int backlog;
{	
	struct tdb_return_data *ret_data;
	struct tdb_listen cmd;
	int ret;

	printd_inet("inet_listen()\n");		
	cmd.cmd	= TDC_LISTEN;
	cmd.sock = sock;
	cmd.backlog = backlog;
	
	tcpdev_inetwrite(&cmd, sizeof(struct tdb_listen));
	
	/* Sleep until tcpdev has news */
	interruptible_sleep_on(sock->wait);
	
	ret_data = td_buf;
	ret = ret_data->ret_value;
	tcpdev_clear_data_avail();
	
	return ret;

}

inet_shutdown()
{
printk("inet_shutdown\n");
}

inet_setsockopt()
{
printk("setsockopt\n");
}

inet_getsockopt()
{
printk("inet_getsockopt\n");
}

inet_fcntl()
{
printk("inet_fcntl\n");
}

inet_sendto()
{
printk("inet_sendto\n");
}

inet_recvfrom()
{
printk("inet_recvfrom\n");
}

static int inet_send(sock, buff, len, nonblock, flags)
struct socket *sock;
void *buff;
int len;
int nonblock;
unsigned int flags;
{
	if (flags != 0)
		return(-EINVAL);
	return(inet_write(sock, (char *) buff, len, nonblock));
}

static int inet_recv(sock, buff, len, nonblock, flags)
struct socket *sock;
void *buff;
int len;
int nonblock;
unsigned flags;
{
	if (flags != 0) {
		return(-EINVAL);
	}
	return(inet_read(sock, (char *) buff, len, nonblock));
}



static struct proto_ops inet_proto_ops = {
        AF_INET,

        inet_create,
        inet_dup,
        inet_release,
        inet_bind,
        inet_connect,
        inet_socketpair,
        inet_accept,
        inet_getname,
        inet_read,
        inet_write,
        inet_select,
        inet_ioctl,
        inet_listen,
		inet_send,
		inet_recv,
		inet_sendto,
		inet_recvfrom,

        inet_shutdown,

        inet_setsockopt,
        inet_getsockopt,
        inet_fcntl,
};








void inet_proto_init(pro)
struct net_proto * pro;

{

	printk("ELKS TCP/IP by Harry Kalogiroy\n");
	sock_register(inet_proto_ops.family, &inet_proto_ops); 

};

#endif
