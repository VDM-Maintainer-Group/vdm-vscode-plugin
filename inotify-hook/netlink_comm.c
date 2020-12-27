/* Netlink Socket Communication Utility
* References:
* 1. https://stackoverflow.com/questions/3299386/how-to-use-netlink-socket-to-communicate-with-a-kernel-module
* 2. https://medium.com/@mdlayher/linux-netlink-and-go-part-1-netlink-4781aaeeaca8
* 3. https://elixir.bootlin.com/linux/latest/source/lib/idr.c#L195
*/
#include "netlink_comm.h"

struct sock *nl_sock;

static int append_message_cb(int pid, char *pathname, void *data)
{
    int ret = 0;
    char buf[PATH_MAX];
    struct nlmsghdr *nlh;
    struct msg_buf_t *msg_buf = data;

    sprintf(buf, "%d,%s\n", pid, pathname);
    //TODO: manipulate with msg_buf
    // nlh = nlmsg_put(msg_buf->skb, 0, seq, 0, msg_size, NLM_F_MULTI); 
    // nlh = nlmsg_put(msg_buf->buf, 0, seq, NLMSG_DONE, msg_size, 0);
    // strncpy(nlmsg_data(nlh), res_msg, msg_size);
    return ret;
}

static void nl_recv_msg(struct sk_buff *skb)
{
    int ret;
    struct nlmsghdr *nlh;
    //req msg
    int usr_pid;
    struct req_msg_t *req_msg;
    //res msg
    struct msg_buf_t msg_buf;

    // decode req_msg from sk_buff
    nlh = (struct nlmsghdr *)skb->data;
    usr_pid = nlh->nlmsg_pid;
    req_msg = (struct req_msg_t *)nlmsg_data(nlh);

    if (req_msg->op & INOTIFY_REQ_ADD)
    {
        ret = comm_list_add_by_name(req_msg->comm_name);
    }
    else if (req_msg->op & INOTIFY_REQ_RM)
    {
        comm_list_rm_by_name(req_msg->comm_name);
    }
    else if (req_msg->op & INOTIFY_REQ_DUMP)
    {
        // initial skb
        msg_buf.skb = nlmsg_new(NLMSG_DEFAULT_SIZE, 0);
        if (unlikely(!msg_buf.skb)) {
            printh("nl_recv_msg: skb allocation failed.\n");
            return;
        }
        NETLINK_CB(msg_buf.skb).dst_group = 0; /* not in mcast group */
        // dump comm record
        ret = comm_record_dump_by_name(req_msg->comm_name, append_message_cb, (void *) &msg_buf);
        // unicast the response
        if ( ret < 0 || nlmsg_unicast(nl_sock, msg_buf.skb, usr_pid) < 0 )
        {
            printh("nl_recv_msg: message response to %d failed.\n", usr_pid);
        }
    }
    else
    {
        printh("nl_recv_msg: bad request.\n");
    }

    return;
}

int netlink_comm_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
    };
    //register netlink socket
    nl_sock = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sock) {
        return -1;
    }
    return 0;
}

void netlink_comm_exit(void)
{
    netlink_kernel_release(nl_sock);
}