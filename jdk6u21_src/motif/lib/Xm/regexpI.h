/*
 *      $XConsortium: regexpI.h /main/2 1995/07/13 18:30:38 drk $
 *
 *      @(#)regexpI.h	1.3 04 Mar 1994
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 2002 Sun Microsystems, Inc.  All rights reserved.
 *
 */

#ifndef _XmRegexpI_h
#define _XmRegexpI_h

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#define NSUBEXP  10
#define	MAGIC	0234

typedef struct _XmRegexpRec {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	long regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} XmRegexpRec;

extern XmRegexpRec *_XmRegcomp(char *s);
extern int _XmRegexec(XmRegexpRec *r, char *s);

#endif /* _XmRegexpI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
