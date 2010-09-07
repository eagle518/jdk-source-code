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
/*   $XConsortium: ShellEP.h /main/9 1995/07/13 18:00:25 drk $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmShellEP_h
#define _XmShellEP_h

#include <Xm/DesktopP.h>

#ifdef __cplusplus
extern "C" {
#endif


#define XmInheritEventHandler		((XtEventHandler)_XtInherit)

#define _XmRAW_MAP 0
#define _XmPOPUP_MAP 1
#define _XmMANAGE_MAP 2


#ifndef XmIsShellExt
#define XmIsShellExt(w)	XtIsSubclass(w, xmShellExtObjectClass)
#endif /* XmIsShellExt */

typedef struct _XmShellExtRec *XmShellExtObject;
typedef struct _XmShellExtClassRec *XmShellExtObjectClass;
externalref WidgetClass xmShellExtObjectClass;


typedef struct _XmShellExtClassPart{
    XtEventHandler	structureNotifyHandler;
    XtPointer		extension;
}XmShellExtClassPart, *XmShellExtClassPartPtr;

typedef struct _XmShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart 	shell_class;
}XmShellExtClassRec;

typedef struct {
    unsigned long	lastConfigureRequest;
    Boolean		useAsyncGeometry;
} XmShellExtPart, *XmShellExtPartPtr;

externalref XmShellExtClassRec 	xmShellExtClassRec;

typedef struct _XmShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
}XmShellExtRec;


/********    Private Function Declarations    ********/

#ifdef _NO_PROTO

extern void _XmSyncShellPosition();

#else

extern void _XmSyncShellPosition(Widget w);

#endif /* _NO_PROTO */

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmShellEP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
