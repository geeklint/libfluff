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

#ifndef FLUFF_COSTRING_H_
#define FLUFF_COSTRING_H_

#include <wchar.h>

#include "co.h"

/*
 * Constructors
 */
struct FluffCOString * fluff_co_string_new_c(char * src);
struct FluffCOString * fluff_co_string_new_w(wchar_t * src);
struct FluffCOString * fluff_co_string_new_cl(char * src, size_t length);
struct FluffCOString * fluff_co_string_new_wl(wchar_t * src, size_t length);

/*
 * Freer
 */
void fluff_co_string_free(struct FluffCOString *);

/*
 * Get length of string
 */
size_t fluff_co_string_length(struct FluffCOString *);

/*
 * Get the internal string pointer. Client code is responsible for
 * consequences of altering this buffer.
 */
void fluff_co_string_read(struct FluffCOString *, wchar_t ** dest);

/*
 * Read a copy of the string
 */
void fluff_co_string_readcopy(
		struct FluffCOString *, wchar_t * dest);

/*
 * Test if two strings are equal
 */
int fluff_co_string_equals(struct FluffCOString * a, struct FluffCOString * b);
int fluff_co_string_eq_c(struct FluffCOString * a, char * b);
int fluff_co_string_eq_w(struct FluffCOString * a, wchar_t * b);

/*
 * Test if a string contains a substring
 */
int fluff_co_string_contains(
		struct FluffCOString *, struct FluffCOString * sub);
int fluff_co_string_contains_c(struct FluffCOString *, char * sub);
int fluff_co_string_contains_w(struct FluffCOString *, wchar_t * sub);

/*
 * Test if a string contains a character
 */
int fluff_co_string_char(struct FluffCOString *, char c);
int fluff_co_string_wchar(struct FluffCOString *, wchar_t c);

/*
 * Get the number of occurances of a character in a string
 */
int fluff_co_string_count_c(struct FluffCOString *, char c);
int fluff_co_string_count_w(struct FluffCOString *, wchar_t c);

/*
 * Get the character at index.
 * Negative numbers understood to be indexes from the end of the string.
 */
wchar_t fluff_co_string_getchar(struct FluffCOString *, int index);

/*
 * Create a new string which is a substring of the current string.
 * Negative numbers understood to be indexes from the end of the string.
 */
struct FluffCOString * fluff_co_string_substring(
		struct FluffCOString *, int start, int end);

/*
 * Create a new string which is the concatenation of two strings
 */
struct FluffCOString * fluff_co_string_concat(
		struct FluffCOString * a, struct FluffCOString * b);

/*
 * Create a new string which is this string concatenated with itself a certain
 * number of times
 */
struct FluffCOString * fluff_co_string_repeat(
		struct FluffCOString *, unsigned int n);

/*
 * Create a new string and fill it with a certain character
 */
struct FluffCOString * fluff_co_string_repeat_c(char c, unsigned int n);
struct FluffCOString * fluff_co_string_repeat_w(wchar_t c, unsigned int n);

/*
 * Create a new string identical but with the first letter capitalized
 */
struct FluffCOString * fluff_co_string_captialize(struct FluffCOString *);
struct FluffCOString * fluff_co_string_captialize_c(char *);
struct FluffCOString * fluff_co_string_captialize_w(wchar_t *);

/*
 * Create a new string of specified length with this string centered, padded
 * with specified fill character
 */
struct FluffCOString * fluff_co_string_center_sc(
		struct FluffCOString *, unsigned int length, char fillchar);
struct FluffCOString * fluff_co_string_center_sw(
		struct FluffCOString *, unsigned int length, wchar_t fillchar);
struct FluffCOString * fluff_co_string_center_cc(
		char *, unsigned int length, char fillchar);
struct FluffCOString * fluff_co_string_center_cw(
		char *, unsigned int length, wchar_t fillchar);
struct FluffCOString * fluff_co_string_center_wc(
		wchar_t *, unsigned int length, char fillchar);
struct FluffCOString * fluff_co_string_center_ww(
		wchar_t *, unsigned int length, wchar_t fillchar);

/*
 * Test if a string ends with another string
 */
struct FluffCOString * fluff_co_string_endswith(
		struct FluffCOString *, struct FluffCOString * end);
struct FluffCOString * fluff_co_string_endswith_c(
		struct FluffCOString *, char * end);
struct FluffCOString * fluff_co_string_endswith_c(
		struct FluffCOString *, wchar_t * end);

/*
 * Concatenate many strings
 */
struct FluffCOString * fluff_co_string_join(
		struct FluffCOString * sep,
		unsigned int count,
		struct FluffCOString ** strings);
struct FluffCOString * fluff_co_string_join_c(
		char * sep,
		unsigned int count,
		struct FluffCOString ** strings);
struct FluffCOString * fluff_co_string_join_w(
		wchar_t * sep,
		unsigned int count,
		struct FluffCOString ** strings);


// TODO: Add more functions

#endif /* FLUFF_COSTRING_H_ */
