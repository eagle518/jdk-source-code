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
static char rcsid[] = "$XConsortium: Command.c /main/16 1996/03/25 17:49:53 barstow $"
#endif
#endif
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include "XmI.h"
#include "CommandI.h"
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include "BulletinBI.h"
#include "MessagesI.h"
#include "RepTypeI.h"
#include "SelectioBI.h"
#include "XmStringI.h"

#include <string.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif


#define ARG_LIST_CNT 25

/* defines for warning messages */
#define WARNING1 _XmMMsgCommand_0000
#define WARNING2 _XmMMsgCommand_0001
#define WARNING3 _XmMMsgCommand_0002
#define WARNING4 _XmMMsgCommand_0003
#define WARNING5 _XmMMsgCommand_0004
#define WARNING6 _XmMMsgCommand_0005

/********    Static Function Declarations    ********/

static void ClassPartInitialize( 
                        WidgetClass wc) ;
static void Initialize( 
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static void ListCallback( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void CommandCallback( 
                        Widget w,
                        XtPointer cl_data,
                        XtPointer call_data) ;
static Boolean CommandParentProcess( 
                        Widget wid,
                        XmParentProcessData event) ;
static Boolean SetValues( 
                        Widget ow,
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;

/********    End Static Function Declarations    ********/


static XtActionsRec actionsList[] =
{
        { "Return",                     _XmCommandReturn },     /* Motif 1.0 */
        { "UpOrDown",                   _XmCommandUpOrDown },   /* Motif 1.0 */
        { "BulletinBoardReturn",        _XmCommandReturn },
        { "SelectionBoxUpOrDown",       _XmCommandUpOrDown },
};


/* define the resource stuff for a command widget */
static XmSyntheticResource syn_resources[] = {

    {   XmNpromptString,
        sizeof (XmString),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.selection_label_string),
        _XmSelectionBoxGetSelectionLabelString,
		NULL },

    {   XmNcommand,
        sizeof (XmString),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.text_string),
        _XmSelectionBoxGetTextString,
		NULL },

    {   XmNhistoryItems,
        sizeof (XmString *),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_items),
        _XmSelectionBoxGetListItems,
		NULL },

    {   XmNhistoryItemCount,
        sizeof(int),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_item_count),
        _XmSelectionBoxGetListItemCount,
		NULL },

    {   XmNhistoryVisibleItemCount,
        sizeof(int),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_visible_item_count),
        _XmSelectionBoxGetListVisibleItemCount,
		NULL },
};


