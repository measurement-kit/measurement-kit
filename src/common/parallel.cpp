// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

namespace mk {

void parallel(std::vector<Continuation<Error>> input, Callback<Error> cb) {
    static const Error template_error = ParallelOperationError();
    Var<Error> overall(new Error(NoError()));
    if (input.size() <= 0) {
        cb(*overall);
        return;
    }
    overall->child_errors.resize(input.size(), nullptr);
    Var<size_t> completed(new size_t(0));
    for (size_t idx = 0; idx < input.size(); ++idx) {
        input[idx]([=](Error error) {
            // XXX: code not thread safe, to make thread safe we could
            // share a mutex or use call_later() to serialize
            if (error and *overall == NoError()) {
                overall->code = template_error.code;
                overall->reason = template_error.reason;
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
        });
    }
}

} // namespace mk
