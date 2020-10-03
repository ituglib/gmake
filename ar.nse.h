#ifdef __TANDEM
#pragma columns 80
#endif
#ifndef _AR
#define _AR

/* T8629D41 - (15MAR96) - ar.h   archive-related definitions */
/**************************************************************************
**  @@@ START COPYRIGHT @@@                                              **
**  Tandem Confidential: Need to Know only                               **
**  Copyright (c) 1992-1996, Tandem Computers Incorporated               **
**  Protected as an unpublished work.                                    **
**  All Rights Reserved.                                                 **
**                                                                       **
**  The computer program listings, specifications, and documentation     **
**  herein are the property of Tandem Computers Incorporated or a third  **
**  party supplier and shall not be reproduced, copied, disclosed, or    **
**  used in whole or in part for any reason without the prior express    **
**  written permission of Tandem Computers Incorporated.                 **
**  @@@ END COPYRIGHT @@@                                                **
**************************************************************************/

#ifdef __cplusplus
   extern "C" {
#endif


/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      @(#)ar.h        5.3 (Berkeley) 3/12/91
 */

#define ARMAG           "!<arch>\n"     /* ar "magic number" */
#define SARMAG          8               /* strlen(ARMAG); */

/*
 * Yosemite definitions of header names for 'Symbol table' and 'Long member name' sections.
 */
#define TNSEHDRMAG		"/"		/* TNS/E symbol table header name */
#define STNSEHDRMAG		1		/* strlen(TNSEHDRMAG) */
#define LNSHDRNAME		"//"	/* TNS/E Long Member Name section header name */
#define SLNSHDRNAME		2		/* strlen(LNSHDRNAME) */
/* Yosemite changes ends */

#define AR_EFMT1        "#1/"           /* extended format #1 */

struct ar_hdr {
        char ar_name[16];               /* name */
        char ar_date[12];               /* modification time */
        char ar_uid[6];                 /* user id */
        char ar_gid[6];                 /* group id */
        char ar_mode[8];                /* octal file permissions */
        char ar_size[10];               /* size in bytes */
#define ARFMAG  "`\n"
        char ar_fmag[2];                /* consistency check */
};


#ifdef __cplusplus
   }
#endif

/*  ----------------- End of ar.h -----------------  */

#endif  /* _AR defined */
