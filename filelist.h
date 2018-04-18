/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * filelist.h
 *
 * Definitions for filelist.c
 */

/* Copyright (c) 2004, 2005, 2018 Edward W. Lemon III
 *
 *  This file is part of GOFER.
 *
 *  GOFER is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GOFER is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GOFER, in a file called COPYING; if not, write
 *  to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307  USA
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct matchzone {
	off_t first_line;
	off_t last_line;
	off_t first_char;
	off_t first_len;
	off_t last_char;
	off_t last_len;
	off_t cur_line;
	off_t cur_char;
} matchzone_t;

typedef struct fileresult {
	char *filename;
	char *reducename;
	char *contents;
	off_t content_length;
	int nzones;
	int maxzones;
	int curzone;
	matchzone_t **matches;
} fileresults_t;

typedef struct filelist {
	int nfiles;
	int maxfiles;
	fileresults_t **files;
	int cur_file;
	int cur_file_seen;
	int cur_match_seen;
} filelist_t;
	
filelist_t *new_filelist(void);
void new_entry(filelist_t *dest,
			   const char *filename, char *contents, off_t content_length,
			   off_t first_line, off_t last_line,
			   off_t first_char, off_t first_len,
			   off_t last_char, off_t last_len,
			   off_t *cur_line, off_t *cur_char);
void filelist_free(filelist_t *fl);

#ifdef __cplusplus
}
#endif

/* Local Variables:  */
/* mode:C */
/* end: */
