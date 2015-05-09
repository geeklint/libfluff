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

#ifndef FLUFF_TRACEBACK_H_
#define FLUFF_TRACEBACK_H_

#include "mm.h"

/*
 * Grab the traceback.
 * name should be a constant, preferably literal
 * Returns 0 on success, -1 on failure
 * If this function succeeds, it must be followed by a single call
 *     to either _set or _drop
 */
int fluff_traceback_grab(char * name);

/*
 * Set the traceback message
 * message should be a constant, preferably literal
 */
void fluff_traceback_set(char * message);

/*
 * Drop the current traceback
 */
void fluff_traceback_drop();

/*
 * Pause sub-traceback reporting (e.g. if ignoring errors)
 */
void fluff_traceback_pause();

/*
 * Resume traceback
 */
void fluff_traceback_resume();

/*
 * Set the memory manager used by these functions
 */
void fluff_traceback_setmm(const struct FluffMM *);

#endif /* FLUFF_TRACEBACK_H_ */
