#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <cextdecs.h>
#ifdef _OSS_HOST
#include <zsysc>
#else
#include <zsysc.h>
#endif
#include <errno.h>

#include "makeint.h"
#include "debug.h"
# define DDLDICT_SUPPRESS_EXTERNALIZATION_VERSION
# define DDLDICT_EXTERN extern
#include "ddldict.h"

/**
 * Copy into a buffer, taking account a terminator, like ., and the maximum size
 * of the buffer. This continues scanning the source buffer until the terminator
 * is encountered.
 *
 * @param dest the buffer to receive contents. Null terminated on exit.
 * @param destSize the maximum size of the buffer, including the null.
 * @param source the null-terminated source buffer.
 * @param terminator the character that terminates the copy.
 * @return a pointer beyond the terminator or at the terminating null.
 */
static const char *safe_copy_to_lower(char *dest, size_t destSize, const char *source,
		const char terminator) {
  char *t = dest;
  const char *s=source;

  for (; *s && *s != terminator && destSize > 1; s++, t++) {
    *t = (char) tolower(*s);
  }
  *t = '\0';
  for (; *s && *s != terminator; s++) {
  }
  if (*s == terminator)
	return s+1;
  return s;
}

/* NSK version of UNIX getwd (similar to getcwd) */

char *getwd(char *pathname)
{
  pathname = getenv("DEFAULTS");
  if (!pathname) {
    strcpy(pathname, "getenv(DEFAULTS) failed");
    return NULL;
  }
  return pathname;
}

/*
 * Numbers are assigned to the enumeration to coordinate with unit tests in t0001.
 */
typedef enum {
  NSKPATHTYPE_UNKNOWN = 0,
  NSKPATHTYPE_SIMPLE_FILE = 1,
  NSKPATHTYPE_SUBVOLUME_FILE = 2,
  NSKPATHTYPE_FULL_FILE = 3,
  NSKPATHTYPE_SUBVOLUME = 4,
  NSKPATHTYPE_VOLUME = 5,
  NSKPATHTYPE_NODE = 6,
  NSKPATHTYPE_EXPAND_VOLUME = 7,
  NSKPATHTYPE_EXPAND_SUBVOLUME = 8,
  NSKPATHTYPE_EXPAND_FILE = 9,
} NskPathType;

/**
 * Figure out what kind of name is supplied. This does not check for full
 * syntax or character types. FILE_GETINFOBYNAMELIST_ can check the syntax.
 * @param pathname the name to check.
 * @return a value from NskPathType.
 */
