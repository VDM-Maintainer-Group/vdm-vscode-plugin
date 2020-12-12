#ifndef __NETLINK_COMM_H__
#define __NETLINK_COMM_H__

#include <net/sock.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>

#define NETLINK_USER 31

struct sock *nl_sock;

int netlink_comm_init(void);
void netlink_comm_exit(void);

#endif