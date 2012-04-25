/* filelist.c
 *
 * Maintain a list of files that matched, and the locations in the files
 * that matched.
 */

/* Copyright (c) 2004, 2005 Edward W. Lemon III
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
#include <unistd.h>
#include <string.h>

#include "filelist.h"

filelist_t *
new_filelist()
{
  filelist_t *rv = (filelist_t *)malloc(sizeof *rv);

  if (rv)
    {
      memset(rv, 0, sizeof *rv);
    }
  rv->cur_file = 0;
  return rv;
}

void
new_entry(filelist_t *dest,
	  const char *filename, char *contents, int content_length,
	  int first_line, int last_line,
	  int first_char, int first_len,
	  int last_char, int last_len,
	  int *cur_line, int *cur_char)
{
  fileresults_t *fr;
  matchzone_t *mz;

  if (dest->nfiles == 0 ||
      strcmp(filename, dest->files[dest->nfiles - 1]->filename))
    {
      fr = malloc(sizeof *fr);
      if (!fr)
	{
	  return; /* XXX */
	}
      memset(fr, 0, sizeof *fr);
      fr->filename = strdup(filename);
      fr->reducename = strrchr(fr->filename, '/');
      if (!fr->reducename)
	fr->reducename = fr->filename;
      fr->contents = contents;
      fr->content_length = content_length;
      if (dest->nfiles == dest->maxfiles)
	{
	  int maxfiles = dest->maxfiles ? dest->maxfiles * 2 : 100;
	  fileresults_t **nf = malloc(maxfiles * sizeof *nf);
	  if (!nf)
	    return; /* XXX */
	  if (dest->maxfiles)
	    memcpy(nf, dest->files, dest->maxfiles * sizeof *nf);
	  free(dest->files);

	  dest->maxfiles = maxfiles;
	  dest->files = nf;
	}
      dest->files[dest->nfiles++] = fr;
    }
  else
    {
      fr = dest->files[dest->nfiles - 1];
    }

  mz = malloc(sizeof *mz);
  if (!mz)
    return; /* XXX */

  if (fr->nzones == fr->maxzones)
    {
      int maxzones = fr->maxzones ? fr->maxzones * 2 : 10;
      matchzone_t **mzs = malloc(maxzones * sizeof *mzs);
      if (fr->maxzones)
	memcpy(mzs, fr->matches, fr->maxzones * sizeof *mzs);
      fr->maxzones = maxzones;
      free(fr->matches);
      fr->matches = mzs;
    }

  mz->first_line = first_line;
  mz->last_line = last_line;
  mz->first_char = first_char;
  mz->first_len = first_len;
  mz->last_char = last_char;
  mz->last_len = last_len;
  mz->cur_line = mz->cur_char = 0;

  fr->matches[fr->nzones++] = mz;
}

void
filelist_free(filelist_t *fl)
{
  int i;
  if (fl->files)
    {
      for (i = 0; i < fl->nfiles; i++)
	{
	  fileresults_t *fr;
	  fr = fl->files[i];
	  if (fr->filename)
	    free(fr->filename);
	  /* We don't free fr->reducename because it's a pointer into
	   * fr->filename.
	   */
	  if (fr->contents)
	    free(fr->contents);
	  if (fr->matches)
	    {
	      int j;
	      for (j = 0; j < fr->nzones; j++)
		free(fr->matches[j]);
	      free(fr->matches);
	    }
	}
      free(fl->files);
    }
  free(fl);
}

/* Local Variables:  */
/* mode:C */
/* c-file-style:"gnu" */
/* end: */
