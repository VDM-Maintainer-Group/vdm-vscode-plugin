#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <vdm/interface/src_api.h>
#include <vdm/capability/inotify_lookup.h>

/* Custom Interface */
int onTrigger(void *args)
{
    return 0;
}

/* SRC API Interface */
int onSave(char *stat_file)
{
    int pos;
    char **result;

    result = inotify_lookup_dump("code");
    while (pos<MAX_DUMP_LEN && result[pos])
    {
        printf("dump [%d]: %s\n", pos, result[pos]);
        pos ++;
    }

    return 0;
}

int onResume(char *stat_file)
{
    return 0;
}

int onClose()
{
    return 0;
}

/* Common Startup */
int onStart()
{
    int ret;
    ret = inotify_lookup_register("code");
    return ret;
}

int onStop()
{
    int ret;
    ret = inotify_lookup_unregister("code");
    return ret;
}

int main(int argc, char const *argv[])
{
    int ret;

    ret = inotify_lookup_register("code");
    printf("add with ret code: %d.\n", ret);

    ret = inotify_lookup_unregister("code");
    printf("rm with ret code: %d.\n", ret);

    return ret;
}
