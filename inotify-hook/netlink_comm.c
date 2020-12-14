/* Netlink Socket Communication Utility
* References:
* 1. https://stackoverflow.com/questions/3299386/how-to-use-netlink-socket-to-communicate-with-a-kernel-module
*/
#include "netlink_comm.h"
#include "main.h"

struct sock *nl_sock;

static void nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int usr_pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *req_msg, *res_msg;
    void **slot;

    nlh = (struct nlmsghdr *)skb->data;
    usr_pid = nlh->nlmsg_pid;
    req_msg = (char *)nlmsg_data(nlh);

    skb_out = nlmsg_new(strlen(req_msg), 0);
    if (!skb_out) {
        printh("New skb allcation fails.\n");
        return;
    }

    //FIXME: send multiple packet of maximum length 4096
    //       (for loop)
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), res_msg, msg_size);

    if ( nlmsg_unicast(nl_sock, skb_out, usr_pid) < 0 )
        printh("Netlink response to %d error.\n", usr_pid);
}

int netlink_comm_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
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