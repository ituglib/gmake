/* Interface to 'ar' archives for GNU Make.
Copyright (C) 1988-2020 Free Software Foundation, Inc.

This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "makeint.h"

#ifndef NO_ARCHIVES

#include "filedef.h"
#include "dep.h"
#include <fnmatch.h>

#if defined(_GUARDIAN_TARGET)
# include <fcntl.h>
# include <unistd.h>
# include "debug.h"
# define POBJ_SUPPRESS_EXTERNALIZATION_VERSION
# include "pobj.h"
# define COPYLIB_SUPPRESS_EXTERNALIZATION_VERSION
# include "copylib.h"
# define DDLDICT_SUPPRESS_EXTERNALIZATION_VERSION
# include "ddldict.h"
#endif

/* Return nonzero if NAME is an archive-member reference, zero if not.  An
   archive-member reference is a name like 'lib(member)' where member is a
   non-empty string.
   If a name like 'lib((entry))' is used, a fatal error is signaled at
   the attempt to use this unsupported feature.  */

int
ar_name (const char *name)
{
  const char *p = strchr (name, '(');
  const char *end;

  if (p == 0 || p == name)
    return 0;

  end = p + strlen (p) - 1;
  if (*end != ')' || end == p + 1)
    return 0;

  if (p[1] == '(' && end[-1] == ')')
    OS (fatal, NILF, _("attempt to use unsupported feature: '%s'"), name);

  return 1;
}


/* Parse the archive-member reference NAME into the archive and member names.
   Creates one allocated string containing both names, pointed to by ARNAME_P.
   MEMNAME_P points to the member.  */

void
ar_parse_name (const char *name, char **arname_p, char **memname_p)
{
  char *p;

  *arname_p = xstrdup (name);
  p = strchr (*arname_p, '(');
  *(p++) = '\0';
  p[strlen (p) - 1] = '\0';
  *memname_p = p;
}


/* This function is called by 'ar_scan' to find which member to look at.  */

/* ARGSUSED */
static long int
ar_member_date_1 (int desc UNUSED, const char *mem, int truncated,
                  long int hdrpos UNUSED, long int datapos UNUSED,
                  long int size UNUSED, long int date,
                  int uid UNUSED, int gid UNUSED, unsigned int mode UNUSED,
                  const void *name)
{
  return ar_name_equal (name, mem, truncated) ? date : 0;
}

/* Return the modtime of NAME.  */

time64_t
ar_member_date (const char *name)
{
  char *arname;
  char *memname;
  int64_t val;

  ar_parse_name (name, &arname, &memname);

  /* Make sure we know the modtime of the archive itself because we are
     likely to be called just before commands to remake a member are run,
     and they will change the archive itself.

     But we must be careful not to enter_file the archive itself if it does
     not exist, because pattern_search assumes that files found in the data
     base exist or can be made.  */
  {
    struct file *arfile;
    arfile = lookup_file (arname);
    if (arfile == 0 && file_exists_p (arname))
      arfile = enter_file (strcache_add (arname));

    if (arfile != 0)
      (void) f_mtime (arfile, 0);
  }

#if defined(_GUARDIAN_TARGET)
  open_pobj_dll();
  open_copylib_dll();
  open_ddldict_dll();
  if (IS_POBJ_ENABLED && is_pobj_func(arname)) {
    if (pobj_member_date_64_func) {
	  val = pobj_scan_64_func (arname, pobj_member_date_64_func, memname);
    } else {
  	  val = pobj_scan_func (arname, pobj_member_date_1_func, memname);
    }
  } else if (IS_COPYLIB_ENABLED && is_copylib_func(arname)) {
    if (copylib_scan_64_func) {
	  val = copylib_scan_64_func (arname, copylib_member_date_64_func, memname);
    } else {
  	  val = copylib_scan_func (arname, copylib_member_date_1_func, memname);
    }
  } else if (IS_DDLDICT_ENABLED && is_ddldict_func(arname)) {
    if (ddldict_scan_64_func) {
      val = ddldict_scan_64_func (arname, ddldict_member_date_64_func, memname);
    } else {
      val = ddldict_scan_func (arname, ddldict_member_date_1_func, memname);
    }
  } else
#endif
  val = ar_scan (arname, ar_member_date_1, memname);

  free (arname);

  return (val <= 0 ? (time64_t) -1 : (time64_t) val);
}

/* Set the archive-member NAME's modtime to now.  */

#ifdef VMS
int
ar_touch (const char *name)
{
  O (error, NILF, _("touch archive member is not available on VMS"));
  return -1;
}
#else
#define TOUCH_ERROR(call) do{ perror_with_name ((call), file->name);    \
                              return us_failed; }while(0)

