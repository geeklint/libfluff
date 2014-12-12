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
 * This function must be followed by a single call
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
 */
int fluff_exception_tb_print(FILE * stream);

#endif /* FLUFF_EXCEPTION_H_ */
