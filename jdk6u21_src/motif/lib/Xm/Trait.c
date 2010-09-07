/* $XConsortium: Trait.c /main/8 1996/10/10 11:34:59 cde-osf $ */
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
/* #define DEBUG */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/IntrinsicP.h>
#include <Xm/XmP.h>
#include <Xm/VendorSP.h>
#include <Xm/TraitP.h>
#include "HashI.h"
#include "TraitI.h"
#include "XmI.h"

/*
 * Internal data structures
 */

static XmHashTable TraitTable;

typedef struct _XmTraitEntry {
  XtPointer obj;
  XrmQuark  name;
} XmTraitEntryRec, *XmTraitEntry;

/*
 * Static functions
 */

static Boolean TraitCompare(XmHashKey, XmHashKey);
static XmHashValue TraitHash(XmHashKey);

/*
 * List all quarks here
 */

externaldef(traits) XrmQuark XmQTmotifTrait = NULLQUARK;
externaldef(traits) XrmQuark XmQTmenuSystem = NULLQUARK;
externaldef(traits) XrmQuark XmQTtransfer = NULLQUARK;
externaldef(traits) XrmQuark XmQTaccessTextual = NULLQUARK;
externaldef(traits) XrmQuark XmQTmenuSavvy = NULLQUARK;
externaldef(traits) XrmQuark XmQTnavigator = NULLQUARK;
externaldef(traits) XrmQuark XmQTscrollFrame = NULLQUARK;
externaldef(traits) XrmQuark XmQTactivatable = NULLQUARK;
externaldef(traits) XrmQuark XmQTdialogShellSavvy = NULLQUARK;
externaldef(traits) XrmQuark XmQTjoinSide = NULLQUARK;
externaldef(traits) XrmQuark XmQTcareParentVisual = NULLQUARK;
externaldef(traits) XrmQuark XmQTspecifyRenderTable = NULLQUARK;
externaldef(traits) XrmQuark XmQTtakesDefault = NULLQUARK;
externaldef(traits) XrmQuark XmQTcontainerItem = NULLQUARK;
externaldef(traits) XrmQuark XmQTcontainer = NULLQUARK;
externaldef(traits) XrmQuark XmQTspecifyLayoutDirection = NULLQUARK;
externaldef(traits) XrmQuark XmQTaccessColors = NULLQUARK;
externaldef(traits) XrmQuark XmQTspecifyUnitType = NULLQUARK;
externaldef(traits) XrmQuark XmQTtraversalControl = NULLQUARK;
externaldef(traits) XrmQuark XmQTspecifyUnhighlight = NULLQUARK;
externaldef(traits) XrmQuark XmQTpointIn = NULLQUARK;
externaldef(traits) XrmQuark _XmQTclipWindow = NULLQUARK;

/* 
 * Initialize traits system
 * 
 * This routine sets up all quarks used by the traits in 
 * Motif
 */

void _XmInitializeTraits()
{
  /* Create Hash Table */
/* Solaris 2.6 Motif diff bug 4085003 1 line */

  TraitTable = _Xm21AllocHashTable(200, TraitCompare, TraitHash);

  XmQTmotifTrait = XrmPermStringToQuark("XmQTmotifTrait");

  /* Menu system manipulation and status */
  XmQTmenuSystem = XrmPermStringToQuark("XmTmenuSystem");
  XmQTmenuSavvy = XrmPermStringToQuark("XmTmenuSavvy");

  /* Transfer Trait */
  XmQTtransfer = XrmPermStringToQuark("XmTtransfer");

  /* String get/set */
  XmQTaccessTextual = XrmPermStringToQuark("XmTaccessTextual");
  
  /* Navigator/Scrolling trait */
  XmQTnavigator = XrmPermStringToQuark("XmTnavigator");
  XmQTscrollFrame = XrmPermStringToQuark("XmTscrollFrame");
  _XmQTclipWindow= XrmPermStringToQuark("_XmTclipWindow");

  /* Activatable trait */
  XmQTactivatable = XrmPermStringToQuark("XmTactivatable");

  /* JoinSide trait */
  XmQTjoinSide = XrmPermStringToQuark("XmTjoinSide");

  /* DialogShellSavvy trait */
  XmQTdialogShellSavvy = XrmPermStringToQuark("XmTdialogShellSavvy");

  /* Care about Parent Visual trait */
  XmQTcareParentVisual = XrmPermStringToQuark("XmTcareParentVisual");

  /* SpecifyRenderTable trait */
  XmQTspecifyRenderTable = XrmPermStringToQuark("XmTspecifyRenderTable");

  /* TakesDefault trait */
  XmQTtakesDefault = XrmPermStringToQuark("XmTtakesDefault");

  /* Container/Item trait */
  XmQTcontainerItem = XrmPermStringToQuark("XmTcontainerItem");
  XmQTcontainer = XrmPermStringToQuark("XmTcontainer");

  /* LayoutDirection trait */
  XmQTspecifyLayoutDirection = 
    XrmPermStringToQuark("XmTspecifyLayoutDirection");

  /* get colors */
  XmQTaccessColors = XrmPermStringToQuark("XmTaccessColors");

  /* Unit type */
  XmQTspecifyUnitType = XrmPermStringToQuark("XmTspecifyUnitType");

  /* Traversal control. */
  XmQTtraversalControl = XrmPermStringToQuark("XmTtraversalControl");

  /* Specify UnHighlight GC trait */
  XmQTspecifyUnhighlight = XrmPermStringToQuark("XmTspecifyUnhighlight");

  /* PointIn trait */
  XmQTpointIn = XrmPermStringToQuark("XmTpointIn");
}

XtPointer
XmeTraitGet(XtPointer obj, XrmQuark name) 
{
  XtPointer trait;
  XmTraitEntryRec entry;

  entry.obj = obj;
  entry.name = name;

  _XmProcessLock();
  trait = _XmGetHashEntry(TraitTable, &entry);
  _XmProcessUnlock();

  return(trait);
}

Boolean
XmeTraitSet(XtPointer object, XrmQuark name, XtPointer data)
{
  XmTraitEntry entry;

  /* Create key,  this will be freed if the record is removed */
  entry = (XmTraitEntry) XtMalloc(sizeof(XmTraitEntryRec));
  entry -> obj = object;
  entry -> name = name;

  _XmProcessLock();
  if (data != NULL) {
    _XmAddHashEntry(TraitTable, entry, data);
  }
  else { /* if data == NULL then remove the context */
    XtPointer key;
    key = _XmRemoveHashEntry(TraitTable, entry);
    XtFree((char*) entry);
    XtFree((char*) key);
  }
  _XmProcessUnlock();

  return True;
}

static Boolean
TraitCompare(XmHashKey key1, XmHashKey key2)
{
  XmTraitEntry entry1, entry2;

  entry1 = (XmTraitEntry) key1;
  entry2 = (XmTraitEntry) key2;

  return(entry1 -> obj == entry2 -> obj &&
	 entry1 -> name == entry2 -> name);
}

static XmHashValue 
TraitHash(XmHashKey key)
{
  XmTraitEntry entry = (XmTraitEntry) key;

  return(((long) entry -> obj) + ((long) entry -> name));
}
