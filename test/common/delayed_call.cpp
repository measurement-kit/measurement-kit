/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "../src/common/poller.h"

#include <iostream>

struct X {
	IghtDelayedCall d;
};

static IghtDelayedCall
move_semantic(void)
{
	return (IghtDelayedCall(3.0, [](void) {
		std::cout << "The right delayed call" << "\n";
		ight_break_loop();
	}));
}

//
// TODO: Once we've chosen a testing framework, this unit test
// be rewritten using the primitives of such framework.
//
int
main(void)
{
	//
	// Make sure that an empty delayed call is successfully
	// destroyed (no segfault) when we leave the scope.
	//
	auto d1 = IghtDelayedCall();

	//
	// Register a first delayed call (which will be overriden
	// later) to ensure that move semantic works.
	//
	auto d2 = IghtDelayedCall(3.0, [](void) {
		std::cout << "The wrong delayed call" << "\n";
		ight_break_loop();
	});

	//
	// Replace the delayed call (which should clear the previous
	// delayed call contained by d2, if any) with a new one, using
	// the move semantic.
	//
	d2 = move_semantic();

	//
	// Make sure that we don't raise an exception in the libevent
	// callback, when we're passed an empty std::function.
	//
	auto d3 = IghtDelayedCall(0.5, std::function<void(void)>());

	//
	// The compiler should refuse the compile the following (I wonder
	// which is the best way to test that):
	//
	// d1 = d3;
	//
	// IghtDelayedCall d4(d1);
	//

	//
	// Make sure that the delayed call is unscheduled when the
	// object X is deleted, which simplifies life when you have
	// a delayed call bound to a connection that may be closed
	// at any time by peer.
	//
	X *x = new X();
	x->d = IghtDelayedCall(0.0, [](void) {
		std::cout << "This message shouldn't be printed" << "\n";
	});
	delete (x);

	ight_loop();
}
