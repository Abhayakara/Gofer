/* combine.c
 *
 * Once a file has been searched, combine the results into the final per-file
 * statistics.
 */

/* Copyright (c) 2003 Edward W. Lemon III
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

#include "st.h"

/* Recursively descend an expression tree and combine the results of
 * any expressions that can be combined.
 */

void
combine(st_expr_t **ep)
{
  st_expr_t *expr = *ep;

  switch (expr->type)
    {
      /* Combine the two subexpressions. */
    case ste_near:
    case ste_near_lines:
    case ste_and:
      combine(&expr->subexpr.exprs[0]);
      combine(&expr->subexpr.exprs[1]);
      break;
      
      /* Single subexpression. */
    case ste_not:
      if (expr->subexpr.expr)
	combine(&expr->subexpr.expr);
      else
	{
	  free_expr(expr);
	  *ep = 0;
	}
      break;

    case ste_or:
      combine(&expr->subexpr.exprs[0]);
      combine(&expr->subexpr.exprs[1]);
      if (expr->subexpr.exprs[0] &&
	  expr->subexpr.exprs[0]->type == ste_matchset &&
	  expr->subexpr.exprs[1] &&
	  expr->subexpr.exprs[1]->type == ste_matchset)
	combine_matchsets(ep,
			  &expr->subexpr.exprs[0],
			  &expr->subexpr.exprs[1]);
      else if (!expr->subexpr.exprs[0] &&
	       expr->subexpr.exprs[1] &&
	       expr->subexpr.exprs[1]->type == ste_matchset)
	{
	  *ep = expr->subexpr.exprs[1];
	  expr->subexpr.exprs[1] = 0;
	  free_expr(expr);
	}
      else if (!expr->subexpr.exprs[1] &&
	       expr->subexpr.exprs[0] &&
	       expr->subexpr.exprs[0]->type == ste_matchset)
	{
	  *ep = expr->subexpr.exprs[0];
	  expr->subexpr.exprs[0] = 0;
	  free_expr(expr);
	}

      break;

    case ste_term:
      term_to_matchset(ep);
      break;

    case ste_matchset:
      break;
    }
}

/* Finalize a linked list of matches into a matchset. */

void
term_to_matchset(st_expr_t **ep)
{
  st_expr_t *expr = *ep;
  search_term_t *term = (*ep)->subexpr.term;
  int count;
  st_match_t *mp, *next;
  matchset_t *ms;
  int len;

  /* Figure out how many matches there are. */
  count = term->curmatch;
  for (mp = term->matches->next; mp; mp = mp->next)
    count += STM_LIMIT * 3;

  if (count == 0)
    {
      free_expr(expr);
      *ep = 0;
      return;
    }

  /* Allocate the buffer. */
  ms = malloc((sizeof *ms) + count * (sizeof (int)));
  if (!ms)
    gofer_fatal("no memory for matchset.");

  ms->count = count;

  /* Copy the matches into the match buffer, starting at the end and
   * working backwards, because we made the linked list backward.
   */
  /* The first match buffer on the list can be incomplete - the amount
   * of data we need to copy out of that buffer is the value of
   * term->curmatch.
   */
  len = term->curmatch;
  for (mp = term->matches; mp; mp = next)
    {
      next = mp->next;
      count -= len;
      memcpy(&ms->data[count], mp->data, len * sizeof (int));
      /* Any subsequent matchsets after the first in the list will be full,
       * so set len to the length of a full matchset buffer.
       */
      len = STM_LIMIT * 3;
      free(mp);
      mp = 0;
    }

  term->matches = 0;
  term->curmatch = 0;

  /* We don't free the term here because this is a copy of the real
   * expression tree, but we do zap the pointer, just in case.
   */
  expr->subexpr.term = 0;

  /* Turn the expression into a matchset expression. */
  expr->type = ste_matchset;
  expr->subexpr.set = ms;

}

/* Given an expression containing two expressions that are matchsets, sort
 * the two matchsets together into a single matchset, free up the two
 * subexpressions and the two matchsets, and make the containing expression
 * a matchset expression instead of a joining expression.
 */

void
combine_matchsets(st_expr_t **ep, st_expr_t **lep, st_expr_t **rep)
{
  st_expr_t *expr = *ep;
  int count, ix, rix, lix;
  matchset_t *lm;
  matchset_t *rm;
  matchset_t *ms;

  lm = (*lep)->subexpr.set;
  rm = (*rep)->subexpr.set;

  /* Figure out how many matches there are. */
  count = lm->count + rm->count;

  /* Allocate the buffer. */
  ms = malloc((sizeof *ms) + count * (sizeof (int)));
  if (!ms)
    gofer_fatal("no memory for matchset.");
  memset(ms, 0, sizeof *ms);
  ms->count = count;

  /* Now mergesort the two matchsets into the combined matchset. */
  ix = rix = lix = 0;

  while (ix < count)	/* This should be safe unless the algorithm's wrong. */
    {
      if (rix == rm->count ||
	  lm->data[lix] < rm->data[rix]) {
	ms->data[ix] = lm->data[lix];
	ms->data[ix + 1] = lm->data[lix + 1];
	ms->data[ix + 2] = lm->data[lix + 2];
	lix += 3;
      } else {
	ms->data[ix] = rm->data[rix];
	ms->data[ix + 1] = rm->data[rix + 1];
	ms->data[ix + 2] = rm->data[rix + 2];
	rix += 3;
      }
      ix += 3;
    }

  /* Now get rid of the old matchsets. */
  free(lm);
  free(*lep);
  *lep = 0;
  free(rm);
  free(*rep);
  *rep = 0;

  /* Now make the expression that joined the two matchsets a matchset
   * expression.
   */
  expr->type = ste_matchset;
  expr->subexpr.set = ms;
}

/* Local Variables:  */
/* mode:C */
/* c-file-style:"gnu" */
/* end: */
