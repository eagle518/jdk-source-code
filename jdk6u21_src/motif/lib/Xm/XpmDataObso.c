/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/************************************************************************* 
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 2002 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *************************************************************************/
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: XpmData.c /main/cde1_maint/2 1995/08/18 19:39:01 drk $"
#endif
#endif
/* Copyright 1990-92 GROUPE BULL -- See license conditions in file COPYRIGHT */
/*****************************************************************************\
 * XpmData.c:                                                                *
 * XPM library  * *  IO utilities                                            *
 * Developed by Arnaud Le Hors                                               *
 * Added for backward compatibility with 1.2 API			     *
\*****************************************************************************/

/* Official version number */
static char *RCS_Version = "$XpmVersion: 3.2c $";

#include <Xm/XpmI.h>
#ifdef VMS
#include "sys$library:stat.h"
#include "sys$library:ctype.h"
#else
#include <sys/stat.h>
#include <ctype.h>
#endif

#define _ungetc_unlocked ungetc

LFUNC(ParseComment, int, (xpmData *mdata));

static int
ParseComment(mdata)
    xpmData *mdata;
{
    FILE *file = mdata->stream.file;
    register int c;
    register unsigned int n = 0;
	int a; /* Wyoming 64-bit fix */ 
    unsigned int notend;
    char *s, *s2;

    s = mdata->Comment;
    *s = mdata->Bcmt[0];

    /* skip the string beginning comment */
    s2 = mdata->Bcmt;
    do {
	c = getc_unlocked(file);
	*++s = c;
	n++;
	s2++;
    } while (c == *s2 && *s2 != '\0'
	     && c != EOF && c != mdata->Bos);

    if (*s2 != '\0') {
	/* this wasn't the beginning of a comment */
	/* put characters back in the order that we got them */
	for (a = n; a > 0; a--, s--)
	    _ungetc_unlocked(*s, file);
	return;
    }

    /* store comment */
    /*
     * Make sure the copying of comment data
     * is limited to the size of the comment 
     * buffer, which is of size BUFSIZ
     *
     */
    mdata->Comment[0] = *s;
    s = mdata->Comment;
    notend = 1;
    n = 0;
    while (notend) {
	s2 = mdata->Ecmt;
	while (*s != *s2 && c != EOF && c != mdata->Bos) {
	    c = getc_unlocked(file);
	    if (n >= (BUFSIZ - 1))
		*s = c;
	    else
		*++s = c;
	    n++;
	}
	mdata->CommentLength = (n >= (BUFSIZ - 1)) ? BUFSIZ - 1 : n;
	do {
	    c = getc_unlocked(file);
	    if (n >= (BUFSIZ - 1))
		*s = c;
	    else
		*++s = c;
	    n++;
	    s2++;
	} while (c == *s2 && *s2 != '\0'
		 && c != EOF && c != mdata->Bos);
	if (*s2 == '\0') {
	    /* this is the end of the comment */
	    notend = 0;
	    _ungetc_unlocked(*s, file);
	}
    }
}

unsigned int
_XmXpmatoui(p, l, ui_return)
    register char *p;
    unsigned int l;
    unsigned int *ui_return;
{
    register int n, i;

    n = 0;
    for (i = 0; i < l; i++)
	if (*p >= '0' && *p <= '9')
	    n = n * 10 + *p++ - '0';
	else
	    break;

    if (i != 0 && i == l) {
	*ui_return = n;
	return 1;
    } else
	return 0;
}

_XmxpmNextString(mdata)
    xpmData *mdata;
{
    if (!mdata->type)
	mdata->cptr = (mdata->stream.data)[++mdata->line];
    else {
	register int c;
	FILE *file = mdata->stream.file;

	/* get to the end of the current string */
	if (mdata->Eos)
	    while ((c = getc_unlocked(file)) != mdata->Eos && c != EOF);

	/* then get to the beginning of the next string
	 * looking for possible comment */
	if (mdata->Bos) {
	    while ((c = getc_unlocked(file)) != mdata->Bos && c != EOF)
		if (mdata->Bcmt && c == mdata->Bcmt[0])
		    ParseComment(mdata);
	    
	} else {			/* XPM2 natural */
	    while (mdata->Bcmt && (c = getc_unlocked(file)) == mdata->Bcmt[0])
		ParseComment(mdata);
	    _ungetc_unlocked(c, file);
	}
    }
}

int
_XmxpmNextUI(mdata, ui_return)
    xpmData *mdata;
    unsigned int *ui_return;
{
    char buf[BUFSIZ];
    int l;

    l = _XmxpmNextWord(mdata, buf, BUFSIZ);
    return _XmXpmatoui(buf, l, ui_return);
}

unsigned int
_XmxpmNextWord(mdata, buf, buflen)
    xpmData *mdata;
    char *buf;
    unsigned int buflen;
{
    register unsigned int n = 0;
    int c;

    if (!mdata->type) {
	while (isspace(c = *mdata->cptr) && c != mdata->Eos)
	    mdata->cptr++;
	do {
	    c = *mdata->cptr++;
	    *buf++ = c;
	    n++;
	} while (!isspace(c) && c != mdata->Eos && n < buflen);
	n--;
	mdata->cptr--;
    } else {
	FILE *file = mdata->stream.file;
	while ((c = getc_unlocked(file)) != EOF && isspace(c) && c != mdata->Eos);
	while (!isspace(c) && c != mdata->Eos && c != EOF && n < buflen) {
	    *buf++ = c;
	    n++;
	    c = getc_unlocked(file);
	}
	_ungetc_unlocked(c, file);
    }
    return (n);
}

