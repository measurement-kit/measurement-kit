# NAME
Worker &mdash; Runs tasks in a background thread (or more)

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class Worker {
  public:
    static Var<Worker> global();
    static Var<Worker> make();

    void run_in_background_thread(Callback<> &&);
    size_t parallelism();
    void set_parallelism(size_t);
};

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The Worker class allows you to run tasks in a background thread (or more
than one background thread).

The `run_in_background_thread` method schedules the specified callback
to run in a pool of one or more background threads. The callback must
be moved, such that the Worker could take single ownership of it.

The `parallelism` method returns the size of the thread pool.

The `set_parallelism` method increases the size of the thread pool. This
method MAY only have effect next time that `run_in_background_thread`
is called. That is, do not assume that the thread pool size will necessarily
scale dynamically.

The implementation is free to decide whether to keep all threads blocked on
a queue or whether to spawn them on demand. In the latter case, the
`parallelism` would more precisely be an upper bound to the number of
threads that can be active at once. Considering that MK is meant to work
on mobile devices, it SHOULD probably adopt the latter approach.

# HISTORY

The `Worker` class appeared in MeasurementKit 0.7.0.
