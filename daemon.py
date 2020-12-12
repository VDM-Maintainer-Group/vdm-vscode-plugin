#!/usr/bin/env python3
'''
1. change to C-version daemon
2. register by a) PID or b) keyword in process command
3. "remove" message may arrive before "add":
    a) "remove" counts -1;
    b) records remove once when "counter==0"
'''

''' Userspace Inotify Lookup Daemon Program
References:
    1. https://github.com/facebook/gnlpy
    2. https://github.com/est/pymnl/blob/master/pymnl/nlsocket.py 
'''
import os, time, struct
import socket
from sys import argv
from gnlpy import netlink

SOL_NETLINK = 270
NETLINK_ADD_MEMBERSHIP  = 1
NETLINK_DROP_MEMBERSHIP = 2
NETLINK_PKTINFO         = 3
NETLINK_BROADCAST_ERROR = 4
NETLINK_NO_ENOBUFS      = 5

def main(pid=argv[1]):
    sockfd = socket.socket(socket.AF_NETLINK, socket.SOCK_RAW, socket.NETLINK_USERSOCK)
    sockfd.bind((os.getpid(), 0))
    # 270 is SOL_NETLINK and 1 is NETLINK_ADD_MEMBERSHIP
    sock.setsockopt(270, NETLINK_ADD_MEMBERSHIP, 31)
    sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
    sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)

    #FIXME: send PID, get string split by ";"
    # sock.send(
    #     Msg('request', attr_list=req_attr(int(pid)))
    # )
    # reply = sock.recv()[0]
    # reply.get_attr_list().get('response')  # the address string
    pass

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        raise e
    finally:
        pass #exit()