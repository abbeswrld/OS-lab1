#define _POSIX_C_SOURCE 200809L

#include "bg.h"

#ifndef _WIN32

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct bg_process {
    pid_t pid;
};

int bg_start(const char *const argv[], bg_process **out) {
    if (!argv || !argv[0] || !out) return BG_BAD_ARGS;
    *out = NULL;

    bg_process *p = malloc(sizeof *p);
    if (!p) return BG_NO_MEMORY;

    p->pid = fork();
    if (p->pid < 0) {
        free(p);
        return BG_START_FAILED;
    }
    if (p->pid == 0) {
        execvp(argv[0], (char *const *)argv);
        _exit(127);
    }

    *out = p;
    return BG_OK;
}

int bg_wait(bg_process *p, int *exit_code) {
    if (!p || !exit_code || p->pid <= 0) return BG_BAD_ARGS;

    int status;
    pid_t r;
    do {
        r = waitpid(p->pid, &status, 0);
    } while (r < 0 && errno == EINTR);
    if (r < 0) return BG_WAIT_FAILED;

    p->pid = -1;
    if (WIFEXITED(status)) *exit_code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) *exit_code = 128 + WTERMSIG(status);
    else *exit_code = 128;
    return BG_OK;
}

void bg_free(bg_process *p) {
    if (!p) return;
    if (p->pid > 0) {
        (void)kill(p->pid, SIGTERM);
        int status;
        while (waitpid(p->pid, &status, 0) < 0 && errno == EINTR) {}
    }
    free(p);
}

#endif
