/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)XDrawingArea.c	1.5 03/12/19
 */

#include <X11/IntrinsicP.h>
#include "XDrawingAreaP.h"
#include <Xm/XmP.h>

#include <stdio.h>
#include <malloc.h>

#ifdef DEBUG
#include <jvm.h>  /* To get jio_fprintf() */
#endif 

/******************************************************************
 * 
 * Provides Canvas widget which allows the X11 visual to be
 * changed (the Motif DrawingArea restricts the visual to that
 * of the parent widget).
 *
 ******************************************************************/


static XmNavigability WidgetNavigable();
static void ClassInitialize();

static XmBaseClassExtRec baseClassExtRec = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    NULL,				/* InitializePrehook	*/
    NULL,				/* SetValuesPrehook	*/
    NULL,				/* InitializePosthook	*/
    NULL,				/* SetValuesPosthook	*/
    NULL,				/* secondaryObjectClass	*/
    NULL,				/* secondaryCreate	*/
    NULL,               		/* getSecRes data	*/
    { 0 },      			/* fastSubclass flags	*/
    NULL,				/* getValuesPrehook	*/
    NULL,				/* getValuesPosthook	*/
    NULL,                               /* classPartInitPrehook */
    NULL,                               /* classPartInitPosthook*/
    NULL,                               /* ext_resources        */
    NULL,                               /* compiled_ext_resources*/
    0,                                  /* num_ext_resources    */
    FALSE,                              /* use_sub_resources    */
    WidgetNavigable,	                /* widgetNavigable      */
    NULL                                /* focusChange          */
};

XDrawingAreaClassRec xDrawingAreaClassRec = {
{
    /* Core class part */

    /* superclass         */    (WidgetClass)&xmDrawingAreaClassRec,
    /* class_name         */    "XDrawingArea",
    /* widget_size        */    sizeof(XDrawingAreaRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_initialize*/  NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    NULL,
    /* num_resources      */    0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/    FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry       */  NULL,
    /* display_accelerator  */  NULL,
    /* extension            */  (XtPointer)&baseClassExtRec 
  },

   {		/* composite_class fields */
      XtInheritGeometryManager,	                /* geometry_manager   */
      XtInheritChangeManaged,	               	/* change_managed     */
      XtInheritInsertChild,			/* insert_child       */
      XtInheritDeleteChild,     		/* delete_child       */
      NULL,                                     /* extension          */
   },

   {		/* constraint_class fields */
      NULL,					/* resource list        */   
      0,					/* num resources        */   
      0,					/* constraint size      */   
      NULL,					/* init proc            */   
      NULL,					/* destroy proc         */   
      NULL,					/* set values proc      */   
      NULL,                                     /* extension            */
   },

   {		/* manager_class fields */
      XtInheritTranslations,			/* translations           */
      NULL,					/* syn_resources      	  */
      0,					/* num_get_resources 	  */
      NULL,					/* syn_cont_resources     */
      0,					/* num_get_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL,					/* extension           */    
   },

   {		/* drawingArea class */     
	   /* extension	*/	NULL
   },
  
   /* XDrawingArea class part */
   {
	/* extension	*/	NULL
   }
};

WidgetClass xDrawingAreaClass = (WidgetClass)&xDrawingAreaClassRec;

static void ClassInitialize( void )
{
    baseClassExtRec.record_type = XmQmotif ;
}

static XmNavigability WidgetNavigable(Widget wid)
{
    return XmCONTROL_NAVIGABLE;
}


