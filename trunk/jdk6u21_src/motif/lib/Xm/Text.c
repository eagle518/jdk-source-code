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
static char rcsid[] = "$XConsortium: Text.c /main/39 1996/11/08 10:57:32 pascale $"
#endif
#endif
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#define TEXT

#include <locale.h>
#include <errno.h> /* Bug Id : 1217687 */
#include <string.h>
#include <X11/Xos.h>
#include <X11/keysymdef.h>
#include <Xm/AccTextT.h>
#include <Xm/AtomMgr.h>
#include <Xm/BaseClassP.h>
#include <Xm/CutPaste.h>
#include <Xm/Display.h>
#include <Xm/DropSMgr.h>
#include <Xm/ManagerP.h>
#include <Xm/ScrolledW.h>
#include <Xm/TraitP.h>
#include <Xm/TransltnsP.h>
#include <Xm/XmosP.h>
#include "MessagesI.h"
#include "RepTypeI.h"
#include "TextI.h"
#include "TextInI.h"
#include "TextOutI.h"
#include "TextSelI.h"
#include "TextStrSoI.h"
#include "VendorSEI.h"
#include "XmI.h"
#include "XmStringI.h"

#ifdef SUN_CTL
#include <XmXOC.h>
#include <XmRenderTI.h>
#endif /* CTL */

/* Resolution independence conversion functions */

#define MESSAGE2	_XmMMsgText_0000
#define MESSAGE3	_XmMMsgText_0003 /* Bud : 1217687 */

/* Memory Management for global line table */
#define INIT_TABLE_SIZE		  64
#define TABLE_INCREMENT		1024
#define XmDYNAMIC_BOOL		 255


/* Change ChangeVSB() and RedisplayHBar from TextOut.c to non-static functions;
 * they are needed for updating the scroll bars after re-enable redisplay.
 * DisableRedisplay prohibits the visuals of the widget from being updated
 * as the widget's contents are changed.  If the widget is a scrolled widget,
 * this change prohibits the scroll bars from being updated until redisplay
 * is re-enabled.
 */
extern void _XmChangeVSB(XmTextWidget widget);
extern void _XmRedisplayHBar(XmTextWidget widget);
extern void _XmChangeHSB(XmTextWidget widget);
extern void _XmRedisplayVBar(XmTextWidget widget);

/********    Static Function Declarations    ********/

static void NullAddWidget(XmTextSource source,
			  XmTextWidget tw);

static void NullRemoveWidget(XmTextSource source,
			     XmTextWidget tw);

static XmTextPosition NullRead(XmTextSource source,
			       XmTextPosition position,
			       XmTextPosition last_position,
			       XmTextBlock block);

static XmTextStatus NullReplace(XmTextWidget tw,
				XEvent *event,
				XmTextPosition *start,
				XmTextPosition *end,
				XmTextBlock block,
#if NeedWidePrototypes
				int call_callbacks);
#else
                                Boolean call_callbacks);
#endif /* NeedsWidePrototypes */

static XmTextPosition NullScan(XmTextSource source,
                               XmTextPosition position,
                               XmTextScanType sType,
                               XmTextScanDirection dir,
			       int n,
#if NeedWidePrototypes
			       int include);
#else
                               Boolean include);
#endif /* NeedWidePrototypes */

static Boolean NullGetSelection(XmTextSource source,
                                XmTextPosition *start,
                                XmTextPosition *end);

static void NullSetSelection(XmTextSource source,
			     XmTextPosition start,
			     XmTextPosition end,
			     Time time);

static void _XmCreateCutBuffers(Widget w);

static Cardinal GetSecResData(WidgetClass w_class,
			      XmSecondaryResourceData **secResDataRtn);

static void ClassPartInitialize(WidgetClass wc);

static void ClassInitialize(void);

static void AddRedraw(XmTextWidget tw,
		      XmTextPosition left,
		      XmTextPosition right);

static _XmHighlightRec * FindHighlight(XmTextWidget tw,
				       XmTextPosition position,
				       XmTextScanDirection dir);

static void DisplayText(XmTextWidget tw,
                        XmTextPosition updateFrom,
                        XmTextPosition updateTo);

static void RedrawChanges(XmTextWidget tw);

static void DoMove(XmTextWidget tw,
		   int startcopy,
		   int endcopy,
		   int destcopy);

static void RefigureLines(XmTextWidget tw);

static void RemoveLines(XmTextWidget tw,
                        int num_lines,
                        unsigned int cur_index);

static void AddLines(XmTextWidget tw,
		     XmTextLineTable temp_table,
		     unsigned int tmp_index,
		     unsigned int current_index);

static void InitializeLineTable(XmTextWidget tw,
				register int size);

static void FindHighlightingChanges(XmTextWidget tw);

static void Redisplay(XmTextWidget tw);

static void InsertHighlight(XmTextWidget tw,
			    XmTextPosition position,
			    XmTextPosition end,
			    XmHighlightMode mode);

static void Initialize(Widget rw,
		       Widget nw,
		       ArgList args,
		       Cardinal *num_args);

static void InitializeHook(Widget wid,
			   ArgList args,
			   Cardinal *num_args_ptr);

static void Realize(Widget w,
		    XtValueMask *valueMask,
		    XSetWindowAttributes *attributes);

static void Destroy(Widget w);

static void Resize(Widget w);

static void DoExpose(Widget w,
		     XEvent *event,
		     Region region);

static void GetValuesHook(Widget w,
			  ArgList args,
			  Cardinal *num_args_ptr);

static Boolean SetValues(Widget oldw,
			 Widget reqw,
			 Widget new_w,
			 ArgList args,
			 Cardinal *num_args);

static XtGeometryResult QueryGeometry(Widget w,
				      XtWidgetGeometry *intended,
				      XtWidgetGeometry *reply);

static void _XmTextSetString(Widget widget,
			     char *value);

static XtPointer TextGetValue(Widget w, 
			      int format);

static void TextSetValue(Widget w, 
			 XtPointer s, 
			 int format);

static int TextPreferredValue(Widget w);

static int PreeditStart(XIC xic,
                        XPointer client_data,
                        XPointer call_data);

static void PreeditDone(XIC xic,
                        XPointer client_data,
                        XPointer call_data);

static void PreeditDraw(XIC xic,
                        XPointer client_data,
                        XIMPreeditDrawCallbackStruct *call_data);

static void PreeditCaret(XIC xic,
                         XPointer client_data,
                         XIMPreeditCaretCallbackStruct *call_data);


/********    End Static Function Declarations    ********/

/*
 * For resource list management. 
 */

static XmTextSourceRec nullsource;
static XmTextSource nullsourceptr = &nullsource;

#define _XmTextEventBindings1	_XmTextIn_XmTextEventBindings1
#define _XmTextEventBindings2	_XmTextIn_XmTextEventBindings2
#define _XmTextEventBindings3	_XmTextIn_XmTextEventBindings3
#define _XmTextVEventBindings	_XmTextIn_XmTextVEventBindings

/* Solaris 2.6 Motif diff bug #4085003 */
#ifdef CDE_INTEGRATE
_XmConst char _XmTextIn_XmTextEventBindings_CDE[] = "\
~c ~s ~m ~a <Btn1Down>:process-press(grab-focus,secondary-drag)\n\
c ~s ~m ~a <Btn1Down>:process-press(move-destination,secondary-drag)\n\
~c s ~m ~a <Btn1Down>:process-press(extend-start,secondary-drag)\n\
~c ~m ~a <Btn1Motion>:extend-adjust()\n\
~c ~m ~a <Btn1Up>:extend-end()";
_XmConst char _XmTextIn_XmTextEventBindings_CDEBtn2[] = "\
<Btn2Down>:extend-start()\n\
<Btn2Motion>:extend-adjust()\n\
<Btn2Up>:extend-end()";

#define _XmTextEventBindingsCDE _XmTextIn_XmTextEventBindings_CDE
#define _XmTextEventBindingsCDEBtn2 _XmTextIn_XmTextEventBindings_CDEBtn2
#endif /* CDE_INTEGRATE */
/* END Solaris 2.6 Motif diff bug #4085003 */

#define EraseInsertionPoint(tw)\
{\
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, off);\
}

#define TextDrawInsertionPoint(tw)\
{\
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);\
}

static XtResource resources[] =
{
  {
    XmNsource, XmCSource, XmRPointer, sizeof(XtPointer),
    XtOffsetOf(struct _XmTextRec, text.source),
    XmRPointer, (XtPointer) &nullsourceptr
  },

  {
    XmNactivateCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.activate_callback),
    XmRCallback, NULL
  },

  {
    XmNfocusCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.focus_callback),
    XmRCallback, NULL
  },

  {
    XmNlosingFocusCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.losing_focus_callback),
    XmRCallback, NULL
  },

  {
    XmNvalueChangedCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.value_changed_callback),
    XmRCallback, NULL
  },

  {
    XmNdestinationCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.destination_callback),
    XmRCallback, NULL
  },

  {
    XmNmodifyVerifyCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.modify_verify_callback),
    XmRCallback, NULL
  },

  {
    XmNmodifyVerifyCallbackWcs, XmCCallback, XmRCallback, 
    sizeof(XtCallbackList), 
    XtOffsetOf(struct _XmTextRec, text.wcs_modify_verify_callback),
    XmRCallback, NULL
  },

  {
    XmNmotionVerifyCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.motion_verify_callback),
    XmRCallback, NULL
  },

  {
    XmNgainPrimaryCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.gain_primary_callback),
    XmRCallback, NULL
  },

  {
    XmNlosePrimaryCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf(struct _XmTextRec, text.lose_primary_callback),
    XmRCallback, NULL
  },

  {
    XmNtextHighlightCallback, XmCCallback,XmRCallback, sizeof(XtCallbackList),
    XtOffsetOf( struct _XmTextRec, text.highlight_callback),
    XmRCallback, NULL
  },

  {
    XmNvalue, XmCValue, XmRString, sizeof(String),
    XtOffsetOf(struct _XmTextRec, text.value),
    XmRString, ""
  },

  {
    XmNvalueWcs, XmCValueWcs, XmRValueWcs, sizeof(wchar_t*),
    XtOffsetOf(struct _XmTextRec, text.wc_value),
    XmRString, NULL
  },

  {
    XmNmaxLength, XmCMaxLength, XmRInt, sizeof(int),
    XtOffsetOf(struct _XmTextRec, text.max_length),
    XmRImmediate, (XtPointer) INT_MAX
  },

  {
    XmNmarginHeight, XmCMarginHeight, XmRVerticalDimension, sizeof(Dimension),
    XtOffsetOf(struct _XmTextRec, text.margin_height),
    XmRImmediate, (XtPointer) 5
  },

  {
    XmNmarginWidth, XmCMarginWidth, XmRHorizontalDimension, sizeof(Dimension),
    XtOffsetOf(struct _XmTextRec, text.margin_width),
    XmRImmediate, (XtPointer) 5
  },

  {
    XmNoutputCreate, XmCOutputCreate,  XmRFunction, sizeof(OutputCreateProc),
    XtOffsetOf(struct _XmTextRec, text.output_create),
    XmRFunction, (XtPointer) NULL
  },

  {
    XmNinputCreate, XmCInputCreate, XmRFunction, sizeof(InputCreateProc),
    XtOffsetOf(struct _XmTextRec, text.input_create),
    XmRFunction, (XtPointer) NULL
  },

  {
    XmNtopCharacter, XmCTopCharacter, XmRTextPosition, sizeof(XmTextPosition),
    XtOffsetOf(struct _XmTextRec, text.top_character),
    XmRImmediate, (XtPointer) 0
  },

  {
    XmNcursorPosition, XmCCursorPosition, XmRTextPosition,
    sizeof (XmTextPosition),
    XtOffsetOf(struct _XmTextRec, text.cursor_position),
    XmRImmediate, (XtPointer) 0
  },

  {
    XmNeditMode, XmCEditMode, XmREditMode, sizeof(int),
    XtOffsetOf(struct _XmTextRec, text.edit_mode),
    XmRImmediate, (XtPointer) XmSINGLE_LINE_EDIT
  },

  {
    XmNautoShowCursorPosition, XmCAutoShowCursorPosition, XmRBoolean,
    sizeof(Boolean),
    XtOffsetOf(struct _XmTextRec, text.auto_show_cursor_position),
    XmRImmediate, (XtPointer) True
  },

  {
    XmNeditable, XmCEditable, XmRBoolean, sizeof(Boolean),
    XtOffsetOf(struct _XmTextRec, text.editable),
    XmRImmediate, (XtPointer) True
  },

  {
    XmNverifyBell, XmCVerifyBell, XmRBoolean, sizeof(Boolean),
    XtOffsetOf(struct _XmTextRec, text.verify_bell),
    XmRImmediate, (XtPointer) XmDYNAMIC_BOOL
  },

    {
      XmNhighlightColorname1, XmCHighlightColorname1,  XmRString,
      sizeof(String), XtOffsetOf( struct _XmTextRec, text.highlightColorname1),
      XmRString, NULL
    },

    {
      XmNhighlightColorname2, XmCHighlightColorname2,  XmRString,
      sizeof(String), XtOffsetOf( struct _XmTextRec, text.highlightColorname2),
      XmRString, NULL
    },

  {
    XmNnavigationType, XmCNavigationType, XmRNavigationType,
    sizeof (unsigned char),
    XtOffsetOf(struct _XmPrimitiveRec, primitive.navigation_type),
    XmRImmediate, (XtPointer) XmTAB_GROUP
  },

  {
    XmNtotalLines, XmCTotalLines, XmRInt,
    sizeof (int),
    XtOffsetOf(struct _XmTextRec, text.total_lines),
    XmRImmediate, (XtPointer) 1
  },


};

/* Definition for resources that need special processing in get values */

static XmSyntheticResource get_resources[] =
{
  {
    XmNmarginWidth,
    sizeof(Dimension),
    XtOffsetOf(struct _XmTextRec, text.margin_width),
    XmeFromHorizontalPixels,
    XmeToHorizontalPixels
  },

  {
    XmNmarginHeight,
    sizeof(Dimension),
    XtOffsetOf(struct _XmTextRec, text.margin_height),
    XmeFromVerticalPixels,
    XmeToVerticalPixels
  },
};

static XmBaseClassExtRec       textBaseClassExtRec = {
  NULL,                                     /* Next extension       */
  NULLQUARK,                                /* record type XmQmotif */
  XmBaseClassExtVersion,                    /* version              */
  sizeof(XmBaseClassExtRec),                /* size                 */
  XmInheritInitializePrehook,               /* initialize prehook   */
  XmInheritSetValuesPrehook,                /* set_values prehook   */
  XmInheritInitializePosthook,              /* initialize posthook  */
  XmInheritSetValuesPosthook,               /* set_values posthook  */
  XmInheritClass,   		      	    /* secondary class      */
  XmInheritSecObjectCreate,                 /* creation proc        */
  GetSecResData,                	    /* getSecResData 	      */
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
  NULL,		      		            /* wrapperData 	      */
};

static XmPrimitiveClassExtRec _XmTextPrimClassExtRec = {
  NULL,
  NULLQUARK,
  XmPrimitiveClassExtVersion,
  sizeof(XmPrimitiveClassExtRec),
  _XmTextGetBaselines,                  /* widget_baseline */
  _XmTextGetDisplayRect,                /* widget_display_rect */
  _XmTextMarginsProc,			/* get/set widget margins */
};

externaldef(xmtextclassrec) XmTextClassRec xmTextClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	(WidgetClass) &xmPrimitiveClassRec,
    /* class_name	  */	"XmText",
    /* widget_size	  */	sizeof(XmTextRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_initiali*/	ClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	InitializeHook,
    /* realize		  */	Realize,
    /* actions		  */    NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	XtExposeCompressMaximal,
    /* compress_enterleave*/	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	Resize,
    /* expose		  */	DoExpose,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook    */	GetValuesHook,
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_private   */	NULL,
    /* tm_table		  */	NULL,
    /* query_geometry     */    QueryGeometry,
    /* display accel	  */	NULL,
    /* extension	  */	(XtPointer)&textBaseClassExtRec,
  },

/* primitive_class fields  */
  {				
    XmInheritBorderHighlight,             /* Primitive border_highlight   */
    XmInheritBorderUnhighlight,           /* Primitive border_unhighlight */
    NULL,         		          /* translations                 */
    NULL,         		          /* arm_and_activate             */
    get_resources,	    	          /* get resources 	          */
    XtNumber(get_resources),	          /* num get_resources            */
    (XtPointer) &_XmTextPrimClassExtRec,  /* extension                    */
  },

  {				/* text class fields */
    NULL,             	        /* extension         */
  }
};

externaldef(xmtextwidgetclass) WidgetClass xmTextWidgetClass =
					 (WidgetClass) &xmTextClassRec;

/****************************************************************
 *
 * Definitions for the null source.
 *
 ****************************************************************/

/* ARGSUSED */
static void 
NullAddWidget(XmTextSource source,
	      XmTextWidget tw)
{
}

/* ARGSUSED */
static void 
NullRemoveWidget(XmTextSource source,
		 XmTextWidget tw)
{
}

/* ARGSUSED */
static XmTextPosition 
NullRead(XmTextSource source,
	 XmTextPosition position,
	 XmTextPosition last_position,
	 XmTextBlock block)
{
  block->ptr = NULL;
  block->length = 0;
  block->format = XmFMT_8_BIT;
  
  return 0;
}

/* ARGSUSED */
static XmTextStatus 
NullReplace(XmTextWidget tw,
	    XEvent * event,
	    XmTextPosition *start,
	    XmTextPosition *end,
	    XmTextBlock block,
#if NeedWidePrototypes
	    int call_callbacks)
#else
            Boolean call_callbacks)
#endif
{
  return EditError;
}

/* ARGSUSED */
static XmTextPosition 
NullScan(XmTextSource source,
	 XmTextPosition position,
	 XmTextScanType sType,
	 XmTextScanDirection dir,
	 int n,
#if NeedWidePrototypes
	 int include)
#else
         Boolean include)
#endif /* NeedWidePrototypes */
{
  return 0;
}
 
/* ARGSUSED */
static Boolean 
NullGetSelection(XmTextSource source,
		 XmTextPosition *start,
		 XmTextPosition *end)
{
  return FALSE;
}

/* ARGSUSED */
static void 
NullSetSelection(XmTextSource source,
		 XmTextPosition start,
		 XmTextPosition end,
		 Time time)
{
}

static void 
_XmCreateCutBuffers(Widget w)
{
  static XContext context = (XContext)NULL;
  char * tmp = NULL;
  Display *dpy = XtDisplay(w);
  Screen *screen = XtScreen(w);
  XContext local_context;

  _XmProcessLock();
  if (context == (XContext)NULL) context = XUniqueContext();

  local_context = context;
  _XmProcessUnlock();

  if (XFindContext(dpy, (Window)screen, local_context, &tmp)) {
    XmTextContextData ctx_data;
    Widget xm_display = (Widget) XmGetXmDisplay(dpy);
    
    ctx_data = (XmTextContextData) XtMalloc(sizeof(XmTextContextDataRec));
    
    ctx_data->screen = screen;
    ctx_data->context = local_context;
    ctx_data->type = '\0';
    
    XtAddCallback(xm_display, XmNdestroyCallback,
		  (XtCallbackProc) _XmTextFreeContextData,
		  (XtPointer) ctx_data);
    
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER0,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER1,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER2,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER3,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER4,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER5,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER6,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    XChangeProperty(dpy, RootWindowOfScreen(screen), XA_CUT_BUFFER7,
		    XA_STRING, 8, PropModeAppend, NULL, 0);
    
    XSaveContext(dpy, (Window)screen, local_context, tmp);
  }
}

