/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * near.c
 *
 * Once a search result tree has been combined, process any "near"
 * subexpressions.
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

#include "st.h"


match_pair_t *
eval_near(st_expr_t **ep)
{
	st_expr_t *expr = *ep;
	match_pair_t *rv = 0;

	if (!expr)
		return 0;

	switch(expr->type)
    {
    case ste_and:
		/* At some point we should make it so that an "and" combining two
		 * st_combineset results makes the file valid if both combinesets
		 * have contents.   Right now, we're not allowing "and" in this
		 * context.
		 */
		break;

    case ste_not:
		/* We should never get a "not" in this context. */
		break;

    case ste_or:
		/* At some point we should make it so that an "or" here combines two
		 * combinesets.   Not too hard, do it soon.
		 */
		break;

    case ste_near:
    case ste_near_lines:
		/* Both sides of a "near" expression should be matchsets.
		 * We don't check for impossible combinations here - the parser
		 * should have prevented them from happening.
		 */
		rv = compute_near(expr);
		break;

    case ste_term:
    case ste_matchset:
		break;
    }
	return rv;
}

/* Given two positive matchsets, and optionally one or two negative matchsets,
 * find matches that are near each other (from the two positive sets) and
 * create a combineset that lists all the cases of nearness.   Then go through
 * any negative matchsets and remove instances from the combineset that
 * are near instances in the negative matchsets.
 */

match_pair_t *
compute_near(st_expr_t *expr)
{
	matchset_t *linc, *rinc;
	match_pair_t *rv = 0, *tmp;
	int lix = 0;
	int rix = 0;

	if (expr->subexpr.exprs[0] == NULL) {
		return NULL;
	}

	if (expr->subexpr.exprs[1] == NULL) {
		return NULL;
	}

	if (expr->subexpr.exprs[0]->type != ste_matchset ||
		expr->subexpr.exprs[1]->type != ste_matchset) {
		printf("compute_near received non-matchset exprs: %d %d\n",
			   expr->subexpr.exprs[0]->type != ste_matchset,
			   expr->subexpr.exprs[1]->type != ste_matchset);
		return NULL;
	}
	
	linc = expr->subexpr.exprs[0]->subexpr.set;
	rinc = expr->subexpr.exprs[1]->subexpr.set;

	while (lix < linc->count && rix < rinc->count)
    {
		off_t diff, cdiff;
     
		/* Diff in char positions. */
		cdiff = linc->m[lix].offset - rinc->m[rix].offset;

		/* Diff in line positions. */
		if (expr->type == ste_near)
			diff = cdiff;
		else
			diff = linc->m[lix].line - rinc->m[rix].line;

		/* We want the absolute difference. */
		if (diff < 0)
			diff = -diff;
		if (cdiff < 0)
			cdiff = -cdiff;

		/* If the difference in character position between the two
		 * positions is less than the specified limit, the two positions
		 * are "near" one another, so we want to add them to the combineset.
		 * Special case: if the two matches start in the same place, don't
		 * consider them nearby, because it's really the same match.
		 */
		if (diff <= expr->n)
		{
			if (cdiff != 0)
			{
				tmp = malloc(sizeof (match_pair_t));
				if (!tmp)
					gofer_fatal("no memory for combineset.");
	      
				/* Remember the character and line positions. */
				if (linc->m[lix].offset < rinc->m[rix].offset)
				{
					tmp->pair[0] = linc->m[lix];
					tmp->pair[1] = rinc->m[rix];
				}
				else
				{
					tmp->pair[0] = rinc->m[rix];
					tmp->pair[1] = linc->m[lix];
				}

				/* Add it to the linked list. */
				tmp->next = rv;
				rv = tmp;

				/* Skip to the next set of matches. */
				lix++;
				rix++;
			}
			else {
				// XXX put the code to do BUT NOT here?
				goto advance;
			}
		}
		else
		{
			/* Advance the earliest of the two positions, so that we can
			 * redo the comparison with the later of the two against the
			 * next position on the other side.
			 *
			 * XXX I *think* this has the effect that every position will
			 * XXX be properly compared against every other positition that
			 * XXX would be a good candidate, without doing an O(N^2)
			 * XXX comparison between the two arrays.   But I haven't really
			 * XXX examined this carefully - it's just my gut feeling.
			 * XXX It would be worth thinking about this some more.
			 */
		advance:
			if (linc->m[lix].offset < rinc->m[rix].offset)
				lix++;
			else
				rix++;
		}
    }
	return merge_combineset(rv);
}

/* Reverse a linked list of match_pair_t's. */

match_pair_t *
reverse_combineset(match_pair_t *cs, match_pair_t *prev)
{
	match_pair_t *cur;
	if (!cs)
		return prev;
	cur = reverse_combineset(cs->next, cs);
	cs->next = prev;
	return cur;
}

match_pair_t *
merge_combineset(match_pair_t *cs)
{
	match_pair_t *cp = cs, *next;

	while (cp && cp->next)
    {
		off_t a = cp->pair[0].line;
		off_t b = cp->pair[1].line;
		off_t c = cp->next->pair[0].line;
		off_t d = cp->next->pair[1].line;

		/* If either endpoint of the current set is within the next set,
		 * merge the two sets.
		 */
		/* In order for there to be an overlap, it has to be true that
		 * either c <= a <= d or c <= b <= d or a <= c <= b or a <= d <= b.
		 */
		if ((c <= a && a <= d) ||
			(c <= b && b <= d) ||
			(a <= c && c <= b) ||
			(a <= d && d <= b))
		{
			// XXX this seems wrong: why are we changing the line number but not
			// the character positions?   Preserving behavior for now. 
			/* Pick the lower of the two low numbers. */
			if (c < a)
				cp->pair[0].line = c;
			/* Pick the higher of the two high numbers. */
			if (d > b)
				cp->pair[1].line = d;
			next = cp->next->next;
			free(cp->next);
			cp->next = next;
		}
		else
			cp = cp->next;
    }
	return cs;
}

/* Local Variables:  */
/* mode:C */
/* end: */
