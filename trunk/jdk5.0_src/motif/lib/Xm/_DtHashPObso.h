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
static char rcsid[] = "$XConsortium: _DtHashP.h /main/cde1_maint/2 1995/08/18 19:40:31 drk $"
#endif
#endif

#ifndef __DtHashP_h
#define __DtHashP_h

#include <X11/Intrinsic.h>
#include <Xm/XmP.h>
 
#ifdef __cplusplus
extern "C" {
#endif
/*
 * the structure is used as a common header part for different
 * users of the hash functions in order to locate the key
 */
typedef XtPointer DtHashKey;

typedef DtHashKey (*DtGetHashKeyFunc)();
typedef Boolean (*DtHashEnumerateFunc)();
typedef void (*DtReleaseKeyProc)();

typedef struct _DtHashEntryPartRec {
    unsigned int	type:16;
    unsigned int	flags:16;
}DtHashEntryPartRec, *DtHashEntryPart;

typedef struct _DtHashEntryRec {
    DtHashEntryPartRec	hash;
}DtHashEntryRec, *DtHashEntry;

typedef struct _DtHashEntryTypePartRec {
    unsigned int		entrySize;
    DtGetHashKeyFunc		getKeyFunc;
    XtPointer			getKeyClientData;
    DtReleaseKeyProc		releaseKeyProc;
}DtHashEntryTypePartRec, *DtHashEntryTypePart;

typedef struct _DtHashEntryTypeRec {
    DtHashEntryTypePartRec	hash;
}DtHashEntryTypeRec, *DtHashEntryType;

typedef struct _DtHashTableRec *DtHashTable;

/********    Private Function Declarations for Hash.c    ********/

extern void _XmRegisterHashEntry(DtHashTable, DtHashKey, DtHashEntry);
extern void _XmUnregisterHashEntry(DtHashTable, DtHashEntry);
extern DtHashEntry _XmEnumerateHashTable(DtHashTable, DtHashEnumerateFunc, XtPointer);
extern DtHashEntry _XmKeyToHashEntry(DtHashTable, DtHashKey);
extern DtHashTable _XmAllocHashTable(DtHashEntryType *, Cardinal, Boolean);
extern void _XmFreeHashTable(DtHashTable);

/********    End Private Function Declarations    ********/



#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtHashP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */



