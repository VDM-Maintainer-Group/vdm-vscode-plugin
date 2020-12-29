#include <stdio.h>
#include "inotify_lookup.h"
// #include <vdm/capability/inotify_lookup.h>

int main(int argc, char const *argv[])
{
    int ret;

    ret = inotify_lookup_register("code");
    printf("add with ret code: %d.\n", ret);

    ret = inotify_lookup_unregister("code");
    printf("rm with ret code: %d.\n", ret);

    return 0;
}