static NskPathType path_type(const char *pathname) {
  const char *dot;
  const char *dot2;

  if (!pathname || !pathname[0])
    return NSKPATHTYPE_UNKNOWN;

  if (pathname[0] == '.' && pathname[1] == '\0')
    return NSKPATHTYPE_SUBVOLUME;

  if (pathname[0] == '\\') {
    /* This could be a \node.$volume.subvolume.file, \node.$volume.subvolume, \node.$volume, or \node. */
    dot = strchr(pathname, '.');
    if (!dot) {
      if (strlen(pathname) > 8)
        return NSKPATHTYPE_UNKNOWN;
      return NSKPATHTYPE_NODE;
    }
    dot++;
    /* dot should point to the $volume. check the node name length. */
    if (!*dot)
      return NSKPATHTYPE_NODE;
    if (*dot != '$' || dot - pathname > 8)
      return NSKPATHTYPE_UNKNOWN;
    dot2 = strchr(dot, '.');
    if (!dot2) {
      if (strlen(dot) > 8)
        return NSKPATHTYPE_UNKNOWN;
      return NSKPATHTYPE_EXPAND_VOLUME;
    }
    dot2++;
    if (dot2 - dot > 9)
      return NSKPATHTYPE_UNKNOWN;
    dot = dot2;
    /* dot should point to the subvolume */
    dot2 = strchr(dot, '.');
    if (!dot2) {
      if (strlen(dot) > 8)
        return NSKPATHTYPE_UNKNOWN;
      return NSKPATHTYPE_EXPAND_SUBVOLUME;
    }
    if (!(dot2[1] >= 'A' && dot2[1] <= 'Z') &&
          !(dot2[1] >= 'a' && dot2[1] <= 'z'))
      return NSKPATHTYPE_UNKNOWN;
    dot = dot2+1;
    /* dot should point to the file. */
    dot2 = strchr(dot, '.');
    if (dot2)
      return NSKPATHTYPE_UNKNOWN;
    if (strlen(dot) > 8)
      return NSKPATHTYPE_UNKNOWN;
    if (!(dot[1] >= 'A' && dot[1] <= 'Z') &&
          !(dot[1] >= 'a' && dot[1] <= 'z'))
      return NSKPATHTYPE_UNKNOWN;
    return NSKPATHTYPE_EXPAND_FILE;

  } else if (pathname[0] == '$') {
    /* This could be a $volume.subvolume.file, $volume.subvolume  or just a $volume. */
    dot = strchr(pathname, '.');
    if (!dot) {
      if (strlen(pathname) > 7)
        return NSKPATHTYPE_UNKNOWN;
      return NSKPATHTYPE_VOLUME;
    }
    dot++;
    /* dot should point to the subvolume */
    if (dot - pathname > 8)
      return NSKPATHTYPE_UNKNOWN;
    dot2 = strchr(dot, '.');
    if (!dot2) {
      if (strlen(dot) > 8)
        return NSKPATHTYPE_UNKNOWN;
      return NSKPATHTYPE_SUBVOLUME;
    }
    dot = dot2+1;
    /* dot should point to the file. */
    dot2 = strchr(dot, '.');
    if (dot2)
      return NSKPATHTYPE_UNKNOWN;
    if (strlen(dot2) > 8)
      return NSKPATHTYPE_UNKNOWN;
    return NSKPATHTYPE_FULL_FILE;

  } else if ((pathname[0] >= 'A' && pathname[0] <= 'Z') ||
          (pathname[0] >= 'a' && pathname[0] <= 'z')) {
    /* This could be a subvolume.file or just a file. */
    dot = strchr(pathname, '.');
    if (!dot) {
      if (strlen(pathname) > 8)
        return NSKPATHTYPE_UNKNOWN;
      return NSKPATHTYPE_SIMPLE_FILE;
    }
    if (dot - pathname > 8)
      return NSKPATHTYPE_UNKNOWN;
    dot2 = strchr(dot, '.');
    if (dot2)
      return NSKPATHTYPE_UNKNOWN;
    if (strlen(dot+1) > 8)
      return NSKPATHTYPE_UNKNOWN;
    return NSKPATHTYPE_SUBVOLUME_FILE;
  }
  return NSKPATHTYPE_UNKNOWN;
}

