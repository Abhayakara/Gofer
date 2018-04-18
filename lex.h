/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
 * lex.h
 * 
 * Definitions for lexical analyzer.
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

/*
 * Copyright (c) 1996-2002 Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about the Internet Software Consortium, see
 * ``http://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

typedef enum token {
	SEMI = ';',
	DOT = '.',
	COLON = ':',
	COMMA = ',',
	SLASH = '/',
	LBRACE = '{',
	RBRACE = '}',
	LPAREN = '(',
	RPAREN = ')',
	EQUAL = '=',
	BANG = '!',
	PERCENT = '%',
	PLUS = '+',
	MINUS = '-',
	ASTERISK = '*',
	AMPERSAND = '&',
	PIPE = '|',
	CARET = '^',

	AND = 256,
	FIRST_TOKEN = AND,
	STRING = 257,
	NUMBER = 258,
	NUMBER_OR_NAME = 259,
	OR = 260,
	WITHIN = 261,
	LINES = 262,
	CHARACTERS = 263,
	OF = 264,
	NAME = 265,
	END_OF_FILE = 266,
	NOT = 267

} token_t;

#define is_identifier(x)	((x) >= FIRST_TOKEN &&	\
							 (x) != STRING &&		\
							 (x) != NUMBER &&		\
							 (x) != END_OF_FILE)

/* A parsing context. */

typedef struct parse {
	int lexline;
	int lexchar;
	char *token_line;
	char *prev_line;
	char *cur_line;
	const char *tlname;
	int eol_token;

	char line1 [81];
	char line2 [81];
	int lpos;
	int line;
	int tlpos;
	int tline;
	token_t token;
	int ugflag;
	char *tval;
	int tlen;
	char tokbuf [1500];

	int warnings_occurred;
	int file;
	char *inbuf;
	unsigned long bufix, buflen;
	unsigned long bufsiz;
} parse_t;

/* lex.c */
int new_parse(parse_t **cfile, int file,
			  char *inbuf, unsigned buflen, const char *name, int eolp);
int end_parse(parse_t **cfile);
enum token next_token(const char **rval, unsigned *rlen, parse_t *cfile);
enum token peek_token(const char **rval,
					  unsigned int *rlen, parse_t *cfile);

/* main.c */
int main(int argc, char **argv);

/* parse.c */
st_expr_t *parse(parse_t *cfile, int top);

/* errwarn.c */
/* ... */
int parse_warn (parse_t *cfile, const char *fmt, ...);

/* Local Variables:  */
/* mode:C */
/* end: */
