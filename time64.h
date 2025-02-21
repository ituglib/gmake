#ifndef _EXT_TIME64_H_
#define _EXT_TIME64_H_
/**
 * @file
 * 64-bit time conversion for GMAKE extension libraries.
 * @author Randall S. Becker
 * @copyright Copyright (c) 2025 Nexbridge Inc. All rights reserved. Proprietary and
 * confidential. Disclosure without written permission violates international
 * laws and will be prosecuted.
 */

#define _LARGEFILE64_SOURCE 1
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1

#include <sys/types.h>
#include <stdint.h>

/**
 * Conversion between Julian seconds to Unix seconds, through 3000+
 */
#define CONVERT_JULIAN_TO_UNIX 210866760000LL

/** Conversion between old and new times. */
#define CONVERT_48_BIT_OFFSET 211024440000000000LL

/**
 * A timestamp is a six byte binary value which indicates the number of
 * hundredths of a second since midnight, January 1, 1975.  The current time
 * is returned, as a timestamp, by the Guardian TIMESTAMP procedure.  Dates
 * and times should be stored in the database using timestamps.
 */
typedef char                            internal_timestamp_def[6];

#pragma fieldalign platform stat64_post2038
/**
 * Structure for the post 2038 stat, compatible with
 */
struct  stat64_post2038 {
        dev_t   st_dev;
        ino_t   st_ino;
        mode_t  st_mode;
        nlink_t st_nlink;
        unsigned int st_acl:1;
        unsigned int __filler_1:7;
        unsigned int st_fileprivs:8;   /* File privileges */
        uid_t   st_uid;
        gid_t   st_gid;
        dev_t   st_rdev;
        int64_t   st_size;
        int64_t st_atime;
        int64_t st_mtime;
        int64_t st_ctime;
        mode_t  st_basemode; /* ACL:  owning user and other permissions */
        int     st_reserved4;
        int64_t st_reserved8[3];
}; /* struct stat */
int    stat64_post2038_fcn(const char *, struct stat64_post2038 *);

#pragma function stat64_post2038_fcn     (alias("statLP64_"), unspecified)

/**
 * Get a 64-bit time from a 32-bit application.
 *
 * @param now the variable into which to place the time. If this variable is
 *     NULL, the time is returned but not set in this argument.
 * @return the time value.
 */
int64_t time64_ext(int64_t *now);

struct tm *localtime64_ext(const int64_t *current);

char *ctime64_ext(const int64_t *current);

/**
 * Convert a julian timestamp to a UNIX 64-bit timestamp.
 * @param julian the 64-bit julian timestamp in microseconds.
 * @return the converted 64-bit UNIX time.
 */
int64_t time64_from_julian(const unsigned long long julian);

/**
 * Convert a UNIX 64-bit timestamp to a julian timestamp.
 * @param unixTime the UNIX time. This can be any epoch within a NonStop value range.
 * @return the converted julian timestamp.
 */
unsigned long long time64_to_julian(const int64_t unixTime);

#endif /* _EXT_TIME64_H_ */
