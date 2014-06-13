/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <stdlib.h>

#include <new>

#include "net/protocol.h"

// Defined here to avoid -Wweak-vtables warning
IghtPoller *
IghtProtocol::get_poller(void)
{
	return (NULL);  // TODO: override
}
