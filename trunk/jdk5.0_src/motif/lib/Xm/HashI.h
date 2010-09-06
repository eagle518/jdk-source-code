/* $XConsortium: HashI.h /main/5 1995/07/15 20:51:35 drk $ */
/*
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 */
/*
 * HISTORY
 */

#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)_HashP.h	4.16 91/09/12";
#endif /* lint */
#endif /* REV_INFO */

#ifndef _XmHashI_h
#define _XmHashI_h
 
#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int	                XmHashValue;
typedef XtPointer               XmHashKey;
typedef struct _XmHashTableRec  *XmHashTable;

typedef Boolean (*XmHashCompareProc)(XmHashKey, XmHashKey);
typedef XmHashValue (*XmHashFunction)(XmHashKey);
typedef Boolean (*XmHashMapProc)(XmHashKey, XtPointer value, XtPointer data);

XmHashTable _Xm21AllocHashTable(Cardinal, XmHashCompareProc, XmHashFunction);
void _XmResizeHashTable(XmHashTable, Cardinal);
void _Xm21FreeHashTable(XmHashTable);
XtPointer _XmGetHashEntryIterate(XmHashTable, XmHashKey, XtPointer*);
void _XmAddHashEntry(XmHashTable, XmHashKey, XtPointer);
XtPointer _XmRemoveHashEntry(XmHashTable, XmHashKey);
XtPointer _XmRemoveHashIterator(XmHashTable, XtPointer*);
Cardinal _XmHashTableCount(XmHashTable);
Cardinal _XmHashTableSize(XmHashTable);
void _XmMapHashTable(XmHashTable, XmHashMapProc, XtPointer);
#ifdef DEBUG
void _XmPrintHashTable(XmHashTable);
#endif

#define _XmGetHashEntry(table, key) _XmGetHashEntryIterate(table, key, NULL)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmHashI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
