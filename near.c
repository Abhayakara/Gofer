/* near.c
 *
 * Once a search result tree has been combined, process any "near"
 * subexpressions.
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
#include <stdio.h>

#include "st.h"


combineset_t *
eval_near(st_expr_t **ep)
{
  st_expr_t *expr = *ep;
  matchset_t *lex, *rex;	/* Exclusion sets. */
  matchset_t *linc, *rinc;	/* Inclusion sets. */
  combineset_t *rv = 0;

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

    case ste_near:
    case ste_near_lines:
      /* There's two possibilities for either side of the near expression.
       * The first is that there's just a matchset.   The second is that
       * there's an "and" subexpression, one side of which is a "not"
       * subexpression, and the other side of which is a matchset.   So
       * we should always have a matchset on both sides, and may have
       * a second matchset on either side that's supposed to exclude
       * something.
       *
       * We don't check for impossible combinations here - the parser
       * should have prevented them from happening.
       */
      rex = lex = linc = rinc = 0;
      if (expr->subexpr.exprs[0] &&
	  expr->subexpr.exprs[0]->type == ste_and)
	{
	  st_expr_t *left, *right;
	  left = expr->subexpr.exprs[0]->subexpr.exprs[0];
	  right = expr->subexpr.exprs[0]->subexpr.exprs[1];

	  if (left && left->type == ste_not)
	    {
	      if (left->subexpr.expr &&
		  left->subexpr.expr->type == ste_matchset)
		lex = left->subexpr.expr->subexpr.set;
	    }
	  else if (left && left->type == ste_matchset)
	    linc = left->subexpr.set;

	  if (right && right->type == ste_not)
	    {
	      if (right->subexpr.expr &&
		  right->subexpr.expr->type == ste_matchset)
		lex = right->subexpr.expr->subexpr.set;
	    }
	  else if (right && right->type == ste_matchset)
	    linc = right->subexpr.set;
	}
      else if (expr->subexpr.exprs[0] &&
	       expr->subexpr.exprs[0]->type == ste_matchset)
	linc = expr->subexpr.exprs[0]->subexpr.set;

      if (expr->subexpr.exprs[1] &&
	  expr->subexpr.exprs[1]->type == ste_and)
	{
	  st_expr_t *left, *right;
	  left = expr->subexpr.exprs[1]->subexpr.exprs[0];
	  right = expr->subexpr.exprs[1]->subexpr.exprs[1];

	  if (left && left->type == ste_not)
	    {
	      if (left->subexpr.expr &&
		  left->subexpr.expr->type == ste_matchset)
		rex = left->subexpr.expr->subexpr.set;
	    }
	  else if (left && left->type == ste_matchset)
	    rinc = left->subexpr.set;

	  if (right && right->type == ste_not)
	    {
	      if (right->subexpr.expr &&
		  right->subexpr.expr->type == ste_matchset)
		rex = right->subexpr.expr->subexpr.set;
	    }
	  else if (right && right->type == ste_matchset)
	    rinc = right->subexpr.set;
	}
      else if (expr->subexpr.exprs[1] &&
	       expr->subexpr.exprs[1]->type == ste_matchset)
	rinc = expr->subexpr.exprs[1]->subexpr.set;

      rv = compute_near(expr, lex, linc, rex, rinc);
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

