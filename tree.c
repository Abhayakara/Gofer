/* tree.c
 * 
 * Functions for searching a tree.
 */

/* Copyright (c) 2003-2005 Edward W. Lemon III
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "st.h"

int
search_tree(const char *name, st_expr_t *root, search_term_t *terms, int n,
	    dumpfunc_t func, filefunc_t filefunc, void *obj,
	    int nosearchp, st_match_type_t exact)
{
  DIR *dir;
  struct dirent *de;
  struct stat st;
  char *subname;
  char *filebuf;
  int result, inf;
  int len;
  int rv = 1;

  dir = opendir(name);
  if (!dir)
    {
      gofer_error("%s: %m", name);
      return rv;
    }
  
  while ((de = readdir(dir)))
    {
      if (!strcmp(de->d_name, ".") ||
	  !strcmp(de->d_name, ".."))
	continue;

      subname = malloc(strlen(de->d_name) + strlen(name) + 2);
      if (!subname)
	gofer_fatal("no memory to make subdirectory.");
      sprintf(subname,
#ifdef _WIN32
	      "%s\\%s",
#else
	      "%s/%s",
#endif
	      name, de->d_name);

      if (stat(subname, &st) < 0)
	{
	  gofer_error("%s: %m", subname);
	  continue;
	}
      
      if (S_ISDIR(st.st_mode))
	{
	  if (!search_tree(subname, root, terms, n,
			   func, filefunc, obj, nosearchp, exact))
	    {
	      rv = 0;
	      free(subname);
	      break;
	    }
	}
      else if (S_ISREG(st.st_mode))
	{
	  if (filefunc && !filefunc(obj, subname))
	    {
	      free(subname);
	      rv = 0;
	      break;
	    }

	  inf = open(subname, O_RDONLY
#ifdef O_BINARY
		     | O_BINARY
#endif
		     );
	  if (inf < 0)
	    {
	      gofer_error("Can't open %s: %m", subname);
	      free(subname);
	      continue;
	    }	
	  
	  len = st.st_size;
	  filebuf = malloc(len);
	  if (!filebuf)
	    gofer_fatal("No memory for file %s", subname);

	  /* If we have the ability to hint to the OS that we don't want
	   * the contents of this file cached, so hint.   Otherwise on some
	   * operating systems (notably Darwin), we can force a bazillion
	   * pages to be flushed while searching a lot of files, which really
	   * trashes system performance.
	   */
#if defined(F_NOCACHE)
	  fcntl(inf, F_NOCACHE, 1);
#endif
      
	  result = read(inf, filebuf, len);
	  if (result != len)
	    {
	      if (result < 0)
		gofer_error("Can't read %s: %m", subname);
	      else
		gofer_error("Short read on %s: %d instead of %d",
			    subname, result, len);
	      free(subname);
	      continue;
	    }

	  /* If we got hits, we need to keep the file's contents around;
	   * otherwise let them go once we've searched it.   Likewise, if
	   * we have been asked not to search the file, don't.
	   */
	  if (nosearchp ||
	      !process_file(subname, filebuf,
			    len, root, terms, n, func, obj, exact))
	    free(filebuf);
	  close(inf);
	}

      free(subname);
    }
  closedir(dir);
  return rv;
}

int
process_file(const char *filename, char *contents, int len,
	     st_expr_t *root, search_term_t *terms, int n,
	     dumpfunc_t func, void *obj, st_match_type_t exact)
{
  st_expr_t *copy;
  combineset_t *cset;
  int line;
  int linecp;
  int hits = 0;
  int i;

  /* Make a copy of the expression. */
  copy = copy_expr(root);
      
  /* Search the file. */
  if (!searchfile(filename, terms, n, contents, len, exact))
    {
      free_expr(copy);
      return 0;
    }
  
  /* Combine common terms. */
  combine(&copy);
  
  /* Evaluate nearness expressions */
  cset = eval_near(&copy);
  
  if (cset)
    {
      combineset_t *next;
      
      cset = reverse_combineset(cset, 0);
      
      line = 1;
      linecp = 0;
      
      for (; cset; cset = next)
	{
	  next = cset->next;
	  func(obj, filename, contents, len,
	       cset->lp[0], cset->lp[1],
	       cset->cp[0], cset->cp[2], cset->cp[1], cset->cp[3],
	       &line, &linecp);
	  free(cset);
	  cset = 0;
	  hits++;
	}
    }
  else
    {
      matchset_t *ms;
      
      if (copy && copy->type == ste_matchset)
	{
	  int j, k = -1;
	  int cp;
	  
	  line = 1;
	  linecp = 0;
	  
	  ms = copy->subexpr.set;
	  for (j = 0; j < ms->count; j += 3)
	    {
	      /* This is a little goo that makes sure that if we have
	       * two matches on the same line or on subsequent lines,
	       * we don't print the same set of lines twice.
	       */
	      if (k == -1) {
		cp = ms->data[j];
		k = ms->data[j + 2];
	      }
#if 0
	      if (j + 3 != ms->count && ms->data[j + 4] < k + 1)
		continue;
#endif
	      func(obj, filename, contents, len,
		   k ? k - 1 : k,  ms->data[j + 2] + 1,
		   cp, ms->data[j + 1], 0, 0,
		   &line, &linecp);
	      k = -1;
	      hits++;
	    }
	}
    }		
  if (copy)
    free_expr(copy);

  /* Free up match buffers. */
  for (i = 0; i < n; i++)
    {
      st_match_t *stm, *next;

      for (stm = terms[i].matches; stm; stm = next)
	{
	  next = stm->next;
	  free(stm);
	}
    }
  return hits;
}

/* Local Variables:  */
/* mode:C */
/* c-file-style:"gnu" */
/* end: */
