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
static char rcsid[] = "$XConsortium: TextFSel.c /main/19 1996/12/12 09:53:51 drk $"
#endif
#endif
/* (c) Copyright 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <X11/Xatom.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragC.h>
#include <Xm/TraitP.h>		/* for XmeTraitSet() */
#include <Xm/TransferP.h>
#include <Xm/TransferT.h>
#include <Xm/XmosP.h>
#include "TextFI.h"
#include "TextFSelI.h"
#include "TransferI.h"		/* for _XmConvertComplete() */
#include "TraversalI.h"		/* for _XmGetFocusPolicy() */
#include "XmI.h"
#ifdef SUN_CTL
#include "XmRenderTI.h"		/* for _XmRend...() */
#endif /* CTL */

/********    Static Function Declarations    ********/

static void InsertSelection( 
                        Widget w,
                        XtPointer closure,
                        Atom *seltype,
                        Atom *type,
                        XtPointer value,
                        unsigned long *length,
                        int *format,
			XtPointer tid) ;
static void HandleInsertTargets(
                        Widget w,
                        XtPointer closure,
                        Atom *seltype,
                        Atom *type,
                        XtPointer value,
                        unsigned long *length,
                        int *format,
			XtPointer tid);
static void HandleDrop(Widget w,
		       XmDropProcCallbackStruct *cb,
		       XmDestinationCallbackStruct *ds);
static void TextFieldConvertCallback(Widget, 
				     XtPointer, 
				     XmConvertCallbackStruct*);

static void TextFieldDestinationCallback(Widget, 
					 XtPointer, 
					 XmDestinationCallbackStruct*);
static void SetDropContext(Widget w);

static void DeleteDropContext(Widget w);

/* leob fix for bug 4191799 */
static void HandleTargets(Widget w, 
			  XtPointer ignore, 
			  XmSelectionCallbackStruct *ds);

static void HandleDrop(Widget w,
		       XmDropProcCallbackStruct *cb,
		       XmDestinationCallbackStruct *ds);
     
static void DropDestroyCB(Widget w,
			  XtPointer clientData,
			  XtPointer callData);

static void DropTransferProc(Widget w, 
			     XtPointer ignore, 
			     XmSelectionCallbackStruct *ds);
static void DoStuff(Widget w, 
		    XtPointer ignore, 
		    XmSelectionCallbackStruct *ds);

/* leob fix for bug 4191799 */
extern Atom _XmTextGetEncodingAtom(Widget w);


/********    End Static Function Declarations    ********/


/* Transfer Trait record for TextField */

static XmConst XmTransferTraitRec textFieldTT = {
  0,  				/* version */
  (XmConvertCallbackProc) 	TextFieldConvertCallback,
  (XmDestinationCallbackProc)	TextFieldDestinationCallback,
  (XmDestinationCallbackProc)	NULL,
};

static XContext _XmTextFDNDContext = 0;
static _XmInsertSelect insert_select;
static _XmTextPrimSelect *prim_select;
 
/*ARGSUSED*/
static void
SetPrimarySelection(Widget w, 
		    XtEnum op,	/* unused */
		    XmTransferDoneCallbackStruct *ts) /* unused */
{
  XmTextFieldWidget tf = (XmTextFieldWidget) w;
  XmTextPosition cursorPos = tf->text.cursor_position;

  _XmProcessLock();
  if (!prim_select) {
    _XmProcessUnlock();
    return;
  }

  if (prim_select->num_chars > 0) {
    tf->text.prim_anchor = prim_select->position;
    cursorPos = prim_select->position + prim_select->num_chars;
    _XmTextFieldStartSelection(tf, tf->text.prim_anchor, cursorPos,
			       prim_select->time);
    tf->text.pending_off = False;
    _XmTextFieldSetCursorPosition(tf, NULL, cursorPos, True, True);
  }
  if (--prim_select->ref_count == 0) {
    XtFree((char *)prim_select);
    prim_select = NULL;
  }
  _XmProcessUnlock();
}


/*ARGSUSED*/
static void
CleanPrimarySelection(Widget w, 
		    XtEnum op,	/* unused */
		    XmTransferDoneCallbackStruct *ts) /* unused */
{
  _XmProcessLock();
  if (!prim_select)
  {
    _XmProcessUnlock();
    return;
  }

  if (--prim_select->ref_count == 0) {
    XtFree((char *)prim_select);
    prim_select = NULL;
  }
  _XmProcessUnlock();
}


static void 
TextFieldSecondaryWrapper(Widget w, XtPointer closure,
			     XmSelectionCallbackStruct *ds)
{
  Atom XA_TARGETS = XInternAtom(XtDisplay(w), XmSTARGETS, False);

  if (ds -> target == XA_TARGETS)
    HandleInsertTargets(w, closure, &(ds -> selection), &(ds -> type),
			ds -> value, &(ds -> length), &(ds -> format),
			ds -> transfer_id);
  else
    InsertSelection(w, closure, &(ds -> selection), &(ds -> type),
		    ds -> value, &(ds -> length), &(ds -> format),
		    ds -> transfer_id);
}

