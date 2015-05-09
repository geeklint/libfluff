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

#include "traceback.h"

struct Traceback {
	char * name;
	char * message;
	struct Traceback * prev;
};

/*
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData tb_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_system;
	}
	tb_size = MM->f_type_new(sizeof(struct Traceback));
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

void fluff_traceback_setmm(const struct FluffMM * mm){
	if (!mm_need_setup){
		MM->f_type_free(tb_size);
		mm_need_setup = 1;
	}
	MM = mm;
	setup_mm();
}

/*
 * Traceback
 */

static unsigned long tb_paused = 0;

static struct Traceback tb_base_data;

static struct Traceback * tb_base = &tb_base_data;

static struct Traceback * tb_current = &tb_base_data;

static struct Traceback * tb_first = NULL;

int fluff_traceback_grab(char * name){
	struct Traceback * tb;

	if (tb_paused){
		return 0;
	}
	if (tb_first == tb_current){
		return -1;
	}
	tb_first = tb_current;
	tb_first->name = name;
	if ((tb = MM->f_alloc(tb_size))){
		tb->prev = tb_current;
		tb_current = tb;
		return 0;
	} else {
		tb_first->message = "failed to grab traceback";
		return -1;
	}
}

void fluff_traceback_set(char * message){
	if (!tb_paused){
		tb_first->message = message;
	}
}

void fluff_traceback_drop(){
	struct Traceback * tb, * to_del;

	if (!tb_paused){
		tb = tb_current;
		while (tb != tb_base){
			to_del = tb;
			tb = tb->prev;
			MM->f_free(to_del);
		}
		tb_current = tb_base;
		tb_first = NULL;
	}
}

void fluff_traceback_pause(){
	tb_paused += 1;
}

void fluff_traceback_resume(){
	tb_paused -= 1;
}

int fluff_exception_tb_print(FILE * stream){
	struct Traceback * tb;

	tb = tb_first;
	while (tb != NULL){
		if (fprintf(stream, "%s: %s\n", tb->name, tb->message) < 0){
			return -1;
		}
		tb = tb->prev;
	}
	return 0;
}

