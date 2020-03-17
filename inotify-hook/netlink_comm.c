/* Netlink Socket Communication Utility
* References:
* 1. https://stackoverflow.com/questions/3299386/how-to-use-netlink-socket-to-communicate-with-a-kernel-module
*/
#include "netlink_comm.h"
#include "main.h"

static void nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;

    //TODO: response to request
    
}

int netlink_comm_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sock = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sock) {
        printh("Netlink socket creation fails.\n");
        return -1;
    }

    return 0;
}

void netlink_comm_exit(void)
{
    netlink_kernel_release(nl_sock);
}