static XtResource resources[] = 
{
    {   XmNcommandEnteredCallback, 
        XmCCallback, 
        XmRCallback, 
        sizeof (XtCallbackList),
        XtOffsetOf( struct _XmCommandRec, command.callback), 
        XmRCallback, 
        (XtPointer)NULL},

    {   XmNcommandChangedCallback, 
        XmCCallback, 
        XmRCallback, 
        sizeof (XtCallbackList),
        XtOffsetOf( struct _XmCommandRec, command.value_changed_callback), 
        XmRCallback, 
        (XtPointer) NULL},

    {   XmNpromptString,
        XmCPromptString, 
        XmRXmString, 
        sizeof (XmString),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.selection_label_string),
        XmRImmediate, 
        (XtPointer) XmUNSPECIFIED},

    {   XmNcommand,
        XmCTextString, 
        XmRXmString, 
        sizeof (XmString),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.text_string),
        XmRImmediate, 
        (XtPointer) XmUNSPECIFIED},

    {   XmNhistoryItems,
        XmCItems, 
        XmRXmStringTable, 
        sizeof (XmString *),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_items),
        XmRImmediate, 
        NULL},

    {   XmNhistoryItemCount,
        XmCItemCount, 
        XmRInt, 
        sizeof(int),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_item_count),
        XmRImmediate, 
        (XtPointer) XmUNSPECIFIED_COUNT},

    {   XmNhistoryMaxItems,
        XmCMaxItems,
        XmRInt, 
        sizeof(int),
        XtOffsetOf( struct _XmCommandRec, command.history_max_items),
        XmRImmediate, 
        (XtPointer) 100},

    {   XmNhistoryVisibleItemCount,
        XmCVisibleItemCount, 
        XmRInt, 
        sizeof(int),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.list_visible_item_count),
        XmRImmediate, 
        (XtPointer) 8},

    {   XmNdialogType,
        XmCDialogType, 
        XmRSelectionType, 
        sizeof (unsigned char),
        XtOffsetOf( struct _XmSelectionBoxRec, selection_box.dialog_type),
        XmRImmediate, 
        (XtPointer) XmDIALOG_COMMAND},

    {   XmNdefaultPosition,
        XmCDefaultPosition, 
        XmRBoolean, 
        sizeof (Boolean),
        XtOffsetOf( struct _XmBulletinBoardRec, bulletin_board.default_position),
        XmRImmediate, 
        (XtPointer) False},

    {   XmNautoUnmanage,
        XmCAutoUnmanage,
        XmRBoolean, 
        sizeof (Boolean),
        XtOffsetOf( struct _XmBulletinBoardRec, bulletin_board.auto_unmanage),
        XmRImmediate,
        (XtPointer) False},

    {   XmNresizePolicy,
        XmCResizePolicy,
        XmRResizePolicy,
        sizeof (unsigned char),
        XtOffsetOf( struct _XmBulletinBoardRec, bulletin_board.resize_policy),
        XmRImmediate,
        (XtPointer) XmRESIZE_NONE},


};


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

externaldef( xmcommandclassrec) XmCommandClassRec xmCommandClassRec =
{
    {                                            /*core class record         */
        (WidgetClass) &xmSelectionBoxClassRec,   /* superclass ptr           */
        "XmCommand",                             /* class_name               */
        sizeof(XmCommandRec),                    /* size of widget instance  */
        NULL,                         /* class init proc          */
        ClassPartInitialize,                     /* class part init proc     */
        FALSE,                                   /* class is not init'ed     */
        Initialize,                              /* widget init proc         */
        NULL,                                    /* widget init hook         */
        XtInheritRealize,                        /* widget realize proc      */
        actionsList,                             /* class action table       */
        XtNumber (actionsList),                  /* class action count       */
        resources,                               /* class resource list      */
        XtNumber (resources),                    /* class  resource_count    */
        NULLQUARK,                               /* xrm_class                */
        TRUE,                                    /* compressed motion        */
        XtExposeCompressSeries,		 	 /* compressed exposure      */
        TRUE,                                    /* compressed enter/exit    */
        FALSE,                                   /* VisibilityNotify         */
        NULL,                                    /* class destroy proc       */
        XtInheritResize,                         /* class resize proc        */
        XtInheritExpose,                         /* class expose proc        */
        SetValues,                               /* class set_value proc     */
        NULL,                                    /* widget setvalues hook    */
        XtInheritSetValuesAlmost,                /* set values almost        */
        NULL,                                    /* widget getvalues hook    */
        NULL,                                    /* class accept focus proc  */
        XtVersion,                               /* version                  */
        NULL,                                    /* callback offset list     */
        XtInheritTranslations,                   /* default translations     */
        XtInheritQueryGeometry,                  /* query geometry           */
        NULL,                                    /* Display Accelerator      */
        NULL,                                    /* Extension pointer        */
    },
    {                                            /*composite class record    */
        XtInheritGeometryManager,                /* childrens geo mgr proc   */
        XtInheritChangeManaged,                  /* set changed proc         */
        XtInheritInsertChild,                    /* add a child              */
        XtInheritDeleteChild,                    /* remove a child           */
        NULL,                                    /* Extension pointer        */
    },
    {                                            /*constraint class record   */
        NULL,                                    /* no additional resources  */
        0,                                       /* num additional resources */
        0,                                       /* size of constraint rec   */
        NULL,                                    /* constraint_initialize    */
        NULL,                                    /* constraint_destroy       */
        NULL,                                    /* constraint_setvalue      */
        NULL,                                    /* Extension pointer        */
    },
    {                                            /*manager class record      */
        XtInheritTranslations,                   /* default translations     */
        syn_resources,                           /* syn_resources            */
        XtNumber(syn_resources),                 /* num_syn_resources        */
        NULL,                                    /* syn_cont_resources       */
        0,                                       /* num_syn_cont_resources   */
        CommandParentProcess,                    /* parent_process           */
        NULL,                                    /* extension                */
    },
    {                                            /*bulletin board class rec  */
        TRUE,                                    /*always_install_accelerators*/
        XmInheritGeoMatrixCreate,                /* geo_matrix_create */
        XmInheritFocusMovedProc,                 /* focus_moved_proc */
        NULL                                     /* Extension pointer - none */
    },
    {                                            /*selection box class rec  */
        ListCallback,                            /* list_callback */
        NULL,                                    /* Extension pointer - none */
    },
    {                                            /*command class record      */
        NULL,                                    /* Extension pointer - none */
    }
};