/* ARGSUSED */
static void
InsertSelection(
        Widget w,
        XtPointer closure,
        Atom *seltype,
        Atom *type,
        XtPointer value,
        unsigned long *length,
        int *format,
	XtPointer tid)
{
  _XmInsertSelect *insert_select = (_XmInsertSelect *) closure;
  XmTextFieldWidget tf = (XmTextFieldWidget) w;
  XmTextPosition left = 0;
  XmTextPosition right = 0;
  Boolean replace_res = False;
  Boolean dest_disjoint = False;
  wchar_t * wc_value;
  char * temp;
  long num_chars = 0; /* Wyoming 64-bit fix */
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  char * total_value = NULL;
  XmAnyCallbackStruct cb;
  
  if (!value) {
    insert_select->done_status = True;
    return;
  }
  
  /* Don't do replace if there is not text to add */
  if (*(char *) value == (char)'\0' || *length == 0){
    XtFree((char *)value);
    insert_select->done_status = True;
    return;
  }
  
  if (insert_select->select_type == XmPRIM_SELECT) {
    if (!tf->text.has_primary ||
	tf->text.prim_pos_left == tf->text.prim_pos_right) {
      XBell(XtDisplay(w), 0);
      XtFree((char *)value);
      insert_select->done_status = True;
      insert_select->success_status = False;
      return;
    } 
  } else if (insert_select->select_type == XmDEST_SELECT) {
    if (tf->text.has_primary &&
	(left = tf->text.prim_pos_left) != (right = tf->text.prim_pos_right)) {
      if ( TextF_CursorPosition(tf) < left ||
	  TextF_CursorPosition(tf) > right ||
	  !tf->text.pending_delete) {
	left = right = TextF_CursorPosition(tf);
	dest_disjoint = True;
      }
    } else
      left = right = TextF_CursorPosition(tf);
  }
  
  
  if (*type == COMPOUND_TEXT || *type == XA_STRING) {
    total_value =  _XmTextToLocaleText(w, value, *type, *format, 
				       *length, NULL);
    if (total_value) {
      if (tf->text.max_char_size == 1) {
	num_chars = strlen(total_value);
	replace_res = _XmTextFieldReplaceText(tf, 
					      (XEvent *)insert_select->event,
					      left, right, total_value, 
					      num_chars, True);
      } else { /* must convert to wchar_t before passing to Replace */
	long len = strlen(total_value) + 1; /* Wyoming 64-bit fix */
	wc_value = (wchar_t *)XtMalloc((size_t) len * sizeof(wchar_t));
	num_chars = mbstowcs(wc_value, total_value, (int)len);
	if (num_chars < 0) 
	  num_chars = _Xm_mbs_invalid(wc_value, total_value, (int)len);
	replace_res = _XmTextFieldReplaceText(tf,
                                                (XEvent *)insert_select->event,
                                                left, right, (char*) wc_value,
                                                num_chars, True);
	XtFree((char *)wc_value);
      }
      XtFree(total_value);
    }
  } else { /* it must be either TEXT or codeset of the locale */
    if (tf->text.max_char_size == 1) {
      /* NOTE: casting *length could result in a truncated long. */
      num_chars = *length;
      replace_res = _XmTextFieldReplaceText(tf, 
					    (XEvent *)insert_select->event,
					    left, right, (char *)value,
					    (size_t)*length, True); /* Wyoming 64-bit fix */
    } else {
      temp = XtMalloc((size_t) *length + 1); /* Wyoming 64-bit fix */
      /* NOTE: casting *length could result in a truncated long. */
      (void)memcpy((void*)temp, (void*)value, (size_t)*length);
      temp[*length] = '\0';
      wc_value = (wchar_t*)XtMalloc((size_t) /* Wyoming 64-bit fix */
				    (*length + 1) * sizeof(wchar_t));
      
      /* NOTE: casting *length could result in a truncated long. */
      num_chars = mbstowcs(wc_value, temp, (size_t)*length + 1); /* Wyoming 64-bit fix */
      if (num_chars < 0)
	num_chars = _Xm_mbs_invalid(wc_value, temp, (size_t)*length + 1);
      replace_res = _XmTextFieldReplaceText(tf, 
					      (XEvent *)insert_select->event,
					      left, right, (char*) wc_value,
					      num_chars, True);
      XtFree(temp);
      XtFree((char *)wc_value);
    }
  }
  
  if (!replace_res) {
    insert_select->success_status = False;
  } else {
    insert_select->success_status = True;
    
    if (!tf->text.add_mode) tf->text.prim_anchor = left;
    
    tf->text.pending_off = True;
    _XmTextFieldSetCursorPosition(tf, NULL, left + num_chars, False, True);
    (void) _XmTextFieldSetDestination(w, TextF_CursorPosition(tf),
				      insert_select->event->time);
    if (insert_select->select_type == XmDEST_SELECT) {
      if (left != right) {
	if (!dest_disjoint || !tf->text.add_mode) {
	  _XmTextFieldStartSelection(tf, TextF_CursorPosition(tf),
				     TextF_CursorPosition(tf),
				     insert_select->event->time);
	}
      }
    }
    cb.reason = XmCR_VALUE_CHANGED;
    cb.event = (XEvent *)insert_select->event;
    XtCallCallbackList((Widget) tf, TextF_ValueChangedCallback(tf),
		       (XtPointer) &cb);
  }
  
  XtFree((char *)value);
  value = NULL;
  insert_select->done_status = True;
}

/* ARGSUSED */
static void
HandleInsertTargets(
        Widget w,
        XtPointer closure,
        Atom *seltype,
        Atom *type,
        XtPointer value,
        unsigned long *length,
        int *format,
	XtPointer tid )
{
  _XmInsertSelect *insert_select = (_XmInsertSelect *) closure;
  Atom TEXT = XInternAtom(XtDisplay(w), XmSTEXT, False);
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom CS_OF_ENCODING = XmeGetEncodingAtom(w);
  Atom target;
  Atom *atom_ptr;
  Boolean supports_encoding_data = False;
  Boolean supports_CT = False;
  Boolean supports_text = False;
  int i;

  if (0 == *length) {
    XtFree((char *)value);
    insert_select->done_status = True;
    return; /* Supports no targets, so don't bother sending anything */
  }
  
  atom_ptr = (Atom *)value;
  
  for (i = 0; i < *length; i++, atom_ptr++) {
    if (*atom_ptr == TEXT)
      supports_text = True;
    
    if (*atom_ptr == CS_OF_ENCODING) 
      supports_encoding_data = True;
    
    if (*atom_ptr == COMPOUND_TEXT) 
      supports_CT = True;
  }
  
  /* The order of choice has been changed so that COMPOUND_TEXT     *
   * takes precedence over TEXT - fix for bug 4117589 - leob. Also  *
   * incoperate fix for bug  4103273 that was only applied to Text  *
   * Widget encodeing must come before COMPOUND_TEXT for OW - leob  */
  if (supports_encoding_data && !supports_text)
    target = CS_OF_ENCODING;
  else if (supports_CT)
    target = COMPOUND_TEXT;
  else if (supports_text && supports_encoding_data)
    target = TEXT;
  else
    target = XA_STRING;

  XmTransferValue(tid, target,
		  (XtCallbackProc) TextFieldSecondaryWrapper,
		  closure, insert_select -> event -> time);
}

#ifdef SUN_CTL
int _XmTextFieldConvertVisual(char 		*tmp_value, 
			      XmTextPosition     tmp_length,
			      XmTextFieldWidget  tf, 
			      XmTextPosition	 left, 
			      XmTextPosition	 right)
{
  wchar_t    *tmp_wc;
  Status      status;
  Position    left_x, right_x, tmp_x;
  int         num_chars_return, i;
  
  Boolean     is_wchar      = (tf->text.max_char_size > 1);
  XOC         xoc           = (XOC)_XmRendFont(TextF_Rendition(tf));
  size_t      length        = tf->text.string_length;
  int         alloca_size   = (length + 1) * sizeof(XSegment);
  XSegment   *logical_array = (XSegment*)ALLOCATE_LOCAL(alloca_size);
  char       *text          = is_wchar ? (char*)TextF_WcValue(tf) : (char*)TextF_Value(tf);
  int         num_collected = 0;
	
  status = XocTextPerCharExtents(xoc, text, is_wchar, length, NULL, logical_array, 
				 length, &num_chars_return, NULL, NULL);

  if (!status) {
      XmeWarning((Widget)tf, "Error in XocTextPerCharExtents in TextFSel.c\n");
  }

  if (is_wchar)
    tmp_wc = (wchar_t*)ALLOCATE_LOCAL((length + 1) * sizeof(wchar_t));

  /* 
     Note that these values returned by both _XmTextFieldFindPixelPosition
     and XocTextPerCharExtents are relative to a starting x offset of 0.
   */
  left_x  = _XmTextFieldFindPixelPosition(tf, text, left, XmEDGE_LEFT);
  right_x = _XmTextFieldFindPixelPosition(tf, text, right, XmEDGE_LEFT);

  if (left_x > right_x) {
    tmp_x   = left_x;
    left_x  = right_x;
    right_x = tmp_x;
  }

  /* Left and right don't actually matter here, except insofar
     as they determine the visual/physical bounds of the selection.  
     Any character in the string can potentially fall in the selction,
     so we have to check them all. */
  for (i = 0; i < length; i++) {
    int x1 = logical_array[i].x1;
    int x2 = logical_array[i].x2;
    
    if (!((x1 < left_x) || (x2 < left_x) || (x1 > right_x) || (x2 > right_x)))
	/* If the logical character's rectangle is not outside of our visual 
	   limits, then add it to tmp_value. */
      if (is_wchar)
	tmp_wc[num_collected++] = ((wchar_t*)text)[i];
      else
	if (num_collected < tmp_length)
	  tmp_value[num_collected++] = ((char*)text)[i];
  }
  
  if (is_wchar) {
    tmp_wc[num_collected] = 0L;
    /* caller needs mbs in tmp_value */
    num_collected = wcstombs(tmp_value, tmp_wc, tmp_length);
    if (num_collected < 0)
       num_collected = _Xm_wcs_invalid(tmp_value, tmp_wc, tmp_length);
  }

  DEALLOCATE_LOCAL((char*) logical_array);
  return num_collected;
}
#endif /* CTL */

