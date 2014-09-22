/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Fetch the correct JsonCpp json.h header
//

#ifndef LIBIGHT_EXT_JSONCPP_HPP
# define LIBIGHT_EXT_JSONCPP_HPP

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_JSONCPP_JSON_JSON_H
# include <jsoncpp/json/json.h>		// Installed
#else
# include "src/ext/jsoncpp/json.h"	// Builtin
#endif

#endif  // LIBIGHT_EXT_JSONCPP_HPP
