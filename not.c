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
#include <string.h>

#include "st.h"

// See if the current term excludes the current match.
static int
term_exclude(match_entry_t *m, char *content,  off_t len, search_term_t *term,
			 st_match_type_t exact)
{
	int i;
	
	// The algorithm is to notice whether the current match contains the end of
	// the current exclusion term.   If it does, then backtrack and see if it
	// also contains the beginning of the exclusion term.

	// looking for "YEM PA" NEAR "DGOS PA"
	// match: "G.|YEM PA"  (| is where the match starts)
	// exclude: "G.YEM"
	// result: excluded (backtrack matched)
	// match: " |YEM PA"
	// exclude: "G.YEM"
	// result: not excluded (backtrack didn't match)
	// match: " |DGOS PA"
	// exclude: "G.YEM"
	// result: not excluded (backtrack didn't happen)

	// What about (match "MMMMM|MM" "MMMMMM"

	// Look for a match with a substring of the exclude term
	for (i = 0; i < term->len; i++) {
		// If in order to get a successful submatch we have to go off the end of
		// the string, the term definitely doesn't match.
		if (m->offset + term->len - i <= len) {
			if (exact == match_exactly) {
				if (!memcmp(content + m->offset, &term->buf[i], term->len - i)) {
					break;
				}
			} else if (exact == match_ignores_spaces) {
				if (cmp(content + m->offset, &term->buf[i],
						 term->len - i, &term->buf[term->len])) {
					break;
				}
			} else {
				if (casecmp(content + m->offset, &term->buf[i],
							 term->len - i, &term->buf[term->len])) {
					break;
				}
			}				
		}
	}
	// Didn't match...
	if (i == term->len)
		return 0;
	// This was a dumb search (but-not '(A) '(A)), but nevertheless we allow it.
	if (i == 0)
		return 1;
	// Not enough characters in the search file for us to backtrack to the beginning
	// of the exclude term.
	if (i > m->offset)
		return 0;

	// The offset of the suffix in the search term tells us where in the search buffer
	// to look for the match.
	if (exact == match_exactly) {
		if (!memcmp(content + m->offset - i, &term->buf[0], term->len)) {
			return 1;
		}
	} else if (exact == match_ignores_spaces) {
		if (cmp(content + m->offset - i, &term->buf[0], 
				term->len, &term->buf[term->len])) {
			return 1;
		}
	} else {
		if (casecmp(content + m->offset - i, &term->buf[0],
					term->len, &term->buf[term->len])) {
			return 1;
		}
	}				
	return 0;
}

// Recursively descend down an expression tree looking for terms that match
// the current match entry.   Return true if one is found.   Only ste_or,
// ste_and and ste_term make sense in this subtree.

static int
match_exclude(match_entry_t *m, char *content,  off_t len, st_expr_t *expr,
			  st_match_type_t exact)
{
	int lhs, rhs;
	
	switch(expr->type) {
    case ste_not:
    case ste_near:
    case ste_near_lines:
    case ste_matchset:
		printf("unexpected expr type in not rhs expression: %d\n",
			   expr->type);
		return 0;

    case ste_and:
		lhs = match_exclude(m, content, len, expr->subexpr.exprs[0], exact);
		rhs = match_exclude(m, content, len, expr->subexpr.exprs[1], exact);
		return lhs && rhs;

    case ste_or:
		lhs = match_exclude(m, content, len, expr->subexpr.exprs[0], exact);
		rhs = match_exclude(m, content, len, expr->subexpr.exprs[1], exact);
		return lhs || rhs;

    case ste_term:
		return term_exclude(m, content, len, expr->subexpr.term, exact);
		break;
	}
}

// Do a recursive descent down the tree looking for but-not expressions.
// For each but-not expression, check that the LHS is a matchset; iterate
// through that matchset eliminating any entries that match the exclusions.

/* Given a but-not expression, apply the exclusions on the RHS to the matchset
 * on the LHS, replacing the but-not expression with the new matchset.  Return
 * 1 on success, 0 on failure (XXX?).
 */

void
do_exclude(st_expr_t *expr, char *contents, off_t len, st_match_type_t exact)
{
	int i, j;
	matchset_t *ms;
	st_expr_t *lhs, *rhs;

	ms = expr->subexpr.exprs[0]->subexpr.set;

	printf("ste_not and tanaka at do_exclude()\n");
	// We evaluate the LHS, and then exclude anything that matches the RHS.
	if (expr->subexpr.exprs[0]->type != ste_matchset) {
		printf("lhs of ste_not is not a matchset (%d)\n",
			   expr->subexpr.exprs[0]->type);
		return;
	}

	j = 0;
	for (i = 0; i < ms->count; i++)
	{
		// See if this one is excluded; if so, eliminate it from
		// the matchset by moving everything down one slot.  The
		// moving-down occurs on non-excluded matches; all that is
		// required to eliminate a match is that we don't increment
		// j.
		if (!match_exclude(&ms->m[i], contents, len, expr->subexpr.exprs[1], exact)) {
			if (i != j) {
				ms->m[j] = ms->m[i];
			}
			j++;
		}
	}		
	// Update the count (if nothing matched this won't change the count).
	ms->count = j;

	// Steal the matchset from the lhs, free the subexpressions, and change
	// this expression into a matchset.
	lhs = expr->subexpr.exprs[0];
	rhs = expr->subexpr.exprs[1];
	expr->type = ste_matchset;
	expr->subexpr.exprs[1] = 0;
	expr->subexpr.exprs[0] = 0;
	expr->subexpr.set = ms;
	lhs->subexpr.set = 0;
	free_expr(lhs);
	free_expr(rhs);
}