/* ARGSUSED */
Boolean
_XmTextFieldConvert(
        Widget w,
        Atom *selection,
        Atom *target,
        Atom *type,
        XtPointer *value,
        unsigned long *length,
        int *format,
	Widget drag_context,
        XEvent *event)
{
  XmTextFieldWidget tf;
  Atom MOTIF_DESTINATION = XInternAtom(XtDisplay(w),
				       XmS_MOTIF_DESTINATION, False);
  Atom INSERT_SELECTION = XInternAtom(XtDisplay(w),
				      XmSINSERT_SELECTION, False);
  Atom DELETE = XInternAtom(XtDisplay(w), XmSDELETE, False);
  Atom TARGETS = XInternAtom(XtDisplay(w), XmSTARGETS, False);
  Atom TEXT = XInternAtom(XtDisplay(w), XmSTEXT, False);
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom TIMESTAMP = XInternAtom(XtDisplay(w), XmSTIMESTAMP, False);
  Atom MOTIF_DROP = XInternAtom(XtDisplay(w), XmS_MOTIF_DROP, False);
  Atom CS_OF_ENCODING = XmeGetEncodingAtom(w);
  Atom CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);
  XSelectionRequestEvent * req_event = (XSelectionRequestEvent *) event;
  Boolean has_selection = False;
  XmTextPosition left = 0;
  XmTextPosition right = 0;
  Boolean is_primary;
  Boolean is_secondary;
  Boolean is_destination;
  Boolean is_drop;
  int target_count = 0;
  XTextProperty tmp_prop;
  char *tmp_value;
  int ret_status = 0;
  Time _time;
  XmAnyCallbackStruct cb;
  
  if (req_event == NULL) 
    _time = XtLastTimestampProcessed(XtDisplay(w));
  else
    _time = req_event -> time;
  
  tf = (XmTextFieldWidget) w;
  
  if (tf == NULL) return False;
  
  if (*selection == XA_PRIMARY || *selection == CLIPBOARD) {
    has_selection = tf->text.has_primary;
    left = tf->text.prim_pos_left;
    right = tf->text.prim_pos_right;
    is_primary = True;
    is_secondary = is_destination = is_drop = False;
  } else if (*selection == MOTIF_DESTINATION) {
    has_selection = tf->text.has_destination;
    is_destination = True;
    is_secondary = is_primary = is_drop = False;
  } else if (*selection == XA_SECONDARY) {
    has_selection = tf->text.has_secondary;
    left = tf->text.sec_pos_left;
    right = tf->text.sec_pos_right;
    is_secondary = True;
    is_destination = is_primary = is_drop = False;
  } else if (*selection == MOTIF_DROP) {
    has_selection = tf->text.has_primary;
    left = tf->text.prim_pos_left;
    right = tf->text.prim_pos_right;
    is_drop = True;
    is_destination = is_primary = is_secondary = False;
  } else
    return False;
  
  /*
   * TARGETS identifies what targets the textfield widget can
   * provide data for.
   */
  if (*target == TARGETS) {
    Atom *targs = XmeStandardTargets(w, 10, &target_count);
    
    *value = (XtPointer) targs;
    if (XA_STRING != CS_OF_ENCODING) {
      targs[target_count] = CS_OF_ENCODING;  target_count++;
    }
    if (is_primary || is_destination) {
      targs[target_count] = INSERT_SELECTION; target_count++;
    }
    if (is_primary || is_secondary || is_drop) {
      targs[target_count] = COMPOUND_TEXT; target_count++;
      targs[target_count] = TEXT; target_count++;
      targs[target_count] = XA_STRING; target_count++;
    }
    if (is_primary || is_drop) {
      targs[target_count] = DELETE; target_count++;
    }
    *type = XA_ATOM;
    *length = target_count;
    *format = 32;
  } else if (*target == TIMESTAMP) {
    Time *timestamp;
    timestamp = (Time *) XtMalloc(sizeof(Time));
    if (is_primary)
      *timestamp = tf->text.prim_time;
    else if (is_destination)
      *timestamp = tf->text.dest_time;
    else if (is_secondary)
      *timestamp = tf->text.sec_time;
    else if (is_drop)
      *timestamp = tf->text.prim_time;
    *value = (XtPointer) timestamp;
    *type = XA_INTEGER;
    *length = sizeof(Time) / 4;
    *format = 32;
  } else if (*target == XA_STRING) {
    *type = (Atom) XA_STRING;
    *format = 8;
    if (is_destination || !has_selection) return False;

    /* put a char* value into tmp_value, then convert to 8859.1 */
    if (tf->text.max_char_size != 1) {
      long stat ; /* Wyoming 64-bit fix */
      
#ifdef SUN_CTL_NYI
      if((tf->text.edit_policy == XmEDIT_VISUAL) &&
	 (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	*length = CTL_MAX_BUF_SIZE;
      else 
#endif /* CTL */
      /* NOTE: casting (right - left) could result in a truncated long. */
	  *length = _XmTextFieldCountBytes(tf, TextF_WcValue(tf) + left, 
				       (int)(right - left));
      
      /* get the selection value */
      tmp_value = XtMalloc((unsigned) *length + 1);
#ifdef SUN_CTL_NYI
      if ((tf->text.edit_policy                 == XmEDIT_VISUAL) && 
	  (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	stat = _XmTextFieldConvertVisual(tmp_value, *length, tf, left, right);
      else
#endif  /* CTL */
      if (*length > 0)
        stat = wcstombs(tmp_value, TextF_WcValue(tf) + left,
		      (size_t)*length); /* Wyoming 64-bit fix */ /* NOTE: casting *length could
					     result in a truncated long. */
      else
        stat = 0;

      if (stat < 0) /* wcstombs will return neg value on conv failure */
	stat = _Xm_wcs_invalid(tmp_value, TextF_WcValue(tf) + left,
				(size_t)*length);
      *length = (unsigned long) stat ;
    } else {
	
#ifdef SUN_CTL_NYI
	if((tf->text.edit_policy == XmEDIT_VISUAL) &&
	   (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	  *length = CTL_MAX_BUF_SIZE;
	else
#endif /* CTL */
	    *length = right - left;
	
	tmp_value = XtMalloc((unsigned) *length + 1);
	/* get the selection value */
#ifdef SUN_CTL_NYI
      if ((tf->text.edit_policy                 == XmEDIT_VISUAL) &&
	  (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	*length = _XmTextFieldConvertVisual(tmp_value, *length, tf, left, right);
      else
#endif  /* CTL */
      (void)memcpy((void*)tmp_value, (void*)(TextF_Value(tf) + left), 
		   (size_t)*length); /* NOTE: casting *length could result
					  in a truncated long. */
    }
    tmp_value[*length] = '\0';
    tmp_prop.value = NULL;
    /* convert tmp_value to 8859.1 */
    ret_status = XmbTextListToTextProperty(XtDisplay(w), &tmp_value, 1, 
					   (XICCEncodingStyle)XStringStyle,
					   &tmp_prop);
    XtFree(tmp_value);
    if (ret_status == Success || ret_status > 0){
      *value = (XtPointer) tmp_prop.value;
      *length = tmp_prop.nitems;
    } else {
      *value = NULL;
      *length = 0;
      return False;
    }
  } else if (*target == TEXT || *target == CS_OF_ENCODING) {
    *type = CS_OF_ENCODING;
    *format = 8;
    if (is_destination || !has_selection) return False;
    if (tf->text.max_char_size != 1) {
      long stat ; /* Wyoming 64-bit fix */
      
      	
#ifdef SUN_CTL_NYI
	if((tf->text.edit_policy == XmEDIT_VISUAL) &&
	   (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	  *length = CTL_MAX_BUF_SIZE;
	else 
#endif /* CTL */
      /* NOTE: casting (right - left) could result in a truncated long. */
      *length = _XmTextFieldCountBytes(tf, TextF_WcValue(tf) + left,
				       (int)(right - left));
      
      *value = XtMalloc((unsigned) *length + 1);
#ifdef SUN_CTL_NYI
      if ((tf->text.edit_policy                 == XmEDIT_VISUAL) && 
	  (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	stat = _XmTextFieldConvertVisual((char*)*value, *length, tf, left, right);
      else
#endif  /* CTL */
      if (*length > 0) 
        stat = wcstombs((char *)*value, TextF_WcValue(tf) + left,
		      (size_t)*length); /* Wyoming 64-bit fix *//* NOTE: casting *length could 
					     result in a truncated long */
      else 
        stat = 0;
      if (stat < 0) /* wcstombs return neg value on conv failure */
	 stat = _Xm_wcs_invalid((char *)*value, TextF_WcValue(tf) + left,
				(size_t)*length);
      *length = (unsigned long) stat ;
    } else {
	
		
#ifdef SUN_CTL_NYI
	if((tf->text.edit_policy == XmEDIT_VISUAL) &&
	   (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	  *length = CTL_MAX_BUF_SIZE;
	else 
#endif /* CTL */
	    *length = right - left;
	
      *value = XtMalloc((unsigned) *length + 1);
      /* get the selection value */
#ifdef SUN_CTL_NYI
      if ((tf->text.edit_policy                 == XmEDIT_VISUAL) && 
	  (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	*length = _XmTextFieldConvertVisual((char*)*value, *length, tf, left, right);
      else
#endif  /* CTL */
      (void)memcpy((void*)*value, (void*)(TextF_Value(tf) + left),
		   (size_t)*length); /* NOTE: casting *length could result
					  in a truncated long. */
    }
    (*(char **)value)[*length]='\0';
  } else if (*target == COMPOUND_TEXT) {
    *type = COMPOUND_TEXT;
    *format = 8;
    if (is_destination || !has_selection) return False;
    if (tf->text.max_char_size != 1) { 
      long stat ; /* Wyoming 64-bit fix */
      
      	
#ifdef SUN_CTL_NYI
	if((tf->text.edit_policy == XmEDIT_VISUAL) &&
	   (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	  *length = CTL_MAX_BUF_SIZE;
	else
#endif /* CTL */
      /* convert to char* before converting to CT.  NOTE: casting
       * (right - left) could result in a truncated long.
       */
      *length = _XmTextFieldCountBytes(tf, TextF_WcValue(tf) + left,
				       (int)(right - left));
      
      tmp_value = XtMalloc((unsigned) *length + 1);
#ifdef SUN_CTL_NYI
      if ((tf->text.edit_policy                 == XmEDIT_VISUAL) && 
	  (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	stat = _XmTextFieldConvertVisual(tmp_value, *length, tf, left, right);
      else
#endif  /* CTL */
      if (*length > 0)
        stat = wcstombs(tmp_value, TextF_WcValue(tf) + left,
		      (size_t)*length);  /* Wyoming 64-bit fix *//* NOTE: casting *length could
					     result in a truncated long. */
      else
        stat = 0;
      if (stat < 0) /* wcstombs will return neg value on conv failure */
	 stat = _Xm_wcs_invalid(tmp_value, TextF_WcValue(tf) + left,
				(size_t)*length);
      *length = (unsigned long) stat ;
    } else { /* malloc the space and copy the data to be converted */
#ifdef SUN_CTL_NYI
	if((tf->text.edit_policy == XmEDIT_VISUAL) &&
	(_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	  *length = CTL_MAX_BUF_SIZE;
	else 
#endif /* CTL */
      *length = right - left;
	
      tmp_value = XtMalloc((unsigned) *length + 1);
      /* get the selection value */
#ifdef SUN_CTL_NYI
      if ((tf->text.edit_policy                 == XmEDIT_VISUAL) &&
	  (_XmRendFontType(TextF_Rendition(tf)) == XmFONT_IS_XOC))
	*length = _XmTextFieldConvertVisual(tmp_value, *length, tf, left, right);
      else
#endif  /* CTL */
      (void)memcpy((void*)tmp_value, (void*)(TextF_Value(tf) + left), 
		   (size_t)*length); /* NOTE: casting *length could result
					  in a truncated long. */
    }
    tmp_value[*length] = '\0';
    tmp_prop.value = NULL;
    /* Convert to compound text */
    ret_status = 
      XmbTextListToTextProperty(XtDisplay(w), &tmp_value, 1,
				(XICCEncodingStyle)XCompoundTextStyle,
				&tmp_prop);
    XtFree(tmp_value);
    if (ret_status == Success || ret_status > 0){
      *length = tmp_prop.nitems;
      *value = (XtPointer)tmp_prop.value;
    } else {
      *value = NULL;
      *length = 0;
      return False;
    }
  } else if (*target == INSERT_SELECTION) {
    if (is_secondary) 
      return False;
    else
      return True;
    /* Delete the selection */
  } else if (*target == DELETE) {
    XmTextPosition left, right;
    Boolean move_cursor = True;
    
    if (!(is_primary || is_drop)) return False;
    
    left = tf->text.prim_pos_left;
    right = tf->text.prim_pos_right;
    
      if (is_drop) {
	if (_XmTextFieldGetDropReciever((Widget)tf) == (Widget) tf)
	  move_cursor = False;
      } else {
	if (req_event != NULL &&
	    req_event->requestor == XtWindow((Widget)tf))
	  move_cursor = False;
      }
    
    if (!_XmTextFieldReplaceText(tf, (XEvent *) req_event,
				 left, right, NULL, 0, move_cursor)) {
      tf->text.has_primary = True;
      return False;
    }
    
    _XmTextFieldStartSelection(tf, tf->text.prim_anchor,
			       tf->text.prim_anchor, _time);
    
    cb.reason = XmCR_VALUE_CHANGED;
    cb.event = (XEvent *) req_event;
    XtCallCallbackList((Widget) tf, TextF_ValueChangedCallback(tf),
		       (XtPointer) &cb);
    
    tf->text.has_primary = True;
    
    if (tf->text.has_destination)
      tf->text.prim_anchor = TextF_CursorPosition(tf);
    
    *type = XInternAtom(XtDisplay(w), XmSNULL, False);
    *value = NULL;
    *length = 0;
    *format = 8;
  } else
    /* unknown selection type */
    return FALSE;
  return TRUE;
}

/* ARGSUSED */
void
_XmTextFieldLoseSelection(
        Widget w,
        Atom *selection )
{
    XmTextFieldWidget tf = (XmTextFieldWidget) w;
    Atom MOTIF_DESTINATION = XInternAtom(XtDisplay(w),
                                        XmS_MOTIF_DESTINATION, False);
/* Losing Primary Selection */
    if (*selection == XA_PRIMARY && tf->text.has_primary) {
        XmAnyCallbackStruct cb;
        _XmTextFieldDeselectSelection(w, False, 0);

        cb.reason = XmCR_LOSE_PRIMARY;
        cb.event = NULL;
        XtCallCallbackList(w, tf->text.lose_primary_callback, (XtPointer) &cb);
/* Losing Destination Selection */
    } else if (*selection == MOTIF_DESTINATION) {
        Boolean orig_ibeam_off = tf->text.refresh_ibeam_off;

        tf->text.has_destination = False;
       /* if we have focus, we have a valid putback area.  If we don't have
	* focus, don't want to update the putback with the destination cursor
	* image.
	*/
	tf->text.refresh_ibeam_off = False;

	_XmTextFieldDrawInsertionPoint(tf, False);
	
	tf->text.blink_on = True;
	_XmTextFieldDrawInsertionPoint(tf, True);
	/* Restore the state of the refresh_ibeam_off flag. */
        tf->text.refresh_ibeam_off = orig_ibeam_off;

/* Losing Secondary Selection */
    } else if (*selection == XA_SECONDARY && tf->text.has_secondary){
        _XmTextFieldSetSel2(w, 0, 0, True, 
			    XtLastTimestampProcessed(XtDisplay(w)));
    }
}


static void
SetDropContext(Widget w)
{
  Display *display = XtDisplay(w);
  Screen *screen = XtScreen(w);
  XContext loc_context;
  

  _XmProcessLock();
  if (_XmTextFDNDContext == 0)
    _XmTextFDNDContext = XUniqueContext();
  loc_context = _XmTextFDNDContext;
  _XmProcessUnlock();
  
  XSaveContext(display, (Window)screen,
	       loc_context, (XPointer)w);
}

static void
DeleteDropContext(Widget w)
{
  Display *display = XtDisplay(w);
  Screen *screen = XtScreen(w);
  XContext loc_context;
  
  _XmProcessLock();
  loc_context = _XmTextFDNDContext;
  _XmProcessUnlock();

  XDeleteContext(display, (Window)screen, loc_context);
}

Widget
_XmTextFieldGetDropReciever(Widget w)
{
  Widget widget;
  XContext loc_context;

  _XmProcessLock();
  loc_context = _XmTextFDNDContext;
  _XmProcessUnlock();

  if (loc_context == 0) return NULL;
  
  if (!XFindContext(XtDisplay(w), (Window) XtScreen(w),
		    loc_context, (char **) &widget)) {
    return widget;
  } 
  
  return NULL;
}


/* ARGSUSED */
static void
DropDestroyCB(Widget      w,
	      XtPointer   clientData,
	      XtPointer   callData)
{
  XmTransferDoneCallbackStruct *ts = 
    (XmTransferDoneCallbackStruct *) callData;
  
  DeleteDropContext(w);
  if (ts->client_data != NULL) XtFree((char*) ts->client_data);
}

static void 
DropTransferProc(Widget w, XtPointer closure,
		 XmSelectionCallbackStruct *ds)
{
  _XmTextDropTransferRec *transfer_rec = (_XmTextDropTransferRec *) closure;
  XmTextFieldWidget tf = (XmTextFieldWidget) w;
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom CS_OF_ENCODING = XmeGetEncodingAtom(w);
  XmTextPosition insertPosLeft, insertPosRight, left, right, cursorPos;
  int max_length = 0;
  Boolean local = tf->text.has_primary;
  char * total_value = NULL;
  wchar_t * wc_total_value;
  long total_length = 0, org_length = 0; 
  Boolean replace = False;
  XmAnyCallbackStruct cb;

  /* When type = NULL, we are assuming a DELETE request has been requested */
  if (ds->type == XInternAtom(XtDisplay(w), XmSNULL, False)) {
    if (transfer_rec->num_chars > 0 && transfer_rec->move) {
      tf->text.prim_anchor = transfer_rec->insert_pos;
      cursorPos = transfer_rec->insert_pos + transfer_rec->num_chars;
      _XmTextFieldSetCursorPosition(tf, NULL, cursorPos,
				    False, True);
      _XmTextFieldStartSelection(tf, tf->text.prim_anchor, 
				 TextF_CursorPosition(tf),
				 XtLastTimestampProcessed(XtDisplay(w)));
      tf->text.pending_off = False;
      _XmTextFieldSetCursorPosition(tf, NULL, TextF_CursorPosition(tf),
				    True, True);
    }
    if (ds->value) {
      XtFree((char*) ds->value);
      ds->value = NULL;
    }
    return;
  }
  
  if (!(ds->value) || 
      (ds->type != CS_OF_ENCODING &&
       ds->type != COMPOUND_TEXT &&
       ds->type != XA_STRING)) {
    XmTransferDone(ds->transfer_id, XmTRANSFER_DONE_FAIL);
    if (ds->value) {
      XtFree((char*) ds->value);
      ds->value = NULL;
    }
    return;
  }
  
  insertPosLeft = insertPosRight = transfer_rec->insert_pos;
  
  if (ds->type == XA_STRING || ds->type == COMPOUND_TEXT) {
    if ((total_value = _XmTextToLocaleText(w, ds->value, ds->type, 
					   8, ds->length, NULL)) != NULL)
      org_length = total_length = strlen(total_value);
    else
      if (ds->value) {
	XtFree((char*) ds->value);
	ds->value = NULL;
      }
  } else {
    total_value = (char*) ds->value;
    org_length = total_length = ds->length;
  }

  
  if (total_value == NULL) return;
  
  if (TextF_PendingDelete(tf) && tf->text.has_primary &&
      tf->text.prim_pos_left != tf->text.prim_pos_right &&
      insertPosLeft > tf->text.prim_pos_left &&
      insertPosRight < tf->text.prim_pos_right) {
    insertPosLeft = tf->text.prim_pos_left;
    insertPosRight = tf->text.prim_pos_right;
  }
  
  transfer_rec->num_chars = _XmTextFieldCountCharacters(tf, total_value, 
							(int)total_length);
  
  _XmTextFieldDrawInsertionPoint(tf, False);
  
  if (transfer_rec->move && local) {
    max_length = TextF_MaxLength(tf);
    TextF_MaxLength(tf) = INT_MAX;
  }
  
  if (tf->text.max_char_size == 1) {
    replace = _XmTextFieldReplaceText(tf, ds->event, insertPosLeft, 
				      insertPosRight, (char *) total_value,
				      (int)total_length, False);
  } else {
    wc_total_value = (wchar_t*)XtMalloc((size_t) /* Wyoming 64-bit fix */
					total_length * sizeof(wchar_t));
    /* Note: casting total_length to an int may result in a truncation. */
    total_length = mbstowcs(wc_total_value, total_value,
			    (int)total_length);
    if (total_length < 0)
      total_length = _Xm_mbs_invalid(wc_total_value, total_value,
                            (int)org_length);  /* fix for bug 4277497 - leob */
    if (total_length != 0)
      replace = _XmTextFieldReplaceText(tf, ds->event, insertPosLeft, 
					insertPosRight, (char *)wc_total_value,
					(int)total_length, False);
    XtFree((char*)wc_total_value);
  }
  
  if (replace) {
    tf->text.pending_off = FALSE;
    if (transfer_rec->num_chars > 0 && !transfer_rec->move) {
      cursorPos = transfer_rec->insert_pos + transfer_rec->num_chars;
      _XmTextFieldSetCursorPosition(tf, NULL, cursorPos, 
				    True, True);
      _XmTextFieldSetDestination((Widget)tf, TextF_CursorPosition(tf),
				 transfer_rec->timestamp);
    }
    left = tf->text.prim_pos_left;
    right = tf->text.prim_pos_right;
    if (tf->text.has_primary) {
      if (transfer_rec->move && left < transfer_rec->insert_pos)
	transfer_rec->insert_pos -= transfer_rec->num_chars;
      if (TextF_CursorPosition(tf) < left ||
	  TextF_CursorPosition(tf) > right)
	tf->text.pending_off = TRUE;
    } else {
      if (!transfer_rec->move && !tf->text.add_mode &&
	  transfer_rec->num_chars != 0)
	tf->text.prim_anchor = insertPosLeft;
    }
    if (transfer_rec->move) {
      XmTransferValue(ds->transfer_id, 
		      XInternAtom(XtDisplay(w),XmSDELETE,False),
		      (XtCallbackProc) DropTransferProc, 
		      (XtPointer) transfer_rec, 0);
    }
    cb.reason = XmCR_VALUE_CHANGED;
    cb.event = ds->event;
    XtCallCallbackList((Widget) tf, TextF_ValueChangedCallback(tf),
		       (XtPointer) &cb);
  }
  
  if (transfer_rec->move && local) {
    TextF_MaxLength(tf) = max_length;
  }
  
  if (total_value && (total_value != (char*)ds->value))
    XtFree(total_value);
  if (ds->value) {
    XtFree((char*) ds->value);
    ds->value = NULL;
  }
  
  _XmTextFieldDrawInsertionPoint(tf, True);
}


/*ARGSUSED*/
static void
DoStuff(Widget w, 
	XtPointer closure,	/* unused */
	XmSelectionCallbackStruct *ds)
{
  XmTextFieldWidget tf = (XmTextFieldWidget) w;
  Atom NULL_ATOM = XInternAtom(XtDisplay(w), XmSNULL, False);
  Atom CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);
  XmTextPosition right=0, left=0, replace_from, replace_to;
  int prim_char_length = 0;
  Boolean replace_res = False;
  XmAnyCallbackStruct cb;

  if (!tf->text.has_focus && _XmGetFocusPolicy(w) == XmEXPLICIT)
    (void) XmProcessTraversal(w, XmTRAVERSE_CURRENT);
  
  if (ds->selection != CLIPBOARD && ds->length == 0 && ds->type != NULL_ATOM) {
    /* Backwards compatibility for 1.0 Selections */
    _XmProcessLock();
    if (prim_select->target == XInternAtom(XtDisplay(w), XmSTEXT, False)) {
      prim_select->target = XA_STRING;
      XmTransferValue(ds->transfer_id, XA_STRING, (XtCallbackProc) DoStuff,
		      (XtPointer) prim_select, prim_select->time);
    }
    _XmProcessUnlock();
    XtFree((char *)ds->value);
    ds->value = NULL;
    return;
  }
  
  /* if ds->length == 0 and ds->type is the NULL atom we are assuming
   * that a DELETE target is requested.
   */
  if (ds->type == NULL_ATOM) {
    _XmProcessLock();
    if (prim_select->num_chars > 0 && tf->text.selection_move) {
      prim_char_length = prim_select->num_chars;
      _XmTextFieldStartSelection(tf, prim_select->position,
				 prim_select->position + prim_char_length,
				 prim_select->time);
      tf->text.pending_off = False;
      _XmTextFieldSetCursorPosition(tf, NULL, 
				    prim_select->position + prim_char_length, 
				    True, True);
      tf->text.prim_anchor = prim_select->position;
    }
    _XmProcessUnlock(); /* for prim_select */
  } else {
    int max_length = 0;
    Boolean local = tf->text.has_primary;
    Boolean dest_disjoint = True;

    if (tf->text.selection_move && local) {
      max_length = TextF_MaxLength(tf);
      TextF_MaxLength(tf) = INT_MAX;
    }
    _XmProcessLock();
    replace_from = replace_to = prim_select->position;
    _XmProcessUnlock();
    if (ds->selection == CLIPBOARD) {
      if (tf->text.has_primary) {
	left = tf->text.prim_pos_left;
	right = tf->text.prim_pos_right;
	if (tf->text.pending_delete &&
	    replace_from >= left && replace_to <= right) {
          replace_from = left;
          replace_to = right;
          dest_disjoint = False;
	}
      }
    }

    if (ds->type == XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False) ||
	ds->type == XA_STRING) {
      char *total_value;
      
      if ((total_value = 
	   _XmTextToLocaleText(w, ds->value, ds->type, ds->format, ds->length, 
			       NULL))
	  != NULL) {
	if (tf->text.max_char_size == 1) {
	  _XmProcessLock();
	  prim_select->num_chars = (int)strlen(total_value); /* Wyoming 64-bit fix */
	  replace_res = _XmTextFieldReplaceText(tf, ds->event, 
						replace_from,
						replace_to, 
						total_value,
						prim_select->num_chars,
						ds->selection == CLIPBOARD);
	  _XmProcessUnlock();
	  XtFree(total_value);
	} else {
	  wchar_t * wc_value;
	  size_t tmp_len = strlen(total_value) + 1;

	  _XmProcessLock();
	  prim_select->num_chars = 0;
	  wc_value = (wchar_t*)XtMalloc ((size_t) tmp_len * sizeof(wchar_t));
	  prim_select->num_chars = mbstowcs(wc_value, total_value, (unsigned)tmp_len);
	  if (prim_select->num_chars < 0) 
	    prim_select->num_chars = _Xm_mbs_invalid(wc_value, total_value,
					(unsigned)tmp_len);
	  replace_res = _XmTextFieldReplaceText(tf, ds->event, 
						  replace_from,
						  replace_to, 
						  (char*)wc_value, 
						  prim_select->num_chars,
						  ds->selection == CLIPBOARD);
	  _XmProcessUnlock();
	  XtFree((char*)wc_value);
	  XtFree(total_value);
	}
      } else { /* initialize prim_select values for possible delete oper */
	_XmProcessLock();
	prim_select->num_chars = 0;
	_XmProcessUnlock();
      }
    } else {
      if (tf->text.max_char_size == 1) {
	/* Note: length may be truncated during cast to int */
	_XmProcessLock();
	prim_select->num_chars = (int) ds->length;
	replace_res = _XmTextFieldReplaceText(tf, ds->event, 
					      replace_from,
					      replace_to, 
					      (char *) ds->value, 
					      prim_select->num_chars,
					      ds->selection == CLIPBOARD);
	_XmProcessUnlock();
      } else {
	wchar_t * wc_value;
	
	wc_value = (wchar_t*)XtMalloc ((size_t)
				       (ds->length * sizeof(wchar_t)));
	_XmProcessLock();
	prim_select->num_chars = mbstowcs(wc_value, (char *) ds->value,
					  (unsigned) ds->length);
	if (prim_select->num_chars < 0) 
	  prim_select->num_chars = _Xm_mbs_invalid(wc_value, (char*)ds->value,
						(unsigned) ds->length);
	replace_res = _XmTextFieldReplaceText(tf, ds->event, 
						replace_from,
						replace_to, 
						(char*)wc_value, 
						prim_select->num_chars,
						ds->selection == CLIPBOARD);
	_XmProcessUnlock();
	XtFree((char*)wc_value);
      }
    }
    
    if (replace_res) {
      XmTextPosition cursorPos;
      
      if (ds->selection != CLIPBOARD) { 
	tf->text.pending_off = FALSE;
	_XmProcessLock();
	cursorPos = replace_from + prim_select->num_chars; 
	if (prim_select->num_chars > 0 && !tf->text.selection_move) {
	  _XmTextFieldSetCursorPosition(tf, NULL, cursorPos, 
					True, True);
	  (void) _XmTextFieldSetDestination(w, cursorPos, prim_select->time);
	_XmProcessUnlock();
	}
      } else {
	_XmProcessLock();
	(void) _XmTextFieldSetDestination(w, TextF_CursorPosition(tf), 
					  prim_select->time);
	_XmProcessUnlock();
      }
      left = tf->text.prim_pos_left;
      right = tf->text.prim_pos_right;
      if (tf->text.has_primary) {
	if (ds->selection == CLIPBOARD) {
	  if (left != right && (!dest_disjoint || !tf->text.add_mode))
	    _XmProcessLock();
	    _XmTextFieldStartSelection(tf, TextF_CursorPosition(tf),
				       TextF_CursorPosition(tf),
				       prim_select->time);
	    _XmProcessUnlock();
	} else {
	  _XmProcessLock();
	  if (tf->text.selection_move && left < prim_select->position)
	    prim_select->position -= prim_select->num_chars;
	  if (left <= cursorPos && right >= cursorPos)
	    tf->text.pending_off = TRUE;
	  _XmProcessUnlock();
	}
      } else {
	_XmProcessLock();
	if (ds->selection == CLIPBOARD)
	  tf->text.prim_anchor = replace_from;
	else if (!tf->text.selection_move && !tf->text.add_mode &&
		 prim_select->num_chars != 0)
	  tf->text.prim_anchor = prim_select->position;
	_XmProcessUnlock();
      }
      cb.reason = XmCR_VALUE_CHANGED;
      cb.event = ds->event;
      XtCallCallbackList((Widget) tf, TextF_ValueChangedCallback(tf),
			 (XtPointer) &cb);
    } else {
      _XmProcessLock();
      prim_select->num_chars = 0; /* Stop SetPrimarySelection from doing
				     anything */
      _XmProcessUnlock();
    }
    
    if (tf->text.selection_move && local) {
      TextF_MaxLength(tf) = max_length;
    }
  }
  
  XtFree((char *)ds->value);
  ds->value = NULL;
}

static void 
HandleTargets(Widget w, XtPointer closure,
	      XmSelectionCallbackStruct *ds)
{
  XmTextFieldWidget tf = (XmTextFieldWidget) w;
  Atom CS_OF_ENCODING = _XmTextGetEncodingAtom(w); /* leob fix for bug 4191799 */
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);
  Atom TEXT = XInternAtom(XtDisplay(w), XmSTEXT, False);
  XmTextPosition left, right;
  Boolean supports_encoding_data = False;
  Boolean supports_CT = False;
  Boolean supports_text = False;
  XPoint *point = (XPoint *)closure;
  Atom *atom_ptr;
  Atom targets[2];
  XmTextPosition select_pos;
  int i;
  
  if (!ds->length) {
    XtFree((char *)ds->value);
    ds->value = NULL;
    return; /* Supports no targets, so don't bother sending anything */
  }
  
  atom_ptr = (Atom *)ds->value;
  
  for (i = 0; i < ds->length; i++, atom_ptr++) {
    if (*atom_ptr == TEXT)
      supports_text = True;

    if (*atom_ptr == CS_OF_ENCODING) 
      supports_encoding_data = True;

    if (*atom_ptr == COMPOUND_TEXT)
      supports_CT = True;
  }
  
  
  /*
   * Set stuff position to the x and y position of
   * the button pressed event for primary pastes.
   */
  if (ds->selection != CLIPBOARD && point) {
    select_pos = XmTextFieldXYToPos((Widget)tf, (Position)point->x, 0);
  } else {
    select_pos = TextF_CursorPosition(tf);
  }
  
  if (ds->selection != CLIPBOARD) {
    left = tf->text.prim_pos_left;
    right = tf->text.prim_pos_right;
    if (tf->text.has_primary &&
	left != right && select_pos > left && select_pos < right) {
      XtFree((char *)ds->value);
      ds->value = NULL;
      return;
    }
  }

  _XmProcessLock();
  if (prim_select) {
    prim_select->ref_count++;
  } else {
    prim_select = (_XmTextPrimSelect *)
      XtMalloc((unsigned) sizeof(_XmTextPrimSelect));
  }
  prim_select->position = select_pos;
  prim_select->time = XtLastTimestampProcessed(XtDisplay(w));
  prim_select->num_chars = 0;
  
  /* The order of choice has been changed so that COMPOUND_TEXT     *
   * takes precedence over TEXT - fix for bug 4117589 - leob. Also  *
   * incoperate fix for bug  4103273 that was only applied to Text  *
   * Widget encodeing must come before COMPOUND_TEXT for OW - leob  */

  if (supports_encoding_data && !supports_text)
    prim_select->target = targets[0] = CS_OF_ENCODING;
  else if (supports_CT)
    prim_select->target = targets[0] = COMPOUND_TEXT;
  else if (supports_text && supports_encoding_data)
    prim_select->target = targets[0] = TEXT;
  else
    prim_select->target = targets[0] = XA_STRING;
  
  prim_select->ref_count = 1;
  /* Make request to call DoStuff() with the primary selection. */
  XmTransferValue(ds->transfer_id, targets[0], (XtCallbackProc) DoStuff, 
		  (XtPointer) prim_select, prim_select->time);
  _XmProcessUnlock();
  XtFree((char *)ds->value);
  ds->value = NULL;
}

static void
HandleDrop(Widget w,
	   XmDropProcCallbackStruct *cb,
	   XmDestinationCallbackStruct *ds)
{
  Widget drag_cont, initiator;
  Cardinal numExportTargets, n;
  Atom *exportTargets;
  Atom desiredTarget = None;
  Arg args[10];
  XmTextPosition insert_pos, left, right;
  Display *display = XtDisplay(w);
  Boolean doTransfer = False;
  _XmTextDropTransferRec *transfer_rec;
  XtPointer tid = ds->transfer_id;
  
  drag_cont = cb->dragContext;
  transfer_rec = (_XmTextDropTransferRec *) NULL;
  
  n = 0;
  XtSetArg(args[n], XmNsourceWidget, &initiator); n++;
  XtSetArg(args[n], XmNexportTargets, &exportTargets); n++;
  XtSetArg(args[n], XmNnumExportTargets, &numExportTargets); n++;
  XtGetValues((Widget) drag_cont, args, n);
  
  insert_pos = XmTextFieldXYToPos(w, cb->x, 0);
  
  left = ((XmTextFieldWidget)w)->text.prim_pos_left;
  right = ((XmTextFieldWidget)w)->text.prim_pos_right;
  if (cb->operation & XmDROP_MOVE && w == initiator && 
      ((XmTextFieldWidget)w)->text.has_primary &&
      left != right && insert_pos >= left && insert_pos <= right) {
    /*EMPTY*/
  } else {
    Atom TEXT = XInternAtom(display, XmSTEXT, False);
    Atom COMPOUND_TEXT = XInternAtom(display, XmSCOMPOUND_TEXT, False);
    Atom CS_OF_ENCODING = XmeGetEncodingAtom(w);
    Boolean encoding_found = False;
    Boolean c_text_found = False;
    Boolean string_found = False;
    Boolean text_found = False;
    
    /* intialize data to send to drop transfer callback */
    transfer_rec = (_XmTextDropTransferRec *)
      XtMalloc(sizeof(_XmTextDropTransferRec));
    transfer_rec->widget = w;
    transfer_rec->insert_pos = insert_pos;
    transfer_rec->num_chars = 0;
    transfer_rec->timestamp = cb->timeStamp;
    transfer_rec->move = False;
    
    if (cb->operation & XmDROP_MOVE) {
      transfer_rec->move = True;
    } else {
      transfer_rec->move = False;
    }
    
    for (n = 0; n < numExportTargets; n++) {
      if (exportTargets[n] == CS_OF_ENCODING) {
	desiredTarget = CS_OF_ENCODING;
	encoding_found = True;
	break;
      }
      if (exportTargets[n] == COMPOUND_TEXT) c_text_found = True;
      if (exportTargets[n] == XA_STRING) string_found = True;
      if (exportTargets[n] == TEXT) text_found = True;
    }
    
    n = 0;
    if (encoding_found || c_text_found || string_found || text_found) {
      if (!encoding_found) {
	if (c_text_found)
	  desiredTarget = COMPOUND_TEXT;
	else if (string_found)
	  desiredTarget = XA_STRING;
	else
	  desiredTarget = TEXT;
      }
      
      if (cb->operation & XmDROP_MOVE || cb->operation & XmDROP_COPY) {
	doTransfer = True;
      } else {
	XmTransferDone(tid, XmTRANSFER_DONE_FAIL);
      }
      
    } else {
      XmTransferDone(tid, XmTRANSFER_DONE_FAIL);
    }
  }
  SetDropContext(w);
  
  if (doTransfer) {
    XmeTransferAddDoneProc(tid, (XmSelectionFinishedProc) DropDestroyCB);
    XmTransferValue(tid, desiredTarget,
		    (XtCallbackProc) DropTransferProc,
		    (XtPointer) transfer_rec, 0);
  }
}


/*ARGSUSED*/
static void
TextFieldConvertCallback(Widget w, 
			 XtPointer ignore, /* unused */
			 XmConvertCallbackStruct *cs)
{
  Atom XA_CS_OF_ENCODING = XmeGetEncodingAtom(w);
  XtPointer value;
  Atom type;
  unsigned long size;
  int format;
  Atom XA_DELETE = XInternAtom(XtDisplay(w), XmSDELETE, False);
  Atom XA_MOTIF_LOSE = XInternAtom(XtDisplay(w), 
				   XmS_MOTIF_LOSE_SELECTION, False);
  Atom XA_MOTIF_EXPORTS = XInternAtom(XtDisplay(w), 
				      XmS_MOTIF_EXPORT_TARGETS, False);
  Atom XA_MOTIF_REQUIRED = XInternAtom(XtDisplay(w), 
				       XmS_MOTIF_CLIPBOARD_TARGETS, False);
  Atom XA_COMPOUND_TEXT =  XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom XA_TEXT =  XInternAtom(XtDisplay(w), XmSTEXT, False);
  Atom XA_TARGETS = XInternAtom(XtDisplay(w), XmSTARGETS, False);
  Atom XA_CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);

  value = NULL;
    
  if (cs->target == XA_MOTIF_LOSE) {
    _XmTextFieldLoseSelection(w, &(cs->selection));
    cs->status = XmCONVERT_DONE;
    return;
  }
  
  if (cs->target == XA_DELETE &&
      cs->selection == XA_SECONDARY) {
    _XmTextFieldHandleSecondaryFinished(w, cs->event);
    cs->status = XmCONVERT_DONE;
    return;
  }
  
  /* When this is called as a result of a clipboard copy link,  we
     don't have any available targets.  Make sure to return immediately
     without modification */
  if (cs->selection == XA_CLIPBOARD &&
      cs->parm == (XtPointer) XmLINK &&
      (cs->target == XA_MOTIF_REQUIRED ||
       cs->target == XA_TARGETS)) return;

  if (!_XmTextFieldConvert(w, &cs->selection, &cs->target,
			   &type, &value, &size, &format,
			   (Widget) cs->source_data, cs->event)) {
    value = NULL;
    type = XA_INTEGER;
    size = 0;
    format = 8;
  }
  
  if (cs->target == XA_DELETE) {
    cs->status = XmCONVERT_DONE;
    cs->type = type;
    cs->value = value;
    cs->length = size;
    cs->format = format;
    return;
  }
  
  if (cs->target == XA_MOTIF_EXPORTS ||
      cs->target == XA_MOTIF_REQUIRED) {
    Atom *targs = (Atom *) XtMalloc(sizeof(Atom) * 4);
    int n = 0;

    value = (XtPointer) targs;
    targs[n] = XA_COMPOUND_TEXT; n++;
    targs[n] = XA_TEXT; n++;
    targs[n] = XA_STRING; n++;
    if (XA_CS_OF_ENCODING != XA_STRING) {
      targs[n] = XA_CS_OF_ENCODING; n++;
    }
    format = 32;
    size = n;
    type = XA_ATOM;
  }

  _XmConvertComplete(w, value, size, format, type, cs);
}

/************************************************
 * Free data allocated for destination callback 
 ************************************************/

/*ARGSUSED*/
static void
FreeLocationData(Widget w,     /* unused */
		 XtEnum op,    /* unused */
		 XmTransferDoneCallbackStruct *ts) 
{
  XmDestinationCallbackStruct *ds;

  ds = _XmTransferGetDestinationCBStruct(ts->transfer_id);

  XtFree((char*) ds->location_data);

  ds->location_data = NULL;
}

/*ARGSUSED*/
static void 
TextFieldDestinationCallback(Widget w, 
			     XtPointer closure, /* unused */
			     XmDestinationCallbackStruct *ds)
{
  Atom XA_TARGETS = XInternAtom(XtDisplay(w), XmSTARGETS, False);
  Atom XA_MOTIF_DROP = XInternAtom(XtDisplay(w), XmS_MOTIF_DROP, False);
  XPoint DropPoint;

  /*
   ** In case of a primary transfer operation where a location_data
   ** has been allocated, register a done proc to be called when 
   ** the data transfer is complete to free the location_data
   */
  if (ds->selection == XA_PRIMARY && ds->location_data)
      XmeTransferAddDoneProc(ds->transfer_id, FreeLocationData);

  /* If we aren't sensitive,  don't allow transfer */
  if (! w -> core.sensitive ||
      ! w -> core.ancestor_sensitive) 
    XmTransferDone(ds -> transfer_id, XmTRANSFER_DONE_FAIL);

  /* We don't handle LINKs internally */
  if (ds->operation == XmLINK) return;

  if (ds->selection == XA_PRIMARY && ds->operation == XmMOVE)
    XmeTransferAddDoneProc(ds->transfer_id, SetPrimarySelection);
  else
    XmeTransferAddDoneProc(ds->transfer_id, CleanPrimarySelection);
    
  if (ds->selection == XA_MOTIF_DROP) {
    XmDropProcCallbackStruct *cb = 
      (XmDropProcCallbackStruct *) ds->destination_data;
    
    DropPoint.x = cb->x;
    DropPoint.y = cb->y;
    
    ds->location_data = (XtPointer) &DropPoint;
    
    if (cb->dropAction != XmDROP_HELP) {
      HandleDrop(w, cb, ds);
    }
  } else if (ds->selection == XA_SECONDARY) {
    Atom CS_OF_ENCODING;
    
    CS_OF_ENCODING = XmeGetEncodingAtom(w);

    _XmProcessLock();
    insert_select.done_status = False;
    insert_select.success_status = False;
    insert_select.event = (XSelectionRequestEvent *) ds->event;
    insert_select.select_type = XmDEST_SELECT;
    
    if (((Atom) ds->location_data) != CS_OF_ENCODING) {
      /*
       * Make selection request to find out which targets
       * the selection can provide.
       */
      XmTransferValue(ds->transfer_id, XA_TARGETS, 
		      (XtCallbackProc) TextFieldSecondaryWrapper,
		      (XtPointer) &insert_select, ds->time);
    } else {
      /*
       * Make selection request to replace the selection
       * with the insert selection.
       */
      XmTransferValue(ds->transfer_id, ((Atom) ds->location_data),
		      (XtCallbackProc) TextFieldSecondaryWrapper,
		      (XtPointer) &insert_select, ds->time);
    }
    _XmProcessUnlock();
  } else 
    /* CLIPBOARD or PRIMARY */
    XmTransferValue(ds->transfer_id, XA_TARGETS,
		    (XtCallbackProc) HandleTargets, 
		    ds->location_data, ds->time);
}

void
_XmTextFieldInstallTransferTrait(void)
{
  XmeTraitSet((XtPointer)xmTextFieldWidgetClass, XmQTtransfer, 
	      (XtPointer) &textFieldTT);
}