/*
 * now make a public symbol that points to this class record
 */

externaldef( xmcommandwidgetclass) WidgetClass xmCommandWidgetClass
                                           = (WidgetClass) &xmCommandClassRec ;


/****************************************************************/
static void 
ClassPartInitialize(
        WidgetClass wc )
{
/****************/

    _XmFastSubclassInit (wc, XmCOMMAND_BOX_BIT);

    return ;
    }
/****************************************************************
 * Set attributes of a command widget
 ****************/
/*ARGSUSED*/
static void 
Initialize(
        Widget rw,		/* unused */
        Widget nw,
        ArgList args,		/* unused */
        Cardinal *num_args )	/* unused */
{
    XmCommandWidget new_w = (XmCommandWidget) nw ;
    int max;
    Arg             argv[5] ;
    Cardinal        argc ;
/****************/
    
    /*	Here we have now to take care of XmUNSPECIFIED (CR 4856).
     */  
    if (new_w->selection_box.selection_label_string == 
	(XmString) XmUNSPECIFIED) {
	XmString local_xmstring ;

	local_xmstring = XmStringCreate(">", XmFONTLIST_DEFAULT_TAG);
	argc = 0 ;
	XtSetArg( argv[argc], XmNlabelString, local_xmstring) ; ++argc ;
	XtSetValues( SB_SelectionLabel( new_w), argv, argc) ;
	XmStringFree(local_xmstring);

	new_w->selection_box.selection_label_string = NULL ;
    }
	   
    /* mustMatch does not apply to command... (it is always false) */
    if (new_w->selection_box.must_match != False) { 
        new_w->selection_box.must_match = False; 
        XmeWarning( (Widget) new_w, WARNING5);
    }

    /* check for and change max history items (remove items if needed) */
    if (new_w->command.history_max_items < 1) {
        new_w->command.history_max_items = 100;
        XmeWarning( (Widget) new_w, WARNING6);
    }
    argc = 0 ;
    XtSetArg( argv[argc], XmNitemCount, &max) ; ++argc ;
    XtGetValues( SB_List( new_w), argv, argc) ;

    if (new_w->command.history_max_items < max)
    {
        while (max > new_w->command.history_max_items) {
            XmListDeletePos (new_w->selection_box.list, 1);
            if (new_w->selection_box.list_selected_item_position > 0)
                new_w->selection_box.list_selected_item_position--;
            max--;
        }
    }
    if (new_w->selection_box.dialog_type != XmDIALOG_COMMAND)
    {   
        new_w->selection_box.dialog_type = XmDIALOG_COMMAND ;
        XmeWarning( (Widget) new_w, WARNING1);
        } 
    XtAddCallback (new_w->selection_box.text, XmNvalueChangedCallback, 
                                              CommandCallback, (XtPointer) new_w);
    new_w->command.error = FALSE;

    return ;
}
   
