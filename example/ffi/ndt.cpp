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
    char buff[128];
    mk_task_t *task = mk_task_start_ex("{\n"
                                       "    \"type\": \"Ndt\",\n"
                                       "    \"verbosity\": \"INFO\"\n"
                                       "}\n",
            buff, sizeof(buff));
    if (task == NULL) {
        fprintf(stderr, "ERROR: cannot create/start task: %s\n", buff);
        exit(1);
    }

    while (mk_task_is_running(task)) {
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
        mk_event_destroy(event);
    }

    mk_task_destroy(task);
    return 0;
}
