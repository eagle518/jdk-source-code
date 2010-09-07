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
static char rcsid[] = "$XConsortium: Cache.c /main/12 1995/07/14 10:12:26 drk $"
#endif
#endif
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include "CacheI.h"
#include <Xm/GadgetP.h>


/********    Static Function Declarations    ********/


/********    End Static Function Declarations    ********/


/************************************************************************
 *
 *  _XmCacheDelete
 *	Delete an existing cache record.  NOTE: <data> is a pointer to the
 *      fourth field in the cache record - It is *not* a pointer to the
 *	cache record itself!
 *
 ************************************************************************/
void 
_XmCacheDelete(
        XtPointer data )
{
    XmGadgetCachePtr ptr;

    ptr = (XmGadgetCachePtr) DataToGadgetCache(data);
    if (--ptr->ref_count <= 0) {
      (ptr->prev)->next = ptr->next;
      if (ptr->next)			/* not the last record */
        (ptr->next)->prev = ptr->prev;
      XtFree( (char *) ptr );
    }
}

/************************************************************************
 *
 *  _XmCacheCopy
 *	Copy <size> bytes from <src> to <dest>. 
 *
 ************************************************************************/
void 
_XmCacheCopy(
        XtPointer src,
        XtPointer dest,
        size_t size )
{
    memcpy( dest, src, size);
}

/************************************************************************
 *
 *  _XmCachePart
 *	Pass in a pointer, <cpart>, to <size> bytes of a temporary Cache
 *	record.  
 *	- If the Class cache head is NULL (no entries yet!), allocate a new
 *	  cache record, copy in temporary Cache bytes, append it to the 
 *	  class-cache linked list, and return the address.
 *	- Else, run through the class linked list.
 *	  = If a match is found, increment the ref_count and return the 
 *	    address.
 *	  = Else, allocate a new cache record, copy in temporary Cache bytes,
 *	    append it to the class-cache linked list, and return the address.
 *
 ************************************************************************/
XtPointer 
_XmCachePart(
        XmCacheClassPartPtr cp,
        XtPointer cpart,
        size_t size )
{
    XmGadgetCachePtr ptr, last;
    
    if (ClassCacheHead(cp).next == NULL)       /* First one */
    {
	ClassCacheHead(cp).next = 
	  (struct _XmGadgetCache *)XtMalloc( size + 
					    XtOffsetOf(XmGadgetCacheRef, data));
        ptr = (XmGadgetCachePtr)ClassCacheHead(cp).next;

        ClassCacheCopy(cp)(cpart, CacheDataPtr(ptr), size );  
	ptr-> ref_count = 1;
        ptr-> next = NULL;
	ptr-> prev = (struct _XmGadgetCache *)&ClassCacheHead(cp);
        return (CacheDataPtr(ptr));
    }    
    ptr = (XmGadgetCachePtr)ClassCacheHead(cp).next;
    do
    {

        if ((ClassCacheCompare(cp)( cpart, CacheDataPtr(ptr))))
        {
            ptr->ref_count++;
            return ((XtPointer) CacheDataPtr(ptr));
        }
        else
        {
            last = ptr;
            ptr = (XmGadgetCachePtr)ptr->next;
        }
    } while (ptr);
    
    /* Malloc a new rec off of last, fill it out*/
    ptr = (XmGadgetCachePtr)XtMalloc( size + 
				     XtOffsetOf(XmGadgetCacheRef, data) );
    last->next = (struct _XmGadgetCache *)ptr;
    ClassCacheCopy(cp)(cpart, CacheDataPtr(ptr), size);
    ptr-> ref_count = 1;
    ptr-> next = NULL;
    ptr-> prev = last;
    return (CacheDataPtr(ptr));
}

