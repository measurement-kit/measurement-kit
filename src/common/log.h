/*
 * Public domain, 2013 Simone Basso.
 */

#ifdef __cplusplus
extern "C" {
#endif

void ight_warn(const char *, ...)
  __attribute__((format(printf, 1, 2)));

void ight_info(const char *, ...)
  __attribute__((format(printf, 1, 2)));

void ight_debug(const char *, ...)
  __attribute__((format(printf, 1, 2)));

void ight_set_verbose(int);

#ifdef __cplusplus
}
#endif