char *toOss(char *ossName, size_t ossNameLength, const char *guardianName) {
  const char *s;
  const char *postNode = NULL;
  char defaultSubvolume[47];
  char *defaultVol = NULL;
  char *defaultSubvol = NULL;
  char node[8];
  char volume[8];
  char subvolume[9];
  char file[9];
  NskPathType pathType;

  if (!guardianName)
    return NULL;

  s = getenv("DEFAULTS");
  if (!s) {
    strcpy(defaultSubvolume, "");
  } else {
    safe_copy_to_lower(defaultSubvolume, sizeof(defaultSubvolume), s, '\0');
    defaultVol = strchr(defaultSubvolume, '$');
    if (defaultVol) {
      defaultVol++;
      defaultSubvol = strchr(defaultVol, '.');
      if (defaultSubvol) {
        *defaultSubvol++ = '\0';
      }
    }
  }
  if (strcmp(guardianName, "\\") == 0) {
    snprintf(ossName, ossNameLength, "/E");
    return ossName;
  }

  if (strcmp(guardianName, ".") == 0 || strcmp(guardianName, "./") == 0) {
    snprintf(ossName, ossNameLength, "/G/%s/%s", defaultVol, defaultSubvolume);
    return ossName;
  }

  pathType = path_type(guardianName);

  switch(pathType) {
  case NSKPATHTYPE_UNKNOWN:
    strcpy(ossName, "");
    return ossName;
  case NSKPATHTYPE_NODE:
  case NSKPATHTYPE_EXPAND_VOLUME:
  case NSKPATHTYPE_EXPAND_SUBVOLUME:
  case NSKPATHTYPE_EXPAND_FILE:
    s = safe_copy_to_lower(node, sizeof(node), guardianName+1, '.');
    if (*s) {
      guardianName = s;
    }
    postNode = s;
    break;
  default:
    break;
  }

  switch(pathType) {
  case NSKPATHTYPE_NODE:
    snprintf(ossName, ossNameLength, "/E/%s", node);
    break;
  case NSKPATHTYPE_UNKNOWN:
    break; // Cannot happen
  case NSKPATHTYPE_SIMPLE_FILE:
    safe_copy_to_lower(file, sizeof(file), guardianName, '.');
    snprintf(ossName, ossNameLength, "/G/%s/%s/%s", defaultVol, defaultSubvol, file);
    break;
  case NSKPATHTYPE_SUBVOLUME_FILE:
    s = safe_copy_to_lower(subvolume, sizeof(subvolume), guardianName, '.');
    safe_copy_to_lower(file, sizeof(file), s, '.');
    snprintf(ossName, ossNameLength, "/G/%s/%s/%s", defaultVol, subvolume, file);
    break;
  case NSKPATHTYPE_FULL_FILE:
    s = safe_copy_to_lower(volume, sizeof(volume), guardianName+1, '.');
    s = safe_copy_to_lower(subvolume, sizeof(subvolume), s, '.');
    safe_copy_to_lower(file, sizeof(file), s, '.');
    snprintf(ossName, ossNameLength, "/G/%s/%s/%s", volume, subvolume, file);
    break;
  case NSKPATHTYPE_SUBVOLUME:
    s = safe_copy_to_lower(volume, sizeof(volume), guardianName+1, '.');
    safe_copy_to_lower(subvolume, sizeof(subvolume), s, '.');
    snprintf(ossName, ossNameLength, "/G/%s/%s", volume, subvolume);
    break;
  case NSKPATHTYPE_VOLUME:
    safe_copy_to_lower(volume, sizeof(volume), guardianName+1, '.');
    snprintf(ossName, ossNameLength, "/G/%s", volume);
    break;
  case NSKPATHTYPE_EXPAND_VOLUME:
    safe_copy_to_lower(volume, sizeof(volume), postNode+1, '.');
    snprintf(ossName, ossNameLength, "/E/%s/G/%s", node, volume);
    break;
  case NSKPATHTYPE_EXPAND_SUBVOLUME:
    s = safe_copy_to_lower(volume, sizeof(volume), postNode+1, '.');
    safe_copy_to_lower(subvolume, sizeof(subvolume), s, '.');
    snprintf(ossName, ossNameLength, "/E/%s/G/%s/%s", node, volume, subvolume);
    break;
  case NSKPATHTYPE_EXPAND_FILE:
    s = safe_copy_to_lower(volume, sizeof(volume), postNode+1, '.');
    s = safe_copy_to_lower(subvolume, sizeof(subvolume), s, '.');
    safe_copy_to_lower(file, sizeof(file), s, '.');
    snprintf(ossName, ossNameLength, "/E/%s/G/%s/%s/%s", node, volume, subvolume, file);
    break;
  default:
    break;
  }
  return ossName;
}

