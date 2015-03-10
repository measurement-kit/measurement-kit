/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <ight/common/log.hpp>
#include <ight/net/connection.hpp>

#include <stdlib.h>

using namespace ight::common::pointer;

int
main(void)
{
	ight::net::connection::Connection s("PF_INET", "127.0.0.1", "54321");

	s.on_connect([&](void) {
		/* nothing */
	});

	s.on_data([&](SharedPointer<IghtBuffer> b) {
		s.send(b);
	});

	s.on_flush([](void) {
		ight_info("echo - connection flushed");
	});

	s.on_error([&](IghtError e) {
		ight_info("echo - connection error %d", e.error);
		s.close();
	});

	s.set_timeout(7.0);

	ight_get_global_poller()->break_loop_on_sigint_();
	ight_loop();
}
