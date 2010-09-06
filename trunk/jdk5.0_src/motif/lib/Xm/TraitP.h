/* $XConsortium: TraitP.h /main/5 1995/07/15 20:56:18 drk $ */
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

#ifndef _XmTraitP_H
#define _XmTraitP_H 1

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

#define XmeTraitRemove(w, t) XmeTraitSet((XtPointer) w, t, NULL)


/********    Private Function Declarations    ********/

/*
 * XmeTraitGet(object, trait) returns a pointer to the trait_record
 * from looking up the trait on this object.  If the trait
 * is not found then NULL is returned.  This can therefore be used
 * in the following cliche'
 *
 * if (trait_rec = XmeTraitGet(XtClass(w), XmQTactivate)) {
 *   trait_rec -> activate();
 *   trait_rec -> disarm();
 * }
 */

extern XtPointer XmeTraitGet(XtPointer, XrmQuark);

/* 
 * Boolean XmeTraitSet(object, traitname, traitrecord)
 *
 * Installs the trait on the object.  Boolean will indicate
 * success of the installation.  
 * 
 * Install will use the direct pointer to traitrecord given.  The
 * implementation is therefore not allowed to use automatic
 * storage for traitrecord,  but can use malloc or static initialization
 *
 */

extern Boolean XmeTraitSet(XtPointer, XrmQuark, XtPointer);

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTraitP_H */

