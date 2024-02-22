#ifndef FRYZ_H
#define FRYZ_H

#include <raylib.h>

#include <stdlib.h>
#include <stdbool.h>

struct fryz
{
    enum mode
    {
        GRAPH_MODE,
    }
    mode;

    struct audio
    {
        Music music;
        bool paused;
    }
    audio;
};

//                identifier       return type,   arguments type(s)
#define FRYZ_LIST                                                       \
    FRYZ_FUNCTION(libfryz_init,    int,           int, char**)         \
    FRYZ_FUNCTION(libfryz_save,    struct fryz *, void)                \
    FRYZ_FUNCTION(libfryz_update,  int,           void)                \
    FRYZ_FUNCTION(libfryz_restore, void,          struct fryz *)       \
    FRYZ_FUNCTION(libfryz_close,   void,          void)

#define FRYZ_TYPEDEF(name, ret, ...) typedef ret (name##_t)(__VA_ARGS__);
#define FRYZ_DECLARE_FUNCTION(name, ret, ...) ret name(__VA_ARGS__);
#define FRYZ_DECLARE(name, ...) name##_t *name = NULL;
#define FRYZ_RELOAD(name, ...)                                          \
    *(void**)&name = dlsym(libfryz, #name);                             \
    if (name == NULL)                                                   \
    {                                                                   \
        TraceLog(LOG_ERROR, "Failed to get addres of " #name ": %s", dlerror()); \
        return 1;                                                       \
    }

#ifdef FRYZ_HOT_RELOAD
    // Declare all the function types
    #define FRYZ_FUNCTION(name, ret, ...) FRYZ_TYPEDEF(name, ret, __VA_ARGS__)
#else
    // Declare the functions regularly
    #define FRYZ_FUNCTION(name, ret, ...) FRYZ_DECLARE_FUNCTION(name, ret, __VA_ARGS__)
#endif // FRYZ_HOT_RELOAD

FRYZ_LIST
#undef  FRYZ_FUNCTION

#endif // FRYZ_H
