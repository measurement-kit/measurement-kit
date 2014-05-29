/* libneubot/stringvector.h */

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

#ifndef LIBNEUBOT_STRINGVECTOR_H
# define LIBNEUBOT_STRINGVECTOR_H
# ifdef __cplusplus

/*-
 * StringVector
 *   A vector of strings that is used to implement the resolver.
 */

struct IghtPoller;

struct IghtStringVector {
    private: 
	char **base;
	size_t count;
	size_t iter;
	size_t pos;
	IghtPoller *poller;
    public:
	IghtStringVector(IghtPoller *, size_t);
	int append(const char *);
	IghtPoller *get_poller(void);
	const char *get_next(void);
	~IghtStringVector(void);
};

# endif  /* __cplusplus */
#endif  /* LIBNEUBOT_STRINGVECTOR_H */
