/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * parse.c
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "st.h"
#include "lex.h"

st_expr_t *
parse(parse_t *cfile, int top)
{
	st_expr_t *cur = NULL;
	st_expr_t *new;
	token_t token;
	const char *val;
	unsigned len;
	search_term_t *cst;
	st_expr_type_t type;
	int n;

again:
	token = peek_token(&val, &len, cfile);
	switch (token)
    {
		/* LPAREN <expr> RPAREN */
    case LPAREN:
		if (cur)
		{
			parse_warn(cfile, "expecting an operator here");
			free_expr(cur);
			return 0;
		}
		token = next_token(&val, &len, cfile);

		cur = parse(cfile, 0);
		if (!cur)
			return 0;
      
		token = next_token(&val, &len, cfile);
		if (token != RPAREN)
		{
			parse_warn(cfile, "expecting right paren here");
			free_expr(cur);
			return 0;
		}
		goto again;

		/* <expr> AND <expr> */
    case AND:
		type = ste_and;
    binary:
		n = 0;
		token = next_token(&val, &len, cfile);
    binary_with_number:
		if (!cur)
			break;
		new = malloc(sizeof *new);
		if (!new)
			gofer_fatal("no memory for new expression.");
		memset(new, 0, sizeof *new);
		new->type = type;
		new->n = n;
		new->subexpr.exprs[0] = cur;
		cur = new;
		new = parse(cfile, 0);
		if (!new)
		{
			free_expr(cur);
			return 0;
		}
		cur->subexpr.exprs[1] = new;
		goto again;

		/* <expr> OR <expr> */
    case OR:
		type = ste_or;
		goto binary;


		/* NOT <expr> */
    case NOT:
		if (cur)
		{
			parse_warn(cfile, "unary operator not appropriate here.");
			free_expr(cur);
			return 0;
		}
		token = next_token(&val, &len, cfile);
		cur = malloc(sizeof *new);
		if (!cur)
			gofer_fatal("no memory for new expression.");
		memset(cur, 0, sizeof *cur);
		cur->type = ste_not;
		cur->n = 0;
		new = parse(cfile, 0);
		if (!new)
		{
			free_expr(cur);
			return 0;
		}
		cur->subexpr.expr = new;
		goto again;

		/* "..." */
    case STRING:
		if (cur)
		{
			parse_warn(cfile, "search term not appropriate here");
			free_expr(cur);
			return 0;
		}
		if (ST_LIMIT - 1 <= len)
		{
			parse_warn(cfile, "Search term %s is too long.", val);
			return 0;
		}
		token = next_token(&val, &len, cfile);

		/* Okay, we can allocate the expression. */
		cur = malloc(sizeof *cur);
		if (!cur)
			gofer_fatal("no memory for initial expression");
		memset(cur, 0, sizeof *cur);

		/* And the search term. */
		cst = malloc(sizeof *cst);
		if (!cst)
			gofer_fatal("no memory for search term");
		memset(cst, 0, sizeof *cst);

		/* Fill out the search term. */
		memcpy(cst->buf, val, len);
		cst->buf[len] = 0;
		cst->len = len;

		/* Fill out the expression. */
		cur->type = ste_term;
		cur->subexpr.term = cst;
		goto again;

		/* <expr> WITHIN <number> LINES OF <expr> */
		/* <expr> WITHIN <number> CHARACTERS OF <expr> */
    case WITHIN:
		if (!cur)
			break;
		token = next_token(&val, &len, cfile);
		token = next_token(&val, &len, cfile);
		if (token != NUMBER)
		{
			parse_warn(cfile, "expecting a number here");
			free_expr(cur);
			return 0;
		}
		n = atoi(val);

		token = next_token(&val, &len, cfile);
		if (token == LINES)
			type = ste_near_lines;
		else if (token == CHARACTERS)
			type = ste_near;
		else
		{
			parse_warn(cfile, "expecting 'lines' or 'characters' here");
			if (cur)
				free(cur);
			return 0;
		}

		/* Check for an optional "of" token." */
		token = peek_token(&val, &len, cfile);
		if (token == OF)
			token = next_token(&val, &len, cfile);
	
		/* Now it's just a regular binary expression. */
		goto binary_with_number;

    case RPAREN:
		if (!cur)
		{
			parse_warn(cfile, "expecting subexpression");
			return 0;
		}
		break;

    default:
		if (token == END_OF_FILE)
		{
			if (cur)
				break;
			parse_warn(cfile, "Unexpected end of file");
		}
		else
			parse_warn(cfile, "Unexpected token: %s", val);
		if (cur)
			free(cur);
		return 0;
    }

	/* If this is the top-level parse, then we expect there to be nothing
	 * left over when it's done - the next token should be the END_OF_FILE
	 * token.
	 */
	if (top)
    {
		token = next_token(&val, &len, cfile);
		if (token != END_OF_FILE)
		{
			parse_warn(cfile, "unexpected token: %s %d", val, token);
			if (cur)
				free(cur);
			return 0;
		}
    }
	return cur;
}

/* Local Variables:  */
/* mode:C */
/* end: */
