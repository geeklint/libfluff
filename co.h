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

#ifndef FLUFF_CO_H_
#define FLUFF_CO_H_

#include <stdint.h>

#include "data.h"
#include "mm.h"
#include "bignum.h"

enum FluffCOTypeID {
	FluffCOTypeIDNone = 0,
	FluffCOTypeIDBool = 1,
	FluffCOTypeIDInt = 2,
	FluffCOTypeIDLong = 3,
	FluffCOTypeIDFloat = 4,
	FluffCOTypeIDString = 5,
	FluffCOTypeIDArray = 6,
	FluffCOTypeIDDict = 7,
};

typedef uint_fast8_t FluffCONoneType;
typedef uint_fast8_t FluffCOBoolType;
struct FluffCOString;
struct FluffCOList;
struct FluffCODict;

union FluffCOType {
	FluffCONoneType f_none;
	FluffCOBoolType f_bool;
	long f_int;
	struct FluffBignum * f_long;
	double f_float;
	struct FluffCOString * f_string;
	struct FluffCOList * f_list;
	struct FluffCODict * f_dict;
};

extern FluffCONoneType fluff_co_none;
extern FluffCOBoolType fluff_co_true;
extern FluffCOBoolType fluff_co_false;

#include "costring.h"
#include "colist.h"
#include "codict.h"

/*
 * Context - when new co objects are allocated, they are attached to the
 * current context, and can all be freed together by clearing the context
 */
struct FluffCOContext;

/*
 * Default context if no other is specified
 */
extern struct FluffCOContext * fluff_co_defaultcontext;

/*
 * Create a new context
 */
struct FluffCOContext * fluff_co_context_new();

/*
 * Free context
 */
void fluff_co_context_free(struct FluffCOContext *);

/*
 * Set the current context. Pass null to disable context operations (you must
 * manage memory yourself then)
 */
void fluff_co_setcontext(struct FluffCOContext *);

/*
 * Free all items associated with the current context
 */
void fluff_co_clearcontext();

/*
 * Set the memory manager used by the CO submodule
 */
void fluff_co_setmm(const struct FluffMM *);

#endif /* FLUFF_CO_H_ */
