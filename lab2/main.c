#include <stdio.h>

#include "bg.h"

static int run(const char *const argv[], int expected) {
    bg_process *p = NULL;
    int code = -1;

    int rc = bg_start(argv, &p);
    if (rc != BG_OK) {
        printf("start failed: %d\n", rc);
        return 1;
    }

    printf("process started\n");
    fflush(stdout);

    rc = bg_wait(p, &code);
    bg_free(p);
    if (rc != BG_OK) {
        printf("wait failed: %d\n", rc);
        return 1;
    }

    printf("exit code: %d\n", code);
    return code == expected ? 0 : 1;
}

int main(void) {
#ifdef _WIN32
    const char *const ok[] = {"ping", "-n", "2", "127.0.0.1", NULL};
    const char *const nonzero[] = {"cmd", "/c", "exit", "7", NULL};
#else
    const char *const ok[] = {"sleep", "1", NULL};
    const char *const nonzero[] = {"sh", "-c", "exit 7", NULL};
#endif
    const char *const bad[] = {"__missing_program_9483__", NULL};

    int failures = 0;
    failures += run(ok, 0);
    failures += run(nonzero, 7);

    bg_process *p = NULL;
    int rc = bg_start(bad, &p);
    if (rc != BG_OK) {
        printf("missing program rejected\n");
    } else {
        int code = -1;
        rc = bg_wait(p, &code);
        bg_free(p);
        if (rc != BG_OK || code != 127) {
            printf("missing program check failed\n");
            ++failures;
        } else {
            printf("missing program exit code: 127\n");
        }
    }

    printf("%s\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
    return failures == 0 ? 0 : 1;
}