/****************************************************************
 * Process callback from the List in a SelectionBox.
 ****************/
/*ARGSUSED*/
static void 
ListCallback(
        Widget w,		/* unused */
        XtPointer client_data,
        XtPointer call_data )
{   
            XmListCallbackStruct * listCB = (XmListCallbackStruct *)
                                                                    call_data ;
            XmCommandCallbackStruct cmdCB ;
            XmCommandWidget cmdWid = (XmCommandWidget) client_data ;
            char *          text_value ;
            Arg             argv[5] ;
            Cardinal        argc ;
            int             count ;
            XmString        tmpXmString ;
/*********** reset is not used ************
            Boolean         reset = FALSE ;
****************/

    argc = 0 ;
    XtSetArg( argv[argc], XmNitemCount, &count) ; ++argc ;
    XtGetValues( SB_List( cmdWid), argv, argc) ;

    if(    !count    )
    {   return ;
        } 
    if(    cmdWid->command.error    )
    {   
        if(    (listCB->item_position == (count - 1))
            || (    (listCB->item_position == count)
                 && (listCB->reason != XmCR_DEFAULT_ACTION))    )
        {   
            /* Selection or default action on the blank line, or selection on
            *   error message.  Restore previous selection and return.
            */
            XmListDeselectPos( SB_List( cmdWid), listCB->item_position) ;

            if(    cmdWid->selection_box.list_selected_item_position    )
            {   XmListSelectPos( SB_List( cmdWid), 
                    cmdWid->selection_box.list_selected_item_position, FALSE) ;
                } 
            return ;
            } 
        XmListDeletePos( SB_List( cmdWid), 0) ;   /* Delete error message.*/
        XmListDeletePos( SB_List( cmdWid), 0) ;   /* Delete blank line.*/
        cmdWid->command.error = FALSE ;

        if(    count <= 2    )
        {   /* List only contained error message and blank line.
            */
            cmdWid->selection_box.list_selected_item_position = 0 ;
            return ;
            } 
        count -= 2 ;

        if(    (listCB->item_position > count)
            && (listCB->reason == XmCR_DEFAULT_ACTION)    )
        {   
            /* Default action on the error message line.  Restore previous
            *   selection, clear text area, and return.
            */
            if(    cmdWid->selection_box.list_selected_item_position    )
            {   XmListSelectPos( SB_List( cmdWid), 
                    cmdWid->selection_box.list_selected_item_position, FALSE) ;
                } 
            XmTextFieldSetString( cmdWid->selection_box.text, "") ;
            return ;
            }
        }
    if(    listCB->reason == XmCR_DEFAULT_ACTION    )
    {   
        /* If we are already at max items, remove first item.
        */
        if(    count >= cmdWid->command.history_max_items    )
        {
            XmListDeletePos( cmdWid->selection_box.list, 1) ;

            if(    cmdWid->selection_box.list_selected_item_position > 0    )
            {   cmdWid->selection_box.list_selected_item_position-- ;
                } 
            }
        tmpXmString = XmStringCopy( listCB->item) ;
        XmListAddItemUnselected( cmdWid->selection_box.list, tmpXmString, 0) ;

        XmListSetBottomPos( cmdWid->selection_box.list, 0) ;
        XmTextFieldSetString( cmdWid->selection_box.text, "") ;
        /* Call the users command entered callback.
        */
        cmdCB.reason = XmCR_COMMAND_ENTERED ;
        cmdCB.event  = NULL ;
        cmdCB.value  = tmpXmString ;
        cmdCB.length = XmStringLength( tmpXmString) ;
        XtCallCallbackList( (Widget) cmdWid, cmdWid->command.callback, &cmdCB) ;

        XmStringFree( tmpXmString) ;
        } 
    else /* listCB->reason is BROWSE_SELECT or SINGLE_SELECT */
    {   /* Update the text widget to relect the latest list selection.
        */
        cmdWid->selection_box.list_selected_item_position = 
                                                        listCB->item_position ;
        if((text_value = _XmStringGetTextConcat( listCB->item)) != NULL)
        {   
            XmTextFieldSetString( SB_Text( cmdWid), text_value) ;
            XmTextFieldSetInsertionPosition( SB_Text( cmdWid),
			       XmTextFieldGetLastPosition( SB_Text( cmdWid))) ;
            XtFree( text_value) ;
            } 
        } 
    return ;
    }
