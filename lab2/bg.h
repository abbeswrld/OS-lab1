#ifndef BG_H
#define BG_H

typedef struct bg_process bg_process;

enum {
    BG_OK = 0,
    BG_BAD_ARGS = 1,
    BG_NO_MEMORY = 2,
    BG_START_FAILED = 3,
    BG_WAIT_FAILED = 4
};

int bg_start(const char *const argv[], bg_process **out);
int bg_wait(bg_process *p, int *exit_code);
void bg_free(bg_process *p);

#endif
