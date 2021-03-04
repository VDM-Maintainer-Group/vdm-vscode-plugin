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
int onSave(const char *stat_file)
{
    int pos = 0;
    char **result;
    FILE *fd;

    result = inotify_lookup_dump("code");

    if ( (fd=fopen(stat_file, "w"))==NULL )
    {
        return -1;
    }

    while (pos<MAX_DUMP_LEN && result[pos])
    {
        fprintf(fd, "%s\n", result[pos]);
        //
        printf("dump [%d]: %s\n", pos, result[pos]);
        pos ++;
    }
    free(result);
    
    return 0;
}

int onResume(const char *stat_file)
{
    FILE *fd;
    char buf[1024];
    char command[255];
    
    if ( (fd=fopen(stat_file, "r"))==NULL )
    {
        return -1;
    }

    while ( fscanf(fd, "%s", buf)==1 )
    {
        sprintf(command, "code %s", buf);
        system(command);
    }

    return 0;
}

int onClose()
{
    system("killall code");
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

    ret = onStart();
    printf("add with ret code: %d.\n", ret);

    ret = onSave("/tmp/vscode_dump.txt");
    printf("save with ret code: %d.\n", ret);

    // ret = onStop();
    // printf("rm with ret code: %d.\n", ret);

    return ret;
}
