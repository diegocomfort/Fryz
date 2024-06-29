#include "include/fryz.h"

#include <raylib.h>

#include <stdlib.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(FRYZ_HOT_RELOAD)
// Pointer to the shared object created by dlopen()
static void *libfryz = NULL;
// Declare function pointers for fryz functions
#define FRYZ_FUNCTION(name, ...) FRYZ_DECLARE(name, __VA_ARGS__)
FRYZ_LIST
#undef  FRYZ_FUNCTION
// Declare reload_libfryz() as an actual function that will reload
// the shared object
int reload_libfryz(void);
#else
// Declare reload_libfryz() as a macro which always results in a
// success because fryz is actually linked in the build process,
// so there is no shared object to reload
#define reload_libfryz() 0
#endif // FRYZ_HOT_RELOAD


int main(int argc, char **argv)
{
	int error;

	error = reload_libfryz();
	if (error)
	{
		return 1;
	}

	error = libfryz_init(argc, argv);
	if (error)
	{
		return 2;
	}

	while(!WindowShouldClose())
	{
#if defined(FRYZ_HOT_RELOAD)
		if (IsKeyPressed(KEY_R))
		{
			struct fryz *state = libfryz_save();
			error = reload_libfryz();
			if (error)
			{
				libfryz_close();
				return 3;
			}
			libfryz_restore(state);
		}
#endif // FRYZ_HOT_RELOAD

		error = libfryz_update();
		if (error)
		{
			libfryz_close();
			return 4;
		}
	}

	libfryz_close();
	return 0;
}

#if defined(FRYZ_HOT_RELOAD)
int reload_libfryz(void)
{
	int error;

	if (libfryz != NULL)
	{
		error = dlclose(libfryz);
		if (error)
		{
			TraceLog(LOG_ERROR,
				 "Failed to to close libfryz: %s\n", dlerror());
			return 1;
		}
	}

	libfryz = dlopen(LIBFRYZ_PATH, RTLD_NOW);
	if (libfryz == NULL)
	{
		TraceLog(LOG_ERROR,
			 "Failed to open "LIBFRYZ_PATH": %s\n", dlerror());
		return 2;
	}

#define FRYZ_FUNCTION(name, ...) FRYZ_RELOAD(name, __VA_ARGS__)
	FRYZ_LIST
#undef  FRYZ_FUNCTION

		return 0;
}
#endif // FRYZ_HOT_RELOAD
