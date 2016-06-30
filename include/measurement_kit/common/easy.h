// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_EASY_H
#define MEASUREMENT_KIT_COMMON_EASY_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct MkEasy_;
typedef int mk_easy_abi_t;
typedef int mk_easy_error_t;
typedef struct MkEasy_ mk_easy_t;
typedef void (mk_easy_log_func_t)(uint32_t, const char *);
typedef void (mk_easy_done_func_t)(mk_easy_error_t);

mk_easy_abi_t mk_easy_abi_get(void);

mk_easy_t *mk_easy_new(const char *);

void mk_easy_on_log(mk_easy_t *, mk_easy_log_func_t *);

void mk_easy_set_verbosity(mk_easy_t *, uint32_t);

void mk_easy_increase_verbosity(mk_easy_t *);

void mk_easy_set_input_filepath(mk_easy_t *, const char *);

void mk_easy_set_output_filepath(mk_easy_t *, const char *);

void mk_easy_set_options(mk_easy_t *, const char *, const char *);

mk_easy_error_t mk_easy_run(mk_easy_t *);

void mk_easy_run_async(mk_easy_t *, mk_easy_done_func_t *);

void mk_easy_delete(mk_easy_t *);

#ifdef __cplusplus
}
#endif
#endif