/****************************************************************
 *
 * Private definitions.
 *
 ****************************************************************/
/************************************************************************
 *
 *  GetSecResData
 *
 ************************************************************************/
/* ARGSUSED */
static Cardinal
GetSecResData(WidgetClass w_class,
	      XmSecondaryResourceData **secResDataRtn)
{
  XmSecondaryResourceData               *secResDataPtr;
  
  secResDataPtr = 
    (XmSecondaryResourceData *) XtMalloc(sizeof(XmSecondaryResourceData) * 2);

  _XmTextInputGetSecResData(&secResDataPtr[0]);
  _XmTextOutputGetSecResData(&secResDataPtr[1]);
  *secResDataRtn = secResDataPtr;

  return 2;
}

/*********************************************************************/
/* Trait definitions                                                 */
/*********************************************************************/

/* AccessXmString Trait record for Text */
static XmConst XmAccessTextualTraitRec TextCS = {
  0,  				/* version */
  TextGetValue,			/* getValue */
  TextSetValue,			/* setValue */
  TextPreferredValue,		/* preferredFormat */
};

/****************************************************************
 *
 * ClassPartInitialize
 *     Set up the fast subclassing for the widget. Set up merged
 *     Translation table.
 *
 ****************************************************************/
static void 
ClassPartInitialize(WidgetClass wc)
{
  XmTextWidgetClass twc = (XmTextWidgetClass) wc;
  WidgetClass super;
  XmPrimitiveClassExt *wcePtr, *scePtr;
  char * event_bindings;

  _XmProcessLock();
  super = twc->core_class.superclass;
  wcePtr = _XmGetPrimitiveClassExtPtr(wc, NULLQUARK);
  scePtr = _XmGetPrimitiveClassExtPtr(super, NULLQUARK);
  
  if ((*wcePtr)->widget_baseline == XmInheritBaselineProc)
    (*wcePtr)->widget_baseline = (*scePtr)->widget_baseline;
  
  if ((*wcePtr)->widget_display_rect == XmInheritDisplayRectProc)
    (*wcePtr)->widget_display_rect  = (*scePtr)->widget_display_rect;
  
  event_bindings = (char *)XtMalloc(strlen(_XmTextEventBindings1) +
				    strlen(_XmTextEventBindings2) +
				    strlen(_XmTextEventBindings3) + 1);
  strcpy(event_bindings, _XmTextEventBindings1);
  strcat(event_bindings, _XmTextEventBindings2);
  strcat(event_bindings, _XmTextEventBindings3);
  xmTextClassRec.core_class.tm_table = 
    (String) XtParseTranslationTable(event_bindings);
  
  XtFree(event_bindings);
  
  _XmFastSubclassInit (wc, XmTEXT_BIT);
  _XmProcessUnlock();
}

/****************************************************************
 *
 * ClassInitialize
 *   
 *
 ****************************************************************/
static void 
ClassInitialize(void)
{
  xmTextClassRec.core_class.actions =
    (XtActionList)_XmdefaultTextActionsTable;
  xmTextClassRec.core_class.num_actions = _XmdefaultTextActionsTableSize;
  
  nullsource.AddWidget = NullAddWidget;
  nullsource.RemoveWidget = NullRemoveWidget;
  nullsource.ReadSource = NullRead;
  nullsource.Replace = NullReplace;
  nullsource.Scan = NullScan;
  nullsource.GetSelection = NullGetSelection;
  nullsource.SetSelection = NullSetSelection;

  textBaseClassExtRec.record_type = XmQmotif;
  /* Install traits */
  _XmTextInstallTransferTrait();
  XmeTraitSet((XtPointer)xmTextWidgetClass, XmQTaccessTextual,
	      (XtPointer) &TextCS);
}


/*
 * Mark the given range of text to be redrawn.
 */
static void 
AddRedraw(XmTextWidget tw,
	  XmTextPosition left,
	  XmTextPosition right)
{
  RangeRec *r = tw->text.repaint.range;
  int i;
  
  if (left == tw->text.last_position &&
      tw->text.output->data->number_lines >= 1)
    left = (*tw->text.source->Scan)(tw->text.source, left,
					XmSELECT_POSITION, XmsdLeft, 1, TRUE);
  
  if (left < right) {
    for (i = 0; i < tw->text.repaint.number; i++) {
      if (left <= r[i].to && right >= r[i].from) {
	r[i].from = MIN(left, r[i].from);
	r[i].to = MAX(right, r[i].to);
	return;
      }
    }
    if (tw->text.repaint.number >= tw->text.repaint.maximum) {
      tw->text.repaint.maximum = tw->text.repaint.number + 1;
      tw->text.repaint.range = r = (RangeRec *)
	XtRealloc((char *)r, tw->text.repaint.maximum * sizeof(RangeRec));
    }
    r[tw->text.repaint.number].from = left;
    r[tw->text.repaint.number].to = right;
    tw->text.repaint.number++;
  }
}

/*
 * Find the highlight record corresponding to the given position.  Returns a
 * pointer to the record.  The third argument indicates whether we are probing
 * the left or right edge of a highlighting range.
 */
static _XmHighlightRec * 
FindHighlight(XmTextWidget tw,
	      XmTextPosition position,
	      XmTextScanDirection dir)
{
    _XmHighlightRec *l = tw->text.highlight.list;
    int i;
    if (dir == XmsdLeft) {
	for (i=tw->text.highlight.number - 1; i>=0;  i--)
	    if (position >= l[i].position) {
		l = l + i;
		break;
	    }
    } else {
	for (i=tw->text.highlight.number - 1; i>=0; i--)
	    if (position > l[i].position) {
		l = l + i;
		break;
	    }
    }
    return(l);
}

#ifdef SUN_CTL
Boolean TextW_LayoutActive(XmTextWidget tw)
{
  Output	output;
  XmRendition	rend;
  Boolean	layout_active;


  output = tw->text.output;
  rend = (output)->data->rendition;
  /* check if rendidtion is NULL fix for bug 4189738 - leob */
  if (!rend)
     return False;

  layout_active = _XmRendLayoutIsCTL(rend);
  return layout_active;
}

Boolean ISVISUAL_EDITPOLICY(XmTextWidget tw)
{
    return (TextW_LayoutActive(tw) && 
	    (tw->text.input->data->edit_policy == XmEDIT_VISUAL));
}

#define MAX_NUM_LINES 500
#define MAX_LINE_HIGHLIGHTS 500
/*
 * Redraw the changed areas of the text.  This should only be called by
 * Redisplay(), below. 
 * The CTL change is as follows: rather than redrawing repaint range by
 * repaint range, redraw every line that has one or more ranges via
 * _XmRenditionDraw(), passing in a temporary highlight list for the line.
 */
static void 
CTLRedrawChanges(XmTextWidget tw)
{
    if (tw->text.repaint.number > 0) {/*?? Do callers all warrant that repaint.number > 0? */
	/* Note (I think) that there is no guarantee that the ranges are sorted. */
	Boolean  b_cache[200];
	Boolean *repaint_line = XmStackAlloc(tw->text.number_lines * sizeof(Boolean), b_cache);
	LineNum line = 0;
	size_t i = tw->text.repaint.number;
	EraseInsertionPoint(tw);
	
	/* Initialize all line repainting to False for all lines */
	memset(repaint_line, False, sizeof(Boolean) * tw->text.number_lines);
	
	/* Run in reverse, turning on repaint for any 
	   line that contains any repaint range. */
	while (tw->text.repaint.number) {
	    size_t  from      = tw->text.repaint.range[--tw->text.repaint.number].from;
	    LineNum from_line = _XmTextPosToLine(tw, from);
	    size_t  to        = tw->text.repaint.range[tw->text.repaint.number].to;
	    LineNum to_line   = (to == PASTENDPOS || to == tw->text.last_position) ? tw->text.number_lines: _XmTextPosToLine(tw,  to); 
	    
	    if (from_line == NOLINE)
		from_line = 0;

	    if (to_line == NOLINE)
		to_line = tw->text.number_lines;
	    
	    while (from_line <= MIN(tw->text.number_lines, to_line))
		repaint_line[from_line++] = True;
	} /* End while (repaint.number) */
	
	/* Now Iterate over all of the lines, painting the ones that need it */
	while (line < tw->text.number_lines) {
	    if (repaint_line[line]) {
		/* Now make a temporary copy of the highlight records that apply
		   to this line. Note that the highlight list _is_ sorted, as 
		   contrasted with the repaint ranges list.  The only thing
		   tricky here is that we have to adjust the highlight positions
		   to make them relative to this line's start position. Note that
		   we rely on tw->text.line[tw->text.number_lines].start to be
		   accurate for the final iteration. */
		unsigned int linestart      = tw->text.line[line].start;
		unsigned int next_linestart = tw->text.line[line+1].start;
		
		if (linestart <= next_linestart) /* Otherwise problem */ {
		    _XmHighlightRec tmp_hl[MAX_LINE_HIGHLIGHTS];
		    _XmHighlightRec *p_tmp_hl = tmp_hl;
		    _XmHighlightData tmp_hl_data;
		    _XmHighlightRec *hlfrom = FindHighlight(tw, linestart, XmsdLeft);
		    _XmHighlightRec *hlto   = FindHighlight(tw, next_linestart, XmsdRight);
		    XmTextBlockRec block;
		    
		    tmp_hl_data.maximum = MAX_LINE_HIGHLIGHTS;
		    tmp_hl_data.list = tmp_hl;
		    
		    if (tw->text.input->data->edit_policy == XmEDIT_LOGICAL) {
			while (hlfrom <= hlto) {
			    int rel_pos;
			    
			    rel_pos = hlfrom->position - linestart; 
			    p_tmp_hl->position = (rel_pos < 0) ? 0 : rel_pos;
			    (p_tmp_hl++)->mode = (hlfrom++)->mode;
			}
		    } 
		    else {/* edit_policy == XmEDIT_VISUAL */
			while(hlfrom <= hlto) {
			    int rel_pos;
			    /* I don't like the idea of poking into hl_data,
			       but it seems we can't help much in this case
			       as we have to rebuild the hl records */
			    /* curr_abs_line indicates the absolute line
			       number of the line from starting. */
			    if (hlfrom->mode !=  XmHIGHLIGHT_NORMAL) {
				LineNum hlfrom_line, hlend_line, curr_abs_line;
				
				/*Note: The _XmTextPosToLine works only for
				  visible region. For invisible region
				  (scrolled region) it returns
				  NOLINE (always). So we can't use that here */
				hlfrom_line = PosToAbsLine(tw, hlfrom->position);
				hlend_line =  PosToAbsLine(tw, (hlfrom + 1)->position);
				curr_abs_line = PosToAbsLine(tw, linestart);
				
				if (hlfrom_line == hlend_line) {
				    if (hlfrom_line == curr_abs_line) {
					/* single line hl record, consume
					   the complete (i.e pair) hl record */
					
					rel_pos = hlfrom->position - linestart; 
					p_tmp_hl->position = (rel_pos < 0) ? 0 : rel_pos;
					(p_tmp_hl++)->mode = (hlfrom++)->mode;
					
					rel_pos = hlfrom->position - linestart; 
					p_tmp_hl->position = (rel_pos < 0) ? 0 : rel_pos;
					(p_tmp_hl++)->mode = hlfrom->mode;
				    }
				} 
				else {/* multi line hl record */
				    if (hlfrom_line == curr_abs_line) {
					/* first line of the hl record  */
					rel_pos = hlfrom->position - linestart; 
					p_tmp_hl->position = (rel_pos < 0) ? 0 : rel_pos;
					(p_tmp_hl++)->mode = hlfrom->mode;
					
					p_tmp_hl->position = next_linestart - linestart - 1;
					(p_tmp_hl++)->mode = XmHIGHLIGHT_NORMAL;
				    } 
				    else if (hlfrom_line < curr_abs_line && hlend_line > curr_abs_line) {
					/* middle lines of the multi line hl record */
					XmTextPosition first_visual_pos;
					
					first_visual_pos = _XmTextVisualConstScan(tw->text.source, linestart, LINE_START);
					
					rel_pos = (first_visual_pos - linestart);
					p_tmp_hl->position = (rel_pos < 0) ? 0 : rel_pos;
					(p_tmp_hl++)->mode = hlfrom->mode;
					
					p_tmp_hl->position = next_linestart - linestart-1;
					(p_tmp_hl++)->mode = XmHIGHLIGHT_NORMAL;
				    } 
				    else if (hlend_line == curr_abs_line) {
					/* last line of the hl record */
					XmTextPosition first_visual_pos;
					
					first_visual_pos = _XmTextVisualConstScan(tw->text.source, linestart, LINE_START);
					
					rel_pos = (first_visual_pos - linestart);
					p_tmp_hl->position = (rel_pos < 0) ? 0 : rel_pos;
					(p_tmp_hl++)->mode = hlfrom->mode;
					
					rel_pos = hlto->position - linestart;
					p_tmp_hl->position =  (rel_pos < 0) ? 0 : rel_pos;
					(p_tmp_hl++)->mode = XmHIGHLIGHT_NORMAL; 
					/* consume the hl record */
					hlfrom++; 
				    }
				}
			    }
			    hlfrom++;
			}
		    }
		    tmp_hl_data.number = p_tmp_hl - tmp_hl;
		    tmp_hl_data.visual = (tw->text.input->data->edit_policy == XmEDIT_VISUAL); 
		    /* Note that we have changed Draw()'s arglist to take 
		       a highlight list, rather than just one mode at a time. */
		    (void)(*tw->text.output->Draw) (tw, line, linestart, next_linestart, &tmp_hl_data);
		} else
		    XmeWarning((Widget)tw, "Text.c:CTLRedrawChanges()\n");
	    }
	    line++;
	}
	XmStackFree((char*)repaint_line, b_cache);
	TextDrawInsertionPoint(tw);
    }
}
#endif /* CTL */

/*
 * Redraw the specified range of text.  Should only be called by
 * RedrawChanges(), below (as well as calling itself recursively).
 */
static void 
DisplayText(XmTextWidget tw,
	    XmTextPosition updateFrom,
	    XmTextPosition updateTo)
{
  LineNum i;
  XmTextPosition nextstart;
  _XmHighlightRec *l1, *l2;
  
  if (updateFrom < tw->text.top_character)
    updateFrom = tw->text.top_character;
  if (updateTo > tw->text.bottom_position)
    updateTo = tw->text.bottom_position;
  if (updateFrom > updateTo) return;
  
  l1 = FindHighlight(tw, updateFrom, XmsdLeft);
  l2 = FindHighlight(tw, updateTo, XmsdRight);
  if ( (l1 != l2) && (l1->position != l2->position) ) {
    DisplayText(tw, updateFrom, l2->position);
    updateFrom = l2->position;
  }
  
  /*
   * Once we get here, we need to paint all of the text from updateFrom to
   * updateTo with current highlightmode.  We have to break this into
   * separate lines, and then call the output routine for each line.
   */
  
  for (i = _XmTextPosToLine(tw, updateFrom);
       updateFrom <= updateTo && i < tw->text.number_lines;
       i++) {
    nextstart = tw->text.line[i+1].start;
    (*tw->text.output->Draw)(tw, i, updateFrom,
				 MIN(updateTo, nextstart), l2->mode);
/*
				 MIN(updateTo, nextstart), (_XmHighlightData *)l2->mode);
*/
    updateFrom = nextstart;
  }
}

/*
 * Redraw the changed areas of the text.  This should only be called by
 * Redisplay(), below. 
 */
static void 
#ifdef SUN_CTL
NONCTLRedrawChanges(XmTextWidget tw)
#else /* CTL */
RedrawChanges(XmTextWidget tw)
#endif /* CTL */
{
  RangeRec *r = tw->text.repaint.range;
  XmTextPosition updateFrom, updateTo;
  int w, i;
  
  EraseInsertionPoint(tw);
  
  while (tw->text.repaint.number != 0) {
    updateFrom = r[0].from;
    w = 0;
    for (i=1; i<tw->text.repaint.number; i++) {
      if (r[i].from < updateFrom) {
	updateFrom = r[i].from;
	w = i;
      }
    }
    updateTo = r[w].to;
    tw->text.repaint.number--;
    r[w].from = r[tw->text.repaint.number].from;
    r[w].to = r[tw->text.repaint.number].to;
    for (i=tw->text.repaint.number-1; i>=0; i--) {
      while (i < tw->text.repaint.number) {
	updateTo = MAX(r[i].to, updateTo);
	tw->text.repaint.number--;
	r[i].from = r[tw->text.repaint.number].from;
	r[i].to = r[tw->text.repaint.number].to;
      }
    }
    DisplayText(tw, updateFrom, updateTo);
  }
  if (tw->text.first_position == tw->text.last_position) {
    (*tw->text.output->Draw)(tw, (LineNum) 0,
			     tw->text.first_position,
			     tw->text.last_position,
			     (XmHIGHLIGHT_NORMAL));
/*
			     (_XmHighlightData *) (XmHIGHLIGHT_NORMAL));
*/
  }
  TextDrawInsertionPoint(tw);
}

/* wrapper function for RedrawChanges */
#ifdef SUN_CTL
static void 
RedrawChanges(XmTextWidget tw)
{
    if (TextW_LayoutActive(tw))
	CTLRedrawChanges (tw);
    else
	NONCTLRedrawChanges(tw);
}
#endif /* CTL */
    
static void 
DoMove(XmTextWidget tw,
       int startcopy,
       int endcopy,
       int destcopy)
{
  Line line = tw->text.line;
  LineNum i;
  
  EraseInsertionPoint(tw);
  if (tw->text.disable_depth == 0 &&
      (*tw->text.output->MoveLines)(tw, (LineNum) startcopy,
					(LineNum) endcopy, (LineNum) destcopy))
    {
      TextDrawInsertionPoint(tw);
      return;
    }
  for (i=destcopy; i <= destcopy + endcopy - startcopy; i++)
    AddRedraw(tw, line[i].start, line[i+1].start);
  TextDrawInsertionPoint(tw);
}

/*
 * Find the starting position of the line that is delta lines away from the
 * line starting with position start.
 */
XmTextPosition 
_XmTextFindScroll(XmTextWidget tw,
		  XmTextPosition start,
		  int delta)
{
  register XmTextLineTable line_table;
  register unsigned int t_index;
  register unsigned int max_index = 0;
  
  line_table = tw->text.line_table;
  t_index = tw->text.table_index;
  
  max_index = tw->text.total_lines - 1;
  
  /* look forward to find the current record */
  if (line_table[t_index].start_pos < TextPosToUInt(start)) {/* Wyoming 64-bit Fix */
    while (t_index <= max_index &&
	   line_table[t_index].start_pos <  TextPosToUInt(start)) t_index++;/* Wyoming 64-bit Fix */
	   /* special handling if last lines of text are blank */
	   if ((line_table[t_index].start_pos == tw->text.last_position) &&
	  	(tw->text.number_lines == -delta) && t_index == max_index)
	  	t_index++;
  } else
    /* look backward to find the current record */
    while (t_index && 
	   line_table[t_index].start_pos > TextPosToUInt(start)) t_index--;
  
  if (delta > 0) {
    t_index += delta;
    if (t_index > tw->text.total_lines - 1)
      t_index = tw->text.total_lines - 1;
  } else {
    if (t_index > -delta)
      t_index += delta;
    else
      t_index = 0;
  }
  
  start = line_table[t_index].start_pos;
  
  tw->text.table_index = t_index;
  
  return start;
}

