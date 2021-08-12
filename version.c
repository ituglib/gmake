/* Record version and build host architecture for GNU make.
Copyright (C) 1988-2014 Free Software Foundation, Inc.
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

/* We use <config.h> instead of "config.h" so that a compilation
   using -I. -I$srcdir will use ./config.h rather than $srcdir/config.h
   (which it would do because makeint.h was found in $srcdir).  */
#include <config.h>

#ifndef MAKE_HOST
# define MAKE_HOST "unknown"
#endif

const char *version_string = VERSION;
const char *make_host = MAKE_HOST;

/*
  Local variables:
  version-control: never
  End:
 */
#ifdef _GUARDIAN_TARGET
#pragma section VERSION_STRING
#define VERSION_STRING "T0593H01 - (30SEP2020)" __DATE__ __TIME__

#pragma section VERSION_GMAKE
void VPROC (void)
{}

#pragma section CHANGES
#if 0
-----------------------------------------------------------------------------
02 Dec 2000                                                     YODL:TLW0001
- Created this file.
-----------------------------------------------------------------------------
13 Jan 2002                                                     YODL:BLS0002
- Updated this file.
-----------------------------------------------------------------------------
02 Apr 2002                                                     YODL:BLS0003
- Updated this file.
-----------------------------------------------------------------------------
20 Jul 2002
- Updated Vproc date to 01MAY2003                               R.BERRY
-----------------------------------------------------------------------------
03 SEP 2003
- Updated Vproc date to 01OCT2004 and PV to H01                 SCONLEY
-----------------------------------------------------------------------------
18 OCT 2006
- Updated Vproc date to 01FEB2007 FOR H01 AAA                   SCONLEY
-----------------------------------------------------------------------------
19 SEP 2014
- Updated Vproc date to 01FEB2014 FOR H01 AAC                   DHERBST
-----------------------------------------------------------------------------
30 SEP 2020
- Port changes for GUARDIAN GNUMake 4.1 for H01 AAD             RBECKER
-----------------------------------------------------------------------------
#endif
#endif