int
ar_touch (const char *name)
{
  char *arname, *memname;
  int val;

  ar_parse_name (name, &arname, &memname);

  /* Make sure we know the modtime of the archive itself before we
     touch the member, since this will change the archive modtime.  */
  {
    struct file *arfile;
    arfile = enter_file (strcache_add (arname));
    f_mtime (arfile, 0);
# ifdef _GUARDIAN_TARGET
    open_pobj_dll();
    open_copylib_dll();
    open_ddldict_dll();
   if (IS_POBJ_ENABLED && is_pobj_func(arname)) {

    } else if (IS_COPYLIB_ENABLED && is_copylib_func(arname)) {

    } else if (IS_DDLDICT_ENABLED && is_ddldict_func(arname)) {

    } else {
    if (!arfile->updated) {
      int fd;
      struct file *file = arfile;

      EINTRLOOP (fd, open (arfile->name, O_RDWR | O_CREAT, 0666));
      if (fd < 0)
        TOUCH_ERROR ("touch: open: ");
      else
        {
          struct stat statbuf;
          char buf = 'x';
          int e;

          EINTRLOOP (e, fstat (fd, &statbuf));
          if (e < 0)
            TOUCH_ERROR ("touch: fstat: ");
          /* Rewrite character 0 same as it already is.  */
          EINTRLOOP (e, read (fd, &buf, 1));
          if (e < 0)
            TOUCH_ERROR ("touch: read: ");
          {
            off_t o;
            EINTRLOOP (o, lseek (fd, 0L, 0));
            if (o < 0L)
              TOUCH_ERROR ("touch: lseek: ");
          }
          EINTRLOOP (e, write (fd, &buf, 1));
          if (e < 0)
            TOUCH_ERROR ("touch: write: ");

          /* If file length was 0, we just changed it, so change it back.  */
          if (statbuf.st_size == 0)
            {
              (void) close (fd);
              EINTRLOOP (fd, open (file->name, O_RDWR | O_TRUNC, 0666));
              if (fd < 0)
                TOUCH_ERROR ("touch: open: ");
            }
          (void) close (fd);
          file->updated = 1;
        }
    }
    }
# endif
  }

  val = 1;

#if defined(_GUARDIAN_TARGET)
  open_pobj_dll();
  open_copylib_dll();
  open_ddldict_dll();
  if (IS_POBJ_ENABLED && is_pobj_func(arname))
  switch (pobj_member_touch_func (arname, memname))
	{
	case -1:
	  OS (error, NILF, _("touch: POBJ '%s' does not exist"), arname);
	  break;
	case -2:
	  OS (error, NILF, _("touch: '%s' is not a valid POBJ"), arname);
	  break;
	case -3:
	  perror_with_name ("touch: ", arname);
	  break;
	case 1:
	  OSS (error, NILF,
		   _("touch: Requester '%s' does not exist in '%s'"), memname, arname);
	  break;
	case 0:
	  val = 0;
	  break;
	default:
	  OS (error, NILF,
		  _("touch: Bad return code from pobj_member_touch on '%s'"), name);
	}
  else if (IS_COPYLIB_ENABLED && is_copylib_func(arname))
  switch (copylib_member_touch_func (arname, memname))
	{
	case -1:
	  OS (error, NILF, _("touch: COPYLIB '%s' does not exist"), arname);
	  break;
	case -2:
	  OS (error, NILF, _("touch: '%s' is not a valid COPYLIB or is not set up with $(indexsection)"), arname);
	  break;
	case -3:
	  perror_with_name ("touch: ", arname);
	  break;
	case 1:
	  OSS (error, NILF,
		   _("touch: Section '%s' does not exist in '%s'"), memname, arname);
	  break;
	case 0:
	  val = 0;
	  break;
	default:
	  OS (error, NILF,
		  _("touch: Bad return code from copylib_member_touch on '%s'"), name);
	}
  else if (IS_DDLDICT_ENABLED && is_ddldict_func(arname))
  switch (ddldict_member_touch_func (arname, memname))
	{
	case -1:
	  OS (error, NILF, _("touch: DDLDICT '%s' does not exist"), arname);
	  break;
	case -2:
	  OS (error, NILF, _("touch: '%s' is not a valid DDLDICT or is not set up with $(indexsection)"), arname);
	  break;
	case -3:
	  perror_with_name ("touch: ", arname);
	  break;
	case 1:
	  OSS (error, NILF,
		   _("touch: Section '%s' does not exist in '%s'"), memname, arname);
	  break;
	case 0:
	  val = 0;
	  break;
	default:
	  OS (error, NILF,
		  _("touch: Bad return code from ddldict_member_touch on '%s'"), name);
	}
  else
#endif
  switch (ar_member_touch (arname, memname))
    {
    case -1:
      OS (error, NILF, _("touch: Archive '%s' does not exist"), arname);
      break;
    case -2:
      OS (error, NILF, _("touch: '%s' is not a valid archive"), arname);
      break;
    case -3:
      perror_with_name ("touch: ", arname);
      break;
    case 1:
      OSS (error, NILF,
           _("touch: Member '%s' does not exist in '%s'"), memname, arname);
      break;
    case 0:
      val = 0;
      break;
    default:
      OS (error, NILF,
          _("touch: Bad return code from ar_member_touch on '%s'"), name);
    }

  free (arname);

  return val;
}
#endif /* !VMS */