/* 
 * Refigure the line breaks in this widget.
 */
static void 
RefigureLines(XmTextWidget tw)
{
  Line line = tw->text.line;
  LineNum i, j;
  Line oldline = NULL;
  static XmTextPosition tell_output_force_display = -1;
  int oldNumLines = tw->text.number_lines;
  int startcopy, endcopy, destcopy, lastcopy; /* %%% Document! */
  
  if (tw->text.in_refigure_lines || !tw->text.needs_refigure_lines)
    return;
  tw->text.in_refigure_lines = TRUE;
  tw->text.needs_refigure_lines = FALSE;
  if (XtIsRealized((Widget)tw)) EraseInsertionPoint(tw);
  oldline = (Line) XtMalloc((oldNumLines + 2) * sizeof(LineRec));

  memcpy((void *) oldline, (void *) line,
	 (size_t) (oldNumLines + 1) * sizeof(LineRec));
  
  
/* Bug fix for Bug 4049264. This call to _XmTextFindScroll should 
   always happen.  */

/* Bug Id : 4139849, This is not the case scrolling should only happen for
   Text widget that have and edit mode of XmMULTILINE_EDIT */

  if (tw->text.edit_mode != XmSINGLE_LINE_EDIT)
  {
    tw->text.new_top = _XmTextFindScroll(tw, tw->text.new_top,
					     tw->text.pending_scroll);
    tw->text.pending_scroll = 0;
  }

  if (tw->text.new_top < tw->text.first_position)
    tw->text.new_top = tw->text.first_position;
  line[0].start = tw->text.top_character = tw->text.new_top;
  line[0].past_end = FALSE;
  line[0].extra = NULL;
  
  tw->text.number_lines = 0;
  j = 0;
  startcopy = endcopy = lastcopy = destcopy = -99;
  for (i = 0; i == 0 || !line[i-1].past_end; i++) {
    if (i+2 > tw->text.maximum_lines) {
      tw->text.maximum_lines = i+2;
      line = tw->text.line = (Line)
	XtRealloc((char *)line, 
		  tw->text.maximum_lines * sizeof(LineRec));
    }
    while (j < oldNumLines && oldline[j].start < line[i].start)
      j++;
    if (j < oldNumLines && oldline[j].start >= oldline[j+1].start)
      j = oldNumLines;
    if (j >= oldNumLines)
      oldline[j].start = -1; /* Make comparisons fail. */
    if (line[i].start >= tw->text.forget_past ||
	line[i].start != oldline[j].start ||
	oldline[j].changed ||
	oldline[j+1].changed) {
      line[i].past_end =
	!(*tw->text.output->MeasureLine)(tw, i, line[i].start,
					     &line[i+1].start, &line[i].extra);
      line[i+1].extra = NULL;
      if (!line[i].past_end && 
	  (line[i+1].start == PASTENDPOS) &&
	  (line[i].start != PASTENDPOS))
	AddRedraw(tw, line[i].start, tw->text.last_position);
    } else {
      line[i] = oldline[j];
      oldline[j].extra = NULL;
      line[i].past_end =
	!(*tw->text.output->MeasureLine)(tw, i, line[i].start,
					     NULL, NULL);
      
      line[i+1].start = oldline[j+1].start;
      line[i+1].extra = oldline[j+1].extra;
    }
    if (!line[i].past_end) {
      if (line[i].start != oldline[j].start ||
	  line[i+1].start != oldline[j+1].start ||
	  line[i].start >= tw->text.forget_past) {
	AddRedraw(tw, line[i].start, line[i+1].start);
      } else {
	if (i != j && line[i+1].start >= tw->text.last_position)
	  AddRedraw(tw, tw->text.last_position,
		    tw->text.last_position);
	if (oldline[j].changed)
	  AddRedraw(tw, oldline[j].changed_position,
		    line[i+1].start);
	if (i != j && line[i].start != PASTENDPOS) {
	  if (endcopy == j-1) {
	    endcopy = j;
	    lastcopy++;
	  } else if (lastcopy >= 0 && j <= lastcopy) {
	    /* This line was stomped by a previous move. */
	    AddRedraw(tw, line[i].start, line[i+1].start);
	  } else {
	    if (startcopy >= 0) 
	      DoMove(tw, startcopy, endcopy, destcopy);
	    startcopy = endcopy = j;
	    destcopy = lastcopy = i;
	  }
	}
      }
    }
    line[i].changed = FALSE;
    if (!line[i].past_end) tw->text.number_lines++;
    else tw->text.bottom_position =
      MIN(line[i].start, tw->text.last_position);
  }
  if (startcopy >= 0) {
    DoMove(tw, startcopy, endcopy, destcopy);
  }
  for (j=0; j<=oldNumLines; j++)
    if (oldline[j].extra) {
      XtFree((char *) oldline[j].extra);
      oldline[j].extra = NULL;
    }

  XtFree((char *)oldline); /* XTHREADS */
  tw->text.in_refigure_lines = FALSE;
  if (tw->text.top_character >= tw->text.last_position &&
      tw->text.last_position > tw->text.first_position &&
      tw->text.output->data->number_lines > 1) {
    tw->text.pending_scroll = -1; /* Try to not ever display nothing. */
    tw->text.needs_refigure_lines = TRUE;
  }
  if (tw->text.force_display >= 0) {
    if (tw->text.force_display < tw->text.top_character) {
      tw->text.new_top = (*tw->text.source->Scan)
	(tw->text.source,
	 tw->text.force_display,
	 XmSELECT_LINE, XmsdLeft, 1, FALSE);
      tw->text.needs_refigure_lines = TRUE;
    } else if (tw->text.force_display > tw->text.bottom_position) {
      /* need to add one to account for border condition,
       * i.e. cursor at begginning of line
       */
      if (tw->text.force_display < tw->text.last_position)
	tw->text.new_top = tw->text.force_display + 1;
      else
	tw->text.new_top = tw->text.last_position;
      tw->text.needs_refigure_lines = TRUE;
      tw->text.pending_scroll -= tw->text.number_lines;
    } else if (tw->text.force_display ==
	       line[tw->text.number_lines].start) {
      tw->text.new_top = tw->text.force_display;
      tw->text.pending_scroll -= (tw->text.number_lines - 1);
      tw->text.needs_refigure_lines = TRUE;
    }
    _XmProcessLock();
    tell_output_force_display = tw->text.force_display;
    _XmProcessUnlock();
    tw->text.force_display = -1;
  }
  if (tw->text.needs_refigure_lines) {
    RefigureLines(tw);
    if (XtIsRealized((Widget)tw)) TextDrawInsertionPoint(tw);
    return;
  }
  AddRedraw(tw, tw->text.forget_past, tw->text.bottom_position);
  tw->text.forget_past = LONG_MAX;
  _XmProcessLock();
  if (tell_output_force_display >= 0) {
    (*tw->text.output->MakePositionVisible)(tw,
					tell_output_force_display);
    tell_output_force_display = -1;
  }
  _XmProcessUnlock();
  if (XtIsRealized((Widget)tw)) TextDrawInsertionPoint(tw);
}

/************************************************************************
 *
 * RemoveLines() - removes the lines from the global line table.
 *      widget - the widget that contains the global table.
 *      num_lines - number of lines to be removed.
 *      cur_line - pointer to the start of the lines to be removed.
 *
 ************************************************************************/
/* ARGSUSED */
static void
RemoveLines(XmTextWidget tw,
	    int num_lines,
	    unsigned int cur_index)
{
  if (!num_lines) return;
  
  /* move the existing lines at the end of the buffer */
  if (tw->text.total_lines > cur_index)
    memmove((void *) &tw->text.line_table[cur_index - num_lines], 
	    (void *) &tw->text.line_table[cur_index],
	    (size_t) ((tw->text.total_lines - (cur_index)) *
		      sizeof (XmTextLineTableRec)));
  
  /* reduce total line count */
  tw->text.total_lines -= num_lines;
  
  /* fix for bug 5166 */
  if (tw->text.total_lines <= tw->text.table_index)
    tw->text.table_index = tw->text.total_lines - 1;
  
  
  /* Shrink Table if Necessary */
  if ((tw->text.table_size > TABLE_INCREMENT &&
       tw->text.total_lines <= tw->text.table_size-TABLE_INCREMENT) ||
      tw->text.total_lines <= tw->text.table_size >> 1) {
    
    tw->text.table_size = INIT_TABLE_SIZE;
    
    while (tw->text.total_lines >= tw->text.table_size) {
      if (tw->text.table_size < TABLE_INCREMENT)
	tw->text.table_size *= 2;
      else
	tw->text.table_size += TABLE_INCREMENT;
    }
    
    tw->text.line_table = (XmTextLineTable)
      XtRealloc((char *) tw->text.line_table,
		tw->text.table_size * sizeof(XmTextLineTableRec));
  }
}

static void
AddLines(XmTextWidget tw,
	 XmTextLineTable temp_table,
	 unsigned int tmp_index,
	 unsigned int current_index)
{
  register unsigned int i;
  register unsigned int size_needed;
  register unsigned int cur_index;
  register unsigned int temp_index;

  cur_index = current_index;
  temp_index = tmp_index;
  size_needed = tw->text.total_lines + temp_index;
  
  /* make sure table is big enough to handle the additional lines */
  if (tw->text.table_size < size_needed) {
    while (tw->text.table_size < size_needed)
      if (tw->text.table_size < TABLE_INCREMENT)
	tw->text.table_size *= 2;
      else
	tw->text.table_size += TABLE_INCREMENT;
    tw->text.line_table = (XmTextLineTable)
      XtRealloc((char *) tw->text.line_table,
		tw->text.table_size * sizeof(XmTextLineTableRec));
  }
  
  /* move the existing lines at the end of the buffer */
  if (tw->text.total_lines > cur_index)
    memmove((void *) &tw->text.line_table[cur_index + temp_index], 
	    (void *) &tw->text.line_table[cur_index],
	    (size_t) ((tw->text.total_lines - cur_index) *
		      sizeof (XmTextLineTableRec)));
  
  tw->text.total_lines += temp_index;
  
  /* Add the lines from the temp table */
  if (temp_table)
    for (i = 0; i < temp_index; i++, cur_index++)
      tw->text.line_table[cur_index] = temp_table[i];
}

void 
_XmTextRealignLineTable(XmTextWidget tw,
			XmTextLineTable *temp_table,
			int *temp_table_size,
			register unsigned int cur_index,
			register XmTextPosition cur_start,
			register XmTextPosition cur_end)

{
  register int table_size;
  register XmTextPosition line_end;
  register XmTextPosition next_start;
  XmTextLineTable line_table;
  
  if (temp_table) {
    line_table = *temp_table;
    table_size = *temp_table_size;
  } else {
    line_table = tw->text.line_table;
    table_size = tw->text.table_size;
  }
  
  line_table[cur_index].start_pos =  next_start =  TextPosToUInt(cur_start); /* Wyoming 64-bit Fix */
  cur_index++;
  
  line_end = (*tw->text.source->Scan)(tw->text.source, cur_start,
					  XmSELECT_LINE, XmsdRight, 1, TRUE);
  while (next_start < cur_end) {
    if (_XmTextShouldWordWrap(tw))
      next_start = _XmTextFindLineEnd(tw, cur_start, NULL);
    else {
      if (cur_start != line_end)
	next_start = line_end;
      else
	next_start = PASTENDPOS;
    }
    if (next_start == PASTENDPOS || next_start == cur_end) break;
    if (next_start == cur_start)
      next_start = (*tw->text.source->Scan) (tw->text.source,
						 cur_start, XmSELECT_POSITION,
						 XmsdRight, 1, TRUE);
    if (cur_index >= table_size) {
      if (table_size < TABLE_INCREMENT)
	table_size *= 2;
      else
	table_size += TABLE_INCREMENT;
      
      line_table = (XmTextLineTable) XtRealloc((char *)line_table,
					       table_size *
					       sizeof(XmTextLineTableRec));
    }
    line_table[cur_index].start_pos = TextPosToUInt(next_start); /* Wyoming 64-bit Fix */
    if (line_end == next_start) {
      line_table[cur_index].virt_line = 0;
      line_end = (*tw->text.source->Scan)(tw->text.source,
					      next_start, XmSELECT_LINE,
					      XmsdRight, 1, TRUE);
    } else
      line_table[cur_index].virt_line = 1;
    cur_index++;
    cur_start = next_start;
  }
  
  if (temp_table) {
    *temp_table = line_table;
    *temp_table_size = cur_index;
  } else {
    tw->text.total_lines = cur_index;
    tw->text.line_table = line_table;
    tw->text.table_size = table_size;
  }
}

static void
InitializeLineTable(XmTextWidget tw,
		    register int size)
{
  register unsigned int t_index;
  register XmTextLineTable line_table;
  
  line_table = (XmTextLineTable) XtMalloc(size * sizeof(XmTextLineTableRec));
  
  for (t_index = 0; t_index < size; t_index++) {
    line_table[t_index].start_pos = 0;
    line_table[t_index].virt_line = 0;
  }
  
  tw->text.line_table = line_table;
  tw->text.table_index = 0;
  tw->text.table_size = size;
}

unsigned int
_XmTextGetTableIndex(XmTextWidget tw,
		     XmTextPosition pos)
{
  register XmTextLineTable line_table;
  register unsigned int cur_index;
  register unsigned int max_index;
  register XmTextPosition position;
  
  position = pos;
  max_index = tw->text.total_lines - 1;
  line_table = tw->text.line_table;
  cur_index = tw->text.table_index;
  
  /* look forward to find the current record */
  if (line_table[cur_index].start_pos < TextPosToUInt(position)) { /* Wyoming 64-bit Fix */
    while (cur_index < max_index &&
	   line_table[cur_index].start_pos < TextPosToUInt(position)) /* Wyoming 64-bit Fix */
      cur_index++;
    /* if over shot it by one */
    if (TextPosToUInt(position) < line_table[cur_index].start_pos) cur_index--; /* Wyoming 64-bit Fix */
  } else
    /* look backward to find the current record */
    while (cur_index &&
	   line_table[cur_index].start_pos > TextPosToUInt(position)) /* Wyoming 64-bit Fix */
      cur_index--;
  
  return (cur_index);
}



void 
_XmTextUpdateLineTable(Widget widget,
		       XmTextPosition start,
		       XmTextPosition end,
		       XmTextBlock block,
#if NeedWidePrototypes
		       int update)
#else
                       Boolean update)
