/* src/gmt_notposix.h.  Generated from gmt_notposix.h.in by configure.  */
/*--------------------------------------------------------------------
 *	$Id: gmt_notposix.h.in,v 1.12 2008/01/23 03:22:48 guru Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * This include file contains ifdefs that tell us if this system has
 * some of the several functions that are not part of POSIX but are
 * often distributed anyway as part of ANSI C.  The set of defines
 * below is automatically assigned by configure and determines if the
 * required functions are present or not.  These macros are then used
 * in gmt_math.h to choose between a function prototype (if found), an
 * alternative GMT function, or simply a macro.  The purpose is to
 * take advantage of the built-in functions if they exist and provide
 * alternative definitions otherwise.  For some non-Unix Operating
 * Systems, like Win32, these settings have already been hard-wired
 * into gmt_notunix.h and SET_IN_NOTUNIX is defined so we do not
 * override the system-specific settings.
 *
 * Version:	4.1.x
 */

#ifndef _GMT_NOTPOSIX_H
#define _GMT_NOTPOSIX_H

#ifndef SET_IN_NOTUNIX	/* Start of configure control */

/* >>> THIS SECTION WILL BE MODIFIED BY configure <<<
 * >>> DO NOT EDIT UNLESS YOU KNOW WHAT YOU ARE DOING!! <<< */

/* HAVE_<func> is set to 0 (FALSE) or 1 (TRUE) depending on whether or
 * not <func> is available on this system.  The default setting is 0:
 * none of the functions are available (the POSIX standard) */

#define HAVE_COPYSIGN 1
#define HAVE_LOG1P 1
#define HAVE_HYPOT 1
#define HAVE_ACOSH 1
#define HAVE_ASINH 1
#define HAVE_ATANH 1
#define HAVE_RINT 1
#define HAVE_IRINT 0
#define HAVE_ISNANF 1
#define HAVE_ISNAND 0
#define HAVE_ISNAN 1
#define HAVE_J0 1
#define HAVE_J1 1
#define HAVE_JN 1
#define HAVE_Y0 1
#define HAVE_Y1 1
#define HAVE_YN 1
#define HAVE_ERF 1
#define HAVE_ERFC 1
#define HAVE_STRDUP 1
#define HAVE_STRTOD 1
#define HAVE_SINCOS 1
#define HAVE_ALPHASINCOS 0
#define WORDS_BIGENDIAN 0

#endif	/* End of configure control */

#endif /* _GMT_NOTPOSIX_H */
