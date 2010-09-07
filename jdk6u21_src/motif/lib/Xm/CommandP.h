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
/*   $XConsortium: CommandP.h /main/11 1995/07/14 10:16:43 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCommandP_h
#define _XmCommandP_h

#include <Xm/SelectioBP.h>
#include <Xm/Command.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constraint part record for Command widget */

typedef struct _XmCommandConstraintPart
{
  char unused;
} XmCommandConstraintPart, * XmCommandConstraint;

/*  New fields for the Command widget class record  */

typedef struct
{
  XtPointer           extension;      /* Pointer to extension record */
} XmCommandClassPart;


/* Full class record declaration */

typedef struct _XmCommandClassRec
{
  CoreClassPart            core_class;
  CompositeClassPart       composite_class;
  ConstraintClassPart      constraint_class;
  XmManagerClassPart       manager_class;
  XmBulletinBoardClassPart bulletin_board_class;
  XmSelectionBoxClassPart  selection_box_class;
  XmCommandClassPart       command_class;
} XmCommandClassRec;

externalref XmCommandClassRec xmCommandClassRec;

/* New fields for the Command widget record */

typedef struct
{
  XtCallbackList   callback;
  XtCallbackList   value_changed_callback;
  int              history_max_items;
  Boolean          error;        /* error has been made visible in list */
} XmCommandPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmCommandRec
{
    CorePart	        core;
    CompositePart       composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmBulletinBoardPart bulletin_board;
    XmSelectionBoxPart  selection_box;
    XmCommandPart       command;
} XmCommandRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCommandP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