#endif /* NeedWidePrototypes */
{
  register unsigned int cur_index;
  register unsigned int begin_index;
  register unsigned int end_index;
  register XmTextLineTable line_table;
  register unsigned int max_index;
  register int lines_avail;
  register int length;
  register long delta;
  unsigned int start_index;
  unsigned int top_index;
  XmTextWidget tw = (XmTextWidget) widget;
  Boolean word_wrap = _XmTextShouldWordWrap(tw);
  XmTextPosition cur_start, cur_end;
  int diff = 0;
  int block_num_chars = 0;
  int char_size = 0;
  
  lines_avail = 0;
  max_index = tw->text.total_lines - 1;
  if (tw->text.char_size != 1) 
	              /* Bug Id : 1217687/4128045/4154215 */
    block_num_chars = TextCountCharacters(widget, block->ptr, block->length);
  else
    block_num_chars = block->length;
  delta = block_num_chars - (end - start);
  length = block_num_chars;
  
  if (tw->text.line_table == NULL)
    if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
      InitializeLineTable(tw, 1);
    else
      InitializeLineTable(tw, INIT_TABLE_SIZE);
  
  /* if there is no change or we expect RelignLineTable()
     to be called before the line table is necessary */
  if ((start == end && length == 0) || 
      (word_wrap && !XtIsRealized(widget)
       && XmIsScrolledWindow(XtParent(widget))
       && XtIsShell(XtParent(XtParent(widget))))) {
    return;
  }
  
  line_table = tw->text.line_table;
  
  cur_index = _XmTextGetTableIndex(tw, start);
  top_index = _XmTextGetTableIndex(tw, tw->text.top_character);
  
  begin_index = start_index = end_index = cur_index;
  
  if (word_wrap && delta > 0)
    cur_end = end + delta;
  else
    cur_end = end;
  
  /* Find the cur_end position.
     Count the number of lines that were deleted. */
  if (end > start) {
    if (end_index < tw->text.total_lines) {
      while (end_index < max_index &&
	     line_table[end_index + 1].start_pos <= TextPosToUInt(cur_end)) { /* Wyoming 64-bit Fix */
	end_index++;
	lines_avail++;
      }
    } else if (line_table[end_index].start_pos > TextPosToUInt(start) && /* Wyoming 64-bit Fix */
	       line_table[end_index].start_pos <=  TextPosToUInt(cur_end)) {/* Wyoming 64-bit Fix */
      lines_avail++;
    }
  }
  
  cur_index = end_index;
  
  if (word_wrap) {
    register int i;
    XmTextLineTable temp_table = NULL;
    int temp_table_size = 0;
    
    if (line_table[start_index].virt_line) start_index--;

    begin_index = start_index;
    
    /* get the start position of the line at the start index. */
    cur_start = line_table[begin_index].start_pos;
    
    /* If we are not at the end of the table, */
    if (cur_index < max_index) {

      /* find the next non-wordwrapped line. */
      while (cur_index < max_index) {
	cur_index++;
	if (!line_table[cur_index].virt_line) break;
      }

      /* Continue only if we have found a non-wordwrapped line. */
      if (!line_table[cur_index].virt_line) {

	  /* Set the cur_end position to the position of
	     the next non-wordwrapped line. */
	  cur_end = line_table[cur_index].start_pos;
	  /* estimate the temp table size - in number of lines */
	  temp_table_size = cur_index - begin_index;
	  /* make sure the size is not zero */
	  if (!temp_table_size) temp_table_size++;
	  /* do initial allocation of the temp_table */
	  temp_table = (XmTextLineTable) XtMalloc(temp_table_size *
						  sizeof(XmTextLineTableRec)); 
	  /* Determine the lines that have changed. */
	  _XmTextRealignLineTable(tw, &temp_table, &temp_table_size,
				  0, cur_start, cur_end + delta);
      
	  /* Compute the difference in the number of lines that have changed */
	  diff = temp_table_size - (cur_index - begin_index);
	  
	  /* if new/wrapped lines were added, push line down*/
	  if (diff > 0)
	      AddLines(tw, NULL, diff, cur_index);
	  /* if new/wrapped lines were deleted, move line up */
	  else
	      RemoveLines(tw, -diff, cur_index);
	  
	  /*
	   * The line table may have been realloc'd in any of the three
	   * previous function calls, so it must be reassigned to prevent
	   * a stale pointer.
	   */
	  line_table = tw->text.line_table;
	  
	  /* Bypass the first entry in the temp_table */
	  begin_index++;
	  
	  /* Add the lines from the temp table */
	  for (i = 1; i < temp_table_size; i++, begin_index++)
	      line_table[begin_index] = temp_table[i];
	  
	  /* Free temp table */
	  XtFree((char *)temp_table);
      
	  /* Adjust the cur_index by the number of lines that changed. */
	  cur_index += diff;
	  max_index += diff;
      
	  /* Adjust start values in table by the amount of change */
	  while (cur_index <= max_index) {
	      line_table[cur_index].start_pos += delta;
	      cur_index++;
	  }
      } else
	  /* we are at the end of the table */
	  _XmTextRealignLineTable(tw, NULL, 0, begin_index,
				  cur_start, PASTENDPOS);
	
    } else
      /* add lines to the end */
      _XmTextRealignLineTable(tw, NULL, 0, begin_index,
			      cur_start, PASTENDPOS);
  } else {
    register char *ptr;
    register XmTextLineTable temp_table;
    register int temp_table_size;
    register int temp_index;
    
    temp_table = NULL;
    temp_table_size = 0;
    temp_index = 0;
    ptr = block->ptr;
    cur_start = start;
    
    while (cur_index < max_index) {
      cur_index++;
      line_table[cur_index].start_pos += delta;
    }
    
    if (tw->text.char_size == 1) {
      char *nl;
      while (length > 0 && (nl = (char *)memchr(ptr, '\012', length)) != NULL) {
	nl++;
	cur_start += (nl - ptr);
	length -= (nl - ptr);
	ptr = nl;
	if (lines_avail && begin_index < tw->text.total_lines) {
	  begin_index++;
	  lines_avail--;
	  line_table[begin_index].start_pos = TextPosToUInt(cur_start); /* Wyoming 64-bit Fix */
	} else {
	  if (temp_index >= temp_table_size) {
	    if (!temp_table_size) {
	      if (tw->text.output->data->columns > 1) {
		temp_table_size = length / 
		  (tw->text.output->data->columns / 2);
		if (!temp_table_size) temp_table_size = 1;
	      } else {
		if (length)
		  temp_table_size = length;
		else
		  temp_table_size = 1;
	      }
	    } else
	      temp_table_size *= 2;
	    temp_table = (XmTextLineTable)XtRealloc((char*)temp_table,
				  temp_table_size * sizeof(XmTextLineTableRec));
	  }
	  temp_table[temp_index].start_pos = TextPosToUInt(cur_start);/* Wyoming 64-bit Fix */
	  temp_table[temp_index].virt_line = (unsigned int) 0;
	  temp_index++;
	}
      }
    } else {
      while (length--) {
	char_size = mblen(ptr, tw->text.char_size);
	if (char_size < 0) char_size = 1; /* error */
	cur_start++;
	if (char_size == 1 && *ptr == '\012') {
	  ptr++;
	  if (lines_avail && begin_index < tw->text.total_lines) {
	    begin_index++;
	    lines_avail--;
	    line_table[begin_index].start_pos = TextPosToUInt(cur_start);/* Wyoming 64-bit Fix */
	  } else {
	    if (temp_index >= temp_table_size) {
	      if (!temp_table_size) {
		if (tw->text.output->data->columns > 1) {
		  temp_table_size = length /
		    (tw->text.output->data->columns / 2);
		  if (!temp_table_size) temp_table_size = 1;
		} else {
		  if (length)
		    temp_table_size = length;
		  else
		    temp_table_size = 1;
		}
	      } else
		temp_table_size *= 2;
	      temp_table =(XmTextLineTable)XtRealloc((char*)temp_table,
                               temp_table_size * sizeof(XmTextLineTableRec));
	    }
	    temp_table[temp_index].start_pos = TextPosToUInt(cur_start);/* Wyoming 64-bit Fix */
	    temp_table[temp_index].virt_line = (unsigned int) 0;
	    temp_index++;
	  }
	} else {
	  ptr += char_size;
	}
      }
    }
    
    /* add a block of lines to the line table */
    if (temp_index) {
      AddLines(tw, temp_table, temp_index, begin_index + 1);
    }
    
    /* remove lines that are no longer necessary */
    if (lines_avail) {
      RemoveLines(tw, lines_avail, end_index + 1);
    }

    /*
     * The line table may have been realloc'd in any of the three
     * previous function calls, so it must be reassigned to prevent
     * a stale pointer.
     */
    line_table = tw->text.line_table;

    diff = temp_index - lines_avail;
    
    if (temp_table) XtFree((char *)temp_table);
  }
  
  if (update) {
    if (start < tw->text.top_character) {
      if (end < tw->text.top_character) {
	tw->text.top_line += diff;
	tw->text.new_top = tw->text.top_character + delta;
      } else {
	int adjusted;
	if (diff < 0)
	  adjusted = diff + (top_index - start_index);
	else
	  adjusted = diff - (top_index - start_index);
	tw->text.top_line += adjusted;
	if (adjusted + (int) start_index <= 0) {
	  tw->text.new_top = 0;
	} else if (adjusted + start_index > max_index) {
	  tw->text.new_top = line_table[max_index].start_pos;
	} else {
	  tw->text.new_top = line_table[start_index + adjusted].start_pos;
	}
      }
      tw->text.top_character = tw->text.new_top;
      tw->text.forget_past = MIN(tw->text.forget_past, tw->text.new_top);
      
      tw->text.top_line = _XmTextGetTableIndex(tw, tw->text.new_top);
      
      if (tw->text.top_line < 0) 
	tw->text.top_line = 0;
      
      if (tw->text.top_line > tw->text.total_lines)
	tw->text.top_line = tw->text.total_lines - 1;
    }
    
    if (tw->text.table_index > tw->text.total_lines)
      tw->text.table_index = tw->text.total_lines;
    
    if (start < tw->text.cursor_position && tw->text.on_or_off == on) {
      XmTextPosition cursorPos = tw->text.cursor_position;
      if (tw->text.cursor_position < end) {
	if (tw->text.cursor_position - start <= block_num_chars)
	  cursorPos = tw->text.cursor_position;
	else
	  cursorPos = start + block_num_chars;
      } else {
	cursorPos = tw->text.cursor_position - (end - start) +
	  block_num_chars;
      }
      _XmTextSetCursorPosition(widget, cursorPos);
    }
  }
}


/*
 * Compare the old_highlight list and the highlight list, determine what
 * changed, and call AddRedraw with the changed areas.
 */
static void 
FindHighlightingChanges(XmTextWidget tw)
{
  int n1 = tw->text.old_highlight.number;
  int n2 = tw->text.highlight.number;
  _XmHighlightRec *l1 = tw->text.old_highlight.list;
  _XmHighlightRec *l2 = tw->text.highlight.list;
  int i1, i2;
  XmTextPosition next1, next2, last_position;
  
  i1 = i2 = 0;
  last_position = 0;
  while (i1 < n1 && i2 < n2) {
    if (i1 < n1-1) next1 = l1[i1+1].position;
    else next1 = tw->text.last_position;
    if (i2 < n2-1) next2 = l2[i2+1].position;
    else next2 = tw->text.last_position;
    if (l1[i1].mode != l2[i2].mode) {
      AddRedraw(tw, last_position, MIN(next1, next2));
    }
    last_position = MIN(next1, next2);
    if (next1 <= next2) i1++;
    if (next1 >= next2) i2++;
  }
}

/*
 * Actually do some work.  This routine gets called to actually paint all the
 * stuff that has been pending. Prevent recursive calls and text redisplays
 * during destroys
 */
static void 
Redisplay(XmTextWidget tw)
{
   /* Prevent recursive calls or text redisplay during detroys. */
  if (tw->text.in_redisplay || tw->core.being_destroyed ||
      tw->text.disable_depth != 0 || !XtIsRealized((Widget)tw)) return;
  
  EraseInsertionPoint(tw);
  
  tw->text.in_redisplay = TRUE;
  
  if (tw->text.needs_refigure_lines) RefigureLines(tw);
  tw->text.needs_redisplay = FALSE;
  
  if (tw->text.highlight_changed) {
    FindHighlightingChanges(tw);
    tw->text.highlight_changed = FALSE;
  }
  
  RedrawChanges(tw);
  
  /* Can be caused by auto-horiz scrolling... */
  if (tw->text.needs_redisplay) {
    RedrawChanges(tw);
    tw->text.needs_redisplay = FALSE;
  }
  tw->text.in_redisplay = FALSE;
  
  TextDrawInsertionPoint(tw);
}



/****************************************************************
 *
 * Definitions exported to output.
 *
 ****************************************************************/

/*
 * Mark the given range of text to be redrawn.
 */

void 
_XmTextMarkRedraw(XmTextWidget tw,
		  XmTextPosition left,
		  XmTextPosition right)
{
  if (left < right) {
    AddRedraw(tw, left, right);
    tw->text.needs_redisplay = TRUE;
    if (tw->text.disable_depth == 0) Redisplay(tw);
  }
}


/*
 * Return the number of lines in the linetable.
 */
LineNum 
_XmTextNumLines(XmTextWidget tw)
{
  if (tw->text.needs_refigure_lines) RefigureLines(tw);
  return tw->text.number_lines;
}

void 
_XmTextLineInfo(XmTextWidget tw,
		LineNum line,
		XmTextPosition *startpos,
		LineTableExtra *extra)
{
  if (tw->text.needs_refigure_lines) RefigureLines(tw);
  if (tw->text.number_lines >= line) {
    if (startpos) *startpos = tw->text.line[line].start;
    if (extra) *extra = tw->text.line[line].extra;
  } else {
    if (startpos) {
      unsigned int cur_index = 
	_XmTextGetTableIndex(tw, tw->text.line[line - 1].start);
      if (cur_index < tw->text.total_lines - 1)
	*startpos = tw->text.line_table[cur_index + 1].start_pos;
      else
	*startpos = tw->text.last_position;
    }
    if (extra) *extra = NULL;
  }
}

/*
 * Return the line number containing the given position.  If text currently
 * knows of no line containing that position, returns NOLINE.
 */
LineNum 
_XmTextPosToLine(XmTextWidget tw,
		 XmTextPosition position)
{
  int i;
  if (tw->text.needs_refigure_lines) RefigureLines(tw);
  if (position < tw->text.top_character ||
      position  > tw->text.bottom_position)
    return NOLINE;
  for (i=0; i<tw->text.number_lines; i++)
    if (tw->text.line[i+1].start > position) return i;
  if (position == tw->text.line[tw->text.number_lines].start)
    return tw->text.number_lines;
  return NOLINE;  /* Couldn't find line with given position */ 
}



/****************************************************************
 *
 * Definitions exported to sources.
 *
 ****************************************************************/
void 
_XmTextInvalidate(XmTextWidget tw,
		  XmTextPosition position,
		  XmTextPosition topos,
		  long delta)
{
  LineNum l;
  int i;
  XmTextPosition p, endpos;
  int shift = 0;
  int shift_start = 0;
  
#define ladjust(p) \
  if ((p > position && p != PASTENDPOS) ||	                            \
      (p == position && delta < 0)) {		                            \
    p += delta;					                            \
    if (p < tw->text.first_position) p = tw->text.first_position;   \
    if (p > tw->text.last_position) p = tw->text.last_position;     \
  }

#define radjust(p) \
  if ((p > position && p != PASTENDPOS) ||		                    \
      (p == position && delta > 0)) {			                    \
    p += delta;					       		            \
    if (p < tw->text.first_position) p = tw->text.first_position;   \
    if (p > tw->text.last_position) p = tw->text.last_position;     \
  }

  tw->text.first_position = 
    (*tw->text.source->Scan)(tw->text.source, 0,
				 XmSELECT_ALL, XmsdLeft, 1, FALSE);
    tw->text.last_position = 
      (*tw->text.source->Scan)(tw->text.source,  0,
				   XmSELECT_ALL, XmsdRight, 1, FALSE);
  if (delta == NODELTA) {
    if (tw->text.top_character == topos && position != topos) {
      tw->text.pending_scroll = -1;
      tw->text.forget_past = MIN(tw->text.forget_past, position);
    }
    if (tw->text.top_character > position &&
	tw->text.bottom_position < topos) {
      tw->text.new_top = position;
      tw->text.pending_scroll = -1;
      tw->text.forget_past = MIN(tw->text.forget_past, position);
    }
    
    if (tw->text.in_resize && tw->text.line_table != NULL) {
      unsigned int top_index, last_index, next_index;
      int index_offset, lines_used;
      
      top_index = tw->text.top_line;
      last_index = _XmTextGetTableIndex(tw, tw->text.last_position);
      
      lines_used = (last_index - top_index) + 1;
      
      if (top_index != 0 &&
	  tw->text.output->data->number_lines > lines_used) {
	index_offset = tw->text.output->data->number_lines-lines_used;
	if (index_offset < tw->text.total_lines - lines_used)
	  next_index = top_index - index_offset;
	else
	  next_index = 0;
	tw->text.new_top = tw->text.top_character =
	  tw->text.line_table[next_index].start_pos;
      }
    }
    
    tw->text.forget_past = MIN(tw->text.forget_past, position);
  } else {
    for (i=0; i<tw->text.repaint.number; i++) {
      radjust(tw->text.repaint.range[i].from);
      ladjust(tw->text.repaint.range[i].to);
    }
    for (i=1; i < tw->text.highlight.number; i++) {
      if (delta < 0 &&
	  tw->text.highlight.list[i].position >= position - delta)
	ladjust(tw->text.highlight.list[i].position);
      if (delta > 0 &&
	  ((tw->text.highlight.list[i].position > position) ||
	   ((tw->text.highlight.list[i].position == position) &&
	    (tw->text.highlight.list[i].mode != XmHIGHLIGHT_NORMAL))))
	radjust(tw->text.highlight.list[i].position);
    }
    for (i=1; i<tw->text.old_highlight.number; i++) {
      if (delta < 0 &&
	  tw->text.old_highlight.list[i].position >= position - delta)
	ladjust(tw->text.old_highlight.list[i].position);
      if (delta > 0 &&
	  ((tw->text.old_highlight.list[i].position > position) ||
	   ((tw->text.old_highlight.list[i].position == position) &&
	    (tw->text.old_highlight.list[i].mode != XmHIGHLIGHT_NORMAL))))
	radjust(tw->text.old_highlight.list[i].position);
    }
    for (i=0; i <= tw->text.number_lines && 
	 tw->text.line[i].start != PASTENDPOS; i++) {
      if (delta > 0) {
	radjust(tw->text.line[i].start);
      } else {
	if (tw->text.line[i].start > position &&
	    tw->text.line[i].start <= topos) {
	  if (i != 0 && shift_start == 0)
	    shift_start = i;
	  shift++;
	} else {
	  radjust(tw->text.line[i].start);
	}
      }
      if (tw->text.line[i].changed) {
	radjust(tw->text.line[i].changed_position);
      }
    }
    if (shift) {
      for (i=shift_start; i < tw->text.number_lines; i++) {
	if ((i < (shift_start + shift)) && tw->text.line[i].extra)
	  XtFree((char *)tw->text.line[i].extra);
	if (i + shift < tw->text.number_lines) {
	  tw->text.line[i].start = tw->text.line[i+shift].start;
	  tw->text.line[i].extra = tw->text.line[i+shift].extra;
	} else {
	  tw->text.line[i].start = PASTENDPOS;
	  tw->text.line[i].extra = NULL;
	}
	tw->text.line[i].changed = TRUE;
	if (tw->text.line[i].start != PASTENDPOS)
	  tw->text.line[i].changed_position = 
	    tw->text.line[i + 1].start - 1;
	else
	  tw->text.line[i].changed_position = PASTENDPOS;
      }
    }
    ladjust(tw->text.bottom_position);
    tw->text.output->data->refresh_ibeam_off = True;
    endpos = topos;
    radjust(endpos);
    
    /* Force _XmTextPosToLine to not bother trying to recalculate. */
    tw->text.needs_refigure_lines = FALSE;
    for (l = _XmTextPosToLine(tw, position), p = position;
	 l < tw->text.number_lines &&
	 tw->text.line[l].start <= endpos;
	 l++, p = tw->text.line[l].start) {
      if (l != NOLINE) {
	if (tw->text.line[l].changed) {
	  tw->text.line[l].changed_position = 
	    MIN(p, tw->text.line[l].changed_position);
	} else {
	  tw->text.line[l].changed_position = p;
	  tw->text.line[l].changed = TRUE;
	}
      }
    }
  }
  (*tw->text.output->Invalidate)(tw, position, topos, delta);
  (*tw->text.input->Invalidate)(tw, position, topos, delta);
  tw->text.needs_refigure_lines = tw->text.needs_redisplay = TRUE;
  if (tw->text.disable_depth == 0) Redisplay(tw);
}

static void 
InsertHighlight(XmTextWidget tw,
		XmTextPosition position,
		XmTextPosition end,
		XmHighlightMode mode)
{
  _XmHighlightRec *l1;
  _XmHighlightRec *l = tw->text.highlight.list;
  long i, j;/* Wyoming 64-bit Fix */
  int upos, old_listnum;
  UrlHighlightRec *url_l = tw->text.url_highlight.list;
  
  l1 = FindHighlight(tw, position, XmsdLeft);
  if (l1->position == position)
    l1->mode = mode;
  else {
    i = (l1 - l) + 1;
    tw->text.highlight.number++;
    if (tw->text.highlight.number > tw->text.highlight.maximum) {
      tw->text.highlight.maximum = tw->text.highlight.number;
      l = tw->text.highlight.list = (_XmHighlightRec *)
	XtRealloc((char *) l, tw->text.highlight.maximum *
		  sizeof(_XmHighlightRec));
    }
    for (j=tw->text.highlight.number-1; j>i; j--)
      l[j] = l[j-1];
    l[i].position = position;
    l[i].mode = mode;
  }

#ifdef ENABLE_URLS
    if (end < 0)
        return;
    if (mode == XmHIGHLIGHT_COLOR_1) {
        old_listnum = tw->text.url_highlight.number;
        tw->text.url_highlight.number++;
        if (tw->text.url_highlight.number
               > tw->text.url_highlight.maximum) {
            tw->text.url_highlight.maximum += 10;
            url_l = tw->text.url_highlight.list = (UrlHighlightRec *)
              XtRealloc((char *) url_l, tw->text.url_highlight.maximum
                                     * sizeof(UrlHighlightRec));
        }
        if (old_listnum == 0)
            upos = 0;
        else if (position > url_l[old_listnum - 1].position)
            upos = old_listnum;
        else {
              /* Find url list position of new URL */
            for (upos = old_listnum - 1; upos >= 0; upos--) {
                if (position > url_l[upos].position) {
                    upos++;
                    break;
                }
            }
            if (upos == -1)
                upos = 0;
            for (i = old_listnum; i > upos; i--)
                url_l[i] = url_l[i-1];
        }
        url_l[upos].position = position;
        url_l[upos].end = end;
        url_l[upos].mode = mode;
    }
    else if (mode == XmHIGHLIGHT_COLOR_2) {
        for (i = tw->text.url_highlight.number - 1 ; i >= 0 ; i--)
           if (position == url_l[i].position)
              break;
        url_l[i].mode = mode;
    }
#endif
}

/****************************************************************
 *
 * Creation definitions.
 *
 ****************************************************************/
/*
 * Create the text widget.  To handle default condition of the core
 * height and width after primitive has already reset it's height and
 * width, use request values and reset height and width to original
 * height and width state.
 */
