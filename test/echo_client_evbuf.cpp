// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>

#include <stdlib.h>

using namespace measurement_kit::common;
using namespace measurement_kit::net;

int
main(void)
{
	Connection s("PF_INET", "127.0.0.1", "54321");

	s.on_connect([&](void) {
		/* nothing */
	});

	s.on_data([&](SharedPointer<Buffer> b) {
		s.send(b);
	});

	s.on_flush([](void) {
		measurement_kit::info("echo - connection flushed");
	});

	s.on_error([&](Error e) {
		measurement_kit::info("echo - connection error %d", (int) e);
		s.close();
	});

	s.set_timeout(7.0);

	measurement_kit::get_global_poller()->break_loop_on_sigint_();
	measurement_kit::loop();
}