combineset_t *
compute_near(st_expr_t *expr,
	     matchset_t *lex, matchset_t *linc,
	     matchset_t *rex, matchset_t *rinc)
{
  combineset_t *rv = 0, *tmp;
  int lix = 0;
  int rix = 0;

  if (!linc || !rinc)
    return 0;

  while (lix < linc->count && rix < rinc->count)
    {
      int diff;
     
      if (expr->type == ste_near)
	/* Diff in char positions. */
	diff = linc->data[lix] - rinc->data[rix];
      else
	/* Diff in line positions. */
	diff = linc->data[lix + 2] - rinc->data[rix + 2];

      /* We want the absolute difference. */
      if (diff < 0)
	diff = -diff;

      /* If the difference in character position between the two
       * positions is less than the specified limit, the two positions
       * are "near" one another, so we want to add them to the combineset.
       * Special case: if the two matches start in the same place, don't
       * consider them nearby, because it's really the same match.
       */
      if (diff <= expr->n && diff != 0)
	{
	  tmp = malloc(sizeof (combineset_t));
	  if (!tmp)
	    gofer_fatal("no memory for combineset.");

	  /* Remember the character and line positions. */
	  if (linc->data[lix] < rinc->data[rix])
	    {
	      tmp->cp[0] = linc->data[lix];
	      tmp->cp[1] = rinc->data[rix];
	      tmp->cp[2] = linc->data[lix + 1];
	      tmp->cp[3] = rinc->data[rix + 1];
	      tmp->lp[0] = linc->data[lix + 2];
	      tmp->lp[1] = rinc->data[rix + 2];
	    }
	  else
	    {
	      tmp->cp[0] = rinc->data[rix];
	      tmp->cp[1] = linc->data[lix];
	      tmp->cp[2] = rinc->data[rix + 1];
	      tmp->cp[3] = linc->data[lix + 1];
	      tmp->lp[0] = rinc->data[rix + 2];
	      tmp->lp[1] = linc->data[lix + 2];
	    }

	  /* Add it to the linked list. */
	  tmp->next = rv;
	  rv = tmp;

	  /* Skip to the next set of matches. */
	  lix += 3;
	  rix += 3;
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
	  if (linc->data[lix] < rinc->data[rix])
	    lix += 3;
	  else
	    rix += 3;
	}
    }
  
  /* Now eliminate exclusions that are "near" the matches we found. */
  if (lex)
    rv = do_exclude(expr, rv, lex);
  if (rex)
    rv = do_exclude(expr, rv, rex);
  return merge_combineset(rv);
}

/* Given a combineset and a matchset, exclude all elements in the combineset
 * that are near elements in the matchset.   The matchset is sorted normally,
 * and the combineset is sorted in reverse.
 */

combineset_t *
do_exclude(st_expr_t *expr, combineset_t *orig, matchset_t *exclusions)
{
  int i, j;
  combineset_t **cp, *cs, *head = orig;

  /* The combineset is sorted in reverse order, so we'll just walk
   * backward through the exclusion set looking for instances of
   * proximity.
   */
  i = exclusions->count - 3;
  cp = &head;
  cs = *cp;

  while (i >= 0 && cs)
    {
      int diff;
     
      /* There are two positions in a combineset, and proximity
       * to either will result in elimination.
       */
      for (j = 0; j < 2; j++)
	{
	  if (expr->type == ste_near)
	    /* Diff in char positions. */
	    diff = cs->cp[j] - exclusions->data[i];
	  else
	    /* Diff in line positions. */
	    diff = cs->lp[j] - exclusions->data[i + 2];
	  
	  /* We want the absolute difference. */
	  if (diff < 0)
	    diff = -diff;

	  if (diff < expr->n)
	    {
	      *cp = cs->next;
	      free(cs);
	      cs = *cp;

	      /* We've advanced already, so skip the code that decides
	       * whether to advance.
	       */
	      goto next_while;
	    }
	}

      /* Decide whether to advance on the combineset, or on the exclusion
       * list.   This is complicated by the fact that the combineset
       * is two entries combined, but it turns out not to be a problem
       * because the only time it would be an issue would be a case where
       * it would have failed the nearness test, and we wouldn't have gotten
       * here.
       */

      if (cs->cp[0] > exclusions->data[i])
	{
	  cp = &cs->next;
	  cs = *cp;
	}
      else
	i -= 3;
    next_while:
      ;
    }

  return head;
}

/* Reverse a linked list of combineset_t's. */

combineset_t *
reverse_combineset(combineset_t *cs, combineset_t *prev)
{
  combineset_t *cur;
  if (!cs)
    return prev;
  cur = reverse_combineset(cs->next, cs);
  cs->next = prev;
  return cur;
}

combineset_t *
merge_combineset(combineset_t *cs)
{
  combineset_t *cp = cs, *next;

  while (cp && cp->next)
    {
      int a = cp->lp[0];
      int b = cp->lp[1];
      int c = cp->next->lp[0];
      int d = cp->next->lp[1];

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
	  /* Pick the lower of the two low numbers. */
	  if (c < a)
	    cp->lp[0] = c;
	  /* Pick the higher of the two high numbers. */
	  if (d > b)
	    cp->lp[1] = d;
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
/* c-file-style:"gnu" */
/* end: */