/* ARGSUSED */
static void 
Initialize(Widget rw,
	   Widget nw,
	   ArgList args,
	   Cardinal *num_args)
{
  XmTextWidget req = (XmTextWidget) rw;
  XmTextWidget newtw = (XmTextWidget) nw;
/* Solaris 2.6 Motif diff bug #4085003 1 line */
  static XtTranslations btn1_xlations, btn2_xlations;
  
  if (MB_CUR_MAX == 0)
    newtw->text.char_size = 1; /* Wyoming 64-bit fix */
  else 
    newtw->text.char_size = (char)MB_CUR_MAX; /* Wyoming 64-bit fix */

  if (req->core.width == 0) newtw->core.width = req->core.width;
  if (req->core.height == 0) newtw->core.height = req->core.height;
  
  /* Flag used in losing focus verification to indicate that a traversal
     key was pressed.  Must be initialized to False */
  newtw->text.traversed = False;
  
  newtw->text.total_lines = 1;
  newtw->text.top_line = 0;
  newtw->text.vsbar_scrolling = False;
  newtw->text.hsbar_scrolling = False;
  newtw->text.in_setvalues = False;
  
  if (newtw->text.output_create == NULL)
    newtw->text.output_create = _XmTextOutputCreate;
  if (newtw->text.input_create == NULL)
    newtw->text.input_create = _XmTextInputCreate;
  
  /*  The following resources are defaulted to invalid values to indicate    */
  /*  that it was not set by the application.  If it gets to this point      */
  /*  and they are still invalid then set them to their appropriate default. */
  
  if (!XmRepTypeValidValue(XmRID_EDIT_MODE,
			   newtw->text.edit_mode, nw)) {
    newtw->text.edit_mode = XmSINGLE_LINE_EDIT;
  }

   /* All 8 buffers must be created to be able to rotate the cut buffers */
   _XmCreateCutBuffers(nw);

   if (newtw->text.verify_bell == (Boolean) XmDYNAMIC_BOOL) {
     if (_XmGetAudibleWarning(nw) == XmBELL) 
       newtw->text.verify_bell = True;
     else
       newtw->text.verify_bell = False;
   }

   /*
    * Initialize on-the-spot data
    */
   newtw->text.onthespot = (OnTheSpotDataTW) XtMalloc(sizeof(OnTheSpotDataRecTW)
);
   newtw->text.onthespot->start = newtw->text.onthespot->end =
   newtw->text.onthespot->cursor = newtw->text.onthespot->over_len =
   newtw->text.onthespot->over_maxlen = 0;
   newtw->text.onthespot->over_str = NULL;
   newtw->text.onthespot->under_preedit = False;
   newtw->text.onthespot->under_verify_preedit = False;
   newtw->text.onthespot->verify_commit = False;

/* Solaris 2.6 Motif diff bug #4085003 */
#ifdef CDE_INTEGRATE
    {
    Boolean btn1_transfer;
    XtVaGetValues((Widget)XmGetXmDisplay(XtDisplay(nw)), "enableBtn1Transfer", &btn1_transfer, NULL);
    if (btn1_transfer) { /* for btn2 extend and transfer cases */
        if (!btn1_xlations)
            btn1_xlations = XtParseTranslationTable(_XmTextEventBindingsCDE);
        XtOverrideTranslations(nw, btn1_xlations);
       }
    if (btn1_transfer == True) { /* for btn2 extend case */
        if (!btn2_xlations)
          btn2_xlations = XtParseTranslationTable(_XmTextEventBindingsCDEBtn2);
        XtOverrideTranslations(nw, btn2_xlations);
       }
    }
#endif /* CDE_INTEGRATE */
/* END Solaris 2.6 Motif diff bug #4085003 */
/* Sun highlight callbacks */
    newtw->text.highlightColor1 = NULL;
    newtw->text.highlightColor2 = NULL;
    
    newtw->text.char_status = XmCHAR_OK; /* Bug : 1217687/4128045/4154215 */
}

/*
 * Create a text widget.  Note that most of the standard stuff is actually
 * to be done by the output create routine called here, since output is in
 * charge of window handling.
 */
static void 
InitializeHook(Widget wid,
	       ArgList args,
	       Cardinal *num_args_ptr)
{
  register XmTextWidget tw;
  Cardinal num_args = *num_args_ptr;
  XmTextSource source;
  XmTextPosition top_character;
  XmTextBlockRec block;
  Position dummy;
  Boolean used_source = False;
  
  tw = (XmTextWidget) wid;
  
  /* If text.wc_value is set, it overrides. Call _Xm..Create with it. */
  if (tw->text.source == nullsourceptr) {
    if (tw->text.wc_value != NULL) {
	       /* Bug Id : 1217687/4128045/4154215 */
      source = StringSourceCreate(wid, (char*)tw->text.wc_value, True);
      tw->text.value = NULL;
      tw->text.wc_value = NULL;
    } else {
	       /* Bug Id : 1217687/4128045/4154215 */
      source = StringSourceCreate(wid, tw->text.value, False);
      tw->text.value = NULL;
    }
  } else {
    source = tw->text.source;
    if (tw->text.wc_value != NULL) {
      char * tmp_value;
      long num_chars, n_bytes; /* Wyoming 64-bit fix */ 
      
      for (num_chars=0; tw->text.wc_value[num_chars]!=0L; num_chars++)
	/*EMPTY*/;
      
      tmp_value = XtMalloc( 
			   (num_chars + 1) * (int)tw->text.char_size);
      n_bytes = wcstombs(tmp_value, tw->text.wc_value,
			 (num_chars + 1) * (int)tw->text.char_size);
      if (n_bytes == -1)
         n_bytes = _Xm_wcs_invalid(tmp_value, tw->text.wc_value,
				   (num_chars + 1) * (int)tw->text.char_size);
      tmp_value[n_bytes] = 0;  /* NULL terminate the string */
      _XmStringSourceSetValue(tw, tmp_value);
      XtFree(tmp_value);
      tw->text.wc_value = NULL;
    } else if (tw->text.value != NULL) {
      /* Default value or argument ? */
      int i;
      for (i = 0; i < num_args; i++)
	if (tw->text.value == (char *)args[i].value &&
	    (args[i].name == XmNvalue || 
	     strcmp(args[i].name, XmNvalue) == 0)) {
	  _XmStringSourceSetValue(tw, tw->text.value);
	  break;
	}
    }
    tw->text.value = NULL;
    used_source = True;
  }
  
  tw->text.disable_depth = 1;
  tw->text.first_position = 0;
  tw->text.last_position = 0;
  tw->text.dest_position = 0;
  
  tw->text.needs_refigure_lines = tw->text.needs_redisplay = TRUE;
  tw->text.number_lines = 0;
  tw->text.maximum_lines = 1;
  tw->text.line = (Line) XtMalloc(sizeof(LineRec));
  tw->text.line->start = PASTENDPOS;
  tw->text.line->changed = False;
  tw->text.line->changed_position = PASTENDPOS;
  tw->text.line->past_end = False;
  tw->text.line->extra = NULL;
  tw->text.repaint.number = tw->text.repaint.maximum = 0;
  tw->text.repaint.range = (RangeRec *) XtMalloc(sizeof(RangeRec));
  tw->text.highlight.number = tw->text.highlight.maximum = 1;
  tw->text.highlight.list = (_XmHighlightRec *)
    XtMalloc(sizeof(_XmHighlightRec));
  tw->text.highlight.list[0].position = 0;
  tw->text.highlight.list[0].mode = XmHIGHLIGHT_NORMAL;
  tw->text.old_highlight.number = 0;
  tw->text.old_highlight.maximum = 1;
  tw->text.old_highlight.list = (_XmHighlightRec *)
    XtMalloc(sizeof(_XmHighlightRec));
  tw->text.url_highlight.number = 0;
  tw->text.url_highlight.maximum = 0;
  tw->text.url_highlight.list = NULL;
  tw->text.highlight_changed = FALSE;
  tw->text.on_or_off = on;
  tw->text.force_display = -1;
  tw->text.in_redisplay = tw->text.in_refigure_lines = FALSE;
  tw->text.in_resize = FALSE;
  tw->text.in_expose = FALSE;
  tw->text.pending_scroll = 0;
  tw->text.new_top = tw->text.top_character;
  tw->text.bottom_position = 0;
  tw->text.add_mode = False;
  tw->text.pendingoff = True;
  tw->text.forget_past = 0;

  /* Translation table overwrite */
  if (XmDirectionMatch(XmPrim_layout_direction(tw),
		       XmTOP_TO_BOTTOM_RIGHT_TO_LEFT)) {
    char *vevent_bindings;
    XtTranslations tm_table;
    
    vevent_bindings =
		(String)XtMalloc(strlen(_XmTextIn_XmTextVEventBindings) + 1);
    strcpy(vevent_bindings, _XmTextIn_XmTextVEventBindings);
    tm_table = (XtTranslations)XtParseTranslationTable(vevent_bindings);
    XtFree(vevent_bindings);
    XtOverrideTranslations(wid, tm_table);
  }
  
  /* Initialize table */
  if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
    InitializeLineTable(tw, 1);
  else
    InitializeLineTable(tw, INIT_TABLE_SIZE);
  
  (*tw->text.source->RemoveWidget)(tw->text.source, tw);
  tw->text.source = source;
  (*tw->text.source->AddWidget)(tw->text.source, tw);
  (*tw->text.output_create)(wid, args, num_args);
  
  _XmTextSetEditable(wid, tw->text.editable, True);
  _XmStringSourceSetMaxLength(GetSrc(tw), tw->text.max_length);
  
  (*tw->text.input_create)(wid, args, num_args);
  
  tw->text.first_position = 
    (*tw->text.source->Scan)(tw->text.source, 0,
				 XmSELECT_ALL, XmsdLeft, 1, FALSE);
  tw->text.last_position = 
    (*tw->text.source->Scan)(tw->text.source, 0,
				 XmSELECT_ALL, XmsdRight, 1, FALSE);
  
  if (tw->text.cursor_position < 0)
    tw->text.cursor_position = 0;
  
  if (tw->text.cursor_position > tw->text.last_position)
    tw->text.cursor_position = tw->text.last_position;
  
  tw->text.dest_position = tw->text.cursor_position;
  
  if (!tw->text.editable || !XtIsSensitive(wid))
    _XmTextSetDestinationSelection(wid, 0, False, (Time)NULL);
  
  if (tw->text.edit_mode == XmMULTI_LINE_EDIT)
    top_character = (*tw->text.source->Scan)(tw->text.source, 
                                                 tw->text.top_character, 
						 XmSELECT_LINE, XmsdLeft, 1, 
						 FALSE);
  else
    top_character = tw->text.top_character;
  
  tw->text.new_top = top_character;
  tw->text.top_character = 0;
  _XmTextInvalidate(tw, top_character, top_character, NODELTA);
  if (tw->text.disable_depth == 0)
    Redisplay(tw);
  
  /*
   * Fix for CR 5704 - If the source has already been created, do not use
   *                   the original code - it has already been processed and
   *                   the gaps are not where they were the first time 
   *                   through for this source.  Instead, use
   *                   code similar to that used in XmTextSetSource().
   */
  if (!used_source) {
    tw->text.source->data->gap_start[0] = '\0'; /*Hack to utilize initial
						  value when setting line
						  table - saves a malloc
						  and free. */
    if (tw->text.char_size == 1) {
      block.ptr = tw->text.source->data->ptr;
      if (block.ptr == NULL) block.length = 0;
      else block.length = strlen(block.ptr);
    } else 
      (void)(*tw->text.source->ReadSource)(source, 0, source->data->length,
					       &block);
  } else
    (void)(*tw->text.source->ReadSource)(source, 0, source->data->length,
					     &block);
  
  _XmTextUpdateLineTable(wid, 0, 0, &block, False);
  
  _XmStringSourceSetGappedBuffer(source->data, tw->text.cursor_position);
  
  tw->text.forget_past = tw->text.first_position;
  
  tw->text.disable_depth = 0;
  (*tw->text.output->PosToXY)(tw, tw->text.cursor_position,
				  &(tw->text.cursor_position_x), &dummy);
}

static void 
Realize(Widget w,
        XtValueMask *valueMask,
        XSetWindowAttributes *attributes)
{
  XmTextWidget tw = (XmTextWidget) w;
  Position dummy;
  Arg im_args[20];
  XIMCallback xim_cb[4];
  Cardinal n = 0;
  
  (*tw->text.output->realize)(w, valueMask, attributes);
  (*tw->text.output->PosToXY)(tw, tw->text.cursor_position,
			      &(tw->text.cursor_position_x), &dummy);

  if (tw->text.editable) {
  /*
   * Register on the spot callbacks.
   */
    xim_cb[0].client_data = (XPointer)tw;
    xim_cb[0].callback = (XIMProc)PreeditStart;
    xim_cb[1].client_data = (XPointer)tw;
    xim_cb[1].callback = (XIMProc)PreeditDone;
    xim_cb[2].client_data = (XPointer)tw;
    xim_cb[2].callback = (XIMProc)PreeditDraw;
    xim_cb[3].client_data = (XPointer)tw;
    xim_cb[3].callback = (XIMProc)PreeditCaret;
    XtSetArg(im_args[n], XmNpreeditStartCallback, &xim_cb[0]); n++;
    XtSetArg(im_args[n], XmNpreeditDoneCallback, &xim_cb[1]); n++;
    XtSetArg(im_args[n], XmNpreeditDrawCallback, &xim_cb[2]); n++;
    XtSetArg(im_args[n], XmNpreeditCaretCallback, &xim_cb[3]); n++;
    XmImSetValues(w, im_args, n);
  }
}


/****************************************************************
 *
 * Semi-public definitions.
 *
 ****************************************************************/
static void 
Destroy(Widget w)
{
  XmTextWidget tw = (XmTextWidget) w;
  int j;
  
  (*tw->text.source->RemoveWidget)(tw->text.source, tw);
  if (tw->text.input->destroy) (*tw->text.input->destroy)(w);
  if (tw->text.output->destroy) (*tw->text.output->destroy)(w);
  
  for (j = 0; j < tw->text.number_lines; j++) {
    if (tw->text.line[j].extra)
      XtFree((char *)tw->text.line[j].extra);
  }
  
  XtFree((char *)tw->text.line);
  
  XtFree((char *)tw->text.repaint.range);
  XtFree((char *)tw->text.highlight.list);
  XtFree((char *)tw->text.old_highlight.list);
  XtFree((char *)tw->text.url_highlight.list);
  
  if (tw->text.line_table != NULL)
    XtFree((char *)tw->text.line_table);
  if (tw->text.onthespot)
    XtFree((char *)tw->text.onthespot);
}

static void 
Resize(Widget w)
{
  XmTextWidget tw = (XmTextWidget) w;
  
  /* this flag prevents resize requests */
  tw->text.in_resize = True;
  
  if (_XmTextShouldWordWrap(tw))
    _XmTextRealignLineTable(tw, NULL, 0, 0, 0, PASTENDPOS);
  
  (*(tw->text.output->resize))(w, FALSE);
  
  tw->text.in_resize = False;
}

static void 
DoExpose(Widget w,
	 XEvent *event,
	 Region region)
{
  XmTextWidget tw = (XmTextWidget) w;
  
  /* this flag prevents resize requests */
  tw->text.in_expose = True;
  
  (*(tw->text.output->expose))(w, event, region);
  
  tw->text.in_expose = False;
}

static void 
GetValuesHook(Widget w,
	      ArgList args,
	      Cardinal *num_args_ptr)
{
  XmTextWidget tw = (XmTextWidget) w;
  Cardinal num_args = *num_args_ptr;
  int i;
  
  XtGetSubvalues((XtPointer) tw,
		 resources, XtNumber(resources), args, num_args);
  
  for (i = 0; i < num_args; i++) {
    if (!strcmp(args[i].name, XmNvalue)) {
      *((XtPointer *)args[i].value) =
	(XtPointer)_XmStringSourceGetValue(GetSrc(tw), False);
    }
  }
  
  for (i = 0; i < num_args; i++) {
    if (!strcmp(args[i].name, XmNvalueWcs)) {
      *((XtPointer *)args[i].value) =
	(XtPointer)_XmStringSourceGetValue(GetSrc(tw), True);
    }
  }
  
  (*tw->text.output->GetValues)(w, args, num_args);
  (*tw->text.input->GetValues)(w, args, num_args);
}

void
_XmTextSetTopCharacter(Widget widget,
		       XmTextPosition top_character)
{
  XmTextWidget tw = (XmTextWidget) widget;
  LineNum line_num;
  
  if (tw->text.edit_mode != XmSINGLE_LINE_EDIT) {
    line_num = _XmTextGetTableIndex(tw, top_character);
    top_character = tw->text.line_table[line_num].start_pos;
  }
  
  if (top_character != tw->text.new_top) {
    EraseInsertionPoint(tw);
    tw->text.new_top = top_character;
    tw->text.pending_scroll = 0;
    tw->text.needs_refigure_lines = tw->text.needs_redisplay = TRUE;
    if (XmSINGLE_LINE_EDIT == tw->text.edit_mode)
	tw->text.output->data->hoffset = 0;
    if (tw->text.disable_depth == 0)
      Redisplay(tw);
    TextDrawInsertionPoint(tw);
  }
}

static void 
LosingFocus(XmTextWidget tw)
{
  XmTextVerifyCallbackStruct  cbdata;
  
  cbdata.reason = XmCR_LOSING_FOCUS;
  cbdata.event = NULL;
  cbdata.doit = True;
  cbdata.currInsert = tw->text.cursor_position;
  cbdata.newInsert = tw->text.cursor_position;
  cbdata.startPos = tw->text.cursor_position;
  cbdata.endPos = tw->text.cursor_position;
  cbdata.text = NULL;
  XtCallCallbackList((Widget)tw, tw->text.losing_focus_callback, 
		     (XtPointer) &cbdata);
  tw->text.source->data->take_selection = True;
}

