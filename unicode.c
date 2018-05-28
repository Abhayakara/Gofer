/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * unicode.c
 * 
 * Functions for searching a tree.
 */

/* Copyright (c) 2018 Edward W. Lemon III
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

/* Scan the file for unicode characters and, if any are encountered, validate them
 * for basic correctness.   If there are unicode characters and there are no errors,
 * return 1; otherwise return 0.   Errors means that although what we saw looked
 * like unicode, it is probably some other encoding, which we will leave alone
 * (e.g., ISO-8859-1 or something).
 */

int
is_unicode(char *contents, off_t len)
{
	unsigned char *cp = (unsigned char *)contents;
	unsigned char *eob = (unsigned char *)contents + len;
	int cplen = 1;
	int seen_uc = 0;

	do {
		if (*cp < 0x80)
			cplen = 1;
		else
		{
			if (*cp < 0xC0)
				return 0; // invalid
			else if (*cp < 0xE0)
				cplen = 2;
			else if (*cp < 0xF0)
				cplen = 3;
			else if (*cp >= 0xF8)
				return 0; // invalid
			if (cp + cplen > eob)
				return 0; // invalid
			seen_uc = 1;
		}
		cp = cp + cplen;
	} while (cp != eob);
	return seen_uc;
}

// Find the length in unicode characters of a UTF-8 string in curfile starting
// at start with length hunklen.
static int
uclen(char *contents, off_t len, off_t start, off_t hunklen)
{
	off_t bp;
	int ulen, cplen;
	unsigned char *cp = (unsigned char *)contents;
	
	bp = start;
	ulen = 0;

	// Scan the file starting from the specified offset
	while (bp != start + hunklen && bp != len)
	{
		if (cp[bp] < 0x80)
		{
			cplen = 1;
		}
		else
		{
			if (cp[bp] < 0xC0)
				gofer_fatal("coding error 2 in unicode_fixups");
			else if (cp[bp] < 0xE0)
				cplen = 2;
			else if (cp[bp] < 0xF0)
				cplen = 3;
			else if (cp[bp] >= 0xF8)
				cplen = 1; // should never happen, already validated.
			if (bp > len)
				gofer_fatal("coding error in unicode_fixups");
		}
		bp = bp + cplen;
		ulen++;
	}
	return ulen;
}

// Compare two points in the unicode fixups array on the basis of the file position
// they reference.
static int
compare_points(const void *ap, const void *bp)
{
	match_entry_t * const *a = ap;
	match_entry_t * const *b = bp;

	if ((*a)->offset < (*b)->offset)
		return -1;
	else if ((*b)->offset < (*a)->offset)
		return 1;
	else
		return 0;
}

void
unicode_fixups(char *contents, off_t len, search_term_t *st, int nterms)
{
	int i, j, k;
	int num_points = 0;
	st_match_t *mp;
	match_entry_t **points;
	unsigned char *cp = (unsigned char *)contents;

	off_t bp = 0, up = 0;
	int cplen = 1;

	// Count the number of matches.
	for (i = 0; i < nterms; i++)
	{
		// First matchset contains <curmatch> matches; subsequent ones are full to STM_LIMIT.
		num_points += st[i].curmatch;
		for (mp = st[i].matches; mp && mp->next; mp = mp->next)
		{
			num_points += STM_LIMIT;
		}
	}

	// Make an array of pointers to match points.
	points = malloc(num_points * sizeof *points);
	if (points == NULL)
	{
		gofer_fatal("no memory for unicode points.");
	}
	
	// Put the addresses of all the points into the array.
	k = 0;
	for (i = 0; i < nterms; i++)
	{
		int num = st[i].curmatch;
		for (mp = st[i].matches; mp; mp = mp->next)
		{
			for (j = 0; j < num; j++)
			{
				points[k++] = &mp->m[j];
			}
			num = STM_LIMIT;
		}
	}

	// Sort the array.
	qsort(points, num_points, sizeof *points, compare_points);

	i = 0;

	// Go through the file adjusting points based on unicode offsets.
	while (i < num_points && bp != len)
	{
		// Update all references to the current location in the file.
		while (i < num_points && points[i]->offset == bp)
		{
			points[i]->unicode_length = uclen(contents, len,
											  points[i]->offset, points[i]->length);
			points[i]->unicode_offset = up;
			i++;
		}
		if (cp[bp] < 0x80)
		{
			cplen = 1;
		}
		else
		{
			if (cp[bp] < 0xC0)
				gofer_fatal("coding error 1 in unicode_fixups");
			else if (cp[bp] < 0xE0)
				cplen = 2;
			else if (cp[bp] < 0xF0)
				cplen = 3;
			else if (cp[bp] >= 0xF8)
				cplen = 1; // should never happen, already validated.
			if (bp > len)
				gofer_fatal("coding error 2 in unicode_fixups");
		}
		// This would happen if the sort didn't work.
		if (i < num_points && bp + cplen > points[i]->offset)
			gofer_fatal("coding error 3 in unicode_fixups");
		bp = bp + cplen;
		up++;
	}

	// This would probably also happen if the sort didn't work.
	if (i < num_points)
		gofer_fatal("coding error 4 in unicode_fixups.");
	free(points);
}

/* Local Variables:  */
/* mode:C */
/* end: */
