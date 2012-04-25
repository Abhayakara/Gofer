/* conflex.c

   Lexical scanner for dhcpd config file... */

/* Copyright (c) 2003-2006 Edward W. Lemon III
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
 * Copyright (c) 1995-2002 Internet Software Consortium.
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

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "st.h"
#include "lex.h"

static int get_char(struct parse *);
static token_t get_token(struct parse *);
static void skip_to_eol(struct parse *);
static token_t read_string(struct parse *);
static token_t read_number(int, struct parse *);
static token_t read_num_or_name(int, struct parse *);
static token_t intern(char *, token_t);

int
new_parse(struct parse **cfile, int file,
	  char *inbuf, unsigned buflen, const char *name, int eolp)
{
  struct parse *tmp;

  tmp = malloc(sizeof (struct parse));
  if (!tmp)
    return 0;
  memset(tmp, 0, sizeof *tmp);

  tmp->token = 0;
  tmp->tlname = name;
  tmp->lpos = tmp->line = 1;
  tmp->cur_line = tmp->line1;
  tmp->prev_line = tmp->line2;
  tmp->token_line = tmp->cur_line;
  tmp->cur_line[0] = tmp->prev_line[0] = 0;
  tmp->warnings_occurred = 0;
  tmp->file = file;
  tmp->eol_token = eolp;

  tmp->bufix = 0;
  tmp->buflen = buflen;
  if (inbuf)
    {
      tmp->bufsiz = 0;
      tmp->inbuf = inbuf;
    }
  else
    {
      tmp->inbuf = malloc(8192);
      if (!tmp->inbuf)
	{
	  free(tmp);
	  return 0;
	}
      tmp->bufsiz = 8192;
    }

  *cfile = tmp;
  return 1;
}

int
end_parse(struct parse **cfile)
{
  if ((*cfile)->bufsiz)
    free((*cfile)->inbuf);
  free(*cfile);
  *cfile = (struct parse *)0;
  return 1;
}

static int
get_char(struct parse *cfile)
{
  int c;
  ssize_t result;

  if (cfile->bufix == cfile->buflen)
    {
      if (cfile->file != -1)
	{
	  result = 
	    read(cfile->file,
		 cfile->inbuf, cfile->bufsiz);
	  if (result == 0)
	    {
	      c = EOF;
	      cfile->bufix = 0;
	    }
	  else if (result < 0)
	    {
	      c = EOF;
	      cfile->bufix = cfile->buflen = 0;
	    }
	  else
	    {
	      c = cfile->inbuf[0];
	      cfile->bufix = 1;
	    }
	  cfile->buflen = (unsigned long)result;
	}
      else
	c = EOF;
    }
  else
    {
      c = cfile->inbuf[cfile->bufix];
      cfile->bufix++;
    }

  if (!cfile->ugflag)
    {
      if (c == '\n')
	{
	  if (cfile->cur_line == cfile->line1)
	    {	
	      cfile->cur_line = cfile->line2;
	      cfile->prev_line = cfile->line1;
	    }
	  else
	    {
	      cfile->cur_line = cfile->line1;
	      cfile->prev_line = cfile->line2;
	    }
	  cfile->line++;
	  cfile->lpos = 1;
	  cfile->cur_line[0] = 0;
	}
      else if (c != EOF)
	{
	  if (cfile->lpos <= 80)
	    {
	      cfile->cur_line[cfile->lpos - 1] = c;
	      cfile->cur_line[cfile->lpos] = 0;
	    }
	  cfile->lpos++;
	}
    }
  else
    cfile->ugflag = 0;
  return c;		
}

static token_t
get_token(struct parse *cfile)
{
  int c;
  token_t ttok;
  static char tb[2];
  int l, p, u;

  do
    {
      l = cfile->line;
      p = cfile->lpos;
      u = cfile->ugflag;

      c = get_char(cfile);
#ifdef OLD_LEXER
      if (c == '\n' && p == 1 && !u
	  && cfile->comment_index < sizeof cfile->comments)
	cfile->comments[cfile->comment_index++] = '\n';
#endif

      if (!(c == '\n' && cfile->eol_token)
	  && isascii(c) && isspace(c))
	continue;
      if (c == '#')
	{
#ifdef OLD_LEXER
	  if (cfile->comment_index < sizeof cfile->comments)
	    cfile->comments[cfile->comment_index++] = '#';
#endif
	  skip_to_eol(cfile);
	  continue;
	}
      if (c == '"')
	{
	  cfile->lexline = l;
	  cfile->lexchar = p;
	  ttok = read_string(cfile);
	  break;
	}
      if ((isascii(c) && isdigit(c)) || c == '-')
	{
	  cfile->lexline = l;
	  cfile->lexchar = p;
	  ttok = read_number(c, cfile);
	  break;
	}
      else if (isascii(c) && isalpha(c))
	{
	  cfile->lexline = l;
	  cfile->lexchar = p;
	  ttok = read_num_or_name(c, cfile);
	  break;
	}
      else if (c == EOF)
	{
	  ttok = END_OF_FILE;
	  cfile->tlen = 0;
	  break;
	}
      else
	{
	  cfile->lexline = l;
	  cfile->lexchar = p;
	  tb[0] = c;
	  tb[1] = 0;
	  cfile->tval = tb;
	  cfile->tlen = 1;
	  ttok = c;
	  break;
	}
    } while (1);
  return ttok;
}

token_t
next_token(const char **rval, unsigned *rlen, struct parse *cfile)
{
  int rv;

  if (cfile->token)
    {
      if (cfile->lexline != cfile->tline)
	cfile->token_line = cfile->cur_line;
      cfile->lexchar = cfile->tlpos;
      cfile->lexline = cfile->tline;
      rv = cfile->token;
      cfile->token = 0;
    }
  else
    {
      rv = get_token(cfile);
      cfile->token_line = cfile->cur_line;
    }
  if (rval)
    *rval = cfile->tval;
  if (rlen)
    *rlen = cfile->tlen;
#ifdef DEBUG_TOKENS
  fprintf(stderr, "%s:%d ", cfile->tval, rv);
#endif
  return rv;
}

token_t
peek_token(const char **rval, unsigned int *rlen, struct parse *cfile)
{
  int x;

  if (!cfile->token)
    {
      cfile->tlpos = cfile->lexchar;
      cfile->tline = cfile->lexline;
      cfile->token = get_token(cfile);
      if (cfile->lexline != cfile->tline)
	cfile->token_line = cfile->prev_line;

      x = cfile->lexchar;
      cfile->lexchar = cfile->tlpos;
      cfile->tlpos = x;

      x = cfile->lexline;
      cfile->lexline = cfile->tline;
      cfile->tline = x;
    }
  if (rval)
    *rval = cfile->tval;
  if (rlen)
    *rlen = cfile->tlen;
#ifdef DEBUG_TOKENS
  fprintf(stderr, "(%s:%d) ", cfile->tval, cfile->token);
#endif
  return cfile->token;
}

static void
skip_to_eol(struct parse *cfile)
{
  int c;
  do
    {
      c = get_char(cfile);
      if (c == EOF)
	return;
#ifdef OLD_LEXER
      if (cfile->comment_index < sizeof (cfile->comments))
	comments [cfile->comment_index++] = c;
#endif
      if (c == '\n')
	{
	  return;
	}
    } while (1);
}

static token_t
read_string(struct parse *cfile)
{
  int i;
  int bs = 0;
  int c;
  int value = 0;
  int hex = 0;

  for (i = 0; i < sizeof cfile->tokbuf; i++)
    {
    again:
      c = get_char(cfile);
      if (c == EOF)
	{
	  parse_warn(cfile, "eof in string constant");
	  break;
	}
      if (bs == 1)
	{
	  switch (c)
	    {
	    case 't':
	      cfile->tokbuf [i] = '\t';
	      break;
	    case 'r':
	      cfile->tokbuf [i] = '\r';
	      break;
	    case 'n':
	      cfile->tokbuf [i] = '\n';
	      break;
	    case 'b':
	      cfile->tokbuf [i] = '\b';
	      break;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	      hex = 0;
	      value = c - '0';
	      ++bs;
	      goto again;
	    case 'x':
	      hex = 1;
	      value = 0;
	      ++bs;
	      goto again;
	    default:
	      cfile->tokbuf [i] = c;
	      bs = 0;
	      break;
	    }
	  bs = 0;
	}
      else if (bs > 1)
	{
	  if (hex)
	    {
	      if (c >= '0' && c <= '9')
		{
		  value = value * 16 + (c - '0');
		}
	      else if (c >= 'a' && c <= 'f')
		{
		  value = value * 16 + (c - 'a' + 10);
		}
	      else if (c >= 'A' && c <= 'F')
		{
		  value = value * 16 + (c - 'A' + 10);
		}
	      else
		{
		  parse_warn(cfile,
			     "invalid hex digit: %x",
			     c);
		  bs = 0;
		  continue;
		}
	      if (++bs == 4)
		{
		  cfile->tokbuf [i] = value;
		  bs = 0;
		}
	      else
		goto again;
	    }
	  else
	    {
	      if (c >= '0' && c <= '9')
		{
		  value = value * 8 + (c - '0');
		}
	      else
		{
		  if (value != 0)
		    {
		      parse_warn(cfile,
				 "invalid octal digit %x",
				 c);
		      continue;
		    }
		  else
		    cfile->tokbuf [i] = 0;
		  bs = 0;
		}
	      if (++bs == 4)
		{
		  cfile->tokbuf [i] = value;
		  bs = 0;
		}
	      else
		goto again;
	    }
	}
      else if (c == '\\')
	{
	  bs = 1;
	  goto again;
	}
      else if (c == '"')
	break;
      else
	cfile->tokbuf [i] = c;
    }

  /* We probably ought to allow arbitrary-length strings, but... */
  if (i == sizeof cfile->tokbuf)
    {
      parse_warn(cfile,
		 "string constant larger than internal buffer");
      --i;
    }
  cfile->tokbuf [i] = 0;
  cfile->tlen = i;
  cfile->tval = cfile->tokbuf;
  return STRING;
}

