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

struct FluffBignum;

extern const struct FluffBignum * fluff_bignum_operand;

struct FluffBignum * fluff_bignum_int(int num);

struct FluffBignum * fluff_bignum_decimal(const char *);

struct FluffBignum * fluff_bignum_hex(const char *);

void fluff_bignum_set_operand(int num);

int fluff_bignum_nonzero(struct FluffBignum *);

FluffHashValue fluff_bignum_hash(struct FluffBignum *);

size_t fluff_bignum_decimal_digits(struct FluffBignum *);

size_t fluff_bignum_hex_digits(struct FluffBignum *);

void fluff_bignum_as_decimal(struct FluffBignum *);

void fluff_bignum_as_hex(struct FluffBignum *);

struct FluffBignum * fluff_bignum_negative(struct FluffBignum *);

struct FluffBignum * fluff_bignum_abs(struct FluffBignum *);

struct FluffBignum * fluff_bignum_add(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_subtract(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_multiply(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_divide(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_modulo(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_equals(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_gt(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_lt(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_gteq(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_lteq(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_and(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_or(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_xor(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_lshift(
		struct FluffBignum * a, int shift);

struct FluffBignum * fluff_bignum_rshift(
		struct FluffBignum * a, int shift);

struct FluffBignum * fluff_bignum_min(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_max(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_pow(
		struct FluffBignum * a, struct FluffBignum * b);

struct FluffBignum * fluff_bignum_powmod(
		struct FluffBignum * a,
		struct FluffBignum * b,
		struct FluffBignum * c);


#endif /* FLUFF_BIGNUM_H_ */