/****************************************************************
 * Callback for Text ValueChanged callback
 ****************/
/*ARGSUSED*/
static void 
CommandCallback(
        Widget w,		/* unused */
        XtPointer cl_data,
        XtPointer call_data )
{
            XmCommandWidget client_data = (XmCommandWidget) cl_data ;
    XmCommandCallbackStruct  cb;
    char *str;
/****************/

    cb.reason = XmCR_COMMAND_CHANGED;
    cb.event = ((XmAnyCallbackStruct *) call_data)->event ;

    /* get char* string from text and convert to XmString type */
    str = XmTextFieldGetString (client_data->selection_box.text);
    cb.value = XmStringGenerate(str, XmFONTLIST_DEFAULT_TAG,
				XmCHARSET_TEXT, NULL);
    XtFree (str);

    cb.length = XmStringLength (cb.value);

    XtCallCallbackList ((Widget) client_data, client_data->command.value_changed_callback, &cb);
    XmStringFree (cb.value);
    return ;
}

/****************************************************************/
static Boolean 
CommandParentProcess(
        Widget wid,
        XmParentProcessData event )
{
            XmCommandWidget cmd = (XmCommandWidget) wid ;
/****************/

    if(    (event->any.process_type == XmINPUT_ACTION)
        && (   (event->input_action.action == XmPARENT_ACTIVATE)
            || (    (event->input_action.action == XmPARENT_CANCEL)
                 && BB_CancelButton( cmd)))    )
    {   
        if(    event->input_action.action == XmPARENT_ACTIVATE    )
        {   
            _XmCommandReturn( (Widget) cmd, event->input_action.event,
                                      event->input_action.params,
                                         event->input_action.num_params) ;
            } 
        else
        {   _XmBulletinBoardCancel( (Widget) cmd,
               event->input_action.event, event->input_action.params,
                                         event->input_action.num_params) ;
            } 
        return( TRUE) ;
        } 

    return( _XmParentProcess( XtParent( cmd), event)) ;
    }

/****************************************************************
 * Return function to "complete" text entry
 ****************/
