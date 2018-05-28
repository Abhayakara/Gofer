/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * main.c
 * 
 * The main function for gofer.
 */

/* Copyright (c) 2003, 2018 Edward W. Lemon III
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "st.h"
#include "lex.h"

static void
dump_range(void *obj, const char *filename, char *contents, off_t content_length,
		   off_t first_line, off_t last_line, off_t first_char, off_t first_len,
		   off_t last_char, off_t last_len, off_t *cur_line, off_t *cur_char);

int
main(int argc, char **argv)
{
	char inbuf[1024];
	char *s;
	int len;
	parse_t *cfile;
	st_expr_t *root;
	int n;
	search_term_t *terms;
	int i;
	int nosearch = 0;

	printf("Please enter your search expression below.   Type 'end' to end.\n");
	s = inbuf;
	while (!feof(stdin) && s - inbuf < sizeof inbuf)
    {
		printf("> ");
		fflush(stdout);
		if (fgets(s, (sizeof inbuf) - (s - inbuf), stdin) == NULL)
			break;
		if (!strcmp(s, "end\n"))
		{
			*s = 0;
			break;
		}
		len = strlen(s);
		s += len;
    }

	if (!new_parse(&cfile, -1, inbuf, strlen(inbuf), "stdin", 0))
		gofer_fatal("Unable to start parse.");

	root = parse(cfile, 1);
	if (!root)
		gofer_fatal("Parse failed.");

	/* Come up with a flat search term array from the expression we parsed. */
	n = extract_search_terms(&terms, root);

	/* This is a hack - ditch it ASAP and just recursively descend the
	 * file tree.
	 */
	for (i = 1; i < argc; i++)
    {
		if (!strcmp(argv[i], "-n"))
		{
			nosearch = 1;
		}
		else
			search_tree(argv[i], root, terms, n, dump_range, 0, 0, nosearch,
						match_ignores_spaces_and_case);
    }
	free_expr(root);
	free(terms);

	return 0;
}

/* Dump the lines starting with first_line and ending with last_line,
 * using cur_line and cur_char as caches so that we don't have to scan
 * the whole file from the beginning every time.
 */
static void
dump_range(void *obj, const char *filename, char *contents, off_t max,
		   off_t first_line, off_t last_line, off_t first_char, off_t first_len,
		   off_t last_char, off_t last_len, off_t *cur_line, off_t *cur_char)
{
	off_t cl = *cur_line;
	off_t cc  = *cur_char;
#if 0
	off_t tmp;
	if (first_line > last_line)
    {
		tmp = first_line;
		first_line = last_line;
		last_line = tmp;
    }
#endif

	/* Search the file for the specified line. */
	while (cl < first_line)
    {
		while (contents[cc] != '\n' && cc != max)
			++cc;
		if (contents[cc] == '\n' && cc != max)
			++cc;
		++cl;
    }
      
	/* Remember start of first_line. */
	*cur_line = cl;
	*cur_char = cc;

	/* Now find the start of the line after the last line specified. */
	while (cl < last_line + 1)
    {
		while (contents[cc] != '\n' && cc != max)
			++cc;
		if (contents[cc] == '\n' && cc != max)
			++cc;
		++cl;
    }
      
	/* Now print it out. */
	printf("File %s, lines %lld-%lld:\n", filename, first_line, last_line);
	fwrite(&contents[*cur_char], cc - *cur_char, 1, stdout);
	printf("\n");
}

/* Local Variables:  */
/* mode:C */
/* end: */
