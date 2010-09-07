/* $XConsortium: ContItemT.h /main/5 1995/07/15 20:49:36 drk $ */
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
#ifndef _XmContainerItemT_H
#define _XmContainerItemT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTcontainerItem;

/* Trait structures and typedefs, place typedefs first */

/* this one can be expanded in the future */
typedef struct _XmContainerItemDataRec {
    Mask valueMask ;        /* on setValues, give the information on
			     what to change in the Icon, on getValues,
			     on what to put in the record returned */
    unsigned char view_type;
    unsigned char visual_emphasis;
    Dimension icon_width ;    /* get value */
    Cardinal detail_count;   /* get value */
} XmContainerItemDataRec, *XmContainerItemData;

#define ContItemAllValid             (0xFFFF)
#define ContItemViewType	     (1L<<0)
#define ContItemVisualEmphasis	     (1L<<1)
#define ContItemIconWidth            (1L<<2)
#define ContItemDetailCount          (1L<<3)


typedef void (*XmContainerItemSetValuesProc)(Widget w, 
					XmContainerItemData contItemData);
typedef void (*XmContainerItemGetValuesProc)(Widget w, 
					XmContainerItemData contItemData);

/* Version 0: initial release. */

typedef struct _XmContainerItemTraitRec {
  int			       version;		/* 0 */
  XmContainerItemSetValuesProc setValues;
  XmContainerItemGetValuesProc getValues;
} XmContainerItemTraitRec, *XmContainerItemTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmContainerItemT_H */