/* ARGSUSED */
static Boolean 
SetValues(Widget oldw,
	  Widget reqw,
	  Widget new_w,
	  ArgList args,
	  Cardinal *num_args)
{
  XmTextWidget old = (XmTextWidget) oldw;
  XmTextWidget newtw = (XmTextWidget) new_w;
  XmTextPosition new_cursor_pos;
  Boolean o_redisplay;
  Position dummy;
  Boolean need_new_cursorPos = False;
  Boolean need_text_redisplay = False;
  Boolean new_source = (newtw->text.source != old->text.source);
  XmTextSource cache_source;

  if (newtw->core.being_destroyed) return False;

  _XmTextResetIC(oldw);
  
  newtw->text.in_setvalues = True;
  
  if (newtw->text.cursor_position<0)
    newtw->text.cursor_position=0;

  /* It is unfortunate that the rest of the Text widget code, particularly the
  ** redisplay code, assumes that the current source is valid; in fact, it may
  ** have been changed by a set-values call. Ideally, we would be able to 
  ** handle before anything else a change in the XmNsource resource of the 
  ** widget; in practice, the changes would be extensive. 
  ** Compromise by temporarily restoring the old value for those pieces of 
  ** code which affect the display of the old source; then restore the new 
  ** value for the display of the current source. That is, deal with the old 
  ** value for just this one line of code.
  */
  if (new_source) {
      cache_source = newtw->text.source;
      newtw->text.source = old->text.source;
  }

  EraseInsertionPoint(newtw); /* assumes newtw->text.source matches values */

  if (new_source)
      newtw->text.source = cache_source;
  
  _XmTextDisableRedisplay(newtw, TRUE);
  
  /* set cursor_position to a known acceptable value (0 is always acceptable)
   */
  new_cursor_pos = newtw->text.cursor_position;
  newtw->text.cursor_position = 0;
  
  if (! XtIsSensitive(new_w) &&
      newtw->text.input->data->has_destination) {
    _XmTextSetDestinationSelection(new_w, 0, True,
				   XtLastTimestampProcessed(XtDisplay(new_w)));
  }
  
  if (!XmRepTypeValidValue(XmRID_EDIT_MODE,
			     newtw->text.edit_mode, new_w)) {
    newtw->text.edit_mode = old->text.edit_mode;
  }
  
  if ((old->text.top_character != newtw->text.top_character) &&
      (newtw->text.top_character != newtw->text.new_top)) {
    XmTextPosition new_top;
    if (newtw->text.output->data->resizeheight &&
     !(newtw->text.output->data->scrollvertical &&
     XmIsScrolledWindow(XtParent((Widget)newtw))) )
      new_top = 0;
    else
      new_top = newtw->text.top_character;
    
    newtw->text.top_character = old->text.top_character;
    _XmTextSetTopCharacter(new_w, new_top);
    if (newtw->text.needs_refigure_lines)
      newtw->text.top_character = new_top;
  }
  
  if (old->text.source != newtw->text.source) {
    XmTextSource source = newtw->text.source;
    newtw->text.source = old->text.source;
    o_redisplay = newtw->text.needs_redisplay;
    XmTextSetSource(new_w, source, old->text.top_character, 0);
    need_text_redisplay = newtw->text.needs_redisplay;
    newtw->text.needs_redisplay = o_redisplay;
  }
  
  if (old->text.editable != newtw->text.editable) {
    Boolean editable = newtw->text.editable;
    newtw->text.editable = old->text.editable;
    _XmTextSetEditable(new_w, editable, False);
  }
  
  _XmStringSourceSetMaxLength(GetSrc(newtw), newtw->text.max_length);
  
  /* Four cases to handle for value:
   *   1. user set both XmNvalue and XmNwcValue.
   *   2. user set the opposite resource (i.e. value is a char*
   *      and user set XmNwcValue, or vice versa).
   *   3. user set the corresponding resource (i.e. value is a char*
   *      and user set XmNValue, or vice versa).
   *   4. user set neither XmNValue nor XmNwcValue
   */
  
  /* OSF says:  if XmNvalueWcs set, it overrides all else */
  
  if (newtw->text.wc_value != NULL) {
    /* user set XmNvalueWcs resource - it rules ! */
    wchar_t * wc_value;
    char * tmp_value;
    long num_chars, n_bytes; /* Wyoming 64-bit fix */ 
    
    num_chars = n_bytes = 0;
    
    for (num_chars = 0, wc_value = newtw->text.wc_value;
	 wc_value[num_chars] != 0L;) num_chars++;
    
    tmp_value = XtMalloc((size_t) /* Wyoming 64-bit Fix */
			 (num_chars + 1) * (int)newtw->text.char_size);
    n_bytes = wcstombs(tmp_value, newtw->text.wc_value,
		       (num_chars + 1) * (int)newtw->text.char_size);
    if (n_bytes == -1)
       n_bytes = _Xm_wcs_invalid(tmp_value, newtw->text.wc_value,
			         (num_chars + 1) * (int)newtw->text.char_size);
    tmp_value[n_bytes] = 0;  /* NULL terminate the string */
    o_redisplay = newtw->text.needs_redisplay;
    newtw->text.wc_value = NULL;
    newtw->text.value = NULL;
    _XmStringSourceSetValue(newtw, tmp_value);
    need_text_redisplay = newtw->text.needs_redisplay;
    newtw->text.needs_redisplay = o_redisplay;
    XtFree(tmp_value);
    need_new_cursorPos = True;
  } else if (newtw->text.value != NULL) {
    char * tmp_value;
    
    newtw->text.pendingoff = TRUE;
    o_redisplay = newtw->text.needs_redisplay;
    tmp_value = newtw->text.value;
    newtw->text.value = NULL;
    _XmStringSourceSetValue(newtw, tmp_value);
    need_text_redisplay = newtw->text.needs_redisplay;
    newtw->text.needs_redisplay = o_redisplay;
    need_new_cursorPos = True;
  }
  
  /* return cursor_position to it's original changed value */
  newtw->text.cursor_position = new_cursor_pos;
  
  if (old->text.cursor_position != newtw->text.cursor_position) {
    XmTextPosition new_position = newtw->text.cursor_position;
    newtw->text.cursor_position = old->text.cursor_position;
    
    if (new_position > newtw->text.source->data->length)
      _XmTextSetCursorPosition(new_w, newtw->text.source->data->length);
    else
      _XmTextSetCursorPosition(new_w, new_position);
  } else if (need_new_cursorPos) {
    XmTextPosition cursorPos = -1;
    int ix;

    for (ix = 0; ix < *num_args; ix++)
      if (strcmp(args[ix].name, XmNcursorPosition) == 0) {
	cursorPos = (XmTextPosition)args[ix].value;
	break;
      }
    if (cursorPos == -1)
      cursorPos = (*newtw->text.source->Scan)(newtw->text.source,
					      newtw->text.cursor_position,
					      XmSELECT_ALL, XmsdLeft, 1, TRUE);
    _XmTextSetCursorPosition(new_w, cursorPos);
  } else 
    if (newtw->text.cursor_position > newtw->text.source->data->length) {
      _XmTextSetCursorPosition(new_w, newtw->text.source->data->length);
    }
   
  
  o_redisplay = (*newtw->text.output->SetValues)
    (oldw, reqw, new_w, args, num_args);
  (*newtw->text.input->SetValues)(oldw, reqw, new_w, args, num_args);
  newtw->text.forget_past = 0;
  newtw->text.disable_depth--;   /* _XmTextEnableRedisplay() is not called
				    because we don't want a repaint yet */
  TextDrawInsertionPoint(newtw); /* increment cursor_on stack in lieu of
				    _XmTextEnableRedisplay() call. */
  (*newtw->text.output->PosToXY)(newtw, newtw->text.cursor_position,
				 &(newtw->text.cursor_position_x), &dummy);
  
  if (o_redisplay) newtw->text.needs_redisplay = True;
  
  TextDrawInsertionPoint(newtw);
  
  if (XtIsSensitive(new_w) != XtIsSensitive(oldw)) {
    if (XtIsSensitive(new_w)) {
      EraseInsertionPoint(newtw);
      newtw->text.output->data->blinkstate = off;
      TextDrawInsertionPoint(newtw);
    } else {
      if (newtw->text.output->data->hasfocus) {
	newtw->text.output->data->hasfocus = False;
	_XmTextChangeBlinkBehavior(newtw, False);
	EraseInsertionPoint(newtw);
	newtw->text.output->data->blinkstate = on;
	TextDrawInsertionPoint(newtw);
        XmImUnsetFocus(new_w);
	/* fix for bug 4367450 - leob */
	(void) VerifyLeave(new_w, NULL);
      }
    }
    if (newtw->text.source->data->length > 0)
      newtw->text.needs_redisplay = True;
  }
  
  if ((!newtw->text.editable || !XtIsSensitive(new_w)) &&
      _XmTextHasDestination(new_w))
    _XmTextSetDestinationSelection(new_w, 0, False, (Time)NULL);
  
  /* don't shrink to nothing */
  if (newtw->core.width == 0) newtw->core.width = old->core.width;
  if (newtw->core.height == 0) newtw->core.height = old->core.height;
  
  /* Optimization for the case when only XmNvalue changes. 
     This considerably reduces flashing due to unneeded redraws */
  if (need_text_redisplay && 
      !newtw->text.needs_redisplay && 
      newtw->text.disable_depth == 0) {
    EraseInsertionPoint(newtw);
    newtw->text.disable_depth++;
    newtw->text.needs_redisplay = True;
    _XmTextEnableRedisplay(newtw);
    newtw->text.needs_redisplay = False;
  }
  
  newtw->text.in_setvalues = newtw->text.needs_redisplay;
  
  return newtw->text.needs_redisplay;
}

static XtGeometryResult 
QueryGeometry(Widget w,
	      XtWidgetGeometry *intended,
	      XtWidgetGeometry *reply)
{
  XmTextWidget tw = (XmTextWidget) w;
  
  if (GMode (intended) & (~(CWWidth | CWHeight)))
    return(XtGeometryNo);
  
  reply->request_mode = (CWWidth | CWHeight);
  
  (*tw->text.output->GetPreferredSize)(w, &reply->width, &reply->height);
  if ((GMode(intended) != GMode(reply)) ||
      (reply->width != intended->width) ||
      (reply->height != intended->height))
    return (XtGeometryAlmost);
  else {
    reply->request_mode = 0;
    return (XtGeometryYes);
  }
}

static void 
_XmTextSetString(Widget widget,
		 char *value)
{
  XmTextWidget tw = (XmTextWidget) widget;
  
  _XmTextResetIC(widget);

  tw->text.pendingoff = TRUE;
  if (value == NULL) value = "";
  _XmStringSourceSetValue(tw, value);
  
  /* after set, move insertion cursor to beginning of string. */
  _XmTextSetCursorPosition(widget, 0);
}

void 
_XmTextSetCursorPosition(Widget widget,
			 XmTextPosition position)
{
  XmTextWidget tw = (XmTextWidget) widget;
  XmTextSource source;
  XmTextVerifyCallbackStruct cb;
  Position dummy;
  int n = 0;
  XPoint xmim_point;
  XRectangle xmim_area;
  Arg args[10];
  
  if (position < 0) {
    position = 0;
  }
  
  if (position > tw->text.last_position) {
    position = tw->text.last_position;
  }
  
  source = GetSrc(tw);
  
  /* if position hasn't changed, don't call the modify verify callback */
  if (position != tw->text.cursor_position) {
    /* Call Motion Verify Callback before Cursor Changes Positon */
    cb.reason = XmCR_MOVING_INSERT_CURSOR;
    cb.event  = NULL;
    cb.currInsert = tw->text.cursor_position;
    cb.newInsert = position;
    cb.doit = True;
    XtCallCallbackList (widget, tw->text.motion_verify_callback,
			(XtPointer) &cb);
    
    /* Cancel action upon application request */
    if (!cb.doit) {
      if (tw->text.verify_bell) XBell(XtDisplay(widget), 0);
      return;
    }
  }
  
  /* Erase insert cursor prior to move */
  EraseInsertionPoint(tw);
  tw->text.cursor_position = position;
  
  /*
   * If not in add_mode and pending delete state is on reset
   * the selection.
   */
  if (!tw->text.add_mode && tw->text.pendingoff &&
      _XmStringSourceHasSelection(source))
    (*source->SetSelection)(source, position, position,
			    XtLastTimestampProcessed(XtDisplay(widget)));
  
  /* ensure that IBeam at new location will be displayed correctly */
  _XmTextMovingCursorPosition(tw, position); /*correct GC for new location */
  
  if (tw->text.auto_show_cursor_position)
    _XmTextShowPosition(widget, position);
  if (tw->text.needs_redisplay && tw->text.disable_depth == 0)
    Redisplay(tw);
  
  (*tw->text.output->PosToXY) (tw, position, &(tw->text.cursor_position_x), 
			       &dummy);

  tw->text.output->data->refresh_ibeam_off = True; /* update IBeam off area
						    * before drawing IBeam  */

  (*tw->text.output->PosToXY)(tw, position, &xmim_point.x, &xmim_point.y);
  (void)_XmTextGetDisplayRect((Widget)tw, &xmim_area);
  n = 0;
  XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
  XtSetArg(args[n], XmNarea, &xmim_area); n++;
  XmImSetValues((Widget)tw, args, n);
  TextDrawInsertionPoint(tw);
}

/* ARGSUSED */
void 
_XmTextDisableRedisplay(XmTextWidget widget,
#if NeedWidePrototypes
			int losesbackingstore)
#else
                        Boolean losesbackingstore)
#endif /* NeedWidePrototypes */
{
  widget->text.disable_depth++;
  EraseInsertionPoint(widget);
}

void 
_XmTextEnableRedisplay(XmTextWidget widget)
{
  if (widget->text.disable_depth) widget->text.disable_depth--;
  if (widget->text.disable_depth == 0 && widget->text.needs_redisplay)
    Redisplay(widget);
  
  /* If this is a scrolled widget, better update the scroll bars to reflect
   * any changes that have occured while redisplay has been disabled.  */
  
  if (widget->text.disable_depth == 0) {
    if (XmDirectionMatch(XmPrim_layout_direction(widget),
			 XmTOP_TO_BOTTOM_RIGHT_TO_LEFT)) {
      if (widget->text.output->data->scrollvertical &&
	  XtClass(widget->core.parent) == xmScrolledWindowWidgetClass)
	_XmRedisplayVBar(widget);
      if (widget->text.output->data->scrollhorizontal &&
	  XtClass(widget->core.parent) == xmScrolledWindowWidgetClass &&
	  !widget->text.hsbar_scrolling)
	_XmChangeHSB(widget);
    } else {
      if (widget->text.output->data->scrollvertical &&
	  XtClass(widget->core.parent) == xmScrolledWindowWidgetClass &&
	  !widget->text.vsbar_scrolling)
	_XmChangeVSB(widget);
      if (widget->text.output->data->scrollhorizontal &&
	  XtClass(widget->core.parent) == xmScrolledWindowWidgetClass)
	_XmRedisplayHBar(widget);
    }
  }
  
  TextDrawInsertionPoint(widget);
}

/* Bug Id : 1217687/4128045/4154215
 * Check new Character Status flag on Text Widget to determine if truncation
 * has taken place because of an illegal byte sequence
 */
Boolean
_XmTextCheckErrorStatus(Widget w)
{
  XmTextWidget tw = (XmTextWidget) w;
  Boolean retval = False;

  if (tw->text.char_status == XmCHAR_EILSEQ)
  {
      tw->text.char_status = XmCHAR_OK;
      retval = True;
  }
  else
      retval = False;

  return retval;
}

/* Count the number of characters represented in the char* str.  By
 * definition, if MB_CUR_MAX == 1 then num_count_bytes == number of characters.
 * Otherwise, use mblen to calculate. */

size_t/* Wyoming 64-bit Fix */
_XmTextCountCharacters(char *str,
		       size_t num_count_bytes)/* Wyoming 64-bit Fix */
{
/* Bug Id : 1217687/4128045/4154215 */
/* Change to be wrapper for new function that set error status */
return (TextCountCharacters((Widget)NULL, str, num_count_bytes));
}

/* Bug Id : 1217687/4128045/4154215 */
/* Same as _XmTextCountCharacters was except with Widget parameter */
size_t/* Wyoming 64-bit Fix */
TextCountCharacters(Widget w,
		       char *str,
		       size_t num_count_bytes)/* Wyoming 64-bit Fix */
{
  XmTextWidget tw = (XmTextWidget) w;
  char * bptr;
  int count = 0;
  int char_size = 0;
  
  /* Wyoming 64-bit Fix */
  if (num_count_bytes == 0)
    return 0;
  
  if (MB_CUR_MAX == 1 || MB_CUR_MAX == 0) /* Sun sets MB_CUR_MAX to 0, Argg!!*/
    return num_count_bytes;
  
  for (bptr = str; num_count_bytes != 0; count++, bptr+= char_size)
  { /* Wyoming 64-bit fix */ 
    char_size = mblen(bptr, MB_CUR_MAX);
    if (char_size == -1) char_size = 1;
    else if (char_size == 0) break; /* Null Character returns 0 length byte */
    num_count_bytes -= char_size;
  }
  return count;
}

void 
_XmTextSetEditable(Widget widget, Boolean editable, Boolean in_init)
{
  Arg args[20];
  XIMCallback xim_cb[4];
  Cardinal n = 0;
  XPoint xmim_point;
  XRectangle xmim_area;
  XmTextWidget tw = (XmTextWidget) widget;
  
  if (!tw->text.editable && editable) {
    OutputData o_data = tw->text.output->data;

    XmImRegister(widget, (unsigned int) NULL);
    
    (*tw->text.output->PosToXY)(tw, tw->text.cursor_position,
				&xmim_point.x, &xmim_point.y);
    (void)_XmTextGetDisplayRect((Widget)tw, &xmim_area);
    n = 0;
    XtSetArg(args[n], XmNfontList, o_data->fontlist); n++;
    XtSetArg(args[n], XmNbackground, widget->core.background_pixel); n++;
    XtSetArg(args[n], XmNforeground, tw->primitive.foreground); n++;
    XtSetArg(args[n], XmNbackgroundPixmap,
	     widget->core.background_pixmap); n++;
    XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
    XtSetArg(args[n], XmNarea, &xmim_area); n++;
    XtSetArg(args[n], XmNlineSpace, o_data->lineheight); n++;

   /*
    * 	Register on the spot callbacks.
    */

   xim_cb[0].client_data = (XPointer)tw;
   xim_cb[0].callback = (XIMProc)PreeditStart;
   xim_cb[1].client_data = (XPointer)tw;
   xim_cb[1].callback = (XIMProc)PreeditDone;
   xim_cb[2].client_data = (XPointer)tw;
   xim_cb[2].callback = (XIMProc)PreeditDraw;
   xim_cb[3].client_data = (XPointer)tw;
   xim_cb[3].callback = (XIMProc)PreeditCaret;
   XtSetArg(args[n], XmNpreeditStartCallback, &xim_cb[0]); n++;
   XtSetArg(args[n], XmNpreeditDoneCallback, &xim_cb[1]); n++;
   XtSetArg(args[n], XmNpreeditDrawCallback, &xim_cb[2]); n++;
   XtSetArg(args[n], XmNpreeditCaretCallback, &xim_cb[3]); n++;

   if (o_data->hasfocus)
      XmImSetFocusValues(widget, args, n);
   else
      XmImSetValues(widget, args, n);
  } else if (tw->text.editable && !editable) {
    XmImUnregister(widget);
  }
  
  tw->text.editable = editable;
  
  n = 0;
  
  if (editable) {
    XtSetArg(args[n], XmNdropSiteActivity, XmDROP_SITE_ACTIVE); n++;
  } else {
    XtSetArg(args[n], XmNdropSiteActivity, XmDROP_SITE_INACTIVE); n++;
  }

  if (!in_init)
   {
    XmDropSiteUpdate(widget, args, n);
   }

  _XmStringSourceSetEditable(GetSrc(tw), editable);
}

