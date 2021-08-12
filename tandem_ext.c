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

/* NSK version of UNIX stat */

int stat(const char *pathname, struct stat *st)
{
  short rc, datetime[8], infolist = ZSYS_VAL_FINF_AGGRMODIFY_LCT; /* 145 */
  int julday;
  long long i64;
  struct tm caltime;

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
  st->st_mode  = 0; /* GUARDIANSECURITY 62 */
  st->st_nlink = 0;
  st->st_uid   = 0; /* UID 167 */
  st->st_gid   = 0; /* GID 164 */
  st->st_rdev  = 0;
  st->st_size  = 0; /* AGGRENDOFFILE 142 */
  st->st_atime = 0; /* LASTOPEN_LCT 117 */
  st->st_ctime = 0; /* CREATION_LCT 119 */

  return rc ? -1 : 0;
}
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
