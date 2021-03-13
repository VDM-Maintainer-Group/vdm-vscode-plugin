#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <vdm/interface/src_api.h>
#include <vdm/capability/inotify_lookup.h>
#include "third-party/sds-master/sds.c"

#define EXPORT __attribute__((visibility("default")))

int check(const char *name)
{
    const char *blacklist[3] = {".git", ".config", ".vscode"};
    for (int i=0; i<3; i++)
    {
        if (strstr(blacklist[i], name)) {return 0;}
    }
    return 1;
}

/* Custom Interface */
EXPORT int onTrigger(void *args)
{
    return 0;
}

/* SRC API Interface */
EXPORT int onSave(const char *stat_file)
{
    int i, count, length;
    char result[4096];
    FILE *fd;

    if ( (fd=fopen(stat_file, "w"))==NULL )
    {
        return -1;
    }

    length = inotify_lookup_fetch("code");

    for (i = 0; i < length; i++)
    {
        sds line;
        sds *tokens;

        inotify_lookup_get(i, result);
        // printf("dump [%d]: %s\n", i, result);
        line = sdsnew(result);
        tokens = sdssplitlen(line, sdslen(line), ",", 1, &count);
        if ( count==2 && check(tokens[1]) )
        {
            fprintf(fd, "%s\n", tokens[1]);
            printf("dump [%d]: %s\n", i, line);
        }
        sdsfreesplitres(tokens, count);
        // sdsfree(line);
    }
    inotify_lookup_free();
    
    fclose(fd);
    return 0;
}

EXPORT int onResume(const char *stat_file)
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

EXPORT int onClose()
{
    system("killall code");
    return 0;
}

/* Common Startup */
EXPORT int onStart()
{
    int ret;
    ret = inotify_lookup_register("code");
    return ret;
}

EXPORT int onStop()
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
