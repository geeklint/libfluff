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

#ifndef FLUFF_EXCEPTION_H_
#define FLUFF_EXCEPTION_H_

#include <stdio.h>

/*
 * Exception object
 */
struct FluffException;

/*
 * Base exception
 */
extern struct FluffException * fluff_exception_base;

/*
 * Construct new exception
 * parent should be non-null
 * name should be a constant, preferably literal
 * Return new exception on success, NULL on failure
 */
struct FluffException * fluff_exception_new(
		struct FluffException * parent, char * name);

/*
 * Set the current exception
 */
void fluff_exception_throw(struct FluffException *);

/*
 * Check if the current exception matches the given exception
 * Returns 1 if exceptions match, 0 otherwise
 */
int fluff_exception_catch(struct FluffException *);

/*
 * Clear the current exception
 */
void fluff_exception_clear();

/*
 * Grab the traceback.
 * name should be a constant, preferably literal
 * Returns 0 on success, -1 on failure
 * If this function succeeds, it must be followed by a single call
 *     to either tb_set or tb_drop
 */
int fluff_exception_tb_grab(char * name);

/*
 * Set the traceback message
 * message should be a constant, preferably literal
 */
void fluff_exception_tb_set(char * message);

/*
 * Drop the current traceback
 */
void fluff_exception_tb_drop();

/*
 * Print the current traceback
 * Returns -1 on failure, 0 on success
 */
int fluff_exception_tb_print(FILE * stream);

#endif /* FLUFF_EXCEPTION_H_ */
