#include "bg.h"

#ifdef _WIN32

#include <process.h>
#include <stdint.h>
#include <stdlib.h>

struct bg_process {
    intptr_t handle;
};

int bg_start(const char *const argv[], bg_process **out) {
    if (!argv || !argv[0] || !out) return BG_BAD_ARGS;
    *out = NULL;

    bg_process *p = malloc(sizeof *p);
    if (!p) return BG_NO_MEMORY;

    intptr_t h = _spawnvp(_P_NOWAIT, argv[0], argv);
    if (h == -1) {
        free(p);
        return BG_START_FAILED;
    }

    p->handle = h;
    *out = p;
    return BG_OK;
}

int bg_wait(bg_process *p, int *exit_code) {
    if (!p || !exit_code || !p->handle) return BG_BAD_ARGS;

    if (_cwait(exit_code, p->handle, 0) == -1) {
        return BG_WAIT_FAILED;
    }
    p->handle = 0;
    return BG_OK;
}

void bg_free(bg_process *p) {
    free(p);
}

#endif
