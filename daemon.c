#include <stdio.h>
#include "daemon.h"

/**
1. compile as dll: exit if netlink could not establish;
2. support register by a) PID or b) keyword in process command;
3. "remove" message may arrive before "add":
    a) "remove" counts -1;
    b) records remove once when "counter==0"
References:
    1. https://github.com/facebook/gnlpy
    2. https://github.com/est/pymnl/blob/master/pymnl/nlsocket.py 
**/

// static const unsigned int SOL_NETLINK = 270;

int main(int argc, char const *argv[])
{
    printf("Hello, World!\n");
    return 0;
}
