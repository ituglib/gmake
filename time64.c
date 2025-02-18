/**
 * @file
 * 64-bit time conversion for GMAKE extension libraries.
 * @author Randall S. Becker
 * @copyright Copyright (c) 2025 Nexbridge Inc. All rights reserved. Proprietary and
 * confidential. Disclosure without written permission violates international
 * laws and will be prosecuted.
 */
#include "time64.h" // Must be first.

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <cextdecs.h>
#include <time.h>


static unsigned long long time64_to_lct(unsigned long long julianGmt) {
	return CONVERTTIMESTAMP(julianGmt, 0);
}

int64_t time64_ext(int64_t *now) {
	int64_t unixNow = time64_from_julian(JULIANTIMESTAMP());

	if (now) {
		*now = unixNow;
	}

	return unixNow;
}

struct tm *localtime64_ext(const int64_t *current) {
	unsigned long long tandemNowLct = time64_to_lct(time64_to_julian(*current));
	short dateAndTime[8];
	struct tm *tm = (struct tm *) calloc(1, sizeof(struct tm));

	INTERPRETTIMESTAMP(tandemNowLct, dateAndTime);
	tm->tm_year = dateAndTime[0] - 1900;
	tm->tm_mon = dateAndTime[1] - 1;
	tm->tm_mday = dateAndTime[2];
	tm->tm_hour = dateAndTime[3];
	tm->tm_min = dateAndTime[4];
	tm->tm_sec = dateAndTime[5];

	return tm;
}

char *ctime64_ext(const int64_t *current) {
	static char buffer[256];
	struct tm *tm = localtime64_ext(current);
	char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", //
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};

	snprintf(buffer, sizeof(buffer), "%04d %s %d %02d:%02d:%02d\n",
			tm->tm_year, tm->tm_mon, tm->tm_mday, //
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return buffer;
}

int64_t time64_from_julian(const unsigned long long julian) {
	int64_t unixTime = (julian/1000000LL) - CONVERT_JULIAN_TO_UNIX;

	return unixTime;
}

unsigned long long time64_to_julian(const int64_t unixTime) {
	unsigned long long julian = (unixTime + CONVERT_JULIAN_TO_UNIX) * 1000000LL;

	return julian;
}

