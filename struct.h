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

#ifndef FLUFF_STRUCT_H_
#define FLUFF_STRUCT_H_

#include "config/stdarg.h"
#include "config/size_t.h"

/*
 * Format | Standard type | Native type   | Size
 *  x     | none          | none          | 1
 *  c     | char          | char          | 1
 *  b     | int8_t        | signed char   | 1
 *  B     | uint8_t       | unsigned char | 1
 *  h     | int16_t       | short         | 2
 *  H     | uint16_t      | unsigned short| 2
 *  i     | int32_t       | int           | 4
 *  I     | uint32_t      | unsigned int  | 4
 *  q     | int64_t       | long long     | 8
 *  Q     | uint64_t      | unsigned ll   | 8
 *  f     | float         | float         | 4
 *  d     | double        | double        | 8
 *  s     | char *        | char *        |
 */

struct FluffStruct;

struct FluffStruct * fluff_struct_new(char * fmt);

size_t fluff_struct_size(struct FluffStruct *);

void fluff_struct_setnextoffset(struct FluffStruct *, size_t offset);

void fluff_struct_pack(struct FluffStruct *, void * buf, ...);
void fluff_struct_pack_n(struct FluffStruct *, void * buf, ...);
void fluff_struct_pack_v(struct FluffStruct *, void * buf, va_list args);
void fluff_struct_pack_vn(struct FluffStruct *, void * buf, va_list args);
void fluff_struct_pack_s(struct FluffStruct *, void * buf, void * struct_);
void fluff_struct_pack_sn(struct FluffStruct *, void * buf, void * struct_);

void fluff_struct_unpack(struct FluffStruct *, void * buf, ...);
void fluff_struct_unpack_n(struct FluffStruct *, void * buf, ...);
void fluff_struct_unpack_v(struct FluffStruct *, void * buf, va_list args);
void fluff_struct_unpack_vn(struct FluffStruct *, void * buf, va_list args);
void fluff_struct_unpack_s(struct FluffStruct *, void * buf, void * struct_);
void fluff_struct_unpack_sn(struct FluffStruct *, void * buf, void * struct_);

#endif /* FLUFF_STRUCT_H_ */