char *toGuardian(char *guardianName, const char *ossName) {
  const char *s;
  char *t = guardianName;

  if (!ossName)
    return NULL;
  if (ossName[0] != '/' || strcmp(ossName, ".") == 0 || strcmp(ossName, "./") == 0 ||
      strcmp(ossName, "/G") == 0 || strcmp(ossName, "") == 0) {
    strcpy(guardianName, getenv("DEFAULTS"));
    for (; *t; *t++) {
      *t = (char) tolower(*t);
    }
    return guardianName;
  }
  ossName++;
  if (ossName[0] == 'G' && ossName[1] == '/') {
    ossName += 2; // points to the volume.
  } else if (ossName[0] == 'E' && ossName[1] == '/') {
    ossName += 2; // points to the node.
    *t++ = '\\';
    // Copy the node
    for (s=ossName; *s && *s != '/'; s++) {
      *t++ = *s;
    }
    if (*s) {
      ossName = s+1; // points to the volume.
      if (strcmp(ossName, "G") == 0 || strcmp(ossName, "G/") == 0) {
        *t = '\0';
        return guardianName;
      }
      if (ossName[0] == 'G') {
        if (ossName[1] == '/') {
          ossName += 2;
        } else {
          ossName += 1;
        }
      }
      *t++ = '.';
    } else {
      ossName = s; // points after node to NULL
    }
  }
  // Copy the volume if it is there.
  if (*ossName) {
    *t++ = '$';
  }
  for (s=ossName; *s && *s != '/'; s++) {
    *t++ = *s;
  }
  // Copy the subvolume, if it is there.
  if (*s) {
    s++;
    *t++ = '.';
  }
  for (; *s && *s != '/'; s++) {
    *t++ = *s;
  }
  // Copy the file, if it is there.
  if (*s) {
    s++;
    *t++ = '.';
  }
  for (; *s && *s != '/'; s++) {
    *t++ = *s;
  }
  *t = '\0';
  return guardianName;
}

#ifndef UNIT_TEST

/* NSK version of UNIX stat */

