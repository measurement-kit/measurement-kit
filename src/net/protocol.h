/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_PROTOCOL_H
# define LIBIGHT_PROTOCOL_H
# ifdef __cplusplus

/*-
 * Protocol
 *   The Protocol is a virtual class that allows to implement the
 *   traits of a protocol on top of, e.g., a Connection.
 */

class IghtPoller;

struct IghtProtocol {
	virtual void on_connect(void) {
		// TODO: override
	}

	virtual void on_ssl(void) {
		// TODO: override
	}

	virtual void on_data(void) {
		// TODO: override
	}

	virtual void on_flush(void) {
		// TODO: override
	}

	virtual void on_eof(void) {
		// TODO: override
	}

	virtual void on_error(void) {
		// TODO: override
	}

	// Defined out-of-line to avoid -Wweak-vtables warning
	virtual IghtPoller *get_poller(void);

	virtual ~IghtProtocol(void) {
		// TODO: override
	}
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_PROTOCOL_H */
