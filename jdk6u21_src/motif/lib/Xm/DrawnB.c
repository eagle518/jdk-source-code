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
static char rcsid[] = "$XConsortium: DrawnB.c /main/18 1996/03/25 17:51:35 barstow $"
#endif
#endif
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 * Include files & Static Routine Definitions
 */

#include <stdio.h>
#include <X11/X.h>
#include <Xm/ActivatableT.h>
#include <Xm/DisplayP.h>
#include <Xm/DrawP.h>   
#include <Xm/DrawnBP.h>
#include <Xm/LabelP.h>
#include <Xm/MenuT.h>
#include <Xm/TraitP.h>
#include <Xm/TransltnsP.h>
#include "XmI.h"
#include "RepTypeI.h"
#include "LabelI.h"
#include "MenuProcI.h"
#include "PrimitiveI.h"
#include "TravActI.h"
#include "TraversalI.h"


#define DELAY_DEFAULT 100	

/********    Static Function Declarations    ********/

static void Arm( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void MultiArm( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void Activate( 
                        Widget wid,
                        XEvent *buttonEvent,
                        String *params,
                        Cardinal *num_params) ;
static void MultiActivate( 
                        Widget wid,
                        XEvent *buttonEvent,
                        String *params,
                        Cardinal *num_params) ;
static void ActivateCommon( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void ArmAndActivate( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void ArmTimeout (
        		XtPointer closure,
        		XtIntervalId *id ) ;
static void Disarm( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void Enter( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void Leave( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void ClassInitialize( void ) ;
static void ClassPartInitialize( 
                        WidgetClass wc) ;
static void InitializePrehook( 
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static void Initialize( 
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static void Resize( 
                        Widget wid) ;
static void Redisplay( 
                        Widget wid,
                        XEvent *event,
                        Region region) ;
static void DrawPushButton( 
                        XmDrawnButtonWidget db,
#if NeedWidePrototypes
                        int armed) ;
#else
                        Boolean armed) ;
#endif /* NeedWidePrototypes */
static Boolean SetValuesPrehook( 
			Widget cw,
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValues( 
                        Widget cw,
                        Widget rw,
                        Widget nw,
                        ArgList args,
                        Cardinal *num_args) ;
static void Realize( 
                        Widget w,
                        XtValueMask *p_valueMask,
                        XSetWindowAttributes *attributes) ;
static void Destroy( 
                        Widget wid) ;

static void ChangeCB(Widget w, 
		     XtCallbackProc activCB,
		     XtPointer closure,
		     Boolean setunset) ;

/********    End Static Function Declarations    ********/

/*************************************<->*************************************
 *
 *
 *   Description:   translation tables for class: DrawnButton
 *   -----------
 *
 *   Matches events with string descriptors for internal routines.
 *
 *************************************<->***********************************/

#define defaultTranslations	_XmDrawnB_defaultTranslations


/*************************************<->*************************************
 *
 *
 *   Description:  action list for class: DrawnButton
 *   -----------
 *
 *   Matches string descriptors with internal routines.
 *   Note that Primitive will register additional event handlers
 *   for traversal.
 *
 *************************************<->***********************************/

static XtActionsRec actionsList[] =
{
  {"Arm", 	Arm		 },
  {"Activate", 	Activate		 },
  {"MultiActivate", MultiActivate		 },
  {"MultiArm",	MultiArm },
  {"ArmAndActivate", ArmAndActivate },
  {"Disarm", 	Disarm		 },
  {"Enter", 	Enter		 },
  {"Leave",	Leave		 },
  {"ButtonTakeFocus", _XmButtonTakeFocus },
};


/*  The resource list for Drawn Button  */

static XtResource resources[] = 
{     
   {
     XmNmultiClick, XmCMultiClick, XmRMultiClick, sizeof (unsigned char),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.multiClick),
     XmRImmediate, (XtPointer) XmMULTICLICK_KEEP
   },

   {
     XmNpushButtonEnabled, XmCPushButtonEnabled, XmRBoolean, sizeof (Boolean),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.pushbutton_enabled),
     XmRImmediate, (XtPointer) False
   },

   {
     XmNshadowType, XmCShadowType, XmRShadowType, sizeof(unsigned char),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.shadow_type),
     XmRImmediate, (XtPointer) XmSHADOW_ETCHED_IN
   },

   {
     XmNactivateCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.activate_callback),
     XmRPointer, (XtPointer) NULL
   },

   {
     XmNarmCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.arm_callback),
     XmRPointer, (XtPointer) NULL
   },

   {
     XmNdisarmCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.disarm_callback),
     XmRPointer, (XtPointer) NULL
   },
   
   {
     XmNexposeCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.expose_callback),
     XmRPointer, (XtPointer) NULL
   },

   {
     XmNresizeCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
     XtOffsetOf( struct _XmDrawnButtonRec, drawnbutton.resize_callback),
     XmRPointer, (XtPointer) NULL
   },
   
   {
     XmNshadowThickness, XmCShadowThickness, XmRHorizontalDimension, 
     sizeof(Dimension),
     XtOffsetOf( struct _XmDrawnButtonRec, primitive.shadow_thickness),
     XmRCallProc, (XtPointer) _XmSetThickness
   },

   {    
     XmNlabelString, XmCXmString, XmRXmString, sizeof(XmString),
     XtOffsetOf( struct _XmDrawnButtonRec, label._label),
     XmRImmediate, (XtPointer) XmUNSPECIFIED
   },
   {
	XmNtraversalOn,
	XmCTraversalOn,
	XmRBoolean,
	sizeof(Boolean),
	XtOffsetOf( struct _XmPrimitiveRec, primitive.traversal_on),
	XmRImmediate,
	(XtPointer) True
   },

   {
	XmNhighlightThickness,
	XmCHighlightThickness,
	XmRHorizontalDimension,
	sizeof (Dimension),
	XtOffsetOf( struct _XmPrimitiveRec, primitive.highlight_thickness),
	XmRCallProc,
	(XtPointer) _XmSetThickness
   }
};

static XmBaseClassExtRec       drawnBBaseClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    XmBaseClassExtVersion,                    /* version              */
    sizeof(XmBaseClassExtRec),                /* size                 */
    InitializePrehook,                        /* initialize prehook   */
    SetValuesPrehook,			      /* set_values prehook   */
    XmInheritInitializePosthook,              /* initialize posthook  */
    XmInheritSetValuesPosthook,               /* set_values posthook  */
    XmInheritClass,                           /* secondary class      */
    XmInheritSecObjectCreate,                 /* creation proc        */
    XmInheritGetSecResData,                   /* getSecResData        */
    {0},                                      /* fast subclass        */
    XmInheritGetValuesPrehook,                /* get_values prehook   */
    XmInheritGetValuesPosthook,               /* get_values posthook  */
    XmInheritClassPartInitPrehook,            /* classPartInitPrehook */
    XmInheritClassPartInitPosthook,           /* classPartInitPosthook*/
    NULL,                                     /* ext_resources        */
    NULL,                                     /* compiled_ext_resources*/
    0,                                        /* num_ext_resources    */
    FALSE,                                    /* use_sub_resources    */
    XmInheritWidgetNavigable,                 /* widgetNavigable      */
    XmInheritFocusChange,                     /* focusChange          */
};



/*************************************<->*************************************
 *
 *
 *   Description:  global class record for instances of class: DrawnButton
 *   -----------
 *
 *   Defines default field settings for this class record.
 *
 *************************************<->***********************************/

externaldef(xmdrawnbuttonclassrec) XmDrawnButtonClassRec xmDrawnButtonClassRec ={
  {
/* core_class record */	
    /* superclass	  */	(WidgetClass) &xmLabelClassRec,
    /* class_name	  */	"XmDrawnButton",
    /* widget_size	  */	sizeof(XmDrawnButtonRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */    ClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */    NULL,
    /* realize		  */	Realize,
    /* actions		  */	actionsList,
    /* num_actions	  */	XtNumber(actionsList),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	XtExposeNoCompress,
    /* compress_enterlv   */    TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	Resize,
    /* expose		  */	Redisplay,
    /* set_values	  */	SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus	  */	NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    (char*)defaultTranslations,
    /* query_geometry     */	NULL, 
    /* display_accelerator */   NULL,
    /* extension          */    (XtPointer) &drawnBBaseClassExtRec,
  },

  { /* primitive_class record       */

    /* Primitive border_highlight   */	XmInheritWidgetProc,
    /* Primitive border_unhighlight */	XmInheritWidgetProc,
    /* translations		    */  XtInheritTranslations,
    /* arm_and_activate		    */  ArmAndActivate,
    /* get resources		    */  NULL,
    /* num get_resources	    */  0,
    /* extension		    */  NULL,
  },

  { /* label_class record */
 
    /* setOverrideCallback*/    XmInheritWidgetProc,
    /* Menu procedures    */    NULL,				
    /* menu trav xlations */	NULL,
    /* extension	  */	NULL,
  },

  { /* drawnbutton_class record */

    /* extension	  */    NULL,	
  }

};
externaldef(xmdrawnbuttonwidgetclass) WidgetClass xmDrawnButtonWidgetClass =
			     (WidgetClass)&xmDrawnButtonClassRec;

/* Trait record for drawnButton */

static XmConst XmActivatableTraitRec drawnButtonAT = {
  0,		/* version */
  ChangeCB,
};

/* Menu Savvy trait record */
static XmMenuSavvyTraitRec MenuSavvyRecord = {
    /* version: */
    -1,
    NULL,
    NULL,
    NULL,
    _XmCBNameActivate,
};


/************************************************************************
 *
 *     Arm
 *
 *     This function processes button 1 down occuring on the drawnbutton.
 *     Mark the drawnbutton as armed if XmNpushButtonEnabled is TRUE.
 *     The callbacks for XmNarmCallback are called.
 *
 ************************************************************************/
/*ARGSUSED*/
static void 
Arm(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
    XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
    XButtonEvent *buttonEvent = (XButtonEvent *) event;
    XmDrawnButtonCallbackStruct call_value;
   
    (void) XmProcessTraversal((Widget) db, XmTRAVERSE_CURRENT);

    db -> drawnbutton.armed = TRUE;
    if (event && (event->type == ButtonPress))
	db -> drawnbutton.armTimeStamp = buttonEvent->time;
    
    if (db->drawnbutton.pushbutton_enabled)
	DrawPushButton(db, db->drawnbutton.armed);

    if (db->drawnbutton.arm_callback) {
	XFlush(XtDisplay (db));

	call_value.reason = XmCR_ARM;
	call_value.event = event;
	call_value.window = XtWindow (db);
	XtCallCallbackList ((Widget) db, db->drawnbutton.arm_callback, 
			    &call_value);
    }
}


/*ARGSUSED*/
static void 
MultiArm(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
    if (((XmDrawnButtonWidget) wid)->drawnbutton.multiClick == XmMULTICLICK_KEEP)
			Arm (wid, event, NULL, NULL);
}

/************************************************************************
 *
 *     Activate
 *
 *     Mark the drawnbutton as unarmed (i.e. inactive).
 *     The foreground and background colors will revert to the 
 *     unarmed state if XmNinvertOnArm is set to TRUE.
 *     If the button release occurs inside of the DrawnButton, the 
 *     callbacks for XmNactivateCallback are called.
 *
 ************************************************************************/
static void 
Activate(
        Widget wid,
        XEvent *buttonEvent,
        String *params,
        Cardinal *num_params )
{
        XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   if (db -> drawnbutton.armed == FALSE)
      return;

   db->drawnbutton.click_count = 1;
   ActivateCommon ((Widget) db, buttonEvent, params, num_params);

}

static void 
MultiActivate(
        Widget wid,
        XEvent *buttonEvent,
        String *params,
        Cardinal *num_params )
{
        XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   /* When a multi click sequence occurs and the user Button Presses and
    * holds for a length of time, the final release should look like a
    * new/separate activate.
    */
  if (db->drawnbutton.multiClick == XmMULTICLICK_KEEP)  
  { if ((buttonEvent->xbutton.time - db->drawnbutton.armTimeStamp) >
	   XtGetMultiClickTime(XtDisplay(db)))
     db->drawnbutton.click_count = 1;
   else
     db->drawnbutton.click_count++;
   ActivateCommon ((Widget) db, buttonEvent, params, num_params) ;
   Disarm ((Widget) db, buttonEvent, params, num_params) ;
 }
}

/*ARGSUSED*/
static void 
ActivateCommon(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
   XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   XmDrawnButtonCallbackStruct call_value;
   XmMenuSystemTrait menuSTrait;

   menuSTrait = (XmMenuSystemTrait) 
     XmeTraitGet((XtPointer) XtClass(XtParent(wid)), XmQTmenuSystem);
      
   if (event && (event->xbutton.type != ButtonRelease))
       return;
      
   db -> drawnbutton.armed = FALSE;
   if (db->drawnbutton.pushbutton_enabled)
	DrawPushButton(db, db->drawnbutton.armed);


  /* CR 9181: Consider clipping when testing visibility. */
  if ((db->drawnbutton.activate_callback) &&
      ((event->xany.type == ButtonPress) || 
       (event->xany.type == ButtonRelease)) &&
      _XmGetPointVisibility(wid, event->xbutton.x_root, event->xbutton.y_root))
   {
      XFlush(XtDisplay (db));

      call_value.reason = XmCR_ACTIVATE;
      call_value.event = event;
      call_value.window = XtWindow (db);
      call_value.click_count = db->drawnbutton.click_count;

      if ((db->drawnbutton.multiClick == XmMULTICLICK_DISCARD) &&
	  (call_value.click_count > 1))
      {
	  return;
      }

      if (menuSTrait != NULL)
      {
	menuSTrait->entryCallback(XtParent(db), (Widget) db, 
					   &call_value);
      }

      if ((! db->label.skipCallback) &&
	  (db->drawnbutton.activate_callback))
      {
	 XtCallCallbackList ((Widget) db, db->drawnbutton.activate_callback,
				&call_value);
      }
   }
}

/************************************************************************
 *
 *     ArmAndActivate
 *
 ************************************************************************/
/*ARGSUSED*/
static void 
ArmAndActivate(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
   XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   XmDrawnButtonCallbackStruct call_value;
   XmMenuSystemTrait menuSTrait;

   menuSTrait = (XmMenuSystemTrait) 
     XmeTraitGet((XtPointer) XtClass(XtParent(wid)), XmQTmenuSystem);

   db -> drawnbutton.armed = TRUE;
   if (db->drawnbutton.pushbutton_enabled)
	DrawPushButton(db, db->drawnbutton.armed);

   XFlush(XtDisplay (db));

   if (db->drawnbutton.arm_callback)
   {
      call_value.reason = XmCR_ARM;
      call_value.event = event;
      call_value.window = XtWindow (db);
      XtCallCallbackList ((Widget) db, db->drawnbutton.arm_callback, &call_value);
   }

   call_value.reason = XmCR_ACTIVATE;
   call_value.event = event;
   call_value.window = XtWindow (db);
   call_value.click_count = 1;		/* always 1 in kselect */

   if (menuSTrait != NULL)
   {
     menuSTrait->entryCallback(XtParent(db), (Widget) db, 
					&call_value);
   }

   if ((! db->label.skipCallback) &&
       (db->drawnbutton.activate_callback))
   {
      XtCallCallbackList ((Widget) db, db->drawnbutton.activate_callback,
			  &call_value);
   }

   db->drawnbutton.armed = FALSE;
   
   if (db->drawnbutton.disarm_callback)
   {
      call_value.reason = XmCR_DISARM;
      XtCallCallbackList ((Widget) db, db->drawnbutton.disarm_callback,
                             &call_value);
   }

   /* If the button is still around, show it released, after a short delay */
   if (!db->core.being_destroyed && db->drawnbutton.pushbutton_enabled)
   {
       db->drawnbutton.timer = XtAppAddTimeOut(
				       XtWidgetToApplicationContext((Widget)db),
                                       (unsigned long) DELAY_DEFAULT,
                                       ArmTimeout,
                                       (XtPointer)db);
   }
}

/*ARGSUSED*/
static void 
ArmTimeout (
	XtPointer closure,
	XtIntervalId *id )
{
  XmDrawnButtonWidget db = (XmDrawnButtonWidget) closure ;

  db -> drawnbutton.timer = 0;

  if (db->drawnbutton.pushbutton_enabled &&
      XtIsRealized((Widget)db) && XtIsManaged((Widget)db))
   {
     DrawPushButton(db, db->drawnbutton.armed);
     XFlush (XtDisplay (db));
   }
}



/************************************************************************
 *
 *    Disarm
 *
 *     Mark the drawnbutton as unarmed (i.e. active).
 *     The foreground and background colors will revert to the 
 *     unarmed state if XmNinvertOnSelect is set to TRUE and the
 *     drawnbutton is not in a menu.
 *     The callbacks for XmNdisarmCallback are called..
 *
 ************************************************************************/
/*ARGSUSED*/
static void 
Disarm(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
   XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   XmDrawnButtonCallbackStruct call_value;

   db -> drawnbutton.armed = FALSE;

   if (db->drawnbutton.disarm_callback)
   {
      XFlush(XtDisplay (db));

      call_value.reason = XmCR_DISARM;
      call_value.event = event;
      call_value.window = XtWindow (db);
      XtCallCallbackList ((Widget) db, db->drawnbutton.disarm_callback, &call_value);
   }
}


/************************************************************************
 *
 *  Enter
 *
 ************************************************************************/
static void 
Enter(
        Widget wid,
        XEvent *event,
        String *params,
        Cardinal *num_params )
{
   XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   _XmPrimitiveEnter (wid, event, params, num_params);

   if (db -> drawnbutton.pushbutton_enabled &&
      db -> drawnbutton.armed == TRUE)
      DrawPushButton(db, TRUE);
}


/************************************************************************
 *
 *  Leave
 *
 ************************************************************************/
static void 
Leave(
        Widget wid,
        XEvent *event,
        String *params,
        Cardinal *num_params )
{
            XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   _XmPrimitiveLeave (wid, event, params, num_params);

   if (db -> drawnbutton.pushbutton_enabled &&
      db -> drawnbutton.armed == TRUE)
      DrawPushButton(db, FALSE);
}


/************************************************************************
 *
 *  ClassInitialize
 *     Set up the base class extension record.
 *
 ************************************************************************/
static void 
ClassInitialize( void )
{
   /* set up base class extension quark */
   drawnBBaseClassExtRec.record_type = XmQmotif;
}

/************************************************************************
 *
 *  ClassPartInitialize
 *     Set up the fast subclassing for the widget
 *
 ************************************************************************/
static void 
ClassPartInitialize(
        WidgetClass wc )
{
  _XmFastSubclassInit (wc, XmDRAWN_BUTTON_BIT);

  /* Install the menu savvy trait record,  copying fields from XmLabel */
  _XmLabelCloneMenuSavvy (wc, &MenuSavvyRecord);

  /* Install the activatable trait for all subclasses */
  XmeTraitSet((XtPointer)wc, XmQTactivatable, (XtPointer) &drawnButtonAT);
}
     
/************************************************************
 *
 * InitializePrehook
 *
 * Put the proper translations in core_class tm_table so that
 * the data is massaged correctly
 *
 ************************************************************/

/*ARGSUSED*/
static void
InitializePrehook(
        Widget rw,		/* unused */
        Widget nw,
        ArgList args,		/* unused */
        Cardinal *num_args )	/* unused */
{
  XmDrawnButtonWidget bw = (XmDrawnButtonWidget) nw ;
  XmBaseClassExt *wcePtr;

  /* Envelop Label's InitializePrehook. */
  wcePtr = _XmGetBaseClassExtPtr
    (xmDrawnButtonWidgetClass->core_class.superclass, XmQmotif);
  if (wcePtr && *wcePtr)
    {
      if ((*wcePtr)->initializePrehook)
	(*((*wcePtr)->initializePrehook)) (rw, nw, args, num_args);
    }

  /* CR 2990: Use XmNbuttonFontList as the default font. */
  if (bw->label.font == NULL)
    bw->label.font = XmeGetDefaultRenderTable (nw, XmBUTTON_FONTLIST);
}

/*************************************<->*************************************
 *
 *  Initialize 
 *
 *************************************<->***********************************/
/*ARGSUSED*/
static void 
Initialize(
        Widget rw,
        Widget nw,
        ArgList args,
        Cardinal *num_args )
{
   XmDrawnButtonWidget new_w = (XmDrawnButtonWidget) nw ;
   XmDrawnButtonWidget req_w = (XmDrawnButtonWidget) rw ;

   /* CR 2990:  Use XmNbuttonFontList as the default font. */
   if (req_w->label.font == NULL)
     {
       XmFontListFree (new_w->label.font);
       new_w->label.font =
	 XmFontListCopy (XmeGetDefaultRenderTable (nw, XmBUTTON_FONTLIST));
     }

   new_w->drawnbutton.armed = FALSE;
   new_w->drawnbutton.timer = 0;

   /* if menuProcs is not set up yet, try again */
   if (xmLabelClassRec.label_class.menuProcs == (XmMenuProc)NULL)
      xmLabelClassRec.label_class.menuProcs =
	 (XmMenuProc) _XmGetMenuProcContext();

   if(    !XmRepTypeValidValue( XmRID_SHADOW_TYPE,
                               new_w->drawnbutton.shadow_type, (Widget) new_w)    )
   {
      new_w -> drawnbutton.shadow_type = XmSHADOW_ETCHED_IN;
   }

}

/*************************************<->*************************************
 *
 *  Resize (db)
 *
 *************************************<->***********************************/
static void 
Resize(
        Widget wid )
{
   XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   XmDrawnButtonCallbackStruct call_value;
   XtWidgetProc resize;

   _XmProcessLock();
   resize = xmLabelClassRec.core_class.resize;
   _XmProcessUnlock();

  (* resize) ((Widget) db);
 
   /* CR 5419: Suppress redundant calls to the resize callbacks. */
   if (db->drawnbutton.resize_callback &&
       !Lab_ComputingSize(db))
   {
      XFlush(XtDisplay (db));
      call_value.reason = XmCR_RESIZE;
      call_value.event = NULL;
      call_value.window = XtWindow (db);
      XtCallCallbackList ((Widget) db, db->drawnbutton.resize_callback, &call_value);
   }
}


/*************************************<->*************************************
 *
 *  Redisplay (db, event, region)
 *
 *************************************<->***********************************/
/*ARGSUSED*/
static void 
Redisplay(
        Widget wid,
        XEvent *event,
        Region region )
{
   XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   XmDrawnButtonCallbackStruct call_value;

   if (XtIsRealized((Widget)db)) 
   {
        if (event) {
	 XtExposeProc expose;

	 _XmProcessLock();
	 expose = xmLabelClassRec.core_class.expose;
	 _XmProcessUnlock();

         (* expose) ((Widget) db, event, region);
        }

 	if (db->drawnbutton.pushbutton_enabled)
 	    DrawPushButton(db, db->drawnbutton.armed);
  
 	else
 	    XmeDrawShadows(XtDisplay((Widget) db),
 			    XtWindow((Widget) db),
 			    db -> primitive.top_shadow_GC,
 			    db -> primitive.bottom_shadow_GC,
 			    db -> primitive.highlight_thickness,
 			    db -> primitive.highlight_thickness,
 			    db -> core.width - 2 *
 			       db -> primitive.highlight_thickness,
 			    db -> core.height - 2 *
 			       db -> primitive.highlight_thickness,	
			    db -> primitive.shadow_thickness,
 			    db->drawnbutton.shadow_type);
 			   
      if (db->drawnbutton.expose_callback)
      {
         XFlush(XtDisplay (db));

	 call_value.reason = XmCR_EXPOSE;
	 call_value.event = event;
	 call_value.window = XtWindow (db);
	 XtCallCallbackList ((Widget) db, db->drawnbutton.expose_callback, &call_value);
      }

   }
}


static void 
DrawPushButton(
        XmDrawnButtonWidget db,
#if NeedWidePrototypes
        int armed )
#else
        Boolean armed )
#endif /* NeedWidePrototypes */
{
  XmDisplay dpy = (XmDisplay) XmGetXmDisplay(XtDisplay((Widget) db));
  Boolean etched_in = dpy -> display.enable_etched_in_menu;
  Boolean in_menu = Lab_IsMenupane((Widget) db);
  Boolean do_draw;
  unsigned int type;

  do_draw = (! in_menu) || (in_menu && armed);

  if (in_menu)
    type = etched_in ? XmSHADOW_IN : XmSHADOW_OUT;
  else
    type = armed ? XmSHADOW_IN : XmSHADOW_OUT;

  if (do_draw)
    XmeDrawShadows (XtDisplay (db), XtWindow (db), 
		    db -> primitive.top_shadow_GC,
		    db -> primitive.bottom_shadow_GC, 
		    db -> primitive.highlight_thickness,
		    db -> primitive.highlight_thickness,
		    db -> core.width - 2 * 
		    db->primitive.highlight_thickness,
		    db -> core.height - 2 * 
		    db->primitive.highlight_thickness,
		    db -> primitive.shadow_thickness,
		    type);
}


/************************************************************************
 *
 *  SetValuesPrehook
 *
 ************************************************************************/

/*ARGSUSED*/
static Boolean 
SetValuesPrehook(
        Widget cw,		/* unused */
        Widget rw,		/* unused */
        Widget nw,
        ArgList args,		/* unused */
        Cardinal *num_args )	/* unused */
{
  XmDrawnButtonWidget bw = (XmDrawnButtonWidget) nw ;

  /* CR 2990: Use XmNbuttonFontList as the default font. */
  if (bw->label.font == NULL)
    bw->label.font = XmeGetDefaultRenderTable (nw, XmBUTTON_FONTLIST);

  return False;
}

/*************************************<->*************************************
 *
 *  SetValues(current, request, new_w)
 *
 *   Description:
 *   -----------
 *     This is the set values procedure for the drawnbutton class.  It is
 *     called last (the set values rtnes for its superclasses are called
 *     first).
 *
 *
 *   Inputs:
 *   ------
 *    current = original widget;
 *    request = original copy of request;
 *    new_w = copy of request which reflects changes made to it by
 *          set values procedures of its superclasses;
 *    last = TRUE if this is the last set values procedure to be called.
 * 
 *   Outputs:
 *   -------
 *
 *   Procedures Called
 *   -----------------
 *
 *************************************<->***********************************/
/*ARGSUSED*/
static Boolean 
SetValues(
        Widget cw,
        Widget rw,
        Widget nw,
        ArgList args,
        Cardinal *num_args )
{
   XmDrawnButtonWidget current = (XmDrawnButtonWidget) cw ;
   XmDrawnButtonWidget new_w = (XmDrawnButtonWidget) nw ;
   Boolean  flag = FALSE;    /* our return value */

    /*  Check the data put into the new widget.  */

   if(    !XmRepTypeValidValue( XmRID_SHADOW_TYPE,
                               new_w->drawnbutton.shadow_type, (Widget) new_w)    )
   {
      new_w->drawnbutton.shadow_type = current->drawnbutton.shadow_type ;
   }

   if (new_w -> drawnbutton.shadow_type != current-> drawnbutton.shadow_type ||
       new_w -> primitive.foreground != current -> primitive.foreground    ||
       new_w -> core.background_pixel != current -> core.background_pixel  ||
       new_w -> primitive.highlight_thickness != 
       current -> primitive.highlight_thickness                          ||
       new_w -> primitive.shadow_thickness !=
       current -> primitive.shadow_thickness)
   {
      flag = TRUE;
   }

   return(flag);
}




/*************************************************************************
 *
 *  Realize
 *	This function sets the bit gravity to forget.
 *
 *************************************************************************/
static void 
Realize(
        Widget w,
        XtValueMask *p_valueMask,
        XSetWindowAttributes *attributes )
{
   Mask valueMask = *p_valueMask;

   valueMask |= CWBitGravity | CWDontPropagate;
   attributes->bit_gravity = ForgetGravity;
   attributes->do_not_propagate_mask =
      ButtonPressMask | ButtonReleaseMask |
      KeyPressMask | KeyReleaseMask | PointerMotionMask;

   XtCreateWindow (w, InputOutput, CopyFromParent, valueMask, attributes);
}



/************************************************************************
 *
 *  Destroy
 *	Clean up allocated resources when the widget is destroyed.
 *
 ************************************************************************/
static void 
Destroy(
        Widget wid )
{
        XmDrawnButtonWidget db = (XmDrawnButtonWidget) wid ;
   if (db->drawnbutton.timer)
   {
       XtRemoveTimeOut (db->drawnbutton.timer);
       /* Fix for 1254749 */
       db->drawnbutton.timer = (XtIntervalId) NULL;
   }
}

/************************************************************************
 *
 *  ChangeCB
 *	add or remove the activate callback list.
 *      
 ************************************************************************/
static void 
ChangeCB(
	 Widget w, 
	 XtCallbackProc activCB,
	 XtPointer closure,
	 Boolean setunset)
{
    if (setunset) {
	XtAddCallback (w, XmNactivateCallback, activCB, closure);
    } else {
	XtRemoveCallback (w, XmNactivateCallback, activCB, closure);
    }
}


/************************************************************************
 *
 *		Application Accessible External Functions
 *
 ************************************************************************/


/************************************************************************
 *
 *  XmCreateDrawnButton
 *	Create an instance of a drawnbutton and return the widget id.
 *
 ************************************************************************/
Widget 
XmCreateDrawnButton(
        Widget parent,
        char *name,
        ArgList arglist,
        Cardinal argcount )
{
   return (XtCreateWidget (name, xmDrawnButtonWidgetClass, 
                           parent, arglist, argcount));
}