void 
_XmTextSetHighlight(Widget w,
		   XmTextPosition left,
		   XmTextPosition right,
		   XmHighlightMode mode)
{
  XmTextWidget tw = (XmTextWidget)w;
  _XmHighlightRec *l;
  XmHighlightMode endmode;
  int i, j;
  _XmWidgetToAppContext(w);
  
  _XmAppLock(app);

  /* If right position is out-bound, change it to the last position. */
  if (right > tw->text.last_position) 
    right = tw->text.last_position;
  
  /* If left is out-bound, don't do anything. */
  if (left >= right || right <= 0) {
    _XmAppUnlock(app);
    return;
  }

  if (left < 0) 
    left = 0;

  EraseInsertionPoint(tw);
  if (!tw->text.highlight_changed) {
    tw->text.highlight_changed = TRUE;
    if (tw->text.old_highlight.maximum < tw->text.highlight.number) {
      tw->text.old_highlight.maximum = tw->text.highlight.number;
      tw->text.old_highlight.list = (_XmHighlightRec *)
	XtRealloc((char *)tw->text.old_highlight.list,
		  tw->text.old_highlight.maximum *
		  sizeof(_XmHighlightRec));
    }
    tw->text.old_highlight.number = tw->text.highlight.number;
    memcpy((void *) tw->text.old_highlight.list,
	   (void *) tw->text.highlight.list,
	   (size_t) tw->text.old_highlight.number *
	   sizeof(_XmHighlightRec));
  }
  endmode = FindHighlight(tw, right, XmsdLeft)->mode;
  InsertHighlight(tw, left, right, mode);
  InsertHighlight(tw, right, -1, endmode);
  l = tw->text.highlight.list;
  i = 1;
  while (i < tw->text.highlight.number) {
    if (l[i].position >= left && l[i].position < right)
      l[i].mode = mode;
    if ((l[i].mode == l[i-1].mode) && (l[i].position > l[i-1].position)) {
      tw->text.highlight.number--;
      for (j=i; j<tw->text.highlight.number; j++)
	l[j] = l[j+1];
    } else i++;
  }

#ifdef ENABLE_URLS
     /* change URLs left in normal highlight back to url highlight */
    for (i=0; i < tw->text.url_highlight.number; i++) {
        int start = tw->text.url_highlight.list[i].position;
        int end = tw->text.url_highlight.list[i].end;
        XmHighlightMode mode1 = tw->text.url_highlight.list[i].mode;

        l = FindHighlight(tw, end - 1, XmsdLeft);
        if ((l->mode != XmHIGHLIGHT_NORMAL) && (l->position <= start))
            continue;
        else if ((l->mode == XmHIGHLIGHT_NORMAL) && (l->position <= start)) {
            endmode = FindHighlight(tw, end, XmsdLeft)->mode;
            InsertHighlight(tw, start, -1, mode1);
            InsertHighlight(tw, end, -1, endmode);
        }
        else if ((l->position > start) && (l->mode == XmHIGHLIGHT_NORMAL)) {
            endmode = FindHighlight(tw, end, XmsdLeft)->mode;
            InsertHighlight(tw, l->position, -1, mode1);
            InsertHighlight(tw, end, -1, endmode);
        }
        else if ((l->position > start) && (l->mode != XmHIGHLIGHT_NORMAL)) {
            l = FindHighlight(tw, l->position - 1, XmsdLeft);
            if ((l->mode == XmHIGHLIGHT_NORMAL) && (l->position <= start))
                InsertHighlight(tw, start, -1, mode1);
            else if (l->mode == XmHIGHLIGHT_NORMAL)
                l->mode = mode1;
        }
    }

      /* sanity check highlight list again */
    l = tw->text.highlight.list;
    i = 1;
    while (i < tw->text.highlight.number) {
        if ((l[i].mode == l[i-1].mode) && (l[i].position > l[i-1].position)) {
            tw->text.highlight.number--;
            for (j=i ; j<tw->text.highlight.number ; j++) {
                l[j] = l[j+1];
	    }
        } else i++;
    }
#endif /* ENABLE_URLS */

  /* Force the image GC to be updated based on the new highlight record */
  _XmTextMovingCursorPosition(tw, tw->text.cursor_position);
  tw->text.needs_redisplay = TRUE;
  if (tw->text.disable_depth == 0)
    Redisplay(tw);
  
  tw->text.output->data->refresh_ibeam_off = True;
  TextDrawInsertionPoint(tw);

  _XmAppUnlock(app);
}

#ifdef ENABLE_URLS
void
_XmTextSetUrlHighlightsFromList(Widget w, 
				UrlHighlightRec *urlHighlightList, 
				int size)
{
    int i;
    XmHighlightMode endmode;
    XmTextWidget tw = (XmTextWidget) w;

    if ((size <=0) || (!urlHighlightList))
	return;

    _XmAppLock(app);
    EraseInsertionPoint(tw);

    /* Since we know we're going to insert a bunch of new highlights,
     * delay most of the processing until we've updated the highlight
     * list, and then insert the very last highlight using the usual
     * call to get everything updated correctly. It's way too slow to
     * do all the updates for each url insertion when we're inserting
     * a large number of highlights. -wluo */

    /* stuff all but the last highlight into the text widget's highlight
       list */
    for (i=0; i<(size-1); i++)
    {
	/* some sanity checks first */
	if (urlHighlightList[i].end > tw->text.last_position)
	    urlHighlightList[i].end = tw->text.last_position;

	if ((urlHighlightList[i].position >= urlHighlightList[i].end) ||
	    (urlHighlightList[i].end <= 0))
	    continue;

	if (urlHighlightList[i].position < 0)
	    urlHighlightList[i].position = 0;

	endmode = FindHighlight(tw, urlHighlightList[i].end, XmsdLeft)->mode;
	InsertHighlight(tw, urlHighlightList[i].position, 
			urlHighlightList[i].end,
			urlHighlightList[i].mode);
	InsertHighlight(tw, urlHighlightList[i].end, -1, endmode);
    }

    /* Insert the last highlight using the normal call. This will update
       additional data structures in the text widget to keep track of
       url highlightings */
    _XmTextSetHighlight(w, urlHighlightList[i].position,
			urlHighlightList[i].end, urlHighlightList[i].mode);
    
    TextDrawInsertionPoint(tw);
    _XmAppUnlock(app);
}
#endif /* ENABLE_URLS */

void 
_XmTextShowPosition(Widget widget,
		    XmTextPosition position)
{
  XmTextWidget tw = (XmTextWidget) widget;
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  
  if (!tw->text.needs_refigure_lines && 
      (position < 0 || (position >= tw->text.top_character &&
			position < tw->text.bottom_position))) {
    (*tw->text.output->MakePositionVisible)(tw, position);
    _XmAppUnlock(app);
    return;
  }
  tw->text.force_display = position;
  tw->text.needs_refigure_lines = tw->text.needs_redisplay = TRUE;
  if (tw->text.disable_depth == 0) Redisplay(tw);

  _XmAppUnlock(app);
}

/* Why is this here? It's never used */
int
_XmTextGetTotalLines(Widget widget)
{
  return(((XmTextWidget)widget)->text.total_lines);
}


/* Used by libDtWidget (DtEditor) */
XmTextLineTable
_XmTextGetLineTable(Widget widget, 
		    int *total_lines)
{
  XmTextWidget tw = (XmTextWidget) widget;
  XmTextLineTable line_table;
  
  *total_lines = tw->text.total_lines;
  line_table = (XmTextLineTable) XtMalloc((unsigned) *total_lines *
					  sizeof(XmTextLineTableRec));
  
  memcpy((void *) line_table, (void *) tw->text.line_table,
	 *total_lines * sizeof(XmTextLineTableRec));
  
  return line_table;
}


/********************************************
 * AccessTextual trait method implementation 
 ********************************************/

static XtPointer
TextGetValue(Widget w, 
	     int format) 
{
  char *str;
  XmString tmp;
  
  switch(format) {
  case XmFORMAT_XmSTRING:
    str = XmTextGetString(w);
    tmp = XmStringCreateLocalized(str);
    if (str != NULL) XtFree(str);
    return((XtPointer) tmp);
  case XmFORMAT_MBYTE:
    return((XtPointer) XmTextGetString(w));
  case XmFORMAT_WCS:
    return((XtPointer) XmTextGetStringWcs(w));
  }

  return(NULL);
}

static void 
TextSetValue(Widget w, 
	     XtPointer s, 
	     int format)
{
  char *str;

  switch(format) {
  case XmFORMAT_XmSTRING:
    str = (char*) _XmStringUngenerate((XmString)s, NULL, 
				      XmMULTIBYTE_TEXT, XmMULTIBYTE_TEXT);
    XmTextSetString(w, str);
    if (str != NULL) XtFree(str);
    break;
  case XmFORMAT_MBYTE:
    XmTextSetString(w, (char*) s);
    break;
  case XmFORMAT_WCS:
    XmTextSetStringWcs(w, (wchar_t *) s);
  }
}

/*ARGSUSED*/
static int
TextPreferredValue(Widget w)	/* unused */
{
  return(XmFORMAT_MBYTE);
}

/**
 ** New functions for the on the spot support.
 **/

/*
 * This function and _XmTextSetCursorPosition are almost the same.
 * The difference is that this function doesn't call any user's
 * callbacks like XmNmotionVerifyCallback
 */
static void
_XmTextPreeditSetCursorPosition(Widget widget,
                                XmTextPosition position)
{
  XmTextWidget tw = (XmTextWidget) widget;
  XmTextSource source;
  Position dummy;

  if (position < 0) {
    position = 0;
  }

  if (position > tw->text.last_position) {
    position = tw->text.last_position;
  }

  source = GetSrc(tw);

  /* Erase insert cursor prior to move */
  EraseInsertionPoint(tw);
  tw->text.cursor_position = position;

  /* ensure that IBeam at new location will be displayed correctly */
  _XmTextMovingCursorPosition(tw, position); /*correct GC for new location */
  (*tw->text.output->PosToXY) (tw, position, &(tw->text.cursor_position_x),
                               &dummy);
  if (tw->text.auto_show_cursor_position)
    XmTextShowPosition(widget, position);
  if (tw->text.needs_redisplay && tw->text.disable_depth == 0)
    Redisplay(tw);

  _XmTextResetClipOrigin(tw, position, False); /* move clip origin */

  tw->text.output->data->refresh_ibeam_off = True; /* update IBeam off area
                                                    * before drawing IBeam */

  TextDrawInsertionPoint(tw);
}

static void PreeditVerifyReplace(Widget w,
			 XmTextPosition frompos,
			 XmTextPosition topos,
			 char *mb,
			 XmTextPosition cursor,
			 Boolean *end_preedit)
{
  XmTextWidget tw = (XmTextWidget)w;

  UnderVerifyPreedit(tw) = True;
  _XmTextReplace(w, frompos, topos, mb, False);
  UnderVerifyPreedit(tw) = False;
  if (VerifyCommitNeeded(tw)) {
    _XmTextResetIC(w);
    *end_preedit = True;
  }
  _XmTextSetCursorPosition(w, cursor);
}
  
  

/*
 * This is the function set to XNPreeditStartCallback resource.
 * This function is called when the preedit process starts.
 * Initialize the preedit data and also treat pending delete.
 */
static int
PreeditStart(XIC xic,
             XPointer client_data,
             XPointer call_data)
{
  XmTextPosition left, right, lastPos;
  Widget w = (Widget) client_data;
  XmTextWidget tw = (XmTextWidget) client_data;

  if (PreUnder(tw))
    return 0;

  /* check editable */
  if (!tw->text.source->data->editable){
    PreUnder(tw) = False;
    return 0;
  }

  PreOverLen(tw) = PreOverMaxLen(tw) = 0L;
  PreOverStr(tw) = NULL;

  /* Treat Pending delete */
  if (_XmTextNeedsPendingDeleteDis(tw, &left, &right, False))
    _XmTextReplace(w, left, right, NULL, False);

  PreStartTW(tw) = PreEndTW(tw) = PreCursorTW(tw) = XmTextGetCursorPosition(w);
  PreUnder(tw) = True;

  /* when overstrike mode, stock text buffer */
  if (tw->text.input->data->overstrike){
    lastPos = (*(tw->text.source->Scan))(tw->text.source,
                                         PreCursorTW(tw),
                                         XmSELECT_LINE, XmsdRight, 1, TRUE);
    PreOverLen(tw) = lastPos - PreCursorTW(tw);
    PreOverStr(tw) = _XmStringSourceGetString(tw, PreCursorTW(tw),
                                              lastPos, False);
  }

  return (-1);
}

/*
 * This is the function set to XNPreeditDoneCallback resource.
 * This function is called when the preedit process is finished.
 */
static void
PreeditDone(XIC xic,
            XPointer client_data,
            XPointer call_data)
{
  char *mb;
  XmTextBlockRec block;
  XmTextWidget tw = (XmTextWidget)client_data;
  Widget w = (Widget)client_data;
  size_t size, num_bytes = 0; /* Wyoming 64-bit fix */ 
  Widget p = w;
  Boolean need_verify, end_preedit = False;

  if (!PreUnder(tw))
    return;

  while (!XtIsShell(p))
    p = XtParent(p);
  XtVaGetValues(p, XmNverifyPreedit, &need_verify, NULL);  
  
  /*
   * Delete preedit string
   */
  if (PreEndTW(tw) > PreStartTW(tw)) {
    if (need_verify) {
	PreeditVerifyReplace(w, PreStartTW(tw), PreEndTW(tw), NULL, 
				PreStartTW(tw), &end_preedit);
	if (end_preedit) return;
    }
    else {
    	block.ptr = NULL;
    	block.length = 0;
    	block.format = XmFMT_8_BIT;
    	(*tw->text.source->Replace)(tw, NULL, &PreStartTW(tw), &PreEndTW(tw), 
				&block, False);
    }
  }

  if (tw->text.input->data->overstrike && PreOverMaxLen(tw) > 0){
    if (PreOverMaxLen(tw) == PreOverLen(tw))
      mb = PreOverStr(tw);
    else {
	int len1;

        mb = XtMalloc((PreOverMaxLen(tw)+1)*tw->text.char_size);
        for (size = PreOverMaxLen(tw); size != 0; size--) /*Wyoming 64-bit fix*/
        {
           len1 = mblen(PreOverStr(tw)+num_bytes, tw->text.char_size);
           num_bytes += len1 == -1 ? 1 : len1;
	}
        memmove (mb, PreOverStr(tw), num_bytes);
        mb[num_bytes] = 0;
    }

    if (need_verify) {
	PreeditVerifyReplace(w, PreStartTW(tw), PreStartTW(tw), mb, 
                                PreStartTW(tw), &end_preedit);
        if (end_preedit) return;	
    }
    else {
	(*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position,
                                                                off);
    	block.ptr = mb;
    	block.length = strlen(block.ptr);
    	block.format = XmFMT_8_BIT;

    	(*tw->text.source->Replace)(tw, NULL, &PreStartTW(tw), &PreStartTW(tw), 
				&block, False);
    	_XmTextPreeditSetCursorPosition(w, PreStartTW(tw));
	(*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position,
                                                                on);
    }

    if (PreOverMaxLen(tw) != PreOverLen(tw))
      XtFree(mb);
    PreOverMaxLen(tw) = PreOverLen(tw) = 0;
    XtFree((char *)PreOverStr(tw));
  }

  PreStartTW(tw) = PreEndTW(tw) = PreCursorTW(tw) = 0;
  PreUnder(tw) = False;
}

/*
 * This function shows the correspondence of rendition data between
 * the input server and XmTextField.
 */
static XmHighlightMode
_XimFeedbackToXmHighlightMode(XIMFeedback fb)
{
  switch (fb) {
  case XIMReverse:
    return(XmHIGHLIGHT_SELECTED);
  case XIMUnderline:
    return(XmHIGHLIGHT_SECONDARY_SELECTED);
  case XIMHighlight:
    return(XmHIGHLIGHT_NORMAL);
  case XIMPrimary:
    return(XmHIGHLIGHT_SELECTED);
  case XIMSecondary:
    return(XmHIGHLIGHT_SECONDARY_SELECTED);
  case XIMTertiary:
    return(XmHIGHLIGHT_SELECTED);
  default:
    return(XmHIGHLIGHT_NORMAL);
  }
}

/*
 * This function treats the rendition data.
 */
static void
PreeditSetRendition(Widget w,
                    XIMPreeditDrawCallbackStruct* data)
{
  XIMText *text = data->text;
  unsigned short cnt;
  XIMFeedback fb;
  XmTextPosition prestart = PreStartTW((XmTextWidget)w)+data->chg_first, left, right;
  XmHighlightMode mode;

  if (!text->length)
    return;

  if (!text->feedback)
    return;

  fb = text->feedback[0];                       /* initial feedback */
  left = right = prestart;                      /* mode start/end position */
  mode = _XimFeedbackToXmHighlightMode(fb);     /* mode */
  cnt = 1;                                      /* counter initialize */

  while (cnt < text->length) {
    if (fb != text->feedback[cnt]) {
      right = prestart + cnt;
      XmTextSetHighlight(w, left, right, mode);

      left = right;                             /* start position update */
      fb = text->feedback[cnt];                 /* feedback update */
      mode = _XimFeedbackToXmHighlightMode(fb);
    }
    cnt++;                                      /* counter increment */
  }
  XmTextSetHighlight(w, left, (prestart + cnt), mode);
                                                /* for the last segment */
}

#define TEXT_MAX_INSERT_SIZE 512

/*
 * This is the function set to XNPreeditDrawCallback resource.
 * This function is called when the input server requests XmText
 * to draw a preedit string.
 */