/*ARGSUSED*/
void 
_XmCommandReturn(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *numParams )	/* unused */
{
            XmCommandWidget w = (XmCommandWidget) wid ;
    XmCommandCallbackStruct cb;
    XmString                   tmpXmString;
    String                     tmpString;
            Arg             argv[5] ;
            Cardinal        argc ;
            int             count ;
/****************/

    /* if an error has been sent to the history list, remove it now */
    if (w->command.error) {
        XmListDeletePos (w->selection_box.list, 0);     /* delete error msg  */
        XmListDeletePos (w->selection_box.list, 0);     /* delete blank line */
        w->command.error = FALSE;
        XmListSetBottomPos (w->selection_box.list, 0);
    }

    /* update the history:  - get text, check null/empty  */
    /*                      - create XmString from text   */
    /*                      - increment local list count  */
    /*                      - add to history list         */
    /*                      - delete first element if too */
    /*                        many items (maxItemCount)   */
    /*                      - position list to end        */
    /*                      - reset selection list ptr    */
    /*                      - reset command to ""         */

    tmpString = XmTextFieldGetString (w->selection_box.text);
    if ((tmpString == NULL) || (strcmp(tmpString, "") == 0)) { 
        if (tmpString) XtFree(tmpString);
        return;
    }
    argc = 0 ;
    XtSetArg( argv[argc], XmNitemCount, &count) ; ++argc ;
    XtGetValues( SB_List( w), argv, argc) ;
    /* if already at max items, remove first item in list */

    if (count >= w->command.history_max_items) 
    {
        XmListDeletePos (w->selection_box.list, 1);
        if (w->selection_box.list_selected_item_position > 0)
            w->selection_box.list_selected_item_position--;
    }
    tmpXmString = XmStringGenerate(tmpString, XmFONTLIST_DEFAULT_TAG,
				   XmCHARSET_TEXT, NULL);
    XmListAddItemUnselected (w->selection_box.list, tmpXmString, 0);

    XmListSetBottomPos (w->selection_box.list, 0);
    XmTextFieldSetString (w->selection_box.text, "");
    /* call the users command entered callback */

    cb.reason = XmCR_COMMAND_ENTERED;
    cb.event  = event;
    cb.value  = tmpXmString;
    cb.length = XmStringLength (tmpXmString);
    XtCallCallbackList ((Widget) w, w->command.callback, &cb);
    XmStringFree (tmpXmString);
    XtFree (tmpString);
    return ;
}
/****************************************************************/
/*ARGSUSED*/
void 
_XmCommandUpOrDown(
        Widget wid,
        XEvent *event,		/* unused */
        String *argv,
        Cardinal *argc )	/* unused */
{
            XmCommandWidget cmd = (XmCommandWidget) wid ;
            int             visible ;
            int	            top ;
            int	            key_pressed ;
            Widget	    list ;
            int	*           position ;
            int	            count ;
            Arg             av[5] ;
            Cardinal        ac ;
            int             selected_count ;
/****************/

    if(    !(list = cmd->selection_box.list)    )
    {   return ;
        } 
    ac = 0 ;
    XtSetArg( av[ac], XmNitemCount, &count) ; ++ac ;
    XtSetArg( av[ac], XmNtopItemPosition, &top) ; ++ac ;
    XtSetArg( av[ac], XmNvisibleItemCount, &visible) ; ++ac ;
    XtSetArg( av[ac], XmNselectedItemCount, &selected_count); ac++;
    XtGetValues( (Widget) list, av, ac) ;

    if(    !count
        || (cmd->command.error  &&  (count <= 2))    )
    {   return ;
        } 
 
 /*
  * Fix for 5237 - Check the selected_count to ensure that a selection 
  *                exists.  No items may be selected if the application
  *                has run XmListDeselect* routine.  If no items in the
  *                list are selected, set list_selected_item_position to 0.
  */
    if (!selected_count)
       cmd->selection_box.list_selected_item_position = 0;

    if (_XmConvertActionParamToRepTypeId((Widget) cmd,
			 XmRID_COMMAND_SELECTION_BOX_UP_OR_DOWN_ACTION_PARAMS,
			 argv[0], True, &key_pressed) == False)
    {
	/* We couldn't convert the value. Just assume a value of 0. */
	key_pressed = 0;
    }

    position = &(cmd->selection_box.list_selected_item_position) ;

    if(    *position == 0    )
    {   
        /* If error is visible, select last item - 2.  Otherwise, select
        *   last item in list.
        */
        if(    cmd->command.error    )
        {   *position = count - 2 ;
            } 
        else
        {   *position = count ;
            } 
        XmListSelectPos( list, *position, True) ;
        } 
    else
    {   if(    !key_pressed && (*position > 1)    )
        {   /*  up  */
            XmListDeselectPos( list, *position) ;
            XmListSelectPos( list, --*position, True) ;
            }
        else
        {   if(    (key_pressed == 1) && (*position < count)    )
            {   /*  down  */
                XmListDeselectPos( list, *position) ;
                XmListSelectPos( list, ++*position, True) ;
                } 
            else
            {   if(    key_pressed == 2    )
                {   /*  home  */
                    XmListDeselectPos( list, *position) ;
                    *position = 1 ;
                    XmListSelectPos( list, *position, True) ;
                    } 
                else
                {   if(    key_pressed == 3    )
                    {   /*  end  */
                        XmListDeselectPos( list, *position) ;
                        *position = count ;
                        XmListSelectPos( list, *position, True) ;
                        } 
                    } 
                } 
            }
        } 
    if(    top > *position    )
    {   XmListSetPos( list, *position) ;
        } 
    else
    {   if(    (top + visible) <= *position    )
        {   XmListSetBottomPos( list, *position) ;
            } 
        } 
    return ;
    }
