# NAME
fcompose &mdash; Compose arbitrary functors using different composing policies

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

mk::fcompose(
    mk::fcompose_policy_sync(),
    [](int x) {
        return std::make_tuple(x, 17.0);
    },
    [](int x, int y) {
        return x + y;
    },
    [](int x) {
        return std::to_string(x);
    },
    [](std::string x) {
        std::cout << x << "\n";
    })(0.0);

auto cb = mk::fcompose(
    mk::fcompose_policy_async(),
    [](std::string data, Callback<Error, std::string> cb) {
        if (mk::startswith(data, "300")) {
            return; // Just ignore this line
        }
        if (!mk::startswith(data, "200")) {
            cb(GenericError(), ""); // Hard error
            return;
        }
        cb(NoError(), data);
    },
    [](Error err, std::string s, Callback<>) {
        if (!err) {
            std::cout << s << "\n";
        }
        cb();
    });

cb("300 ignore", [cb]() {
    cb("200 ok", []() {
        /* NOTHING TO DO */
    });
});

auto cb = mk::fcompose(
    mk::fcompose_policy_async_robust(
        [](const std::exception &exc) {
            std::clog << "Error occurred: " << exc.what() << "\n";
        }),
    // SAME AS ABOVE
);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fcompose` template function composes the functions passed as arguments
according to the policy specified as its first argument.

The `fcompose_policy_sync` function is such that, given two functions `f`
and `g`, the f-composed-g function takes in input the arguments of `f` and
has the return value of `g`.

The `fcompose_policy_async` function is that that, given two functions `f`
and `g` that have a callback as their last argument, the f-composed-g
function takes in input the arguments of `f`, minus the callback, which
is the callback of `g`.

The `fcompose_policy_async_robust` function is like `fcompose_policy_async`
except that it routes exceptions occurring in `f` prior to calling `g` to
the callable passed as the first argument to the policy function itself.

# HISTORY

The `fcompose` template function appeared in MeasurementKit 0.7.0.
