
// /usr/local/lib/vdm/capability/libvdm-inotify-lookup.so

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main(int argc, char const *argv[])
{
    int ret;
    void *handle;
    int (*onStart)(void);
    int (*onSave)(const char *);

    handle = dlopen("./libvscode-plugin.so", RTLD_LAZY);
    if (!handle)
    {
        /* fail to load the library */
        fprintf(stderr, "Error: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    *(int**)(&onStart) = dlsym(handle, "onStart");
    if (!onStart)
    {
        fprintf(stderr, "Error: %s\n", dlerror());
        dlclose(handle);
        return EXIT_FAILURE;
    }
    *(int**)(&onSave) = dlsym(handle, "onSave");
    if (!onSave)
    {
        fprintf(stderr, "Error: %s\n", dlerror());
        dlclose(handle);
        return EXIT_FAILURE;
    }

    ret = onStart();
    printf("start %d\n", ret);
    ret = onSave("/tmp/vscode_dump.txt");
    printf("save %d\n", ret);

    dlclose(handle);

    return EXIT_SUCCESS;
}