/****************************************************************
 * Set attributes of a command widget
 ****************/
/*ARGSUSED*/
static Boolean 
SetValues(
        Widget ow,
        Widget rw,		/* unused */
        Widget nw,
        ArgList args,		/* unused */
        Cardinal *num_args )	/* unused */
{
            XmCommandWidget old = (XmCommandWidget) ow ;
            XmCommandWidget new_w = (XmCommandWidget) nw ;
            int max;
            Arg             argv[5] ;
            Cardinal        argc ;
/****************/

    /* mustMatch does not apply to command... (it is always false) */
    if (new_w->selection_box.must_match != False) { 
        new_w->selection_box.must_match = False; 
        XmeWarning( (Widget) new_w, WARNING5);
    }

    if (new_w->selection_box.dialog_type != XmDIALOG_COMMAND) {
        new_w->selection_box.dialog_type = XmDIALOG_COMMAND;
        XmeWarning( (Widget) new_w, WARNING1);
    }

    /* check for and change max history items (remove items if needed) */
    if (new_w->command.history_max_items < 1) {
        new_w->command.history_max_items = old->command.history_max_items;
        XmeWarning( (Widget) new_w, WARNING6);
    }
    if (new_w->command.history_max_items < 
        old->command.history_max_items)
    {
        argc = 0 ;
        XtSetArg( argv[argc], XmNitemCount, &max) ; ++argc ;
        XtGetValues( SB_List( new_w), argv, argc) ;

        while (max > new_w->command.history_max_items) {
            XmListDeletePos (new_w->selection_box.list, 1);
            if (new_w->selection_box.list_selected_item_position > 0)
                new_w->selection_box.list_selected_item_position--;
            max--;
        }
    }
    /* require redisplay */
    return(True);
}
/****************************************************************
 * Create an instance of the widget
 ****************/
Widget 
XmCreateCommand(
        Widget parent,
        String name,
        ArgList al,
        Cardinal ac )
{
    Widget   w;
    ArgList  argsNew;
/****************/

    /* add dialogType to arglist and force to XmDIALOG_COMMAND... */
    /* big time bad stuff will happen if they use prompt type...  */
    /* (like, no list gets created, but used all through command) */

    /*  allocate arglist, copy args, add dialog type arg */
    argsNew = (ArgList) XtMalloc (sizeof(Arg) * (ac + 1));
    memcpy( argsNew, al, sizeof(Arg) * ac);
    XtSetArg (argsNew[ac], XmNdialogType, XmDIALOG_COMMAND);  ac++;

    /*  create Command, free argsNew, return */
    w = XtCreateWidget (name, xmCommandWidgetClass, parent, argsNew, ac);
    XtFree ((char *) argsNew);

    return (w);
}
/****************************************************************/
Widget 
XmCommandGetChild(
        Widget widget,
#if NeedWidePrototypes
        unsigned int child )
#else
        unsigned char child )
