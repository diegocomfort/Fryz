#include "../include/fryz.h"

extern struct fryz *fryz;

struct fryz *libfryz_save(void)
{
    return fryz;
}

void libfryz_restore(struct fryz *state)
{
    fryz = state;
}
