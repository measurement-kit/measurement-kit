// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

namespace mk {

static void run(
        size_t idx, std::vector<Continuation<Error>> input, Callback<Error> cb,
        Var<Error> overall, Var<size_t> completed, size_t parallelism) {
    if (idx < input.size()) {
        input[idx]([=](Error error) {
            // XXX: code not thread safe, to make thread safe we could
            // share a mutex or use call_later() to serialize
            if (error and *overall == NoError()) {
                static const Error template_error = ParallelOperationError();
                overall->code = template_error.code;
                overall->reason = template_error.reason;
                // FALLTHROUGH
            }
            overall->child_errors[idx] = Var<Error>(new Error(error));
            *completed += 1;
            if (*completed == input.size()) {
                cb(*overall);
                return;
            }
            if (*completed > input.size()) {
                // Use exception, not assert, so it cannot be disabled using
                // compiler flags and we always make this check
                throw std::runtime_error("unexpected *complete value");
            }
            run(idx + parallelism, input, cb, overall, completed, parallelism);
        });
    }
}

void parallel(std::vector<Continuation<Error>> input, Callback<Error> cb,
              size_t parallelism) {
    Var<Error> overall(new Error(NoError()));
    if (input.size() <= 0) {
        cb(*overall);
        return;
    }
    overall->child_errors.resize(input.size(), nullptr);
    Var<size_t> completed(new size_t(0));
    if (parallelism <= 0) {
        parallelism = input.size();
    }
    /*
     * Possible improvement: use an atomic shared integer as opposed to
     * using a counter to speed up the parallel process at the end, when
     * some "threads" could terminate earlier than others.
     *
     * Enhancement suggested by @AntonioLangiu.
     */
    for (size_t idx = 0; idx < parallelism; ++idx) {
        run(idx, input, cb, overall, completed, parallelism);
    }
}

} // namespace mk
