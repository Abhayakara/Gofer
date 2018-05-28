/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * searchfile.c
 *
 * Look through a file, finding instances of each search term specified.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "st.h"

/* Compare the pattern, a, against the input buffer, b, up to len characters,
 * ignoring excess whitespace in the input buffer, and not going off the
 * end of the buffer.   Return zero if the whole string doesn't match;
 * otherwise return the length of the part of the buffer that matched, which,
 * because we're skipping excess whitespace, could be longer than the length
 * of the pattern we're matching against.
 *
 * We should optimize the heck out of this subroutine, but right now it's been
 * done in a mostly straightforward way, so is probably quite slow.
 */

int
cmp(const char *a, const char *b, size_t len, const char *limit)
{
	const char *search_start = b;
	while (len)
	{
		/* If we hit a space in the comparison string, let that space match
		 * any nonzero amount of space in the input.
		 */
		if (isascii(*a) && isspace(*a) && isascii(*b) && isspace(*b))
		{
			int matched = 0;

			/* Skip over the space. */
			while (isascii(*b) && isspace(*b) && b < limit)
			{
				matched = 1;
				++b;
			}

			/* If there's no longer space, we can't do a match. */
			if (!matched || limit - b + 1 < len)
				return 0;
		}
		else
		{
			if (*a != *b)
			{
				return 0;
			}
			++b;
		}
		++a; --len;
	}
	return (int)(b - search_start);
}

static int
cmp_exact(const char *a, const char *b, size_t len, const char *limit)
{
	const char *search_start = b;
	while (len)
	{
		if (*a != *b)
		{
			return 0;
		}
		++b; ++a; --len;
	}
	return (int)(b - search_start);
}

int
casecmp(const char *a, const char *b, size_t len, const char *limit)
{
    const char *search_start = b;
    while (len)
	{
		/* If we hit a space in the comparison string, let that space match
		 * any nonzero amount of space in the input.   Also let it match any
		 * ACIP page markers or tseks in the input.
		 */
		if (isascii(*a) && isspace(*a))
		{
			int inpage = 0;
			int matched = 0;
			int first;
			int inskip = 0;
            
			if (b == search_start)
				first = 1;
			else
				first = 0;
            
			/* Skip over the space. */
			while (b < limit)
			{
				if (inskip)
				{
					if (*b == '}')
						inskip = 0;
				} else {
					if (*b == '{')
					{
						inskip = 1;
					}
					if (*b == '@')
					{
						inpage = 1;
					}
					else if (isascii(*b) && isspace(*b))
					{
						inpage = 0;
						matched++;
						if (first && matched > 1)
							return 0;
					}
					else if (*b == ',' || *b == '*')
					{
						inpage = 0;
					}
					else if (inpage && isascii(*b) && isdigit(*b))
						;
					else if (inpage && *b >= 'A' && *b <= 'Z')
						inpage = 0;
					else
						break;
				}
				++b;
			}
			/* If there's no longer space, we can't do a match. */
			if (!matched || limit - b + 1 < len)
				return 0;
		}
		else
		{
			if (isascii(*a) && isascii(*b))
			{
				if (tolower(*a) != tolower(*b))
				{
					return 0;
				}
			}
			else if (*a != *b)
			{
				return 0;
			}
			++b;
		}
		++a; --len;
	}
//    printf("match: |%.*s|\n", (int)(b - search_start), search_start);
    return (int)(b - search_start);
}

int
searchfile(const char *filename, search_term_t *terms, int nterms,
		   const char *file, off_t flen, st_match_type_t exact)
{
	int ti;
	const char *fp;
	const char *fmax;
	size_t left;
	int line = 1;
	const char *linec;
	int length;

	fmax = file + flen;
	fp = linec = file;

	/* Pre-allocate match buffers for all the search terms, so that we don't
	 * stall the pipeline for every initial match of a search term.
	 * (Right now the code probably is stalling the pipeline all over the
	 * place, but at some point we want to go over it and try to make it
	 * not do that...)
	 */
	for (ti = 0; ti < nterms; ti++)
		new_st_matchbuf(&terms[ti]);
  

	/* While there are characters left in the file, search it, and by the way,
	 * remember how many characters are left.
	 */
	while ((left = fmax - fp))
	{
		/* Count lines.   Long lines get broken automatically at 80 characters */
		if (*fp == '\n' || fp - linec > 80)
		{
			if (fp - linec > 90)
				return 0;

			linec = fp;
			++line;
		}

		/* Go through the list of search terms seeing if we have a match for
		 * any of them.   Remember the matches we find.
		 */
#define CMPLOOP(comparator)												\
		for (ti = 0; ti < nterms; ti++)									\
		{																\
			/* If we have a match, remember it. */						\
			if (terms[ti].len <= left &&								\
				(length = comparator(terms[ti].buf, fp, terms[ti].len, fmax))) \
			{															\
				if (terms[ti].curmatch == STM_LIMIT)					\
				{														\
					new_st_matchbuf(&terms[ti]);						\
					terms[ti].curmatch = 0;								\
				}														\
				terms[ti].matches->m[terms[ti].curmatch].offset = fp - file; \
				terms[ti].matches->m[terms[ti].curmatch].unicode_offset = fp - file; \
				terms[ti].matches->m[terms[ti].curmatch].length = length; \
				terms[ti].matches->m[terms[ti].curmatch].unicode_length = length; \
				terms[ti].matches->m[terms[ti].curmatch].line = line;	\
				terms[ti].curmatch++;									\
			}															\
		}

		switch(exact)
		{
		case match_ignores_spaces:
			CMPLOOP(cmp);
			break;
		case match_ignores_spaces_and_case:
			CMPLOOP(casecmp);
			break;
		default:
			CMPLOOP(cmp_exact);
		}
		++fp;
	}
	return 1;
}

/* Allocate a new match buffer for a search term. */

void
new_st_matchbuf(search_term_t *st)
{
	st_match_t *new;

	/* Allocate the new match buffer. */
	new = malloc(sizeof (st_match_t));
	if (!new)
		gofer_fatal("no memory for st_match_t!");
	memset(new, 0, sizeof *new);

	/* Stash the new match buffer at the beginning of the list. */
	new->next = st->matches;
	st->matches = new;
}

/* Local Variables:  */
/* mode:C */
/* end: */
