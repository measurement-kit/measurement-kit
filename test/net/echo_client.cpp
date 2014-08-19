/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "common/log.h"
#include "net/connection.h"

#include <stdlib.h>

int
main(void)
{
	auto s = new IghtConnection("PF_INET", "127.0.0.1", "54321");

	s->on_connect = [=](void) {
		if (s->enable_read() != 0)
			s->close();
	};

	s->on_data = [=](evbuffer *b) {
		if (s->write_from(b) != 0)
			s->close();
	};

	s->on_flush = [](void) {
		ight_info("echo - connection flushed");
	};

	s->on_error = [=](IghtError e) {
		ight_info("echo - connection error %d", e.error);
		s->close();
	};

	if (s->set_timeout(7.0) != 0)
		exit(EXIT_FAILURE);

	ight_get_global_poller()->break_loop_on_sigint_();
	ight_loop();
}