static token_t
read_number(int c, struct parse *cfile)
{
  int seenx = 0;
  int i = 0;
  int token = NUMBER;

  cfile->tokbuf [i++] = c;
  for (; i < sizeof cfile->tokbuf; i++)
    {
      c = get_char(cfile);
      if (!seenx && c == 'x')
	{
	  seenx = 1;
#ifndef OLD_LEXER
	}
      else if (isascii(c) && !isxdigit(c) &&
	       (c == '-' || c == '_' || isalpha(c)))
	{
	  token = NAME;
	}
      else if (isascii(c) && !isdigit(c) && isxdigit(c))
	{
	  token = NUMBER_OR_NAME;
#endif
	}
      else if (!isascii(c) || !isxdigit(c))
	{
	  if (c != EOF)
	    {
	      cfile->bufix--;
	      cfile->ugflag = 1;
	    }
	  break;
	}
      cfile->tokbuf [i] = c;
    }
  if (i == sizeof cfile->tokbuf)
    {
      parse_warn(cfile,
		 "numeric token larger than internal buffer");
      --i;
    }
  cfile->tokbuf [i] = 0;
  cfile->tlen = i;
  cfile->tval = cfile->tokbuf;
  return token;
}

static token_t
read_num_or_name(int c, struct parse *cfile)
{
  int i = 0;
  token_t rv = NUMBER_OR_NAME;
  cfile->tokbuf [i++] = c;
  for (; i < sizeof cfile->tokbuf; i++)
    {
      c = get_char(cfile);
      if (!isascii(c) ||
	  (c != '-' && c != '_' && !isalnum(c)))
	{
	  if (c != EOF)
	    {
	      cfile->bufix--;
	      cfile->ugflag = 1;
	    }
	  break;
	}
      if (!isxdigit(c))
	rv = NAME;
      cfile->tokbuf [i] = c;
    }
  if (i == sizeof cfile->tokbuf)
    {
      parse_warn(cfile, "token larger than internal buffer");
      --i;
    }
  cfile->tokbuf [i] = 0;
  cfile->tlen = i;
  cfile->tval = cfile->tokbuf;
  return intern(cfile->tval, rv);
}

