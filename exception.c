#include "exception.h"

#include "mm.h"

struct FluffException {
	char * name;
	struct FluffException * parent;
};

struct Traceback {
	char * name;
	char * message;
	struct Traceback * prev;
};

/*
 * MM
 */
static struct FluffMMType * exception_size = NULL;
static struct FluffMMType * tb_size = NULL;

static void setup_mm_types(){
    exception_size = fluff_mm_new(sizeof(struct FluffException));
    tb_size = fluff_mm_new(sizeof(struct Traceback));
}

#define ENSURE_MM if (!exception_size) setup_mm_types();

/*
 * Exceptions
 */

static struct FluffException exception_base_data = {
	.name = "Base Exception",
	.parent = NULL,
};

struct FluffException * fluff_exception_base = &exception_base_data;

struct FluffException * fluff_exception_new(
		struct FluffException * parent, char * name){
	struct FluffException * self;

	ENSURE_MM;

	if ((self = fluff_mm_alloc(exception_size))){
		self->name = name;
		self->parent = parent;
	}

	return self;
}

static struct FluffException * current;

void fluff_exception_throw(struct FluffException * error){
	current = error;
}

int fluff_exception_catch(struct FluffException * catch){
	struct FluffException * test;

	test = current;
	while (test){
		if (test == catch){
			return 1;
		}
		test = test->parent;
	}
	return 0;
}

void fluff_exception_clear(){
	current = NULL;
}

/*
 * Traceback
 */

static struct Traceback tb_base_data;

static struct Traceback * tb_base = &tb_base_data;

static struct Traceback * tb_current = &tb_base_data;

static struct Traceback * tb_first = NULL;

int fluff_exception_tb_grab(char * name){
	struct Traceback * tb;

	if (tb_first == tb_current){
		return -1;
	}
	tb_first = tb_current;
	tb_first->name = name;
	if ((tb = fluff_mm_alloc(tb_size))){
		tb->prev = tb_current;
		tb_current = tb;
		return 0;
	} else {
		tb_first->message = "failed to allocate traceback";
		return -1;
	}
}

void fluff_exception_tb_set(char * message){
	tb_first->message = message;
}

void fluff_exception_tb_drop(){
	struct Traceback * tb, * to_del;

	tb = tb_current;
	while (tb != tb_base){
		to_del = tb;
		tb = tb->prev;
		fluff_mm_free(to_del);
	}
	tb_current = tb_base;
	tb_first = NULL;
}

int fluff_exception_tb_print(FILE * stream){
	struct Traceback * tb;

	tb = tb_first;
	while (tb){
		if (fprintf(stream, "%s: %s\n", tb->name, tb->message) < 0){
			return -1;
		}
		tb = tb->prev;
	}
	if (current){
		if (fprintf(stream, "%s\n", current->name) < 0){
			return -1;
		}
	}
	return 0;
}

