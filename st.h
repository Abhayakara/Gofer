/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * st.h
 *
 * Search term structure definition.
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

typedef struct match_entry {
	off_t offset;
	off_t length;
	off_t line;
	off_t unicode_offset;
	off_t unicode_length;
} match_entry_t;

/* Buffer containing matches found for a search term in a file.
 * We record the character position in the file, and also the line it
 * appeared on, so that we can do proximity searches based on line
 * distance.
 */

typedef struct stmbuf {
	struct stmbuf *next;
	match_entry_t m[STM_LIMIT];
} st_match_t;

/* Finalized set of matches after the search is complete and we are
 * combining (or have combined) subexpressions.
 */

typedef struct stmset {
	int count;
	match_entry_t m[1]; /* arbitrarily long */
} matchset_t;

/* Search term.  Search terms are just strings.  A search term match
 * buffer list is attached to the search term.  The match buffer list
 * is per-file - once we've searched a file, we process all the match
 * buffers for that file and then stash the results in our global
 * result table.
 */

typedef struct st {
	size_t len;			/* Length of string. */
	st_match_t *matches;		/* Places in the file where we got a match. */
	int curmatch;			/* Current index into active match buffer. */
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
	match_ignores_spaces = 1,
	match_exactly = 2,
	match_ignores_spaces_and_case = 3
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
	match_entry_t pair[2];
} match_pair_t;

typedef void (*dumpfunc_t)(void *obj, const char *filename,
						   char *contents, off_t content_length,
						   off_t first_line, off_t last_line,
						   off_t first_char, off_t first_len,
						   off_t last_char, off_t last_len,
						   off_t *cur_line, off_t *cur_char);
typedef int (*filefunc_t)(void *obj, const char *filename);

/* Function declarations... */

/* searchfile.c */
int searchfile(const char *filename, search_term_t *terms, int nterms,
			   const char *file, off_t flen, st_match_type_t exact);
void new_st_matchbuf(search_term_t *st);
int cmp(const char *a, const char *b, size_t len, const char *limit);
int cmp_exact(const char *a, const char *b, size_t len, const char *limit);
int casecmp(const char *a, const char *b, size_t len, const char *limit);
	

/* near.c */
match_pair_t *eval_near(st_expr_t **ep);
match_pair_t *compute_near(st_expr_t *expr);
match_pair_t *reverse_combineset(match_pair_t *cs, match_pair_t *prev);
match_pair_t *merge_combineset(match_pair_t *cs);

/* combine.c */
void combine(st_expr_t **ep, char *contents, off_t len, st_match_type_t exact);
void term_to_matchset(st_expr_t **ep);
void combine_matchsets(st_expr_t **ep, st_expr_t **lep, st_expr_t **rep);

/* expr.c */
char *print_expr(st_expr_t *expr);
void free_expr(st_expr_t *expr);
st_expr_t *copy_expr(st_expr_t *expr);
int extract_search_terms(search_term_t **terms, st_expr_t *root);

/* exclude.c */
void do_exclude(st_expr_t *not_expr, char *contents, off_t len, 
				st_match_type_t exact);

/* tree.c */
extern int abortSearch;
int search_tree(const char *name, st_expr_t *root, search_term_t *terms,
				int n, dumpfunc_t func, filefunc_t filefunc, void *obj,
				int nosearchp, st_match_type_t exact);
int process_file(const char *filename, char *contents, off_t len,
				 st_expr_t *root, search_term_t *terms, int n,
				 dumpfunc_t func, void *obj, st_match_type_t exact);

/* unicode.c */
int is_unicode(char *contents, off_t len);
void unicode_fixups(char *contents, off_t len, search_term_t *st, int nterms);


/* These need to be defined somewhere... */
void gofer_fatal(const char * fmt, ... ) __attribute__((analyzer_noreturn));
int gofer_error(const char * fmt, ...);
int gofer_info(const char *fmt, ...);
int gofer_debug(const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif

/* Local Variables:  */
/* mode:C */
/* end: */
