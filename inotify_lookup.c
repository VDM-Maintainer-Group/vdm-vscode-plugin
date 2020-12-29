#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "inotify_lookup.h"

/**
References:
    1. https://github.com/est/pymnl/blob/master/pymnl/nlsocket.py
    2. https://stackoverflow.com/questions/3299386/how-to-use-netlink-socket-to-communicate-with-a-kernel-module
    3. https://stackoverflow.com/questions/62526613/how-to-send-multipart-messages-using-libnl-and-generic-netlink
**/

/* global variables declaration */
int sock_fd = 0;
struct sockaddr_nl src_addr, dest_addr;
struct iovec iov;
struct msghdr msg;

static int init_socket(void)
{
    int msg_size = sizeof(struct req_msg_t);
    struct nlmsghdr *nlh = NULL;

    // reuse existing fd
    if (sock_fd > 0)
    {
        goto out;
    }
    // create socket fd
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
    {
        goto out;
    }
    // bind to current pid
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    // init dest_addr
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; //pid==0, to kernel
    dest_addr.nl_groups = 0; //unicast
    // init nlmsg struct for "nlh"
    nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(msg_size));
    memset(nlh, 0, NLMSG_SPACE(msg_size));
    nlh->nlmsg_len = NLMSG_SPACE(msg_size);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    // init nlmsg struct for "msghdr"
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

out:
    return sock_fd;
}

static int send_request(struct req_msg_t *p_req_msg)
{
    int ret;
    memcpy(NLMSG_DATA(iov.iov_base), p_req_msg, sizeof(struct req_msg_t));
    ret = sendmsg(sock_fd, &msg, 0);
    return ret;
}

int inotify_lookup_register(const char *name)
{
    int ret = 0;
    struct req_msg_t req_msg = {
        .op        = INOTIFY_REQ_ADD
    };
    strcpy(req_msg.comm_name, name);

    if ((ret=init_socket()) < 0)
    {
        goto out;
    }

    if((ret=send_request(&req_msg)) < 0)
    {
        goto out;
    }

out:
    return ret;
}

int inotify_lookup_unregister(const char *name)
{
    int ret = 0;
    struct req_msg_t req_msg = {
        .op        = INOTIFY_REQ_RM
    };
    strcpy(req_msg.comm_name, name);

    if ((ret=init_socket()) < 0)
    {
        goto out;
    }

    if((ret=send_request(&req_msg)) < 0)
    {
        goto out;
    }

out:
    return ret;
}

char** inotify_lookup_dump(const char *name)
{
    int ret = 0;
    char **result = NULL;
    struct nlmsghdr *nlh = NULL;
    struct req_msg_t req_msg = {
        .op        = INOTIFY_REQ_DUMP
    };
    strcpy(req_msg.comm_name, name);

    if ((ret=init_socket()) < 0)
    {
        goto out;
    }

    if((ret=send_request(&req_msg)) < 0)
    {
        goto out;
    }

    //TODO:

out:
    return result;
}
