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
/* $XConsortium: VirtKeysP.h /main/10 1995/07/13 18:21:10 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmVirtKeysP_h
#define _XmVirtKeysP_h

#ifndef MOTIF12_HEADERS

#include <Xm/XmP.h>
#include <Xm/VirtKeys.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmKEYCODE_TAG_SIZE 32

typedef struct _XmDefaultBindingStringRec {
    String	vendorName;
    String	defaults;
} XmDefaultBindingStringRec, *XmDefaultBindingString;

typedef	struct _XmVirtualKeysymRec {
    String		name;
    KeySym		keysym;
} XmVirtualKeysymRec, *XmVirtualKeysym;


typedef struct _XmIndexedVirtualKeysymRec{
    XmVirtualKeysym virtRec;
    int index;
} XmIndexedVirtualKeysymRec, *XmIndexedVirtualKeysym;



/* For converting a Virtual keysym to a real keysym. */
typedef struct _XmVKeyBindingRec
{
  KeySym	keysym;
  Modifiers	modifiers;
  KeySym	virtkey;
  int           index; /* points to bound XmVirtualKeysymRec */
} XmVKeyBindingRec, *XmVKeyBinding;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


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
/*   $XConsortium: VirtKeysP.h /main/cde1_maint/2 1995/08/18 19:33:56 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/XmP.h>
#include <Xm/VirtKeys.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmKEYCODE_TAG_SIZE 32

typedef struct _XmDefaultBindingStringRec{
    _XmConst String	vendorName;
    _XmConst String	defaults;
}XmDefaultBindingStringRec, *XmDefaultBindingString;

typedef	struct _XmKeyBindingRec{
    KeySym		keysym;
    unsigned int	modifiers;
    int			index; /* points to bound XmVirtualKeysymRec */
}XmKeyBindingRec, *XmKeyBinding;

typedef	struct _XmVirtualKeysymRec{
    String		name;
    KeySym		keysym;
}XmVirtualKeysymRec, *XmVirtualKeysym;

typedef struct _XmIndexedVirtualKeysymRec{
    XmVirtualKeysym virtRec;
    int index;
} XmIndexedVirtualKeysymRec, *XmIndexedVirtualKeysym;

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmVirtKeysInitialize() ;
extern void _XmVirtKeysDestroy() ;
extern void _XmVirtKeysHandler() ;
extern void _XmVirtualToActualKeysym() ;
extern void _XmVirtualToActualKeysymList() ;
extern void _XmVirtKeysStoreBindings() ;
extern Boolean _XmVirtKeysLoadFileBindings() ;
extern int _XmVirtKeysLoadFallbackBindings() ;

#else

extern void _XmVirtKeysInitialize( 
                        Widget widget) ;
extern void _XmVirtKeysDestroy( 
                        Widget widget) ;
extern void _XmVirtKeysHandler( 
                        Widget widget,
                        XtPointer client_data,
                        XEvent *event,
                        Boolean *dontSwallow) ;
extern void _XmVirtualToActualKeysym( 
                        Display *dpy,
                        KeySym virtKeysym,
                        KeySym *actualKeysymRtn,
                        Modifiers *modifiersRtn) ;
extern void _XmVirtualToActualKeysymList( 
                        Display *dpy,
                        KeySym virtKeysym,
                        KeySym **actualKeysymRtn,
                        Modifiers **modifiersRtn,
                        int *numKeysymRtn) ;
extern void _XmVirtKeysStoreBindings( 
                        Widget shell,
                        String binding) ;
extern Boolean _XmVirtKeysLoadFileBindings( 
                        char *fileName,
                        String *binding) ;
extern int _XmVirtKeysLoadFallbackBindings(
			Display *display,
			String *binding) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmVirtKeysP_h */
