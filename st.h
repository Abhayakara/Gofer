/* st.h
 *
 * Search term structure definition.
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

#if !defined(ST_H)
#define ST_H

#define ST_LIMIT	256	/* Maximum length of a search term.   We
				 * pick a small number because we are
				 * optimizing for quick word matches,
				 * rather than trying to be totally general.
				 * The search term buffer is at the end of
				 * the struct, so we should probably just
				 * let it be arbitrarily long.
				 */
#define STM_LIMIT	20	/* Number of matches per st_match_t.
				 * We want this to be big enough that we
				 * almost never have to allocate a new
				 * match buffer, but small enough that
				 * we don't make the match buffer bigger
				 * than a cache line.  Obviously, something's
				 * got to give.
				 */

#ifdef __cplusplus
extern "C" {
#endif

/* Buffer containing matches found for a search term in a file.
 * We record the character position in the file, and also the line it
 * appeared on, so that we can do proximity searches based on line
 * distance.
 */

typedef struct stmbuf {
  struct stmbuf *next;
  int data[STM_LIMIT * 3];
} st_match_t;

/* Finalized set of matches after the search is complete and we are
 * combining (or have combined) subexpressions.
 */

typedef struct stmset {
  int count;
  int data[1];		/* We just allocate the right amount of space. */
} matchset_t;

/* Search term.  Search terms are just strings.  A search term match
 * buffer list is attached to the search term.  The match buffer list
 * is per-file - once we've searched a file, we process all the match
 * buffers for that file and then stash the results in our global
 * result table.
 */

typedef struct st {
  int len;			/* Length of string. */
  st_match_t *matches;		/* Places in the file where we got a match. */
  size_t curmatch;		/* Current index into active match buffer. */
  char buf[ST_LIMIT];		/* Search term string. */
} search_term_t;

/* Search term expression types. */

typedef enum {
  ste_and,			/* Two subexpressions are true. */
  ste_not,			/* The subexpression is not true. */
  ste_or,			/* Either subexpression is true. */
  ste_near,			/* The two subexprs are near each other. */
  ste_near_lines,		/* The two subexprs are within N lines. */
  ste_term,			/* The subexpr is a search term. */
  ste_matchset			/* The subexpr is a completed matchset_t. */
} st_expr_type_t;

/* Comparison types. */
typedef enum {
  match_ignores_spaces,
  match_exactly,
  match_ignores_spaces_and_case
} st_match_type_t;

/* Search expression node.   Search expression nodes combine search terms,
 * or sometimes just modify them.
 */

typedef struct sten {
  st_expr_type_t type;
  union {
    search_term_t *term;
    struct sten *expr;
    struct sten *exprs[2];
    matchset_t *set;
  } subexpr;
  int n;
} st_expr_t;

typedef struct stcs {
  struct stcs *next;
  int cp[4];
  int lp[2];
} combineset_t;

typedef void (*dumpfunc_t)(void *obj, const char *filename,
			   char *contents, int content_length,
			   int first_line, int last_line,
			   int first_char, int first_len,
			   int last_char, int last_len,
			   int *cur_line, int *cur_char);
typedef int (*filefunc_t)(void *obj, const char *filename);

/* Function declarations... */

/* searchfile.c */
int searchfile(const char *filename, search_term_t *terms, int nterms,
	       const char *file, int flen, st_match_type_t exact);
void new_st_matchbuf(search_term_t *st);
void combine(st_expr_t **ep);

/* near.c */
combineset_t *eval_near(st_expr_t **ep);
combineset_t * compute_near(st_expr_t *expr,
			    matchset_t *lex, matchset_t *linc,
			    matchset_t *rex, matchset_t *rinc);
combineset_t *do_exclude(st_expr_t *expr, combineset_t *orig,
			 matchset_t *exclusions);
combineset_t *reverse_combineset(combineset_t *cs, combineset_t *prev);
combineset_t *merge_combineset(combineset_t *cs);

/* combine.c */
void combine(st_expr_t **ep);
void term_to_matchset(st_expr_t **ep);
void combine_matchsets(st_expr_t **ep, st_expr_t **lep, st_expr_t **rep);

/* expr.c */
char *print_expr(st_expr_t *expr);
void free_expr(st_expr_t *expr);
st_expr_t *copy_expr(st_expr_t *expr);
int extract_search_terms(search_term_t **terms, st_expr_t *root);

/* tree.c */
extern int abortSearch;
int search_tree(const char *name, st_expr_t *root, search_term_t *terms,
		int n, dumpfunc_t func, filefunc_t filefunc, void *obj,
		int nosearchp, st_match_type_t exact);
int process_file(const char *filename, char *contents, int len,
		 st_expr_t *root, search_term_t *terms, int n,
		 dumpfunc_t func, void *obj, st_match_type_t exact);

/* These need to be defined somewhere... */
void gofer_fatal(const char * fmt, ... );
int gofer_error(const char * fmt, ...);
int gofer_info(const char *fmt, ...);
int gofer_debug(const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif

/* Local Variables:  */
/* mode:C */
/* c-file-style:"gnu" */
/* end: */
