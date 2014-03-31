/*
 * Public domain, 2013 Simone Basso.
 */

#ifdef __cplusplus
extern "C" {
#endif

void neubot_warn(const char *, ...)
  __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
