/*
	Copyright 2014 Sky Leonard
	This file is part of libfluff.

    libfluff is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libfluff is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libfluff.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FLUFF_COLIST_H_
#define FLUFF_COLIST_H_

#include "co.h"

struct FluffCOArray * fluff_co_array_new(size_t hint);
void fluff_co_array_free(struct FluffCOArray *);
size_t fluff_co_array_length(struct FluffCOArray *);
int fluff_co_array_get(
		struct FluffCOArray *,
		unsigned int index,
		enum FluffCOTypeID * type,
		union FluffCOType * item);
int fluff_co_array_replace(
		struct FluffCOArray *,
		unsigned int index,
		enum FluffCOTypeID type_new,
		union FluffCOType item_new,
		enum FluffCOTypeID * type_old,
		union FluffCOType * item_old);
int fluff_co_array_append(
		struct FluffCOArray *,
		enum FluffCOTypeID type,
		union FluffCOType item);

#endif /* FLUFF_COLIST_H_ */