static void
PreeditDraw(XIC xic,
            XPointer client_data,
            XIMPreeditDrawCallbackStruct *call_data)
{
  Widget w = (Widget) client_data;
  XmTextWidget tw = (XmTextWidget) client_data;
  InputData data = tw->text.input->data;
  XmTextPosition startPos, endPos, rest_len, tmp_end;
  char *mb, *over_mb;
  XmTextBlockRec block;
  unsigned short insert_length=0;
  int i;
  int total_mb_len;
  int recover_len = 0;
  char *ptr;
  OutputData o_data = tw->text.output->data;
  XFontStruct *font = o_data->font;
  XRectangle overall_ink;
  int escapement;
  size_t mb_siz;
  Widget p =w;
  Boolean need_verify, end_preedit = False;

  if (!PreUnder(tw))
    return;

  /* if no data in callback structs simply return - nothing to do */
  if (!call_data->caret && !call_data->chg_first && !call_data->chg_length
      && !call_data->text) 
    return;

  /* have we exceeded max size of preedit buffer? - then punt */
  if (call_data->text &&
      ((insert_length = call_data->text->length) > TEXT_MAX_INSERT_SIZE))
    return;

  if (call_data->chg_length>PreEndTW(tw)-PreStartTW(tw))
    call_data->chg_length = PreEndTW(tw)-PreStartTW(tw);

  /* loop to determine parent shell widget id */
  while (!XtIsShell(p))
    p = XtParent(p);

  /* determine whether verify preedit is set in shell widget */
  XtVaGetValues(p, XmNverifyPreedit, &need_verify, NULL);

  /* turn cursor off */
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, off);
  XmTextSetHighlight(w, PreStartTW(tw)+call_data->chg_first,
                PreStartTW(tw) + call_data->chg_first + call_data->chg_length,
                XmHIGHLIGHT_NORMAL);

  /* preedit deletion */
  if (!data->overstrike && (!call_data->text || !insert_length)){
    startPos = PreStartTW(tw) + call_data->chg_first;
    endPos = startPos + call_data->chg_length;
    PreCursorTW(tw) = startPos;
    PreEndTW(tw) -= endPos - startPos;
    
    if (need_verify) {
      PreeditVerifyReplace(w, startPos, endPos, NULL, startPos, &end_preedit);
      if (end_preedit) {
	(*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, 
					       on);
	return;
      }
    } 
    else {
      block.ptr = NULL;
      block.length = 0;
      block.format = XmFMT_8_BIT;

      if ((*tw->text.source->Replace)(tw, NULL, &startPos, &endPos,
				      &block, False) != EditDone) {
      	  XBell(XtDisplay(tw), 0);
	  return;
      } 
      else 
      	_XmTextPreeditSetCursorPosition(w, PreCursorTW(tw));
    }
    (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on); 
    return;
  }

  /* sanity check data to make sure its *really* there */
  if (call_data->text) 
    if ((call_data->text->encoding_is_wchar &&
	 !call_data->text->string.wide_char) ||
	(!call_data->text->encoding_is_wchar &&
	 !call_data->text->string.multi_byte)){

      PreeditSetRendition(w, call_data);
      (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
      return;
    }

  /* convert text data to char - it may be wchar or char */
  if (insert_length != 0){ /* Wyoming 64-bit fix */ 
    if (o_data->use_fontset) {
      if (call_data->text->encoding_is_wchar){
        mb = XtMalloc((insert_length+1)*tw->text.char_size);
        mb_siz = wcstombs(mb, call_data->text->string.wide_char, 
                  insert_length);
        if (mb_siz < 0)
	   mb_siz = _Xm_wcs_invalid(mb, call_data->text->string.wide_char,
				    insert_length);
      } 
      else {
	mb = XtMalloc((insert_length+1)*tw->text.char_size);
        strcpy(mb,call_data->text->string.multi_byte);
      }

      /* set TextExtents for preedit data, if unable, punt */
      escapement = XmbTextExtents((XFontSet)font, mb, (int)strlen(mb), /* Wyoming 64-bit fix */ 
                                  &overall_ink, NULL );
      if (escapement == 0 && overall_ink.width == 0 &&
          strchr(mb, '\t') == 0 ) {

        XtFree(mb);
	(*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position,
					       on);
        return;
      }
    } 
    else {
      (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
      return;
    }
  }
  else {
    mb = XtMalloc(4);
    mb[0] = '\0';
  }

  /* setup overstrike buffers and data */
  if (data->overstrike) {
    startPos = PreStartTW(tw) + call_data->chg_first;
    tmp_end = (XmTextPosition)(PreEndTW(tw) + insert_length -
                               call_data->chg_length);
    if (PreOverMaxLen(tw) < tmp_end - PreStartTW(tw)){
      if (tmp_end - PreStartTW(tw) > PreOverLen(tw)){
        endPos = startPos + call_data->chg_length;
        PreOverMaxLen(tw) = PreOverLen(tw);
      } 
      else {
        endPos = PreEndTW(tw) + tmp_end - PreStartTW(tw) - PreOverMaxLen(tw);
        PreOverMaxLen(tw) = tmp_end - PreStartTW(tw);
      }
    } 
    else {
      if (PreOverMaxLen(tw) > tmp_end - PreStartTW(tw)){
	endPos = PreEndTW(tw);
	recover_len = PreOverMaxLen(tw) - tmp_end + PreStartTW(tw);
	PreOverMaxLen(tw) = tmp_end - PreStartTW(tw);
      } 
      else
	endPos = startPos + call_data->chg_length;
    }

    rest_len = PreEndTW(tw) - PreStartTW(tw) - call_data->chg_first -
      call_data->chg_length;
    if (rest_len) {
      over_mb = _XmStringSourceGetString(tw, (XmTextPosition)
		  (PreStartTW(tw)+call_data->chg_first+call_data->chg_length),
		  PreEndTW(tw), False);
      mb = XtRealloc(mb, strlen(mb)+strlen(over_mb)+1);
      strcat(mb, over_mb);
      XtFree(over_mb);
    }
    
    if (recover_len > 0) {
      int len1;

      mb = XtRealloc(mb, strlen(mb) + (recover_len + 1 ) * tw->text.char_size);
      ptr = PreOverStr(tw);
      for (i=0; i<PreOverMaxLen(tw); i++)
      {
	len1 = mblen(ptr, 4);
	ptr += len1 == -1 ? 1 : len1;
      }
      total_mb_len = 0;
      for (i=0; i<recover_len; i++)
      {
	len1 = mblen(ptr + total_mb_len, 4);
	total_mb_len += len1 == -1 ? 1 : len1;
      }
      i = strlen(mb);
      strncat(mb, ptr, total_mb_len);
      mb[i+total_mb_len] = '\0';
    }
  } 
  else {
    startPos = PreStartTW(tw) + call_data->chg_first;
    endPos = startPos + call_data->chg_length;
  }

  if (data->overstrike)
      PreEndTW(tw) = startPos + insert_length;
  else
      PreEndTW(tw) += insert_length - endPos + startPos;
  PreCursorTW(tw) = PreStartTW(tw) + call_data->caret;

  /* verify preedit set, so call PreeditVerifyReplace */
  if (need_verify) {
    PreeditVerifyReplace(w, startPos, endPos, mb, 
			 PreCursorTW(tw), &end_preedit);
    if (end_preedit) {
      (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
      return;
    }
  }
  /* no need to verify, just insert text into buffer */
  else {
    block.ptr = mb;
    block.length = strlen(mb);
    block.format = XmFMT_8_BIT;
    if ((*tw->text.source->Replace)(tw, NULL, &startPos, &endPos, 
				    &block, False) != EditDone) {
      XBell(XtDisplay(tw), 0);
      return;
    }
    else 
      _XmTextPreeditSetCursorPosition(w, PreCursorTW(tw));
  }

  /* set feedback */
  if (insert_length != 0) /* Wyoming 64-bit fix */ 
    PreeditSetRendition(w, call_data);

  /* turn cursor back on */
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
  if (mb)
    XtFree(mb);
}

/*
 * This is the function set to XNPreeditCaretCallback resource.
 * This function is called when the input server requests XmText to move
 * the caret.
 */
static void
PreeditCaret(XIC xic,
             XPointer client_data,
             XIMPreeditCaretCallbackStruct *call_data)
{
  XmTextWidget tw = (XmTextWidget) client_data;
  XmSourceData data = tw->text.source->data;
  Widget w = (Widget) client_data;
  XmTextPosition new_position, start = 0;
  Widget p = (Widget) tw;
  Boolean need_verify;

  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, off);
  while (!XtIsShell(p))
    p = XtParent(p);
  XtVaGetValues(p, XmNverifyPreedit, &need_verify, NULL);

  switch (call_data->direction) {
  case XIMForwardChar:
    new_position = PreCursorTW(tw) + 1 - PreStartTW(tw);
    break;
  case XIMBackwardChar:
    new_position = PreCursorTW(tw) - 1 - PreStartTW(tw);
    break;
  case XIMAbsolutePosition:
    new_position = (XmTextPosition) call_data->position;
    break;
  default:
    new_position = PreCursorTW(tw) - PreStartTW(tw);
  }

  _XmTextValidate(&start, &new_position, data->length);
  PreCursorTW(tw) = PreStartTW(tw) + new_position;
  
  if (need_verify) {
    UnderVerifyPreedit(tw) = True;
    _XmTextSetCursorPosition (w, PreCursorTW(tw));
    UnderVerifyPreedit(tw) = False;
  }
  else 
    _XmTextPreeditSetCursorPosition(w, PreCursorTW(tw));
  
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
}

/*
 * Resets input method context.
 *
 * 1. Call XmImMbResetIC to reset the input method and get the current
 *    preedit string.
 * 2. Set the string to XmText
 */
void
_XmTextResetIC(Widget widget)
{
  long escapement, n, size; /* Wyoming 64-bit fix */ 
  char *mb, *tmp_mb;
  XRectangle overall_ink;
  XmTextPosition cursorPos, beginPos, nextPos, lastPos;
  XmTextWidget tw = (XmTextWidget) widget;
  InputData data = tw->text.input->data;
  OutputData o_data = tw->text.output->data;
  XFontStruct *font = o_data->font;

  if (!PreUnder((XmTextWidget) widget))
    return;

  if (VerifyCommitNeeded(tw)) {
    VerifyCommitNeeded(tw) = False;
    mb = _XmStringSourceGetString(tw, PreStartTW(tw), PreEndTW(tw), False);
    XmImMbResetIC(widget, &tmp_mb);
    if (tmp_mb) XtFree(tmp_mb);
  }
  else 
    XmImMbResetIC(widget, &mb);
     
  if (!mb)
    return;
  n = strlen(mb);

  if (n > TEXT_MAX_INSERT_SIZE)
    return;

  if (n > 0) {
    (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, off);
    mb[n]='\0';
    if (o_data->use_fontset) {
      escapement = XmbTextExtents((XFontSet)font, mb, n, &overall_ink, NULL );
      if (escapement == 0 && overall_ink.width == 0 &&
          strchr(mb, '\t') == 0 ) {
        (*tw->text.output->DrawInsertionPoint)(tw,
                                               tw->text.cursor_position, on);
        return;
      }
    } else {
      (*tw->text.output->DrawInsertionPoint)(tw,
                                             tw->text.cursor_position, on);
      return;
    }
    beginPos = nextPos = XmTextGetCursorPosition(widget);

    if (data->overstrike) {
      tmp_mb = XtMalloc((n+1)*tw->text.char_size);
      size = _XmTextBytesToCharacters(tmp_mb, mb, n, False,
                                      tw->text.char_size);
      nextPos += size;
      XtFree(tmp_mb);
      lastPos = (*(tw->text.source->Scan))(tw->text.source,
                                           beginPos, XmSELECT_LINE,
                                           XmsdRight, 1, TRUE);
      if (nextPos > lastPos) nextPos = lastPos;
    }

    _XmTextReplace(widget, beginPos, nextPos, mb, False);
    (*tw->text.output->DrawInsertionPoint)(tw,
                                             tw->text.cursor_position, on);

    XtFree(mb);
  }
}



/****************************************************************
 *
 * Public definitions.
 *
 ****************************************************************/

char * 
XmTextGetString(Widget widget)
{
  char *text_copy;
  _XmWidgetToAppContext(widget);
  
  _XmAppLock(app);
  if (XmIsTextField(widget)) {
    XmAccessTextualTrait textT;
    textT = (XmAccessTextualTrait)
      XmeTraitGet((XtPointer) XtClass(widget), XmQTaccessTextual);
    if (textT)
      text_copy = (char*) textT->getValue(widget, XmFORMAT_MBYTE);
  } else
    text_copy = _XmStringSourceGetValue(GetSrc(widget), False);
  
  _XmAppUnlock(app);
  return (text_copy);
}


wchar_t *
XmTextGetStringWcs(Widget widget)
{
  wchar_t *text_copy;
  _XmWidgetToAppContext(widget);
  
  _XmAppLock(app);
  if (XmIsTextField(widget)){
    XmAccessTextualTrait textT;
    textT = (XmAccessTextualTrait)
      XmeTraitGet((XtPointer) XtClass(widget), XmQTaccessTextual);
    if (textT)
      text_copy = (wchar_t *) textT->getValue(widget, XmFORMAT_WCS);
  }
  else
    text_copy = (wchar_t *) _XmStringSourceGetValue(GetSrc(widget), True);
  _XmAppUnlock(app);
  return (text_copy);
}

void 
XmTextSetString(Widget widget,
		char *value)
{
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  if (XmIsTextField(widget)) {
    XmAccessTextualTrait textT;
    textT = (XmAccessTextualTrait)
      XmeTraitGet((XtPointer) XtClass(widget), XmQTaccessTextual);
    if (textT)
      textT->setValue(widget, (XtPointer)value, XmFORMAT_MBYTE);
  } else 
    _XmTextSetString(widget, value);

  _XmAppUnlock(app);
}

void 
XmTextSetStringWcs(Widget widget,
		   wchar_t *wc_value)
{
  char * tmp;
  int num_chars = 0;
  size_t result;/* Wyoming 64-bit Fix */
  XmTextWidget tw = (XmTextWidget) widget;
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  if(XmIsTextField(widget)) {
    XmAccessTextualTrait textT;
    textT = (XmAccessTextualTrait)
      XmeTraitGet((XtPointer) XtClass(widget), XmQTaccessTextual);
    if (textT)
      textT->setValue(widget, (XtPointer)wc_value, XmFORMAT_WCS);
  } else {

    for (num_chars = 0; wc_value[num_chars] != (wchar_t)0L; num_chars++)
      /*EMPTY*/; 
  
    tmp = XtMalloc((unsigned) (num_chars + 1) * (int)tw->text.char_size);
    result = wcstombs(tmp, wc_value, 
		      (num_chars + 1) * (int)tw->text.char_size);
    
    if (result == (size_t) -1)
      result = _Xm_wcs_invalid(tmp, wc_value,
                      (num_chars + 1) * (int)tw->text.char_size);
    _XmTextSetString(widget, tmp);
    XtFree(tmp);
  }
  _XmAppUnlock(app);
}

XmTextPosition 
XmTextGetTopCharacter(Widget widget)
{
  XmTextWidget tw = (XmTextWidget) widget;
  XmTextPosition ret_val;
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  if (tw->text.needs_refigure_lines)
    RefigureLines(tw);
  ret_val = tw->text.top_character;
  _XmAppUnlock(app);
  return ret_val;
}
    

void 
XmTextSetTopCharacter(Widget widget,
		      XmTextPosition top_character)
{
  XmTextWidget tw = (XmTextWidget) widget;
  _XmWidgetToAppContext(widget);
 
  _XmAppLock(app); 
   if (tw->text.output->data->resizeheight &&
       !(tw->text.output->data->scrollvertical &&
       XmIsScrolledWindow(XtParent(widget))) ) {
    if (tw->text.top_character == 0) {
      _XmAppUnlock(app); 
      return;
    }
    else
      top_character = 0;
  }
  
  _XmTextSetTopCharacter(widget, top_character);
  _XmAppUnlock(app); 
}


XmTextSource 
XmTextGetSource(Widget widget)
{
  XmTextWidget tw = (XmTextWidget) widget;
  XmTextSource ret_val;
  _XmWidgetToAppContext(widget);
  
  _XmAppLock(app);
  ret_val = tw->text.source;
  _XmAppUnlock(app);
  return ret_val;
}

void 
XmTextSetSource(Widget widget,
		XmTextSource source,
		XmTextPosition top_character,
		XmTextPosition cursor_position)
{
  XmTextWidget tw = (XmTextWidget) widget;
  XmTextPosition pos = 0;
  XmTextPosition last_pos = 0;
  XmTextPosition old_pos = 0;
  XmTextBlockRec block;
  int n = 0;
  XPoint xmim_point;
  XRectangle xmim_area;
  Arg args[10];
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  
  _XmTextResetIC(widget);
  EraseInsertionPoint(tw);
  if (source == NULL) {
    XmeWarning(widget, MESSAGE2);
    _XmAppUnlock(app);
    return;
  }
  
  /* zero out old line table */
  block.ptr = NULL;
  block.length = 0;
  _XmTextUpdateLineTable(widget, 0, 0, &block, False);
  tw->text.total_lines = 1;
  
  (*tw->text.source->RemoveWidget)(tw->text.source, tw);
  tw->text.source = source;
  
  if (cursor_position > source->data->length)
    cursor_position = source->data->length;
  else if (cursor_position < 0)
    cursor_position = 0;
  
  tw->text.cursor_position = cursor_position;
  _XmTextMovingCursorPosition(tw, cursor_position); /*correct GC for
						     * new location */
  tw->text.output->data->refresh_ibeam_off = True;
  (*tw->text.source->AddWidget)(tw->text.source, tw);
  _XmStringSourceSetGappedBuffer(source->data, cursor_position);
  if (tw->text.edit_mode == XmMULTI_LINE_EDIT)
    top_character = (*tw->text.source->Scan)(tw->text.source, top_character,
					     XmSELECT_LINE, XmsdLeft, 1, 
					     FALSE);

  tw->text.new_top = top_character;
  tw->text.top_character = 0;
  
  /* reset line table with new source */
  last_pos = (XmTextPosition) source->data->length;
  while (pos < last_pos) {
    pos = (*tw->text.source->ReadSource)(source, pos, last_pos, &block);
    if (block.length == 0)
      break;
    _XmTextUpdateLineTable(widget, old_pos, old_pos, &block, False);
    old_pos = pos;
  }
  
  _XmTextInvalidate(tw, top_character, top_character, NODELTA);
  if (tw->text.disable_depth == 0)
    Redisplay(tw);
  
  /* Tell the input method the new x,y location of the cursor */
  (*tw->text.output->PosToXY)(tw, cursor_position, &xmim_point.x,
			      &xmim_point.y);
  (void)_XmTextGetDisplayRect((Widget)tw, &xmim_area);
  n = 0;
  XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
  XtSetArg(args[n], XmNarea, &xmim_area); n++;
  XmImSetValues((Widget)tw, args, n);
  
  TextDrawInsertionPoint(tw);

  _XmAppUnlock(app);
}


void 
XmTextScroll(Widget widget,
	     int n)
{
  XmTextWidget tw = (XmTextWidget) widget;
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  
  tw->text.pending_scroll += n;
  tw->text.needs_refigure_lines = tw->text.needs_redisplay = TRUE;
  
  if (tw->text.disable_depth == 0) Redisplay(tw);

  _XmAppUnlock(app);
}

void 
XmTextDisableRedisplay(Widget widget)
{
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  _XmTextDisableRedisplay((XmTextWidget)widget, False);
  _XmAppUnlock(app);
}

void 
XmTextEnableRedisplay(Widget widget)
{
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  _XmTextEnableRedisplay((XmTextWidget)widget);
  _XmAppUnlock(app);
}

Widget 
XmCreateScrolledText(Widget parent,
		     char *name,
		     ArgList arglist,
		     Cardinal argcount)
{
  Widget swindow;
  Widget stext;
  Arg args_cache[30];
  ArgList merged_args;
  int n;
  char s_cache[30];
  char *s;
  size_t s_size;/* Wyoming 64-bit Fix */
  size_t arg_size = argcount + 5;/* Wyoming 64-bit Fix */
  _XmWidgetToAppContext(parent);

  _XmAppLock(app);
  s_size = ((name) ? strlen(name) : 0) + 3;
  
  s = (char *) XmStackAlloc(s_size, s_cache);  /* Name + NULL + "SW" */
  if (name) {
    strcpy(s, name);
    strcat(s, "SW");
  } else {
    strcpy(s, "SW");
  }
  
  /*
   * merge the application arglist with the required preset arglist, for
   * creating the scrolled window portion of the scroll text.
   */
  merged_args = (ArgList)XmStackAlloc(arg_size*sizeof(Arg), args_cache);
  for (n=0; n < argcount; n++) {
    merged_args[n].name = arglist[n].name;
    merged_args[n].value = arglist[n].value;
  }
  XtSetArg(merged_args[n], XmNscrollingPolicy,
	   (XtArgVal) XmAPPLICATION_DEFINED); n++;
  XtSetArg(merged_args[n], XmNvisualPolicy, (XtArgVal)XmVARIABLE); n++;
  XtSetArg(merged_args[n], XmNscrollBarDisplayPolicy, (XtArgVal)XmSTATIC); n++;
  XtSetArg(merged_args[n], XmNshadowThickness, (XtArgVal) 0); n++;
  
  swindow = XtCreateManagedWidget(s, xmScrolledWindowWidgetClass, parent,
				  merged_args, n);
  XmStackFree(s, s_cache);
  XmStackFree((char *)merged_args, args_cache);
  
  /* Create Text widget.  */
  stext = XtCreateWidget(name, xmTextWidgetClass, swindow, arglist, argcount);
  
  /* Add callback to destroy ScrolledWindow parent. */
  XtAddCallback (stext, XmNdestroyCallback, _XmDestroyParentCallback, NULL);

  _XmAppUnlock(app);
  /* Return Text.*/
  return (stext);
}

Widget 
XmCreateText(Widget parent,
	     char *name,
	     ArgList arglist,
	     Cardinal argcount)
{
  return XtCreateWidget(name, xmTextWidgetClass, parent, arglist, argcount);
}

