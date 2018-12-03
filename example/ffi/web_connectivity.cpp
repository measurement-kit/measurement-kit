/* Public domain 2017, Simone Basso <bassosimone@gmail.com. */

#include "test/winsock.hpp"

#include <measurement_kit/ffi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Ideally it would be nice to have this written in C. For this to work,
 * however, we would need to specifically compile Measurement Kit to link
 * statically with its own version of libc++.
 */
int main() {
    mk_task_t *task = mk_nettest_start(R"({
        "inputs": [
            "https://www.google.com",
            "https://www.x.org"
        ],
        "name": "WebConnectivity",
        "log_level": "INFO",
        "options": {
            "net/ca_bundle_path": "cacert.pem"
        }})");
    if (task == nullptr) {
        fprintf(stderr, "ERROR: cannot create/start task\n");
        exit(1);
    }

    while (!mk_task_is_done(task)) {
        mk_event_t *event = mk_task_wait_for_next_event(task);
        if (event == NULL) {
            fprintf(stderr, "ERROR: cannot wait for next event\n");
            exit(1);
        }
        const char *serio = mk_event_serialize(event);
        if (serio == NULL) {
            fprintf(stderr, "ERROR: cannot serialize event\n");
            exit(1);
        }
        printf("%s\n", serio);
        fflush(stdout);
        mk_event_destroy(event);
    }

    mk_task_destroy(task);
    return 0;
}
