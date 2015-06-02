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

#include "struct.h"

#include "config/string.h"

#include "data.h"
#include "mm.h"
#include "stdfmtnum.h"

struct Member {
	char format;
	size_t length;
	size_t offset;
};

struct FluffStruct {
	size_t size;
	size_t nmemb;
	unsigned int noffset;
	struct Member * members;
};

/*
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData struct_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_default;
	}
    struct_size = MM->f_type_new(sizeof(struct FluffStruct));
    mm_need_setup = 0;
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

static int get_digit(char c, unsigned int * dest){
	char * sc;
	char * s = "0123456789";

	if ((sc = strchr(s, c))){
		*dest = sc - s;
		return 1;
	}
	return 0;
}

#define STD_CASE(n)           \
		count = count ?: 1;   \
		size += (n) * count;    \
		nmemb += count;       \
		count = 0;

struct FluffStruct * fluff_struct_new(char * fmt){
	size_t size = 0, nmemb = 0, count = 0;
	unsigned int digit, i;
	char c;
	char * iter;
	struct FluffStruct * self;
	struct Member * members;

	ENSURE_MM;

	iter = fmt;
	while ((c = *iter++)){
		switch (c) {
			case 'x':
				count = count ?: 1;
				size += count;
				nmemb += 1;
				count = 0;
				break;
			case 'c':
				STD_CASE(1);
				break;
			case 'b':
				STD_CASE(1);
				break;
			case 'B':
				STD_CASE(1);
				break;
			case 'h':
				STD_CASE(2);
				break;
			case 'H':
				STD_CASE(2);
				break;
			case 'i':
				STD_CASE(4);
				break;
			case 'I':
				STD_CASE(4);
				break;
			case 'q':
				STD_CASE(8);
				break;
			case 'Q':
				STD_CASE(8);
				break;
			case 'f':
				STD_CASE(4);
				break;
			case 'd':
				STD_CASE(8);
				break;
			case 's':
				if (!count) return NULL;
				size += count;
				nmemb += 1;
				count = 0;
				break;
			default:
				if (!get_digit(c, &digit)) return NULL;
				count *= 10;
				count += digit;
				break;
		}
	}
	if (!(self = MM->f_alloc(struct_size))){
		return NULL;
	}
	self->size = size;
	self->nmemb = nmemb;
	self->noffset = 0;
	if (!(members = MM->f_alloc_size(nmemb * sizeof(struct Member)))){
		MM->f_free(self);
		return NULL;
	}
	self->members = members;
	count = 0;
	nmemb = 0;
	iter = fmt;
	while ((c = *iter++)){
		if (get_digit(c, &digit)){
			count *= 10;
			count += digit;
		} else {
			switch (c) {
				case 'c':
				case 'b':
				case 'B':
				case 'h':
				case 'H':
				case 'i':
				case 'I':
				case 'q':
				case 'Q':
				case 'f':
				case 'd':
					for (i = (count ?: 1); i; --i) {
						members[nmemb++].format = c;
					}
					break;
				case 'x':
				case 's':
					members[nmemb].format = c;
					members[nmemb].length = count ?: 1;
					nmemb += 1;
					break;
				default:
					break; // Never reached
			}
			count = 0;
		}
	}
	return self;
}

size_t fluff_struct_size(struct FluffStruct * self){
	return self->size;
}

void fluff_struct_setnextoffset(struct FluffStruct * self, size_t offset){
	if (self->noffset < self->nmemb){
		self->members[self->noffset++].offset = offset;
	}
}

void fluff_struct_pack(struct FluffStruct * self, void * buf, ...){
	va_list args;

	va_start(args, buf);
	fluff_struct_pack_v(self, buf, args);
	va_end(args);
}

void fluff_struct_pack_n(struct FluffStruct * self, void * buf, ...){
	va_list args;

	va_start(args, buf);
	fluff_struct_pack_vn(self, buf, args);
	va_end(args);
}

#undef STD_CASE
#define STD_CASE(C, D, S, F)                       \
		case C:                                    \
			*((D *) head) = F(va_arg(args, S));    \
			head += sizeof(D);                     \
			break;

#define CAST_CASE(C, D, S)                          \
		case C:                                     \
			*((D *) head) = (D) va_arg(args, S);    \
			head += sizeof(D);                      \
			break;

void fluff_struct_pack_v(struct FluffStruct * self, void * buf, va_list args){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, int); // (char)
			CAST_CASE('b', int8_t, int); // (int8_t)
			CAST_CASE('B', uint8_t, int); // (uint8_t)
			STD_CASE('h', FluffShort, int, fluff_short_htof); // (int16_t)
			STD_CASE('H', FluffUShort, int, fluff_ushort_htof); // (uint16_t)
			STD_CASE('i', FluffInt, int32_t, fluff_int_htof);
			STD_CASE('I', FluffUInt, uint32_t, fluff_uint_htof);
			STD_CASE('q', FluffLong, int64_t, fluff_long_htof);
			STD_CASE('Q', FluffULong, uint64_t, fluff_ulong_htof);
			STD_CASE('f', FluffFloat, double, fluff_float_htof); // (float)
			STD_CASE('d', FluffDouble, double, fluff_double_htof);
			case 'x':
				memset(head, 0, member->length);
				head += member->length;
				break;
			case 's':
				strncpy(head, va_arg(args, char *), member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

void fluff_struct_pack_vn(struct FluffStruct * self, void * buf, va_list args){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, int); // (char)
			CAST_CASE('b', int8_t, int); // (signed char)
			CAST_CASE('B', uint8_t, int); // (unsigned char)
			STD_CASE('h', FluffShort, int, fluff_short_htof); // (short)
			STD_CASE('H', FluffUShort, int, fluff_ushort_htof); // (u short)
			STD_CASE('i', FluffInt, int, fluff_int_htof);
			STD_CASE('I', FluffUInt, unsigned int, fluff_uint_htof);
			STD_CASE('q', FluffLong, long, fluff_long_htof);
			STD_CASE('Q', FluffULong, unsigned long, fluff_ulong_htof);
			STD_CASE('f', FluffFloat, double, fluff_float_htof); // (float)
			STD_CASE('d', FluffDouble, double, fluff_double_htof);
			case 'x':
				memset(head, 0, member->length);
				head += member->length;
				break;
			case 's':
				strncpy(head, va_arg(args, char *), member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

#undef STD_CASE
#define STD_CASE(C, D, S, F)                                          \
		case C:                                                       \
			*((D *) head) = F(*((S *)(struct_ + member->offset)));    \
			head += sizeof(D);                                        \
			break;

#undef CAST_CASE
#define CAST_CASE(C, D, S)                                             \
		case C:                                                        \
			*((D *) head) = (D) *((S *)(struct_ + member->offset));    \
			head += sizeof(D);                                         \
			break;

void fluff_struct_pack_s(
		struct FluffStruct * self, void * buf, void * struct_){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, char);
			CAST_CASE('b', int8_t, int8_t);
			CAST_CASE('B', uint8_t, uint8_t);
			STD_CASE('h', FluffShort, int16_t, fluff_short_htof);
			STD_CASE('H', FluffUShort, uint16_t, fluff_ushort_htof);
			STD_CASE('i', FluffInt, int32_t, fluff_int_htof);
			STD_CASE('I', FluffUInt, uint32_t, fluff_uint_htof);
			STD_CASE('q', FluffLong, int64_t, fluff_long_htof);
			STD_CASE('Q', FluffULong, uint64_t, fluff_ulong_htof);
			STD_CASE('f', FluffFloat, float, fluff_float_htof);
			STD_CASE('d', FluffDouble, double, fluff_double_htof);
			case 'x':
				memset(head, 0, member->length);
				head += member->length;
				break;
			case 's':
				strncpy(
					head,
					((char *)(struct_ + member->offset)),
					member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

void fluff_struct_pack_sn(
		struct FluffStruct * self, void * buf, void * struct_){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, char);
			CAST_CASE('b', int8_t, signed char);
			CAST_CASE('B', uint8_t, unsigned char);
			STD_CASE('h', FluffShort, short, fluff_short_htof);
			STD_CASE('H', FluffUShort, unsigned short, fluff_ushort_htof);
			STD_CASE('i', FluffInt, int, fluff_int_htof);
			STD_CASE('I', FluffUInt, unsigned int, fluff_uint_htof);
			STD_CASE('q', FluffLong, long, fluff_long_htof);
			STD_CASE('Q', FluffULong, unsigned long, fluff_ulong_htof);
			STD_CASE('f', FluffFloat, float, fluff_float_htof);
			STD_CASE('d', FluffDouble, double, fluff_double_htof);
			case 'x':
				memset(head, 0, member->length);
				head += member->length;
				break;
			case 's':
				strncpy(
					head,
					((char *)(struct_ + member->offset)),
					member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

void fluff_struct_unpack(struct FluffStruct * self, void * buf, ...){
	va_list args;

	va_start(args, buf);
	fluff_struct_unpack_v(self, buf, args);
	va_end(args);
}

void fluff_struct_unpack_n(struct FluffStruct * self, void * buf, ...){
	va_list args;

	va_start(args, buf);
	fluff_struct_unpack_vn(self, buf, args);
	va_end(args);

}

#undef STD_CASE
#define STD_CASE(C, D, S, F)                              \
		case C:                                           \
			*(va_arg(args, S *)) = F(*((D *) head));    \
			head += sizeof(D);                            \
			break;

#undef CAST_CASE
#define CAST_CASE(C, D, S)                             \
		case C:                                        \
			*(va_arg(args, S *)) = *((D *) head);    \
			head += sizeof(D);                         \
			break;

void fluff_struct_unpack_v(
		struct FluffStruct * self, void * buf, va_list args){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, char);
			CAST_CASE('b', int8_t, int8_t);
			CAST_CASE('B', uint8_t, uint8_t);
			STD_CASE('h', FluffShort, int16_t, fluff_short_ftoh);
			STD_CASE('H', FluffUShort, uint16_t, fluff_ushort_ftoh);
			STD_CASE('i', FluffInt, int32_t, fluff_int_ftoh);
			STD_CASE('I', FluffUInt, uint32_t, fluff_uint_ftoh);
			STD_CASE('q', FluffLong, int64_t, fluff_long_ftoh);
			STD_CASE('Q', FluffULong, uint64_t, fluff_ulong_ftoh);
			STD_CASE('f', FluffFloat, float, fluff_float_ftoh);
			STD_CASE('d', FluffDouble, double, fluff_double_ftoh);
			case 'x':
				head += member->length;
				break;
			case 's':
				strncpy(va_arg(args, char *), head, member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

void fluff_struct_unpack_vn(
		struct FluffStruct * self, void * buf, va_list args){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, char);
			CAST_CASE('b', int8_t, signed char);
			CAST_CASE('B', uint8_t, unsigned char);
			STD_CASE('h', FluffShort, short, fluff_short_ftoh);
			STD_CASE('H', FluffUShort, unsigned short, fluff_ushort_ftoh);
			STD_CASE('i', FluffInt, int, fluff_int_ftoh);
			STD_CASE('I', FluffUInt, unsigned int, fluff_uint_ftoh);
			STD_CASE('q', FluffLong, long, fluff_long_ftoh);
			STD_CASE('Q', FluffULong, unsigned long, fluff_ulong_ftoh);
			STD_CASE('f', FluffFloat, float, fluff_float_ftoh);
			STD_CASE('d', FluffDouble, double, fluff_double_ftoh);
			case 'x':
				head += member->length;
				break;
			case 's':
				strncpy(va_arg(args, char *), head, member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

#undef STD_CASE
#define STD_CASE(C, D, S, F)                                          \
		case C:                                                       \
			*((S *)(struct_ + member->offset)) = F(*((D *) head));    \
			head += sizeof(D);                                        \
			break;

#undef CAST_CASE
#define CAST_CASE(C, D, S)                                             \
		case C:                                                        \
			*((S *)(struct_ + member->offset)) = (S) *((D *) head);    \
			head += sizeof(D);                                         \
			break;

void fluff_struct_unpack_s(
		struct FluffStruct * self, void * buf, void * struct_){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, char);
			CAST_CASE('b', int8_t, int8_t);
			CAST_CASE('B', uint8_t, uint8_t);
			STD_CASE('h', FluffShort, int16_t, fluff_short_ftoh);
			STD_CASE('H', FluffUShort, uint16_t, fluff_ushort_ftoh);
			STD_CASE('i', FluffInt, int32_t, fluff_int_ftoh);
			STD_CASE('I', FluffUInt, uint32_t, fluff_uint_ftoh);
			STD_CASE('q', FluffLong, int64_t, fluff_long_ftoh);
			STD_CASE('Q', FluffULong, uint64_t, fluff_ulong_ftoh);
			STD_CASE('f', FluffFloat, float, fluff_float_ftoh);
			STD_CASE('d', FluffDouble, double, fluff_double_ftoh);
			case 'x':
				head += member->length;
				break;
			case 's':
				strncpy(
					((char *) struct_ + member->offset),
					head,
					member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}

void fluff_struct_unpack_sn(
		struct FluffStruct * self, void * buf, void * struct_){
	int i;
	struct Member * member;
	void * head = buf;

	for (i = 0; i < self->nmemb; ++i){
		member = self->members + i;
		switch (member->format) {
			CAST_CASE('c', uint8_t, char);
			CAST_CASE('b', int8_t, signed char);
			CAST_CASE('B', uint8_t, unsigned char);
			STD_CASE('h', FluffShort, short, fluff_short_ftoh);
			STD_CASE('H', FluffUShort, unsigned short, fluff_ushort_ftoh);
			STD_CASE('i', FluffInt, int, fluff_int_ftoh);
			STD_CASE('I', FluffUInt, unsigned int, fluff_uint_ftoh);
			STD_CASE('q', FluffLong, long, fluff_long_ftoh);
			STD_CASE('Q', FluffULong, unsigned long, fluff_ulong_ftoh);
			STD_CASE('f', FluffFloat, float, fluff_float_ftoh);
			STD_CASE('d', FluffDouble, double, fluff_double_ftoh);
			case 'x':
				head += member->length;
				break;
			case 's':
				strncpy(
					((char *) struct_ + member->offset),
					head,
					member->length);
				head += member->length;
				break;
			default:
				break; // Never reached
		}
	}
}
