/* libneubot/stringvector.cpp */

/*-
 * Copyright (c) 2014
 *     Nexa Center for Internet & Society, Politecnico di Torino (DAUIN)
 *     and Simone Basso <bassosimone@gmail.com>.
 *
 * This file is part of Neubot <http://www.neubot.org/>.
 *
 * Neubot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neubot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Neubot.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <new>
#include <string.h>
#include <stdlib.h>

#include "common/stringvector.h"

#define NEUBOT_STRINGVECTOR_MAX 512  // Large enough

IghtStringVector::IghtStringVector(IghtPoller *p, size_t cnt)
{
	if (p == NULL || cnt == 0 || cnt > NEUBOT_STRINGVECTOR_MAX)
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
IghtStringVector::append(const char *str)
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

IghtPoller *
IghtStringVector::get_poller(void)
{
	return (this->poller);
}

const char *
IghtStringVector::get_next(void)
{
	if (this->iter > this->pos)
		abort();
	if (this->iter == this->pos)
		return (NULL);
	return (this->base[this->iter++]);
}

IghtStringVector::~IghtStringVector(void)
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
