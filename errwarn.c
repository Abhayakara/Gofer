/* errwarn.c

   Errors and warnings... */

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

/*
 * Copyright (c) 1995 RadioMail Corporation.
 * Copyright (c) 1996-2000 Internet Software Consortium.
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
 * This software was written for RadioMail Corporation by Ted Lemon
 * under a contract with Vixie Enterprises.   Further modifications have
 * been made for the Internet Software Consortium under a contract
 * with Vixie Laboratories.
 */

#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "st.h"
#include "lex.h"

#define CVT_BUF_MAX 1023
static char mbuf [CVT_BUF_MAX + 1];
static char fbuf [CVT_BUF_MAX + 1];

static void do_percentm (char *obuf, const char *ibuf);
#ifdef NO_STRERROR
static char *strerror (int err);
#endif

/* Log an error message, then exit... */

void
gofer_fatal(const char * fmt, ... )
{
  va_list list;

  do_percentm(fbuf, fmt);

  va_start(list, fmt);
  vsnprintf(mbuf, sizeof mbuf, fbuf, list);
  va_end(list);

  write (2, mbuf, strlen (mbuf));
  write (2, "\n", 1);

  exit (1);
}

/* Log an error message... */

int
gofer_error(const char * fmt, ...)
{
  va_list list;

  do_percentm(fbuf, fmt);

  va_start(list, fmt);
  vsnprintf(mbuf, sizeof mbuf, fbuf, list);
  va_end(list);

  write(2, mbuf, strlen (mbuf));
  write(2, "\n", 1);

  return 0;
}

/* Log a note... */

int
gofer_info(const char *fmt, ...)
{
  va_list list;

  do_percentm(fbuf, fmt);

  va_start(list, fmt);
  vsnprintf(mbuf, sizeof mbuf, fbuf, list);
  va_end(list);

  write(2, mbuf, strlen (mbuf));
  write(2, "\n", 1);

  return 0;
}

/* Log a debug message... */

int
gofer_debug (const char *fmt, ...)
{
  va_list list;

  do_percentm(fbuf, fmt);

  va_start(list, fmt);
  vsnprintf(mbuf, sizeof mbuf, fbuf, list);
  va_end(list);

  write(2, mbuf, strlen (mbuf));
  write(2, "\n", 1);

  return 0;
}

int
parse_warn (struct parse *cfile, const char *fmt, ...)
{
  va_list list;
  char lexbuf[256];
  char mbuf[1024];
  char fbuf[1024];
  unsigned i, lix;
  
  do_percentm(mbuf, fmt);
#ifndef NO_SNPRINTF
  snprintf(fbuf, sizeof fbuf, "%s line %d: %s",
	   cfile->tlname, cfile->lexline, mbuf);
#else
  sprintf(fbuf, "%s line %d: %s",
	  cfile->tlname, cfile->lexline, mbuf);
#endif
	
  va_start(list, fmt);
  vsnprintf(mbuf, sizeof mbuf, fbuf, list);
  va_end(list);

  lix = 0;
  for (i = 0; cfile->token_line[i] && i < (cfile->lexchar - 1); i++)
    {
      if (lix < (sizeof lexbuf) - 1)
	lexbuf[lix++] = ' ';
      if (cfile->token_line[i] == '\t')
	{
	  for (; (lix < (sizeof lexbuf) - 1) && (lix & 7); lix++)
	    lexbuf[lix] = ' ';
	}
    }
  lexbuf[lix] = 0;

  write(2, mbuf, strlen (mbuf));
  write(2, "\n", 1);
  write(2, cfile->token_line, strlen (cfile->token_line));
  write(2, "\n", 1);
  if (cfile->lexchar < 81)
    write(2, lexbuf, lix);
  write (2, "^\n", 2);

  cfile->warnings_occurred = 1;

  return 0;
}

/* Find %m in the input string and substitute an error message string. */

static void
do_percentm (char *obuf, const char *ibuf)
{
  const char *s = ibuf;
  char *p = obuf;
  int infmt = 0;
  const char *m;
  int len = 0;
  
  while (*s)
    {
      if (infmt)
	{
	  if (*s == 'm')
	    {
#if !defined(_WIN32) || 1
	      m = strerror(errno);
#else
	      m = pWSAError();
#endif
	      if (!m)
		m = "<unknown error>";
	      len += strlen (m);
	      if (len > CVT_BUF_MAX)
		goto out;
	      strcpy (p - 1, m);
	      p += strlen (p);
	      ++s;
	    }
	  else
	    {
	      if (++len > CVT_BUF_MAX)
		goto out;
	      *p++ = *s++;
	    }
	  infmt = 0;
	}
      else
	{
	  if (*s == '%')
	    infmt = 1;
	  if (++len > CVT_BUF_MAX)
	    goto out;
	  *p++ = *s++;
	}
    }
 out:
  *p = 0;
}

