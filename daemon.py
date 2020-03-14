#!/usr/bin/env python3
import os, time, struct
import socket
from sys import argv
from gnlpy import netlin

KERNEL_NAME='inotify_hook'
req_attr = netlink.create_attr_list_type('req',
                ('PID', netlink.U32Type)
            )
res_attr = netlink.create_attr_list_type('res',
                ('var_name', netlink.NulStringType)
            )
msg_format = netlink.create_genl_message_type('Msg', KERNEL_NAME
                ('request', req_attr),
                ('response', res_attr)
            )

def main(pid=argv[1]):
    sockfd = socket.socket(socket.AF_NETLINK, socket.SOCK_RAW, socket.NETLINK_USERSOCK)
    sockfd.bind((os.getpid(), 0))
    sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
    sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)

    sock.send(
        Msg('request', attr_list=req_attr(int(pid)))
    )
    reply = sock.recv()[0]
    reply.get_attr_list().get('response')  # the address string
    pass

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        raise e
    finally:
        pass #exit()