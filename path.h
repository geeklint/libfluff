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

#ifndef FLUFF_PATH_H_
#define FLUFF_PATH_H_

#include "data.h"

/*
 * Functions below which take a (char * buf, size_t maxlen) and return an int
 * will return 0 on success and -1 on error, including copying a path longer
 * than maxlen.
 */

/*
 * Get current working directory
 */
int fluff_path_getcwd(char * buf, size_t maxlen);

/*
 * Get current user's home directory
 */
int fluff_path_gethome(char * buf, size_t maxlen);

/*
 * Get appropriate user-specific directory for containing application data for
 * and application named appname.
 */
int fluff_path_getappdata(char * buf, size_t maxlen, char * appname);

/*
 * Normalize a pathname by collapsing redundant separators and up-level
 * references
 */
int fluff_path_normpath(char * buf, size_t maxlen, char * path);

/*
 * Get a normalized absolutized version of the pathname
 */
int fluff_path_abspath(char * buf, size_t maxlen, char * path);

/*
 * Join two paths such that there is exactly one directory separator between
 * them.
 */
int fluff_path_join(char * buf, size_t maxlen, char * head, char * tail);

/*
 * Join one or more path components intelligently.
 */
int fluff_path_joinmany(
		char * buf, size_t maxlen, char ** parts, size_t nparts);

/*
 * Return 1 if path exists and 0 if not
 */
int fluff_path_exists(char * path);

/*
 * Return 1 if path exists and is a directory, 0 otherwise
 */
int fluff_path_isdir(char * path);

/*
 * Return 1 if path exists and is a file, 0 otherwise
 */
int fluff_path_isfile(char * path);

/*
 * Return an index into path which indicates the first character after the
 * last separator.
 */
unsigned long fluff_path_split(char * path);

/*
 * Create a directory.
 * Return 0 on success, -1 on failure
 */
int fluff_path_mkdir(char * path);

#endif /* FLUFF_PATH_H_ */