#endif /* NeedWidePrototypes */
{
    XmCommandWidget   w = (XmCommandWidget)widget;
    Widget ret_widget = NULL;
    _XmWidgetToAppContext(widget);
/****************/
    _XmAppLock(app);

    switch (child) {
        case XmDIALOG_COMMAND_TEXT:    
		ret_widget = w->selection_box.text;
		break;
        case XmDIALOG_HISTORY_LIST:    
		ret_widget = w->selection_box.list;
		break;
        case XmDIALOG_PROMPT_LABEL:    
		ret_widget = w->selection_box.selection_label;
		break;
	case XmDIALOG_WORK_AREA:       
		ret_widget = w->selection_box.work_area;
		break;
        default: 
		XmeWarning( (Widget) w, WARNING2);
		break;
    }
    _XmAppUnlock(app);
    return ret_widget;
}
/****************************************************************
 * Replace the text value with "value" 
 *     note: selection_box.text_string isn't updated, as we are not
 *           going to guarantee that text_string is up to date with
 *           the text widget string at all times...
 ****************/
void 
XmCommandSetValue(
        Widget widget,
        XmString value )
{
    XmCommandWidget   w = (XmCommandWidget)widget;
    char *str;
    _XmWidgetToAppContext(widget);
/****************/

    _XmAppLock(app);
    if(    !(str = _XmStringGetTextConcat( value))    )
    {   
        XmeWarning( (Widget) w, WARNING3);
	_XmAppUnlock(app);
        return;
        }
    XmTextFieldSetString(w->selection_box.text, str);
    XtFree(str);
    _XmAppUnlock(app);
    return ;
    }
/****************************************************************
 * Append "value" to the end of the current text value
 ****************/
void 
XmCommandAppendValue(
        Widget widget,
        XmString value )
{
    XmCommandWidget   w = (XmCommandWidget)widget;
    char            *strNew;
    XmTextPosition   endPosition;
    _XmWidgetToAppContext(widget);
/****************/

    if (value == NULL) return;

    _XmAppLock(app);
    /* return if invalid string from "value" passed in */
    if(    !(strNew = _XmStringGetTextConcat( value))    )
    {   
        XmeWarning( (Widget) w, WARNING3);
	_XmAppUnlock(app);
        return;
        }
    if(    !strNew  ||  !*strNew    ) {
        XmeWarning( (Widget) w, WARNING4);
	_XmAppUnlock(app);
        return;
        }
    /* get string length of current command string */
    endPosition = XmTextFieldGetLastPosition( w->selection_box.text) ;
    /* append new string to existing string */
    XmTextFieldReplace (w->selection_box.text, endPosition, endPosition,
			                                              strNew) ;
    /* reset insertion position to end of text, and free new string */
    XmTextFieldSetInsertionPosition( w->selection_box.text,
			  XmTextFieldGetLastPosition( w->selection_box.text)) ;
    XtFree (strNew);
    _XmAppUnlock(app);
    return ;
    }
/****************************************************************
 * Print a blank string and error string, which will get removed the next
 *   time a string is added to the history.
 ****************/
void 
XmCommandError(
        Widget widget,
        XmString error )
{
    XmCommandWidget   w = (XmCommandWidget)widget;
    XmString  blankXmString;

    _XmWidgetToAppContext(widget);
/****************/

    if (error == NULL) return;

    _XmAppLock(app);

    /* If error is currently in list remove it, but leave (or add) blank item.
    */
    if(    w->command.error    )
    {
        XmListDeletePos (w->selection_box.list, 0);
        }
    else
    {   blankXmString = XmStringCreate(" ", XmFONTLIST_DEFAULT_TAG);
        XmListAddItemUnselected (w->selection_box.list, blankXmString, 0);
        XmStringFree (blankXmString);
        w->command.error = TRUE;
        }

    /* create and add the error string as a list item */
    XmListAddItemUnselected (w->selection_box.list, error, 0);
    XmListSetBottomPos (w->selection_box.list, 0);

    _XmAppUnlock(app);
    return ;
    }


/****************************************************************
 * This convenience function creates a DialogShell and a Command
 *   child of the shell; returns the Command widget.
 ****************/
Widget 
XmCreateCommandDialog(
        Widget ds_p,
        String name,
        ArgList com_args,
        Cardinal com_n )
{
    return XmeCreateClassDialog (xmCommandWidgetClass,
				 ds_p, name, com_args, com_n) ;
}
