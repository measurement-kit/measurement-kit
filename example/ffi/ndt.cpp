/* Public domain 2017, Simone Basso <bassosimone@gmail.com. */

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
    mk_task_t *task = mk_task_start(
            "{\n"
            "    \"type\": \"Ndt\",\n"
            "    \"verbosity\": \"INFO\"\n"
            "}\n");
    if (task == NULL) {
        fprintf(stderr, "ERROR: cannot create/start task\n");
        exit(1);
    }

    int done = 0;
    do {
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
        // A bit ugly but this is `null` as a serialized JSON
        done = (strcmp(serio, "null") == 0);
        printf("%s\n", serio);
        mk_event_destroy(event);
    } while (!done);

    mk_task_destroy(task);
    return 0;
}
