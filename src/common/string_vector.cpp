/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <new>
#include <string.h>
#include <stdlib.h>

#include <ight/common/string_vector.hpp>

#define IGHT_STRINGVECTOR_MAX 512  // Large enough

using namespace ight::common::string_vector;
using namespace ight::common::poller;

StringVector::StringVector(Poller *p, size_t cnt)
{
	if (p == NULL || cnt == 0 || cnt > IGHT_STRINGVECTOR_MAX)
		throw new (std::bad_alloc);

	this->base = (char **) calloc(cnt, sizeof (char *));
	if (this->base == NULL)
		throw new (std::bad_alloc);

	this->count = cnt;
	this->iter = 0;
	this->pos = 0;
	this->poller = p;
}

int
StringVector::append(const char *str)
{
	if (this->pos > this->count)
		abort();
	if (str == NULL || this->pos == this->count)
		return (-1);
	this->base[this->pos] = strdup(str);
	if (this->base[this->pos] == NULL)
		return (-1);
	++this->pos;
	return (0);
}

Poller *
StringVector::get_poller(void)
{
	return (this->poller);
}

const char *
StringVector::get_next(void)
{
	if (this->iter > this->pos)
		abort();
	if (this->iter == this->pos)
		return (NULL);
	return (this->base[this->iter++]);
}

StringVector::~StringVector(void)
{
	size_t i;

	if (this->base == NULL)
		return;
	if (this->pos > this->count)
		abort();
	for (i = 0; i < this->pos; ++i)
		free(this->base[i]);
	free(this->base);
}
