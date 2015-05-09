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

#ifndef FLUFF_STREAM_H_
#define FLUFF_STREAM_H_

#include "data.h"

/*
 * Base stream interface
 */
struct FluffStreamBase {
	/*
	 * Extra data associated with stream
	 */
	union FluffData data;

	/*
	 * Closed indicator
	 */
	int closed;

	/*
	 * EOF indicator
	 */
	int eof;

	/*
	 * Error indicator
	 */
	int error;

	/*
	 * Close the stream
	 */
	void (*f_close)(struct FluffStream *);
};

/*
 * Read stream interface
 */
struct FluffReadStream {
	struct FluffStreamBase base;

	/*
	 * Read function. Attempt to read `length` bytes into the buffer
	 * Return the number of bytes read
	 */
	size_t (*f_read)(struct FluffStream *, void * buffer, size_t length);

};

struct FluffWriteStream {
	struct FluffStreamBase base;

	/*
	 * Write function. Attempt to write `length` bytes from the buffer
	 * Return the number of bytes written
	 */
	size_t (*f_write)(struct FluffStream *, void * buffer, size_t length);
};

/*
 * Open a disk file for reading
 * Return the new stream or NULL on failure
 */
struct FluffReadStream * fluff_stream_file_read(char * filename);

/*
 * Open a disk file for writing
 * Return the new stream or NULL on failure
 */
struct FluffReadStream * fluff_stream_file_write(char * filename);

#endif /* FLUFF_STREAM_H_ */
