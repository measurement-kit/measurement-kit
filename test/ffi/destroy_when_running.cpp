/* Public domain 2017, Simone Basso <bassosimone@gmail.com. */

#include "test/winsock.hpp"

#include <measurement_kit/ffi.h>

#include <chrono>
#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Prepare and run a NDT task
    mk_task_t *task = mk_nettest_start(R"({
        "name": "Ndt",
        "log_level": "INFO",
        "options": {
            "net/ca_bundle_path": "cacert.pem"
        }})");
    if (task == NULL) {
        fprintf(stderr, "ERROR: cannot create/start task\n");
        exit(1);
    }

    // Ignore pending events and give the test some time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Destroy the task. This will destory the memory that the background
    // thread running the task is using. If we don't synchronize with
    // the thread, this will cause memory errors when running a asan build.
    mk_task_destroy(task);

    // Sleep for some more time, otherwise the main process will exit
    // because we're currently using a background thread
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
