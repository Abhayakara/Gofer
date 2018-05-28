/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * expr.c
 * 
 * Utility functions for expressions.
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
#include <string.h>
#include <stdio.h>

#include "st.h"

/* Recursively descend an expression tree, printing the contents of the
 * tree in a human-readable infix (lisp) notation.
 */

char *
print_expr(st_expr_t *expr)
{
	char *r, *l;
	char tbuf[256];
	char *rv;
	unsigned long len;

	switch(expr->type)
    {
    case ste_near_lines:
		sprintf(tbuf, "(near-lines %d ", expr->n);
    two:
		r = print_expr(expr->subexpr.exprs[0]);
		l = print_expr(expr->subexpr.exprs[1]);
		len = strlen(r) + strlen(l) + strlen(tbuf) + 3;
		rv = malloc(len);
		if (!rv)
			gofer_fatal("No memory for combined printed expr.");
		sprintf(rv, "%s%s %s)", tbuf, r, l);
		free(r);
		free(l);
		break;
        
    case ste_near:
		sprintf(tbuf, "(near ");
		goto two;
		break;
            
    case ste_or:
		sprintf(tbuf, "(or ");
		goto two;
		break;
        
    case ste_and:
		sprintf(tbuf, "(and ");
		goto two;
        
    case ste_not:
		sprintf(tbuf, "(but-not ");
		goto two;
        
    case ste_term:
		rv = malloc(expr->subexpr.term->len + 3);
		if (!rv)
			gofer_fatal("No memory for printed expr.");
		rv[0] = '"';
		memcpy(rv + 1, expr->subexpr.term->buf, expr->subexpr.term->len);
		rv[expr->subexpr.term->len + 1] = '"';
		rv[expr->subexpr.term->len + 2] = 0;
		break;

    case ste_matchset:
		sprintf(tbuf, "#<ste-matchset>");
		len = strlen(tbuf) + 1;
		rv = malloc(len);
		if (!rv)
			gofer_fatal("No memory for printed expr.");
		strcpy(rv, tbuf);
		break;

    default:
		return (char *)0;
    }
	return rv;
}

/* Recursively descend an expression tree, freeing everything we find.
 * If free_terms is zero, we don't free search terms.   This is needed
 * because when we copy an expression tree, we *don't* copy the search
 * terms.
 */

void
free_expr(st_expr_t *expr)
{
	switch(expr->type)
    {
    case ste_near_lines:
    case ste_near:
    case ste_or:
    case ste_and:
    case ste_not:
		if (expr->subexpr.exprs[0])
			free_expr(expr->subexpr.exprs[0]);
		if (expr->subexpr.exprs[1])
			free_expr(expr->subexpr.exprs[1]);
		break;

    case ste_term:
		/* We always free the matches, if there are any. */
		if (expr->subexpr.term && expr->subexpr.term->matches)
		{
			st_match_t *mp, *next;

			for (mp = expr->subexpr.term->matches; mp; mp = next)
			{
				next = mp->next;
				free(mp);
			}
			expr->subexpr.term->matches = 0;
			expr->subexpr.term->curmatch = 0;
		}
		/* We never actually free the terms here, because they should
		 * be in an array by now.
		 */
		break;

    case ste_matchset:
		if (expr->subexpr.set)
			free(expr->subexpr.set);
		break;
    }

	free(expr);
}

/* Make a copy of an expression tree. */

st_expr_t *
copy_expr(st_expr_t *expr)
{
	st_expr_t *new;

	/* If we're copying the null expression, the copy is also the null
	 * expression.
	 */
	if (!expr)
		return 0;

	new = malloc(sizeof (st_expr_t));
	if (!new)
		gofer_fatal("Can't copy expression");
	memset(new, 0, sizeof *new);
	new->type = expr->type;
	new->n = expr->n;

	switch(expr->type)
    {
    case ste_near_lines:
    case ste_near:
    case ste_or:
    case ste_and:
    case ste_not:
		new->subexpr.exprs[0] = copy_expr(expr->subexpr.exprs[0]);
		new->subexpr.exprs[1] = copy_expr(expr->subexpr.exprs[1]);
		break;

    case ste_term:
		new->subexpr.term = expr->subexpr.term;
		break;

    case ste_matchset:
		new->subexpr.set = expr->subexpr.set;
		break;
    }

	return new;
}

/* Worker function to count search terms in an expression tree. */

static int
count_search_terms(st_expr_t *expr)
{
	if (!expr)
		return 0;

	switch(expr->type)
    {
    case ste_near_lines:
    case ste_near:
    case ste_or:
    case ste_and:
		return (count_search_terms(expr->subexpr.exprs[0]) +
				count_search_terms(expr->subexpr.exprs[1]));
		break;

    case ste_not:
		return count_search_terms(expr->subexpr.exprs[0]);
		break;
		
    case ste_term:
		return 1;
		break;

    case ste_matchset:
		return 0;
		break;
    }
	return 0;
}

/* Worker function to flatten search terms out of an expression tree. */

static void
flatten_search_terms(search_term_t *terms, int *n, st_expr_t *expr)
{
	switch(expr->type)
    {
    case ste_near_lines:
    case ste_near:
    case ste_or:
    case ste_and:
		flatten_search_terms(terms, n, expr->subexpr.exprs[0]);
		flatten_search_terms(terms, n, expr->subexpr.exprs[1]);
		break;

		/*
		 * BUT NOT doesn't flatten the rhs of the expression, because we don't
		 * need to search the file for matches--we just exclude anything that
		 * matches the RHS.
		 */
		/*
		 * XXX think about how this is going to work if we have an
		 * expression like (near (but-not (match A B C) (match D)) (match F G H))
		 */
		/*
		 * I think it works correctly, because flatten_search_terms will have
		 * found all of those match terms.
		 */
    case ste_not:
		flatten_search_terms(terms, n, expr->subexpr.exprs[0]);
		break;

    case ste_term:
		/* We had allocated an individual term buffer here, but we want the
		 * terms all in an array, so we copy the original term buffer contents
		 * into the new term buffer, free the old buffer, and stash a pointer
		 * to the new buffer on the expression.
		 */
		terms[*n] = *expr->subexpr.term;
		free(expr->subexpr.term);
		expr->subexpr.term = &terms[*n];
		(*n)++;
		break;

    case ste_matchset:
		break;
    }
}

/* Given an expression tree, make a flat array containing all the search
 * terms that we can pass to searchfile().
 */

int
extract_search_terms(search_term_t **terms, st_expr_t *root)
{
	int count, cur = 0;
	search_term_t *tp;

	/* Figure out number of terms in expression. */
	count = count_search_terms(root);

	/* Allocate flattened search term buffer. */
	tp = malloc(count * sizeof (search_term_t));
	if (!tp)
		gofer_fatal("No memory to flatten search terms.");
	memset(tp, 0, count * sizeof (search_term_t));

	/* Now flatten the buffer. */
	flatten_search_terms(tp, &cur, root);
	if (cur != count)
		gofer_fatal("mismatch between cur and n!");

	/* Return the flattened buffer and the count. */
	*terms = tp;
	return count;
}

/* Local Variables:  */
/* mode:C */
/* end: */
