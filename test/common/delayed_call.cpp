/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "../src/common/poller.h"

#include <iostream>

static IghtDelayedCall
move_semantic(void)
{
	return (IghtDelayedCall(3.0, [](void) {
		std::cout << "The right delayed call" << "\n";
		ight_break_loop();
	}));
}

int
main(void)
{
	auto d1 = IghtDelayedCall();

	auto d2 = IghtDelayedCall(3.0, [](void) {
		std::cout << "The wrong delayed call" << "\n";
		ight_break_loop();
	});

	d2 = move_semantic();

	auto d3 = IghtDelayedCall(0.5, std::function<void(void)>());

	//
	// The compiler should refuse the compile the following (I wonder
	// which is the best way to test that):
	//
	// d1 = d3;
	//
	// IghtDelayedCall d4(d1);
	//

	ight_loop();
}
