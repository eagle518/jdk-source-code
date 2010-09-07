/* $XConsortium: BBUtil.c /main/6 1996/06/14 23:09:00 pascale $ */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */


#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/TraitP.h>
#include <Xm/TakesDefT.h>
#include "BulletinBI.h"
#include "MessagesI.h"

/* defines for label string resources coming from message catalog */
#define OK_STRING         _XmMMsgResource_0001
#define CANCEL_STRING     _XmMMsgResource_0002
#define SELECTION_STRING  _XmMMsgResource_0003
#define APPLY_STRING      _XmMMsgResource_0004
#define HELP_STRING       _XmMMsgResource_0005
#define FILTER_STRING     _XmMMsgResource_0006
#define DIRLIST_STRING    _XmMMsgResource_0008
#define ITEMS_STRING      _XmMMsgResource_0009
#define DIRTEXT_STRING    _XmMMsgResource_0011
#define PROMPT_STRING     _XmMMsgResource_0012



/****************************************************************/
static char *
GetLabelString(
       XmLabelStringLoc l_loc )
{
    switch (l_loc)
	{
	case XmOkStringLoc:
	    return (OK_STRING);
	    break;
	case XmCancelStringLoc:
	    return (CANCEL_STRING);
	    break;
	case XmSelectionStringLoc:
	    return (SELECTION_STRING);
	    break;
	case XmApplyStringLoc:
	    return (APPLY_STRING);
	    break;
	case XmHelpStringLoc:
	    return (HELP_STRING);
	    break;
	case XmFilterStringLoc:
	    return (FILTER_STRING);
	    break;
	case XmDirListStringLoc:
	    return (DIRLIST_STRING);
	    break;
	case XmItemsStringLoc:
	    return (ITEMS_STRING);
	    break;
	case XmDirTextStringLoc:
	    return (DIRTEXT_STRING);
	    break;
	case XmPromptStringLoc:
	    return (PROMPT_STRING);
	    break;
	}
}


/****************************************************************/
Widget 
_XmBB_CreateButtonG(
        Widget bb,
        XmString l_string,
	char *name,
        XmLabelStringLoc l_loc )
{
    Arg		        al[10] ;
    register Cardinal   ac = 0 ;
    Widget              button ;
    XmTakesDefaultTrait trait_default ;
    XmString            default_label_string_loc = NULL;
/****************/

    if(    l_string    )
	{
	    XtSetArg( al[ac], XmNlabelString, l_string) ; ac++ ;
        }
    else
	{
	    default_label_string_loc = XmStringCreate(GetLabelString(l_loc),
						      XmFONTLIST_DEFAULT_TAG);
	    XtSetArg( al[ac], XmNlabelString, default_label_string_loc); ac++;
	}
	    
    XtSetArg( al[ac], XmNstringDirection, BB_StringDirection( bb)) ; ac++ ;

    button = XmCreatePushButtonGadget( (Widget) bb, name, al, ac) ;

    trait_default = (XmTakesDefaultTrait) XmeTraitGet((XtPointer)
						      XtClass(button), 
						      XmQTtakesDefault) ;
    if (trait_default) 
	trait_default->showAsDefault  (button, XmDEFAULT_READY);

    if (default_label_string_loc)
	XmStringFree(default_label_string_loc);

    return( button ) ;
}

/****************************************************************/
Widget 
_XmBB_CreateLabelG(
        Widget bb,
        XmString l_string,
        char *name,
        XmLabelStringLoc l_loc )
{
            Arg		    al[10] ;
    register int            ac = 0 ;
    Widget                  label ;
    XmString                default_label_string_loc = NULL;
/****************/

    if(    l_string    )
	{
	    XtSetArg( al[ac], XmNlabelString, l_string) ; ac++ ;
        }
    else
	{
	    default_label_string_loc = XmStringCreate(GetLabelString(l_loc),
						      XmFONTLIST_DEFAULT_TAG);
	    XtSetArg( al[ac], XmNlabelString, default_label_string_loc); ac++;
	}

    XtSetArg( al[ac], XmNstringDirection, BB_StringDirection( bb)) ; ac++ ;
    XtSetArg( al[ac], XmNhighlightThickness, 0) ; ac++ ;
    XtSetArg( al[ac], XmNtraversalOn, False) ; ac++ ;
    XtSetArg( al[ac], XmNalignment, XmALIGNMENT_BEGINNING) ; ac++ ;

    label = XmCreateLabelGadget( bb, name, al, ac);

    if (default_label_string_loc)
	XmStringFree(default_label_string_loc);

    return( label ) ;
	
}