static token_t
intern(char *atom, token_t dfv)
{
  if (!isascii(atom [0]))
    return dfv;

  switch (tolower(atom [0]))
    {
    case '-':
      if (atom [1] == 0)
	return MINUS;
      break;

    case 'a':
      if (!strcasecmp(atom + 1, "nd"))
	return AND;
      break;
    case 'b':
      break;
    case 'c':
      if (!strcasecmp(atom + 1, "haracters"))
	return CHARACTERS;
      break;
    case 'd':
      break;
    case 'e':
      break;
    case 'f':
      break;
    case 'g':
      break;
    case 'h':
      break;
    case 'i':
      break;
    case 'k':
      break;
    case 'l':
      if (!strcasecmp(atom + 1, "ines"))
	return LINES;
      break;
    case 'm':
      break;
    case 'n':
      if (!strcasecmp(atom + 1, "ot"))
	return NOT;
      break;
    case 'o':
      if (!strcasecmp(atom + 1, "r"))
	return OR;
      if (!strcasecmp(atom + 1, "f"))
	return OF;
      break;
    case 'p':
      break;
    case 'r':
      break;
    case 's':
      break;
    case 't':
      break;
    case 'u':
      break;
    case 'v':
      break;
    case 'w':
      if (!strcasecmp(atom + 1, "ithin"))
	return WITHIN;
      break;
    case 'y':
      break;
    case 'z':
      break;
    }
  return dfv;
}

/* Local Variables:  */
/* mode:C */
/* c-file-style:"gnu" */
/* end: */