/* State of an 'ar_glob' run, passed to 'ar_glob_match'.  */

/* On VMS, (object) modules in libraries do not have suffixes. That is, to
   find a match for a pattern, the pattern must not have any suffix. So the
   suffix of the pattern is saved and the pattern is stripped (ar_glob).
   If there is a match and the match, which is a module name, is added to
   the chain, the saved suffix is added back to construct a source filename
   (ar_glob_match). */

struct ar_glob_state
  {
    const char *arname;
    const char *pattern;
#ifdef VMS
    char *suffix;
#endif
    size_t size;
    struct nameseq *chain;
    unsigned int n;
  };

/* This function is called by 'ar_scan' to match one archive
   element against the pattern in STATE.  */

static long int
ar_glob_match (int desc UNUSED, const char *mem, int truncated UNUSED,
               long int hdrpos UNUSED, long int datapos UNUSED,
               long int size UNUSED, long int date UNUSED, int uid UNUSED,
               int gid UNUSED, unsigned int mode UNUSED, const void *arg)
{
  struct ar_glob_state *state = (struct ar_glob_state *)arg;

  if (fnmatch (state->pattern, mem, FNM_PATHNAME|FNM_PERIOD) == 0)
    {
      /* We have a match.  Add it to the chain.  */
      struct nameseq *new = xcalloc (state->size);
#ifdef VMS
      if (state->suffix)
        new->name = strcache_add(
            concat(5, state->arname, "(", mem, state->suffix, ")"));
      else
#endif
        new->name = strcache_add(concat(4, state->arname, "(", mem, ")"));
      new->next = state->chain;
      state->chain = new;
      ++state->n;
    }

  return 0L;
}

/* Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.  */
static int
ar_glob_pattern_p (const char *pattern, int quote)
{
  const char *p;
  int opened = 0;

  for (p = pattern; *p != '\0'; ++p)
    switch (*p)
      {
      case '?':
      case '*':
        return 1;

      case '\\':
        if (quote)
          ++p;
        break;

      case '[':
        opened = 1;
        break;

      case ']':
        if (opened)
          return 1;
        break;
      }

  return 0;
}

/* Glob for MEMBER_PATTERN in archive ARNAME.
   Return a malloc'd chain of matching elements (or nil if none).  */

struct nameseq *
ar_glob (const char *arname, const char *member_pattern, size_t size)
{
  struct ar_glob_state state;
  struct nameseq *n;
  const char **names;
  unsigned int i;
#ifdef VMS
  char *vms_member_pattern;
#endif
  if (! ar_glob_pattern_p (member_pattern, 1))
    return 0;

  /* Scan the archive for matches.
     ar_glob_match will accumulate them in STATE.chain.  */
  state.arname = arname;
  state.pattern = member_pattern;
#ifdef VMS
    {
      /* In a copy of the pattern, find the suffix, save it and  remove it from
         the pattern */
      char *lastdot;
      vms_member_pattern = xstrdup(member_pattern);
      lastdot = strrchr(vms_member_pattern, '.');
      state.suffix = lastdot;
      if (lastdot)
        {
          state.suffix = xstrdup(lastdot);
          *lastdot = 0;
        }
      state.pattern = vms_member_pattern;
    }
#endif
  state.size = size;
  state.chain = 0;
  state.n = 0;

#if defined(_GUARDIAN_TARGET)
  open_pobj_dll();
  open_copylib_dll();
  open_ddldict_dll();
  if (IS_POBJ_ENABLED && is_pobj_func(arname))
  pobj_scan_func (arname, pobj_glob_match_func, &state);
  else if (IS_COPYLIB_ENABLED && is_copylib_func(arname))
  copylib_scan_func (arname, copylib_glob_match_func, &state);
  else if (IS_DDLDICT_ENABLED && is_ddldict_func(arname))
  ddldict_scan_func (arname, ddldict_glob_match_func, &state);
  else
#endif
  ar_scan (arname, ar_glob_match, &state);

#ifdef VMS
  /* Deallocate any duplicated string */
  free(vms_member_pattern);
  if (state.suffix)
    {
      free(state.suffix);
    }
#endif

  if (state.chain == 0)
    return 0;

  /* Now put the names into a vector for sorting.  */
  names = (const char **) alloca (state.n * sizeof (const char *));
  i = 0;
  for (n = state.chain; n != 0; n = n->next)
    names[i++] = n->name;

  /* Sort them alphabetically.  */
  /* MSVC erroneously warns without a cast here.  */
  qsort ((void *)names, i, sizeof (*names), alpha_compare);

  /* Put them back into the chain in the sorted order.  */
  i = 0;
  for (n = state.chain; n != 0; n = n->next)
    n->name = names[i++];

  return state.chain;
}

#endif  /* Not NO_ARCHIVES.  */
