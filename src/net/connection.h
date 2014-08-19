/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_CONNECTION_H
# define LIBIGHT_CONNECTION_H
# ifdef __cplusplus

#include "src/common/poller.h"

struct bufferevent;
struct evbuffer;

struct IghtStringVector;
struct IghtProtocol;

class IghtConnection {

	long long filedesc;
	bufferevent *bev;
	IghtProtocol *protocol;
	evbuffer *readbuf;
	unsigned int closing;
	unsigned int connecting;
	unsigned int reading;
	char *address;
	char *port;
	IghtStringVector *addrlist;
	char *family;
	IghtStringVector *pflist;
	unsigned int must_resolve_ipv4;
	unsigned int must_resolve_ipv6;
	IghtDelayedCall start_connect;

	IghtConnection(void);

	// Private destructor because destruction may be delayed
	~IghtConnection(void);

	// Libevent callbacks
	static void handle_read(bufferevent *, void *);
	static void handle_write(bufferevent *, void *);
	static void handle_event(bufferevent *, short, void *);

	// Functions used by connect_hostname()
	void connect_next(void);
	static void handle_resolve(int, char, int, int,
	    void *, void *);
	static void resolve(void *);

	// Function used by write_rand() and write_rand_readbuf()
	int write_rand_evbuffer(struct evbuffer *buf, size_t count);

    public:
	static IghtConnection *attach(IghtProtocol *, long long);

	static IghtConnection *connect(IghtProtocol *, const char *,
	    const char *, const char *);

	static IghtConnection *connect_hostname(IghtProtocol *, const char *,
	    const char *, const char *);

	IghtProtocol *get_protocol(void);

	int set_timeout(double);

	int clear_timeout(void);

	int start_tls(unsigned int);

	int read(char *, size_t);
	int readline(char *, size_t);
	int readn(char *, size_t);
	int discardn(size_t);
	int write(const char *, size_t);
	int puts(const char *);
	int write_rand(size_t);
	int write_readbuf(const char *, size_t);
	int puts_readbuf(const char *);
	int write_rand_readbuf(size_t);

	// Internally-used zero-copy read and write
#ifndef SWIG
	int read_into_(evbuffer *);
	int write_from_(evbuffer *);
#endif

	int enable_read(void);
	int disable_read(void);

	void close(void);
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_CONNECTION_H */
