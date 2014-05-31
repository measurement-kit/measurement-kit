/*
 * Public domain, 2013 Simone Basso.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * TODO: this function should be only compiled if the system does not
 * already provide its own strtonum().
 */
long long ight_strtonum(const char *, long long, long long,
                          const char **);

#ifdef __cplusplus
}
#endif