int stat(const char *pathname, struct stat *st)
{
  short rc, datetime[8], infolist = ZSYS_VAL_FINF_AGGRMODIFY_LCT; /* 145 */
  int julday;
  long long i64;
  struct tm caltime;
  char oss_pathname[PATH_MAX+1];

  if (pathname && pathname[0] == '.' && pathname[1] == '/') {
    pathname += 2;
  }
  if (pathname && pathname[0] == '/') {
	    pathname = toGuardian(oss_pathname, pathname);
  }

  memset(st, 0, sizeof(struct stat));

  switch (path_type(pathname)) {
  case NSKPATHTYPE_EXPAND_FILE:
  case NSKPATHTYPE_SUBVOLUME_FILE:
  case NSKPATHTYPE_FULL_FILE:
  case NSKPATHTYPE_SIMPLE_FILE:
    if (ISDB(DB_BASIC)) {
	      printf("%s resolves to a Guardian file\n", pathname);
    }
    rc = FILE_GETINFOLISTBYNAME_( pathname, (short) strlen(pathname), &infolist,
                                1, (short *) &i64, sizeof( i64 ) );
    if (rc == 11) {
      open_ddldict_dll();
      if (IS_DDLDICT_ENABLED && is_ddldict_func(pathname)) {
        char dictpath[ZSYS_VAL_LEN_FILENAME+1];
        strcpy(dictpath, pathname);
        strcat(dictpath, "odf");
        rc = FILE_GETINFOLISTBYNAME_( dictpath, (short) strlen(dictpath), &infolist,
                                      1, (short *) &i64, sizeof( i64 ) );
      }
    }
    if (rc) {
      if (ISDB(DB_BASIC)) {
        printf("FILE_GETINFOLISTBYNAME_ %s failed, rc = %d\n", pathname, rc);
      }
      if (rc == 11) {
        errno = ENOENT;
        return ENOENT;
      }
      return -1;
    }
    julday = INTERPRETTIMESTAMP(i64, datetime);
    if (julday == -1) {
      if (ISDB(DB_BASIC)) {
        printf("INTREPRETTIMESTAMP %Ld failed, rc = -1\n", i64);
      }
      return -1;
    }
    if (ISDB(DB_BASIC)) {
      printf("%s AGGRMODIFY_LCT = %04d/%02d/%02d %02d:%02d:%02d\n", pathname,
             datetime[0], datetime[1], datetime[2],
             datetime[3], datetime[4], datetime[5]);
    }
    tzset(); /* set daylight if needed */
    memset(&caltime, 0, sizeof(caltime));
    caltime.tm_year = datetime[0] - 1900;
    caltime.tm_mon  = datetime[1] - 1;
    caltime.tm_mday = datetime[2];
    caltime.tm_hour = datetime[3];
    caltime.tm_min  = datetime[4];
    caltime.tm_sec  = datetime[5];
    caltime.tm_isdst = daylight;

    st->st_mtime = mktime(&caltime);
    /* If mktime returns an error, something is very wrong... probably the DST */
    /* errno is usually 0 in this case so printing it would just be confusing */
    if (st->st_mtime == -1) {
      printf("mktime returned -1 for the timestamp from %s.\n"
             "The DST table may need to be rebuilt.\n", pathname);
      if (ISDB(DB_BASIC)) {
        printf("mktime updated the tm struct to %04d/%02d/%02d %02d:%02d:%02d\n"
               "and set wday to %d, yday to %d and isdst to %d: see time.h\n",
               caltime.tm_year, caltime.tm_mon, caltime.tm_mday, caltime.tm_hour,
               caltime.tm_min, caltime.tm_sec, caltime.tm_wday, caltime.tm_yday,
               caltime.tm_isdst);
      }
      /* Returning will just result in showing the file timestamp in the debug */
      /* info and then having gmake say the file does not exist. */
      abort();
    } else {
	      if (ISDB(DB_BASIC)) {
	        printf("mktime AGGRMODIFY_LCT is %d\n", st->st_mtime);
	      }
    }
    st->st_dev   = 0;
    st->st_ino   = 0;
    st->st_mode  = _S_IFREG; /* GUARDIANSECURITY 62 */
    st->st_nlink = 0;
    st->st_uid   = 0; /* UID 167 */
    st->st_gid   = 0; /* GID 164 */
    st->st_rdev  = 0;
    st->st_size  = 0; /* AGGRENDOFFILE 142 */
    st->st_atime = 0; /* LASTOPEN_LCT 117 */
    st->st_ctime = 0; /* CREATION_LCT 119 */

    return rc ? -1 : 0;
  case NSKPATHTYPE_EXPAND_SUBVOLUME:
  case NSKPATHTYPE_SUBVOLUME:
    if (ISDB(DB_BASIC)) {
	      printf("%s resolves to a Guardian subvolume\n", pathname);
    }
    st->st_mtime = 0;
    st->st_dev   = 0;
    st->st_ino   = 0;
    st->st_mode  = _S_IFDIR; /* GUARDIANSECURITY 62 */
    st->st_nlink = 0;
    st->st_uid   = 0; /* UID 167 */
    st->st_gid   = 0; /* GID 164 */
    st->st_rdev  = 0;
    st->st_size  = 0; /* AGGRENDOFFILE 142 */
    st->st_atime = 0; /* LASTOPEN_LCT 117 */
    st->st_ctime = 0; /* CREATION_LCT 119 */
    return 0;
  default:
    break;
  }
  return -1;
}
#endif /* ! UNIT_TEST */

/*===========================================================================*/
#pragma page "T0593 GUARDIAN GNU Make- tandem_ext.c Change Descriptions"
/*===========================================================================*/

/*
------------------------------------------------------------------------------
Yosemite 09/23/06                                                  H01:TLW0008
  - Added error handling for stat mktime
------------------------------------------------------------------------------
 End stanfunc change descriptions
*/
