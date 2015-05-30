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

#ifndef FLUFF_BIGNUM_H_
#define FLUFF_BIGNUM_H_

#include "data.h"

/*
 * Bignum datatype
 */
struct FluffBignum;

/*
 * Static operand for using a small int as an operand to functions which expect
 * a bignum
 */
extern const struct FluffBignum * fluff_bignum_operand;

/*
 * Create a new bignum from an int
 */
struct FluffBignum * fluff_bignum_int(int num);

/*
 * Create a new bignum from a character string of decimal digits
 */
struct FluffBignum * fluff_bignum_decimal(const char *);

/*
 * Create a new bignum from a character string of hex digits
 */
struct FluffBignum * fluff_bignum_hex(const char *);

/*
 * Set the value of the operand described above
 */
void fluff_bignum_set_operand(int num);

/*
 * Returns true if this bignum is not equal to zero
 */
int fluff_bignum_nonzero(struct FluffBignum *);

/*
 * Get a hash of the bignum
 */
FluffHashValue fluff_bignum_hash(struct FluffBignum *);

/*
 * Get the number of characters needed to display this bignum in decimal
 */
size_t fluff_bignum_decimal_digits(struct FluffBignum *);

/*
 * Get the number of characters needed to display this bignum in hex
 */
size_t fluff_bignum_hex_digits(struct FluffBignum *);

/*
 * Write the decimal representation of this bignum to a buffer
 */
void fluff_bignum_as_decimal(struct FluffBignum *, char * buffer);

/*
 * Write the hex representation of this bignum to a buffer
 */
void fluff_bignum_as_hex(struct FluffBignum *, char * buffer);

/*
 * Return a new bignum which has the same value with opposite sign
 */
struct FluffBignum * fluff_bignum_negative(struct FluffBignum *);

/*
 * Return a new bignum which has the same value with positive sign
 */
struct FluffBignum * fluff_bignum_abs(struct FluffBignum *);

/*
 * Return a new bignum (a + b)
 */
struct FluffBignum * fluff_bignum_add(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a - b)
 */
struct FluffBignum * fluff_bignum_subtract(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a * b)
 */
struct FluffBignum * fluff_bignum_multiply(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a / b)
 */
struct FluffBignum * fluff_bignum_divide(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a % b)
 */
struct FluffBignum * fluff_bignum_modulo(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Comparison functions
 */
int fluff_bignum_equals(struct FluffBignum * a, struct FluffBignum * b);
int fluff_bignum_gt(struct FluffBignum * a, struct FluffBignum * b);
int fluff_bignum_lt(struct FluffBignum * a, struct FluffBignum * b);
int fluff_bignum_gteq(struct FluffBignum * a, struct FluffBignum * b);
int fluff_bignum_lteq(struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a & b)
 */
struct FluffBignum * fluff_bignum_and(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a | b)
 */
struct FluffBignum * fluff_bignum_or(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a ^ b)
 */
struct FluffBignum * fluff_bignum_xor(
		struct FluffBignum * a, struct FluffBignum * b);


/*
 * Return a new bignum (a << shift)
 */
struct FluffBignum * fluff_bignum_lshift(
		struct FluffBignum * a, int shift);

/*
 * Return a new bignum (a >> shift)
 */
struct FluffBignum * fluff_bignum_rshift(
		struct FluffBignum * a, int shift);

/*
 * Return the smaller bignum
 */
struct FluffBignum * fluff_bignum_min(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return the larger bignum
 */
struct FluffBignum * fluff_bignum_max(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a ** b)
 */
struct FluffBignum * fluff_bignum_pow(
		struct FluffBignum * a, struct FluffBignum * b);

/*
 * Return a new bignum (a ** b % c)
 */
struct FluffBignum * fluff_bignum_powmod(
		struct FluffBignum * a,
		struct FluffBignum * b,
		struct FluffBignum * c);


#endif /* FLUFF_BIGNUM_H_ */