int
_XmxpmGetString(mdata, sptr, l)
    xpmData *mdata;
    char **sptr;
    unsigned int *l;
{
    unsigned int i;
	long n = 0;
    int c;
    char *p, *q, buf[BUFSIZ];

    if (!mdata->type) {
	if (mdata->cptr) {
	    char *start;
	    while (isspace(c = *mdata->cptr) && c != mdata->Eos)
		mdata->cptr++;
	    start = mdata->cptr;
	    while (c = *mdata->cptr)
		mdata->cptr++;
	    n = mdata->cptr - start + 1;
	    p = (char *) malloc(n);
	    if (!p)
		return (XpmNoMemory);
	    strncpy(p, start, n);
	}
    } else {
	FILE *file = mdata->stream.file;
	while (isspace(c = getc_unlocked(file)) && c != mdata->Eos);
	if (c == EOF)
	    return (XpmFileInvalid);
	p = NULL;
	i = 0;
	q = buf;
	p = (char *) malloc(1);
	while (c != mdata->Eos && c != EOF) {
	    if (i == BUFSIZ) {
		/* get to the end of the buffer */
		/* malloc needed memory */
		q = (char *) realloc(p, n + i);
		if (!q) {
		    free(p);
		    return (XpmNoMemory);
		}
		p = q;
		q += n;
		/* and copy what we already have */
		strncpy(q, buf, i);
		n += i;
		i = 0;
		q = buf;
	    }
	    *q++ = c;
	    i++;
	    c = getc_unlocked(file);
	}
	if (c == EOF) {
	    free(p);
	    return (XpmFileInvalid);
	}
	if (n + i != 0) {
	    /* malloc needed memory */
	    q = (char *) realloc(p, n + i + 1);
	    if (!q) {
		free(p);
		return (XpmNoMemory);
	    }
	    p = q;
	    q += n;
	    /* and copy the buffer */
	    strncpy(q, buf, i);
	    n += i;
	    p[n++] = '\0';
	} else {
	    *p = '\0';
	    n = 1;
	}
	_ungetc_unlocked(c, file);
    }
    *sptr = p;
    *l = n;
    return (XpmSuccess);
}

_XmxpmGetCmt(mdata, cmt)
    xpmData *mdata;
    char **cmt;
{
    if (!mdata->type)
	*cmt = NULL;
    else
	if (mdata->CommentLength) {
	    *cmt = (char *) malloc(mdata->CommentLength + 1);
	    strncpy(*cmt, mdata->Comment, mdata->CommentLength);
	    (*cmt)[mdata->CommentLength] = '\0';
	    mdata->CommentLength = 0;
	} else
	    *cmt = NULL;
}

int
_XmxpmReadFile(filename, mdata)
    char *filename;
    xpmData *mdata;
{
    char *compressfile, buf[BUFSIZ];
    struct stat status;

    if (!filename) {
	mdata->stream.file = (stdin);
	mdata->type = XPMFILE;
    } else {
#ifdef ZPIPE
	if ((strlen(filename) > 2) &&
	    !strcmp(".Z", filename + (strlen(filename) - 2))) {
	    mdata->type = XPMPIPE;
	    sprintf(buf, "uncompress -c %s", filename);
	    if (!(mdata->stream.file = popen(buf, "r")))
		return (XpmOpenFailed);

	} else {
	    if (!(compressfile = (char *) malloc(strlen(filename) + 3)))
		return (XpmNoMemory);

	    strcpy(compressfile, filename);
	    strcat(compressfile, ".Z");
	    if (!stat(compressfile, &status)) {
		sprintf(buf, "uncompress -c %s", compressfile);
		if (!(mdata->stream.file = popen(buf, "r"))) {
		    free(compressfile);
		    return (XpmOpenFailed);
		}
		mdata->type = XPMPIPE;
	    } else {
#endif
		if (!(mdata->stream.file = fopen(filename, "r"))) {
#ifdef ZPIPE
		    free(compressfile);
#endif
		    return (XpmOpenFailed);
		}
		mdata->type = XPMFILE;
#ifdef ZPIPE
	    }
	    free(compressfile);
	}
#endif
    }
    mdata->CommentLength = 0;
    return (XpmSuccess);
}

void
_XmxpmOpenArray(data, mdata)
    char **data;
    xpmData *mdata;
{
    mdata->type = XPMARRAY;
    mdata->stream.data = data;
    mdata->cptr = *data;
    mdata->line = 0;
    mdata->CommentLength = 0;
    mdata->Bcmt = mdata->Ecmt = NULL;
    mdata->Bos = mdata->Eos = '\0';
}

_XmXpmDataClose(mdata)
    xpmData *mdata;
{
    switch (mdata->type) {
    case XPMARRAY:
	break;
    case XPMFILE:
	if (mdata->stream.file != (stdout) && mdata->stream.file != (stdin))
	    fclose(mdata->stream.file);
	break;
#ifdef ZPIPE
    case XPMPIPE:
	pclose(mdata->stream.file);
	break;
#endif
    }
}
