// Public domain 2017, Simone Basso <bassosimone@gmail.com>.

#include <measurement_kit/ffi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test/winsock.hpp"  // auto-configure winsockets

int main() {
    auto settings = R"({
        "name": "Ndt",
        "log_level": "INFO"
    })";
    auto task = mk_task_start(settings);
    if (task == nullptr) {
        fprintf(stderr, "ERROR: mk_task_start() failed\n");
        exit(1);
    }
    auto exitvalue = 0;
    while (!mk_task_is_done(task)) {
        auto event = mk_task_wait_for_next_event(task);
        if (event == nullptr) {
            fprintf(stderr, "ERROR: mk_task_wait_for_next_event() failed\n");
            exitvalue = 1;
            break;
        }
        auto serialization = mk_event_serialize(event);
        if (serialization == nullptr) {
            fprintf(stderr, "ERROR: mk_event_serialize() failed\n");
            exitvalue = 1;
            break;
        }
        printf("%s\n", serialization);
        fflush(stdout);
        mk_event_destroy(event);
    }
    mk_task_destroy(task);
    return exitvalue;
}