#ifdef NO_STRERROR
static char *
strerror (int err)
{
  extern char *sys_errlist[];
  extern int sys_nerr;
  static char errbuf[128];
  
  if (err < 0 || err >= sys_nerr)
    {
      sprintf (errbuf, "Error %d", err);
      return errbuf;
    }
  return sys_errlist [err];
}
#endif /* NO_STRERROR */

#if defined (_WIN32) && 0
char *pWSAError ()
{
  int err = WSAGetLastError();

  switch (err)
    {
    case WSAEACCES:
      return "Permission denied";
    case WSAEADDRINUSE:
      return "Address already in use";
    case WSAEADDRNOTAVAIL:
      return "Cannot assign requested address";
    case WSAEAFNOSUPPORT:
      return "Address family not supported by protocol family";
    case WSAEALREADY:
      return "Operation already in progress";
    case WSAECONNABORTED:
      return "Software caused connection abort";
    case WSAECONNREFUSED:
      return "Connection refused";
    case WSAECONNRESET:
      return "Connection reset by peer";
    case WSAEDESTADDRREQ:
      return "Destination address required";
    case WSAEFAULT:
      return "Bad address";
    case WSAEHOSTDOWN:
      return "Host is down";
    case WSAEHOSTUNREACH:
      return "No route to host";
    case WSAEINPROGRESS:
      return "Operation now in progress";
    case WSAEINTR:
      return "Interrupted function call";
    case WSAEINVAL:
      return "Invalid argument";
    case WSAEISCONN:
      return "Socket is already connected";
    case WSAEMFILE:
      return "Too many open files";
    case WSAEMSGSIZE:
      return "Message too long";
    case WSAENETDOWN:
      return "Network is down";
    case WSAENETRESET:
      return "Network dropped connection on reset";
    case WSAENETUNREACH:
      return "Network is unreachable";
    case WSAENOBUFS:
      return "No buffer space available";
    case WSAENOPROTOOPT:
      return "Bad protocol option";
    case WSAENOTCONN:
      return "Socket is not connected";
    case WSAENOTSOCK:
      return "Socket operation on non-socket";
    case WSAEOPNOTSUPP:
      return "Operation not supported";
    case WSAEPFNOSUPPORT:
      return "Protocol family not supported";
    case WSAEPROCLIM:
      return "Too many processes";
    case WSAEPROTONOSUPPORT:
      return "Protocol not supported";
    case WSAEPROTOTYPE:
      return "Protocol wrong type for socket";
    case WSAESHUTDOWN:
      return "Cannot send after socket shutdown";
    case WSAESOCKTNOSUPPORT:
      return "Socket type not supported";
    case WSAETIMEDOUT:
      return "Connection timed out";
    case WSAEWOULDBLOCK:
      return "Resource temporarily unavailable";
    case WSAHOST_NOT_FOUND:
      return "Host not found";
#if 0
    case WSA_INVALID_HANDLE:
      return "Specified event object handle is invalid";
    case WSA_INVALID_PARAMETER:
      return "One or more parameters are invalid";
    case WSAINVALIDPROCTABLE:
      return "Invalid procedure table from service provider";
    case WSAINVALIDPROVIDER:
      return "Invalid service provider version number";
    case WSA_IO_PENDING:
      return "Overlapped operations will complete later";
    case WSA_IO_INCOMPLETE:
      return "Overlapped I/O event object not in signaled state";
    case WSA_NOT_ENOUGH_MEMORY:
      return "Insufficient memory available";
#endif
    case WSANOTINITIALISED:
      return "Successful WSAStartup not yet performer";
    case WSANO_DATA:
      return "Valid name, no data record of requested type";
    case WSANO_RECOVERY:
      return "This is a non-recoverable error";
#if 0
    case WSAPROVIDERFAILEDINIT:
      return "Unable to initialize a service provider";
    case WSASYSCALLFAILURE:
      return "System call failure";
#endif
    case WSASYSNOTREADY:
      return "Network subsystem is unavailable";
    case WSATRY_AGAIN:
      return "Non-authoritative host not found";
    case WSAVERNOTSUPPORTED:
      return "WINSOCK.DLL version out of range";
    case WSAEDISCON:
      return "Graceful shutdown in progress";
#if 0
    case WSA_OPERATION_ABORTED:
      return "Overlapped operation aborted";
#endif
    }
  return "Unknown WinSock error";
}
#endif /* _WIN32 */

/* Local Variables:  */
/* mode:C */
/* c-file-style:"gnu" */
/* end: */
