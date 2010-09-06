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
/*   $XConsortium: SeparatorP.h /main/12 1995/07/13 17:59:57 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSeparatorP_h
#define _XmSeparatorP_h

#include <Xm/Separator.h>
#include <Xm/PrimitiveP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Separator class structure  */

typedef struct _XmSeparatorClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmSeparatorClassPart;


/*  Full class record declaration for Separator class  */

typedef struct _XmSeparatorClassRec
{
   CoreClassPart         core_class;
   XmPrimitiveClassPart  primitive_class;
   XmSeparatorClassPart  separator_class;
} XmSeparatorClassRec;

externalref XmSeparatorClassRec xmSeparatorClassRec;


/*  The Separator instance record  */

typedef struct _XmSeparatorPart
{
   Dimension	  margin;
   unsigned char  orientation;
   unsigned char  separator_type;
   GC             separator_GC;
} XmSeparatorPart;


/*  Full instance record declaration  */

typedef struct _XmSeparatorRec
{
   CorePart	    core;
   XmPrimitivePart  primitive;
   XmSeparatorPart  separator;
} XmSeparatorRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSeparatorP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
