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
static char rcsid[] = "$XConsortium: TextSel.c /main/18 1996/12/12 09:53:26 drk $"
#endif
#endif
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <X11/Xatom.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragC.h>
#include <Xm/DropTrans.h>
#include <Xm/TraitP.h>		/* for XmeTraitSet() */
#include <Xm/TransferT.h>
#include <Xm/XmosP.h>
#include "TextI.h"
#include "TextInI.h"
#include "TextSelI.h"
#include "TextStrSoI.h"
#include "TransferI.h"		/* for _XmConvertComplete() */
#include "TraversalI.h"
#include "XmI.h"
#ifdef SUN_CTL
#include "XmRenderTI.h"		/* for _XmRendFontType() */
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
			XtPointer tid) ;

/* leob fix for bug 4191799 */
Atom _XmTextGetEncodingAtom(Widget w);

static void HandleDrop(Widget w,
		       XmDropProcCallbackStruct *cb,
		       XmDestinationCallbackStruct *ds);

static void HandleTargets(Widget w, 
			  XtPointer ignore, 
			  XmSelectionCallbackStruct *ds);

static void DoStuff(Widget w, 
		    XtPointer closure, 
		    XmSelectionCallbackStruct *ds);

static void DropDestroyCB(Widget w,
			  XtPointer clientData,
			  XtPointer callData);

static void DropTransferProc(Widget w, XtPointer ignore, 
			     XmSelectionCallbackStruct *ds);

static void SetDropContext(Widget w);

static void DeleteDropContext(Widget w);
static void TextSecondaryWrapper(Widget, XtPointer, 
				 XmSelectionCallbackStruct *);
static void TextConvertCallback(Widget, XtPointer, 
				XmConvertCallbackStruct*);
static void TextDestinationCallback(Widget, XtPointer,
				    XmDestinationCallbackStruct*);

/********    End Static Function Declarations    ********/

/* Transfer Trait record for Text */

static XmConst XmTransferTraitRec TextTransfer = {
  0,  				/* version */
  (XmConvertCallbackProc) 	TextConvertCallback,
  (XmDestinationCallbackProc)	TextDestinationCallback,
  (XmDestinationCallbackProc)	NULL,
};

static XContext _XmTextDNDContext = 0;
static _XmTextPrimSelect *prim_select;
static _XmInsertSelect insert_select;

/*ARGSUSED*/
static void
SetPrimarySelection(Widget w, 
		    XtEnum op,	/* unused */
		    XmTransferDoneCallbackStruct *ts) /* unused */
{
  XmTextWidget tw = (XmTextWidget) w;
  InputData data = tw->text.input->data;
  XmTextPosition cursorPos = tw->text.cursor_position;
 
  _XmProcessLock();
  if (!prim_select) {
    _XmProcessUnlock();
    return;
  }

  if (prim_select->num_chars > 0) {
    data->anchor = prim_select->position;
    cursorPos = prim_select->position + prim_select->num_chars;
    _XmTextSetCursorPosition(w, cursorPos);
    _XmTextSetDestinationSelection(w, tw->text.cursor_position,
				   False, prim_select->time);
    (*tw->text.source->SetSelection)(tw->text.source, data->anchor,
				     tw->text.cursor_position,
				     prim_select->time);
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
  if (!prim_select) {
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
TextSecondaryWrapper(Widget w, XtPointer closure,
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
  _XmInsertSelect *insert_select = (_XmInsertSelect *)closure;
  XmTextWidget tw = (XmTextWidget) w;
  XmTextPosition left = 0;
  XmTextPosition right = 0;
  Boolean dest_disjoint = False;
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  char * total_value = NULL;
  XmTextBlockRec block, newblock;
  XmTextPosition cursorPos;
  Boolean freeBlock;
  
  if (!value) {
    insert_select->done_status = True;
    return;
  }
  
  /* Don't do replace if there is no text to add */
  if (*(char *)value == '\0' || *length == 0){
    XtFree((char*)value);
    value = NULL;
    insert_select->done_status = True;
    return;
  }
  
  if (insert_select->select_type == XmPRIM_SELECT) {
    if (!(*tw->text.source->GetSelection)(tw->text.source, &left, &right) ||
	left == right) {
      XBell(XtDisplay(w), 0);
      XtFree((char*)value);
      value = NULL;
      insert_select->done_status = True;
      insert_select->success_status = False;
      return;
    }
  } else if (insert_select->select_type == XmDEST_SELECT) {
    if ((*tw->text.source->GetSelection)(tw->text.source, &left, &right) && 
	left != right) {
      if (tw->text.cursor_position < left ||
	  tw->text.cursor_position > right ||
	  tw->text.pendingoff) {
	left = right = tw->text.cursor_position; 
	dest_disjoint = True;
      }
    } else
      left = right = tw->text.cursor_position;
  }
  
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, off);
  
  block.format = XmFMT_8_BIT;
  
  if (*type == COMPOUND_TEXT || *type == XA_STRING) {
    if ((total_value =
	 _XmTextToLocaleText(w, value, *type, *format, 
			     *length, NULL)) != NULL) {
      block.ptr = total_value;
      block.length = (int)strlen(block.ptr); /* Wyoming 64-bit fix */ 
    } else {
      insert_select->done_status = True;
      insert_select->success_status = False;
      (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
      return;
    }
  } else {  /* it must be either CS_OF_ENCODING or TEXT */
    block.ptr = (char*)value;
    /* NOTE: casting *length could result in a truncated long. */
    block.length = (int) *length; /* Wyoming 64-bit fix */ 
    block.format = XmFMT_8_BIT;
  }
  
  if (_XmTextModifyVerify(tw, (XEvent *)insert_select->event, &left, &right,
			  &cursorPos, &block, &newblock, &freeBlock)) {
    if ((*tw->text.source->Replace)(tw, (XEvent *)insert_select->event, 
				    &left, &right,
				    &newblock, False) != EditDone) {
      if (tw->text.verify_bell) XBell(XtDisplay(w), 0);
      insert_select->success_status = False;
    } else {
      insert_select->success_status = True;
      
      if (!tw->text.add_mode) tw->text.input->data->anchor = left;
      
      if (tw->text.add_mode && cursorPos >= left && cursorPos <= right)
	tw->text.pendingoff = FALSE;
      else
	tw->text.pendingoff = TRUE;
      
      _XmTextSetCursorPosition(w, cursorPos);
      _XmTextSetDestinationSelection(w, tw->text.cursor_position, False,
				     insert_select->event->time);
      
      if (insert_select->select_type == XmDEST_SELECT) {
	if (left != right) {
	  if (!dest_disjoint || !tw->text.add_mode) {
	    (*tw->text.source->SetSelection)(tw->text.source,
					     tw->text.cursor_position,
					     tw->text.cursor_position,
					     insert_select->event->time);
	  } 
	}
      }
      _XmTextValueChanged(tw, (XEvent *)insert_select->event);
    }
    if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
  }
  
  (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
  if (total_value) XtFree(total_value);
  XtFree((char*)value);
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

  /* The order of choice has been changed so that COMPOUND_TEXT *
   * takes precedence over TEXT - fix for bug 4117589 - leob    */
  if (supports_encoding_data && !supports_text)
    target = CS_OF_ENCODING;
  else if (supports_CT)
    target = COMPOUND_TEXT;
  else if (supports_text && supports_encoding_data)
    target = TEXT;
  else
    target = XA_STRING;

  XmTransferValue(tid, target,
		  (XtCallbackProc) TextSecondaryWrapper,
		  closure, insert_select -> event -> time);
}

#ifdef SUN_CTL
/* returns the maximum possible length between the logical positions left
/* and right
 */
int CTLMaxSelLength(XmTextWidget tw,
		    XmTextPosition left,
		    XmTextPosition right)
{
    int ret_max_length = 0;
    LineNum start_line, last_line, curr_line;
    
    start_line = PosToAbsLine(tw, left);
    if (start_line > tw->text.total_lines)
	start_line = tw->text.total_lines;
    
    last_line  = PosToAbsLine(tw, right);
    if (last_line > tw->text.total_lines)
	last_line = tw->text.total_lines;
    
    if (start_line > last_line) {
	int temp = start_line;
	
	start_line = last_line;
	last_line = temp;
    }
    
    for (curr_line = start_line; curr_line <= last_line; curr_line++) {
	XmTextPosition linestart, next_linestart;
	
	linestart = tw->text.line[curr_line].start;
	if (linestart > tw->text.last_position)
	    linestart = tw->text.last_position;
	
	next_linestart = tw->text.line[curr_line + 1].start;
	if (next_linestart > tw->text.last_position)
	    next_linestart = tw->text.last_position;
	
	if (next_linestart >= linestart) /* else problem */
	    ret_max_length += next_linestart - linestart;
    }
    return ret_max_length;
}

XmTextPosition Get_Linestart(XmTextWidget tw, 
			     XmTextPosition line_num)
{
    XmTextPosition ret_pos;
    
    CTLLineInfo (tw, line_num, &ret_pos);
    return ret_pos;
}
    
void  
_XmTextGetVisualCharList(XmTextWidget      tw, 
			 XmTextPosition    left, 
			 XmTextPosition    right, 
			 XmTextPosition   *char_list, 
			 int              *num_chars)
{
    int i;
    XmTextPosition curr_pos;
    Position l_x, l_y, r_x, r_y;
    
    int  list_ptr    = 0;
    int  len         = CTLMaxSelLength(tw, left, right);
    
    *num_chars = 0;
    if (PosToAbsXY(tw, left, XmEDGE_LEFT, &l_x, &l_y) && 
	PosToAbsXY(tw, right, XmEDGE_LEFT, &r_x, &r_y)) {
	Position char_x, char_y;
	Position curr_x, curr_y;
	
	_XmWidgetToAppContext((Widget)tw);
	_XmAppLock(app);
	
	/* collect the chars in the first line */
	{
	    LineNum line = PosToAbsLine(tw, left);
	    XmTextPosition linestart = Get_Linestart(tw, line);
	    XmTextPosition next_linestart = Get_Linestart(tw, line + 1);
	    
	    /* get the selection  in the  first line */
	    if (l_y == r_y) {/* selection in a single line */
		/* if the l_x > r_x then swap */
		if (r_x < l_x) {
		    int temp_x = r_x;
		    r_x = l_x;
		    l_x = temp_x;
		}
		
		for (i = 0; i < next_linestart - linestart; i++) {
		    PosToAbsXY(tw, linestart + i, XmEDGE_LEFT, &char_x, &char_y);
		    if (char_x < r_x && char_x >= l_x)
			char_list[(*num_chars)++] = linestart + i;
		}
		return;
	    }
	    for (i = 0; i < next_linestart - linestart; i++) {
		PosToAbsXY(tw, linestart + i, XmEDGE_LEFT, &char_x, &char_y);
		if (char_x >= l_x)
		    char_list[(*num_chars)++] = linestart + i;	
	    }
	}
	
	/* get the selection till the last line of selection */
	{
	    LineNum curr_line = PosToAbsLine(tw, left) + 1;
	    
	    if (curr_line > tw->text.total_lines - 1)
		curr_line = tw->text.total_lines - 1;
	    
	    /* copy all the lines */
	    PosToAbsXY(tw, Get_Linestart(tw, curr_line), XmEDGE_LEFT, &curr_x, &curr_y);
	    while (curr_y < r_y) {
		XmTextPosition linestart = Get_Linestart(tw, curr_line);
		XmTextPosition next_linestart = Get_Linestart(tw, curr_line + 1);
		int line_length = next_linestart - linestart;
		int i;
		
		for (i = 0; i < line_length; i++)
		    char_list[(*num_chars)++] = linestart + i;
		
		PosToAbsXY(tw, next_linestart, XmEDGE_LEFT, &curr_x, &curr_y);
		curr_line++;
	    }
	}
	/* check whether we had already picked the chars in the last line */
	if (l_y != r_y) {
	    /* we have the last line to gather some characters*/
	    LineNum line = PosToAbsLine(tw, right);
	    XmTextPosition linestart = Get_Linestart(tw, line);
	    XmTextPosition next_linestart = Get_Linestart(tw, line+1);
	    
	    /* get the selection  in the  last line */
	    int i;
	    for (i = 0; i < next_linestart - linestart; i++) {
		PosToAbsXY(tw, linestart + i, XmEDGE_LEFT, &char_x, &char_y);
		if (char_x < r_x)
		    char_list[(*num_chars)++] = linestart + i;
	    }
	}
	_XmAppUnlock(app);
    }
    return;
}

char * _XmTextConvertVisual(XmTextWidget	tw, 
			    XmTextPosition    	left, 
			    XmTextPosition    	right)
{
    XmTextPosition curr_pos;
    Position l_x, l_y, r_x, r_y;
    
    int      len        = CTLMaxSelLength(tw, left, right);
    Boolean  is_wchar   = (tw->text.char_size > 1);
    char    *ret_val    = (char*)XtMalloc((len + 1) * (is_wchar ? MB_CUR_MAX : 1));
    char    *ret_mbs    = ret_val;
    
    if (PosToAbsXY(tw, left, XmEDGE_LEFT, &l_x, &l_y) && 
	PosToAbsXY(tw, right, XmEDGE_LEFT, &r_x, &r_y)) {
	Position char_x, char_y;
	Position curr_x, curr_y;
	
	_XmWidgetToAppContext((Widget)tw);
	_XmAppLock(app);
	/* collect the chars in the first line */
	{
	    LineNum line = PosToAbsLine(tw, left);
	    XmTextPosition linestart = Get_Linestart(tw, line);
	    XmTextPosition next_linestart = Get_Linestart(tw, line + 1);
	    
	    /* get the selection  in the  first line */
	    if (l_y == r_y) {/* selection in a single line */
		int i, len1;
		for (i = 0; i < next_linestart - linestart; i++) {
		    PosToAbsXY(tw, linestart + i, XmEDGE_LEFT, &char_x, &char_y);
		    if (char_x < r_x && char_x >= l_x) {
			if (is_wchar) {
			    wchar_t tmp_wcs[2];
			    (void)XmTextGetSubstringWcs((Widget)tw,	linestart + i, 1, 2, tmp_wcs);
			    len1 = wctomb(ret_mbs, tmp_wcs[0]);
			    if (len1 == -1) *ret_mbs = tmp_wcs[0];
			    ret_mbs += len1 == -1 ? 1 : len1;
			} else {
			    char tmp_str[2];
			    (void)XmTextGetSubstring((Widget)tw, linestart + i, 1, 2, tmp_str);
			    *ret_mbs++ = tmp_str[0];
			}
		    }
		}
	    } else {
		int i, len1;
		for (i = 0; i < next_linestart - linestart; i++) {
		    PosToAbsXY(tw, linestart + i, XmEDGE_LEFT, &char_x, &char_y);
		    if (char_x >= l_x) {
			if(is_wchar) {
			    wchar_t tmp_wcs[2];
			    (void)XmTextGetSubstringWcs((Widget)tw,	linestart + i, 1, 2, tmp_wcs);
			    len1 = wctomb(ret_mbs, tmp_wcs[0]);
			    if (len1 == -1) *ret_mbs = tmp_wcs[0];
			    ret_mbs += len1 == -1 ? 1 : len1;
			} else {
			    char tmp_str[2];
			    (void)XmTextGetSubstring((Widget)tw, linestart + i, 1, 2, tmp_str);
			    *ret_mbs++ = tmp_str[0];
			}
		    }
		}
	    }
	}
	
	/* get the selection till the last line of selection */
	{
	    LineNum curr_line = PosToAbsLine(tw, left) + 1;
	    
	    if (curr_line > tw->text.total_lines - 1)
		curr_line = tw->text.total_lines - 1;
	    
	    /* copy all the lines */
	    PosToAbsXY(tw, Get_Linestart(tw, curr_line), XmEDGE_LEFT, &curr_x, &curr_y);
	    while (curr_y < r_y) {
		XmTextPosition linestart = Get_Linestart(tw, curr_line);
		XmTextPosition next_linestart = Get_Linestart(tw, curr_line + 1);
		int line_length = next_linestart - linestart;
		char  tmp_cache[200];
		char *tmp_str = (char*)XmStackAlloc((line_length + 1),
						    tmp_cache);
		int i;
		
		(void)XmTextGetSubstring((Widget)tw, linestart, line_length, line_length + 1, tmp_str);
		if (is_wchar) {
		    int index = 0;
		    for (i = 0; i < line_length; i++) {
			int j, char_length;
			
			char_length = wctomb(tmp_str + index, 0);
			if (char_length == -1) char_length = 1;
			for (j = 0; j < char_length; j++)
			    *ret_mbs++=tmp_str[index++];
		    }
		} else {
		    for(i = 0; i < line_length; i++)
			*ret_mbs++ = tmp_str[i];
		}
		XmStackFree(tmp_str, tmp_cache);
		
		PosToAbsXY(tw, next_linestart, XmEDGE_LEFT, &curr_x, &curr_y);
		curr_line++;
	    }
	    
	}
	
	if (l_y != r_y) {/* we have the last line to gather some character*/
	    LineNum line = PosToAbsLine(tw, right);
	    XmTextPosition linestart = Get_Linestart(tw, line);
	    XmTextPosition next_linestart = Get_Linestart(tw, line+1);
	    
	    /* get the selection  in the  last line */
	    int i, len1;
	    for (i = 0; i < next_linestart - linestart; i++) {
		PosToAbsXY(tw, linestart + i, XmEDGE_LEFT, &char_x, &char_y);
		if (char_x < r_x) {    
		    if(is_wchar) {
			wchar_t tmp_wcs[2];
			
			(void)XmTextGetSubstringWcs((Widget)tw, linestart + i, 1, 2, tmp_wcs);
			len1 = wctomb(ret_mbs, tmp_wcs[0]);	
			if (len1 == -1) *ret_mbs = tmp_wcs[0];
			ret_mbs += len1 == -1 ? 1 : len1;
		    } else {
			char tmp_str[2];
			
			(void)XmTextGetSubstring((Widget)tw, linestart + i, 1, 2, tmp_str);
			*ret_mbs++ = tmp_str[0];
		    }
		}
	    }
	}
	_XmAppUnlock(app);
    }
    
    /* Caller is going to convert value to a TextProperty and XtFree */
    *ret_mbs = '\0';
    
    return ret_val;
}
#endif /* CTL */

/* ARGSUSED */
Boolean
_XmTextConvert(
        Widget w,
        Atom *selection,
        Atom *target,
        Atom *type,
        XtPointer *value,
        unsigned long *length,
        int *format,
	Widget drag_context,
        XEvent *event )
{
  XmTextWidget tw = (XmTextWidget) w;
  XmTextSource source;
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
  int status = 0;
  char * tmp_value;
  Time _time;
  
  if (w == NULL) return False;
  
  if (req_event == NULL) 
    _time = XtLastTimestampProcessed(XtDisplay(w));
  else
    _time = req_event -> time;
  
  source = tw->text.source;
  
  if (*selection == XA_PRIMARY || *selection == CLIPBOARD) {
    has_selection = (*tw->text.source->GetSelection)(source, &left, &right);
    is_primary = True;
    is_secondary = is_destination = is_drop = False;
  } else if (*selection == MOTIF_DESTINATION) {
    has_selection = tw->text.input->data->has_destination;
    is_destination = True;
    is_secondary = is_primary = is_drop = False;
  } else if (*selection == XA_SECONDARY) {
    has_selection = _XmTextGetSel2(tw, &left, &right);
    is_secondary = True;
    is_destination = is_primary = is_drop = False;
  } else if (*selection == MOTIF_DROP) {
    has_selection = (*tw->text.source->GetSelection)(source, &left, &right);
    is_drop = True;
    is_destination = is_primary = is_secondary = False;
  } else
    return False;
  
  
  /*
   * TARGETS identifies what targets the text widget can
   * provide data for.
   */
  if (*target == TARGETS) {
    Atom *targs = XmeStandardTargets(w, 10, &target_count);
    
    *value = (XtPointer) targs;
    if (XA_STRING != CS_OF_ENCODING) {
      targs[target_count] = CS_OF_ENCODING;  target_count++;
    }
    if (is_primary || is_destination) {
      targs[target_count] = INSERT_SELECTION;  target_count++;
    }
    if (is_primary || is_secondary || is_drop) {
      targs[target_count] = COMPOUND_TEXT;  target_count++;
      targs[target_count] = TEXT;  target_count++;
      targs[target_count] = XA_STRING;  target_count++;
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
      *timestamp = source->data->prim_time;
    else if (is_destination)
      *timestamp = tw->text.input->data->dest_time;
    else if (is_secondary)
      *timestamp = tw->text.input->data->sec_time;
    else if (is_drop)
      *timestamp = tw->text.input->data->sec_time;
    *value = (XtPointer) timestamp;
    *type = XA_INTEGER;
    *length = sizeof(Time) / 4;
    *format = 32;
  } else if (*target == XA_STRING) {
    /* Provide data for XA_STRING requests */
    *type = (Atom) XA_STRING;
    *format = 8;
    if (is_destination || !has_selection) return False;
    tmp_prop.value = NULL;
#ifdef SUN_CTL_NYI
    if (TextW_LayoutActive(tw) && (tw->text.input->data->edit_policy == XmEDIT_VISUAL) && 
	(_XmRendFontType(tw->text.output->data->rendition) == XmFONT_IS_XOC))
	tmp_value = _XmTextConvertVisual(tw, left, right);
    else
#endif  /* CTL */
    tmp_value = _XmStringSourceGetString(tw, left, right, False);
    status = XmbTextListToTextProperty(XtDisplay(tw), &tmp_value, 1,
				       (XICCEncodingStyle)XStringStyle, 
				       &tmp_prop);
    XtFree(tmp_value);
    if (status == Success || status > 0){
      /* NOTE: casting tmp_prop.nitems could result in a truncated long. */
      *value = (XtPointer) XtMalloc((size_t)tmp_prop.nitems); /* Wyoming 64-bit fix */ 
      memcpy((void*)*value, (void*)tmp_prop.value,(size_t)tmp_prop.nitems);
      if (tmp_prop.value != NULL) XFree((char*)tmp_prop.value);
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
#ifdef SUN_CTL_NYI
    if (TextW_LayoutActive(tw) && (tw->text.input->data->edit_policy == XmEDIT_VISUAL) && 
	(_XmRendFontType(tw->text.output->data->rendition) == XmFONT_IS_XOC) )
      *value = _XmTextConvertVisual(tw, left, right);
    else
#endif  /* CTL */
    *value = (XtPointer)_XmStringSourceGetString(tw, left, right, False);
    *length = strlen((char*) *value);
  } else if (*target == COMPOUND_TEXT) {
    *type = COMPOUND_TEXT;
    *format = 8;
    if (is_destination || !has_selection) return False;
    tmp_prop.value = NULL;
#ifdef SUN_CTL_NYI
    if (TextW_LayoutActive(tw) && (tw->text.input->data->edit_policy == XmEDIT_VISUAL) && 
	(_XmRendFontType(tw->text.output->data->rendition) == XmFONT_IS_XOC))
      tmp_value = _XmTextConvertVisual(tw, left, right);
    else
#endif  /* CTL */
    tmp_value =_XmStringSourceGetString(tw, left, right, False);
    status = XmbTextListToTextProperty(XtDisplay(tw), &tmp_value, 1,
				       (XICCEncodingStyle)XCompoundTextStyle,
				       &tmp_prop);
    XtFree(tmp_value);
    if (status == Success || status > 0) {
      /* NOTE: casting tmp_prop.nitems could result in a truncated long. */
      *value = (XtPointer) XtMalloc((size_t) tmp_prop.nitems); /* Wyoming 64-bit fix */ 
      memcpy((void*)*value, (void*)tmp_prop.value,(size_t)tmp_prop.nitems);
      if (tmp_prop.value != NULL) XFree((char*)tmp_prop.value);
      *length = tmp_prop.nitems;
    } else {
      *value =  NULL;
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
    XmTextBlockRec block, newblock;
    XmTextPosition cursorPos;
    Boolean freeBlock;
    
    if (!(is_primary || is_drop)) return False;
    
    /* The on_or_off flag is set to prevent unecessary
       cursor shifting during the Replace operation */
    tw->text.on_or_off = off;
    
    block.ptr = "";
    block.length = 0;
    block.format = XmFMT_8_BIT;
    
    if (_XmTextModifyVerify(tw, event, &left, &right,
			    &cursorPos, &block, &newblock, &freeBlock)) {
      if ((*tw->text.source->Replace)(tw, event, &left, &right, 
				      &newblock, False) != EditDone) {
	if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
	return False;
      } else {
	if (is_drop) {
	  if (_XmTextGetDropReciever((Widget)tw) != (Widget) tw)
	    _XmTextSetCursorPosition((Widget)tw, cursorPos);
	} else {
	  if ((*selection == CLIPBOARD) || 
	      (req_event != NULL &&
	      req_event->requestor != XtWindow((Widget) tw)))
	    _XmTextSetCursorPosition(w, cursorPos);
	}
	_XmTextValueChanged(tw, (XEvent *) req_event);
      }
      if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
    } 
    if (!tw->text.input->data->has_destination)
      tw->text.input->data->anchor = tw->text.cursor_position;
    
    (*tw->text.source->SetSelection)(tw->text.source,
				     tw->text.cursor_position,
				     tw->text.cursor_position,
				     _time);
    
    *type = XInternAtom(XtDisplay(w), XmSNULL, False);
    *value = NULL;
    *length = 0;
    *format = 8;
    
    tw->text.on_or_off = on;
    
    /* unknown selection type */
  } else
    return FALSE;
  return TRUE;
}

/* ARGSUSED */
void
_XmTextLoseSelection(
        Widget w,
        Atom *selection )
{
  XmTextWidget tw = (XmTextWidget) w;
  XmTextSource source = tw->text.source;
  Atom MOTIF_DESTINATION = XInternAtom(XtDisplay(w),
				       XmS_MOTIF_DESTINATION, False);
  /* Losing Primary Selection */
  if (*selection == XA_PRIMARY && _XmStringSourceHasSelection(source)) {
    XmAnyCallbackStruct cb;
    (*source->SetSelection)(source, 1, -999,
			    XtLastTimestampProcessed(XtDisplay(w)));
    cb.reason = XmCR_LOSE_PRIMARY;
    cb.event = NULL;
    XtCallCallbackList(w, tw->text.lose_primary_callback, (XtPointer) &cb);
    /* Losing Destination Selection */
  } else if (*selection == MOTIF_DESTINATION) {
    tw->text.input->data->has_destination = False;
    (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, off);
    tw->text.output->data->blinkstate = on;
    (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position, on);
    /* Losing Secondary Selection */
  } else if (*selection == XA_SECONDARY && tw->text.input->data->hasSel2){
    _XmTextSetSel2(tw, 1, -999, XtLastTimestampProcessed(XtDisplay(w)));
  }
}


static void
HandleDrop(Widget w,
	   XmDropProcCallbackStruct *cb,
	   XmDestinationCallbackStruct *ds)
{
  Widget drag_cont, initiator;
  XmTextWidget tw = (XmTextWidget) w;
  Cardinal numExportTargets, n;
  Atom *exportTargets;
  Atom desiredTarget = None;
  Arg args[10];
  XmTextPosition insert_pos, left, right;
  Boolean doTransfer = False;
  XtPointer tid = ds->transfer_id;
  _XmTextDropTransferRec *transfer_rec = NULL;
  
  drag_cont = cb->dragContext;
  
  n = 0;
  XtSetArg(args[n], XmNsourceWidget, &initiator); n++;
  XtSetArg(args[n], XmNexportTargets, &exportTargets); n++;
  XtSetArg(args[n], XmNnumExportTargets, &numExportTargets); n++;
  XtGetValues((Widget) drag_cont, args, n);
  
  if (tw->text.output)
     insert_pos = (*tw->text.output->XYToPos)(tw, cb->x, cb->y);
  else
     insert_pos = 0;
  
  if (cb->operation & XmDROP_MOVE && w == initiator &&
      ((*tw->text.source->GetSelection)(tw->text.source, &left, &right) &&
       left != right && insert_pos >= left && insert_pos <= right)) {
    XmTransferDone(tid, XmTRANSFER_DONE_FAIL);
  } else {
    Atom TEXT = XInternAtom(XtDisplay(w), XmSTEXT, False);
    Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
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

/* Request targets from selection receiver; move the rest of this
 * to a new routine (the name of which is passed during the request
 * for targets).  The new routine will look at the target list and
 * determine what target to place in the pair.  It will then do
 * any necessary conversions before "thrusting" the selection value
 * onto the receiver.  This will guarantee the best chance at a
 * successful exchange.
 */

static void 
HandleTargets(Widget w, 
	      XtPointer closure,
	      XmSelectionCallbackStruct *ds)
{
  XmTextWidget tw = (XmTextWidget) w;
  Atom CS_OF_ENCODING = _XmTextGetEncodingAtom(w); /* leob fix for bug 4191799 */
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);
  Atom TEXT = XInternAtom(XtDisplay(w), XmSTEXT, False);
  Boolean supports_encoding_data = False;
  Boolean supports_CT = False;
  Boolean supports_text = False;
  Atom *atom_ptr;
  XPoint *point = (XPoint *)closure;
  Atom targets[2];
  XmTextPosition select_pos;
  XmTextPosition left, right;
  int i;
  
  if (!ds->length) {
    XtFree((char *)ds->value);
    ds->value = NULL;
    return;
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
    select_pos = 
      (*tw->text.output->XYToPos)(tw, (Position)point->x, (Position)point->y);
  } else {
    select_pos = tw->text.cursor_position;
  }
  
  if (ds->selection != CLIPBOARD) {
    if ((*tw->text.source->GetSelection)(tw->text.source, &left, &right) && 
	left != right && select_pos > left && 
	select_pos < right) {
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
  
  /* If owner supports TEXT and the current locale, ask for TEXT.  
   * If not, and if the owner supports compound text, ask for 
   * compound text. If not, and owner and I have the same encoding, 
   * ask for that encoding. If not, fall back position is to ask for 
   * STRING and try to convert it locally.
   *
   * Fix for BUG 4103273
   *
   * OW apps say they can send COMPOUND_TEXT but when requested
   * cannot. THe logical order of choice has been changed so
   * that CS_OF_ENCODING takes precedence over COMPOUND_TEXT
   *
   * This order has also change in the HandleInsertTargets
   */
  
  /* The order of choice has been changed so that COMPOUND_TEXT *
   * takes precedence over TEXT - fix for bug 4117589 - leob    */

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

/* Pastes the primary selection to the stuff position. */
static void
DoStuff(Widget w, 
	XtPointer closure, 
	XmSelectionCallbackStruct *ds)
{
  XmTextWidget tw = (XmTextWidget) w;
  InputData data = tw->text.input->data;
  OutputData o_data = tw->text.output->data;
  Atom NULL_ATOM = XInternAtom(XtDisplay(w), XmSNULL, False);
  Atom CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);
  XmTextBlockRec block, newblock;
  XmTextPosition cursorPos = tw->text.cursor_position;
  XmTextPosition right, left, replace_from, replace_to;
  _XmTextPrimSelect *prim_select = (_XmTextPrimSelect *) closure;
  char * total_value = NULL;
  Boolean freeBlock;

  if (!o_data->hasfocus && _XmGetFocusPolicy(w) == XmEXPLICIT)
    (void) XmProcessTraversal(w, XmTRAVERSE_CURRENT);
  
  if (ds->selection != CLIPBOARD && !(ds->length) && ds->type != NULL_ATOM) {
    /* Backwards compatibility for 1.0 Selections */
    _XmProcessLock();
    if (prim_select->target == XInternAtom(XtDisplay(tw), XmSTEXT, False)) {
      prim_select->target = XA_STRING;
      XmTransferValue(ds->transfer_id, XA_STRING, (XtCallbackProc) DoStuff,
		      (XtPointer) prim_select, prim_select->time);
    }
    _XmProcessUnlock();
    XtFree((char *)ds->value);
    ds->value = NULL;
    return;
  }
  
  /* if length == 0 and ds->type is the NULL atom we are assuming
   * that a DELETE target is requested.
   */
  if (ds->type == NULL_ATOM) {
    _XmProcessLock();
    if (prim_select->num_chars > 0 && data->selectionMove) {
      data->anchor = prim_select->position;
      cursorPos = prim_select->position + prim_select->num_chars;
      _XmTextSetCursorPosition(w, cursorPos);
      _XmTextSetDestinationSelection(w, tw->text.cursor_position,
				     False, prim_select->time);
      (*tw->text.source->SetSelection)(tw->text.source, data->anchor,
				       tw->text.cursor_position,
				       prim_select->time);
    }
    _XmProcessUnlock();
  } else {
    XmTextSource source = GetSrc(w);
    int max_length = 0;
    Boolean local = _XmStringSourceHasSelection(source);
    Boolean *pendingoff = NULL;
    Boolean dest_disjoint = True;
    
    block.format = XmFMT_8_BIT;
    
    if (ds->type == XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False) ||
	ds->type == XA_STRING) {
      if ((total_value = 
	   _XmTextToLocaleText(w, ds->value, ds->type, ds->format,
			       ds->length, NULL)) != NULL) {
	block.ptr = total_value;
	block.length = (int)strlen(block.ptr); /* Wyoming 64-bit fix */ 
      } else {
	block.ptr = total_value = XtMalloc((unsigned)1);
	*(block.ptr) = '\0';
	block.length = 0;
      }
    } else {
      block.ptr = (char*)ds->value;
      block.length = (int) ds->length; /* NOTE: this causes a truncation on
					  some architectures */
    }
    
    if (data->selectionMove && local) {
      max_length = _XmStringSourceGetMaxLength(source);
      _XmStringSourceSetMaxLength(source, INT_MAX);
    }
    
    replace_from = replace_to = prim_select->position;
    pendingoff = _XmStringSourceGetPending(tw);

    if (ds->selection == CLIPBOARD) {
      if ((*tw->text.source->GetSelection)(tw->text.source, &left, &right)) {
	if (tw->text.input->data->pendingdelete &&
	    replace_from >= left && replace_to <= right){
	  replace_from = left;
	  replace_to = right;
	  dest_disjoint = False;
	}
      }
   } else {  
      /* The on_or_off flag is set to prevent unnecessary
	 cursor shifting during the Replace operation */
      tw->text.on_or_off = off;

      _XmStringSourceSetPending(tw, (Boolean *)FALSE);
    }
    
    if (_XmTextModifyVerify(tw, ds->event, &replace_from, &replace_to,
			    &cursorPos, &block, &newblock, &freeBlock)) {
      _XmProcessLock();
				    /* Bug Id : 1217687/4128045/4154215 */
      prim_select->num_chars = (int)TextCountCharacters(w, newblock.ptr,  /* Wyoming 64-bit fix */ 
						      newblock.length);
      _XmProcessUnlock();
      if ((*tw->text.source->Replace)(tw, ds->event, 
				      &replace_from, &replace_to,
				      &newblock, False) != EditDone) {
	XtCallActionProc(w, "beep", NULL, (String *) NULL, (Cardinal) 0);
	_XmProcessLock();
	prim_select->num_chars = 0; /* Stop SetPrimarySelection from doing
				       anything */
	_XmProcessUnlock();
	_XmStringSourceSetPending(tw, pendingoff);
      } else {
	if ((newblock.length > 0 && !data->selectionMove) || 
	    ds->selection == CLIPBOARD) {
	  _XmTextSetCursorPosition(w, cursorPos);
	  _XmTextSetDestinationSelection(w, tw->text.cursor_position,
					 False, prim_select->time);
	}
	if ((*tw->text.source->GetSelection)(tw->text.source, &left, &right)) {
	  if (ds->selection == CLIPBOARD) {
	    data->anchor = replace_from;
	    if (left != right && (!dest_disjoint || !tw->text.add_mode))
	      (*source->SetSelection)(source, tw->text.cursor_position,
				      tw->text.cursor_position, 
				      prim_select->time);
	  } else {
	    if (data->selectionMove) {
	      _XmProcessLock();
	      if (left < replace_from) {
		prim_select->position = replace_from -
		  prim_select->num_chars;
	      } else {
		prim_select->position = replace_from;
	      }
	      _XmProcessUnlock();
	    }
	    if (cursorPos < left || cursorPos > right)
	      _XmStringSourceSetPending(tw, (Boolean *)TRUE);
	    else
	      _XmStringSourceSetPending(tw, pendingoff);
	  }
	} else {
	  _XmProcessLock();
	  if (ds->selection == CLIPBOARD)
	    data->anchor = replace_from;
	  else if (!data->selectionMove && !tw->text.add_mode &&
	      prim_select->num_chars != 0)
	    data->anchor = prim_select->position;
	  _XmProcessUnlock();
	}
	_XmTextValueChanged(tw, ds->event);
      }
      if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
    } else {
      XtCallActionProc(w, "beep", NULL, (String *) NULL, (Cardinal) 0);
      _XmProcessLock();
      prim_select->num_chars = 0; /* Stop SetPrimarySelection from doing
				     anything */
      _XmProcessUnlock();
      _XmStringSourceSetPending(tw, pendingoff);
    }
    
    if (data->selectionMove && local) {
      _XmStringSourceSetMaxLength(source, max_length);
    }
    
    if (ds->selection != CLIPBOARD) 
      tw->text.on_or_off = on;

    if (pendingoff) XtFree((char *)pendingoff);
  }

  if (total_value) XtFree(total_value);
  XtFree((char *)ds->value);
  ds->value = NULL;
}

/* ARGSUSED */
static void
DropDestroyCB(Widget      w,
	      XtPointer   clientData,
	      XtPointer   callData)
{
  XmTransferDoneCallbackStruct *ts = (XmTransferDoneCallbackStruct *)callData;
  
  DeleteDropContext(w);
  if (ts->client_data != NULL) XtFree((char*) ts->client_data);
}

/* ARGSUSED */
static void
DropTransferProc(Widget w, 
		 XtPointer closure, 
		 XmSelectionCallbackStruct *ds)
{
  _XmTextDropTransferRec *transfer_rec = (_XmTextDropTransferRec *) closure;
  XmTextWidget tw = (XmTextWidget) transfer_rec->widget;
  InputData data = tw->text.input->data;
  Atom COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom CS_OF_ENCODING = XmeGetEncodingAtom(w);
  XmTextPosition insertPosLeft, insertPosRight, left, right, cursorPos;
  XmTextBlockRec block, newblock;
  XmTextSource source = GetSrc((Widget)tw);
  int max_length = 0;
  Boolean local = _XmStringSourceHasSelection(source);
  char * total_value = NULL;
  Boolean pendingoff;
  Boolean freeBlock;
  
  /* When type = NULL, we are assuming a DELETE request has been requested */
  if (ds->type == XInternAtom(XtDisplay(tw), XmSNULL, False)) {
    if (transfer_rec->num_chars > 0 && transfer_rec->move) {
      data->anchor = transfer_rec->insert_pos;
      cursorPos = transfer_rec->insert_pos + transfer_rec->num_chars;
      _XmTextSetCursorPosition((Widget)tw, cursorPos);
      _XmTextSetDestinationSelection((Widget)tw, tw->text.cursor_position,
				     False, 
				     XtLastTimestampProcessed(XtDisplay(w)));
      (*tw->text.source->SetSelection)(tw->text.source, data->anchor,
				       tw->text.cursor_position,
				       XtLastTimestampProcessed(XtDisplay(w)));
      if (ds->value) {
	XtFree((char *) ds->value);
	ds->value = NULL;
      }
      return;
    }
  }
  
  if (!ds->value || 
      (ds->type != COMPOUND_TEXT && 
       ds->type != CS_OF_ENCODING &&
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
					   8, ds->length, NULL)) != NULL) {
      block.ptr = total_value;
      block.length = (int)strlen(block.ptr); /* Wyoming 64-bit fix */ 
    } else {
      if (ds->value) {
	XtFree((char*) ds->value);
	ds->value = NULL;
      }
      return;
    }
  } else {
    block.ptr = (char *) ds->value;
    block.length = (int) ds->length; /* NOTE: this causes a truncation on
					  some architectures */
  }
  
  block.format = XmFMT_8_BIT;
  
  if (data->pendingdelete && 
      ((*tw->text.source->GetSelection)(tw->text.source, &left, &right) &&
       left != right) && insertPosLeft > left && insertPosRight < right) {
    insertPosLeft = left;
    insertPosRight = right;
  }
  
  if (transfer_rec->move && local) {
    max_length = _XmStringSourceGetMaxLength(source);
    _XmStringSourceSetMaxLength(source, INT_MAX);
  }
  
  /* The on_or_off flag is set to prevent unecessary
     cursor shifting during the Replace operation */
  tw->text.on_or_off = off;

  pendingoff = tw->text.pendingoff;
  tw->text.pendingoff = FALSE;
  
  if (_XmTextModifyVerify(tw, ds->event, &insertPosLeft, &insertPosRight,
			  &cursorPos, &block, &newblock, &freeBlock)) {
    if ((*tw->text.source->Replace)(tw, ds->event, 
				    &insertPosLeft, &insertPosRight,
				    &newblock, False) != EditDone) {
      if (tw->text.verify_bell) XBell(XtDisplay(tw), 0);
      tw->text.pendingoff = pendingoff;
    } else {
				     /* Bug Id : 1217687/4128045/4154215 */
      transfer_rec->num_chars = (int)TextCountCharacters(w, newblock.ptr, /* Wyoming 64-bit fix */ 
						       newblock.length);
      if (transfer_rec->num_chars > 0 && !transfer_rec->move) {
	_XmTextSetCursorPosition((Widget)tw, cursorPos);
	_XmTextSetDestinationSelection((Widget)tw,
				       tw->text.cursor_position,False,
				       transfer_rec->timestamp);
      }
      if ((*tw->text.source->GetSelection)(tw->text.source, &left, &right)) {
	if (transfer_rec->move && left < insertPosLeft)
	  transfer_rec->insert_pos = insertPosLeft -
	    transfer_rec->num_chars;
	if (cursorPos < left || cursorPos > right)
	  tw->text.pendingoff = TRUE;
      } else {
	if (!transfer_rec->move && !tw->text.add_mode &&
	    transfer_rec->num_chars != 0)
	  data->anchor = insertPosLeft;
      }
      if (transfer_rec->move) {
	XmTransferValue(ds->transfer_id, 
			XInternAtom(XtDisplay(w),XmSDELETE,False),
			(XtCallbackProc) DropTransferProc, 
			(XtPointer) transfer_rec, 0);
      }
      
      if (transfer_rec->move && local) {
	_XmStringSourceSetMaxLength(source, max_length);
      }

      _XmTextValueChanged(tw, (XEvent *) ds->event);
    }
    if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
  } else {
    if (tw->text.verify_bell) XBell(XtDisplay(tw), 0);
    tw->text.pendingoff = pendingoff;
  }
  tw->text.on_or_off = on;

  if (total_value) XtFree(total_value);
  if (ds->value != NULL) XtFree((char*) ds->value);
  ds->value = NULL;
}

static void
SetDropContext(Widget w)
{
  Display *display = XtDisplay(w);
  Screen  *screen = XtScreen(w);
  XContext loc_context;

  _XmProcessLock();
  if (_XmTextDNDContext == 0)
	_XmTextDNDContext = XUniqueContext();
  loc_context = _XmTextDNDContext;
  _XmProcessUnlock();

  XSaveContext(display, (Window)screen,
	       loc_context, (XPointer)w);
}


static void
DeleteDropContext(Widget w)
{
  Display *display = XtDisplay(w);
  Screen  *screen = XtScreen(w);

  _XmProcessLock();
  XDeleteContext(display, (Window)screen, _XmTextDNDContext);
  _XmProcessUnlock();
}


Widget
_XmTextGetDropReciever(Widget w)
{
  Display *display = XtDisplay(w);
  Screen  *screen = XtScreen(w);
  Widget widget;
  XContext loc_context;
 
  _XmProcessLock();
  loc_context = _XmTextDNDContext;
  _XmProcessUnlock();
  if (loc_context == 0) return NULL;
  
  if (!XFindContext(display, (Window)screen,
		    loc_context, (char **) &widget)) {
    return widget;
  }
  
  return NULL;
}



/********************************************
 * Transfer trait method implementation 
 ********************************************/

/*ARGSUSED*/
static void
TextConvertCallback(Widget w, 
		    XtPointer ignore, /* unused */
		    XmConvertCallbackStruct *cs)
{
  Atom XA_CS_OF_ENCODING = XmeGetEncodingAtom(w);
  XtPointer value;
  Atom type;
  unsigned long size;
  int format;
  Atom XA_DELETE = XInternAtom(XtDisplay(w), XmSDELETE, False);
  Atom XA_MOTIF_LOSE = XInternAtom(XtDisplay(w), XmS_MOTIF_LOSE_SELECTION,
				   False);
  Atom XA_MOTIF_EXPORTS = XInternAtom(XtDisplay(w), XmS_MOTIF_EXPORT_TARGETS,
				      False);
  Atom XA_TEXT = XInternAtom(XtDisplay(w), XmSTEXT, False);
  Atom XA_COMPOUND_TEXT = XInternAtom(XtDisplay(w), XmSCOMPOUND_TEXT, False);
  Atom XA_TARGETS = XInternAtom(XtDisplay(w), XmSTARGETS, False);
  Atom XA_MOTIF_REQUIRED = XInternAtom(XtDisplay(w),
				       XmS_MOTIF_CLIPBOARD_TARGETS, False);
  Atom XA_CLIPBOARD = XInternAtom(XtDisplay(w), XmSCLIPBOARD, False);
  
  value = NULL;

  if (cs->target == XA_MOTIF_LOSE) {
    _XmTextLoseSelection(w, &(cs->selection));
    cs->status = XmCONVERT_DONE;
    return;
  }
  
  if (cs->target == XA_DELETE &&
      cs->selection == XA_SECONDARY) {
    _XmTextHandleSecondaryFinished(w, cs->event);
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
  
  if (!_XmTextConvert(w, &cs->selection, &cs->target,
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
TextDestinationCallback(Widget w, 
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
  }
  else if (ds->selection == XA_SECONDARY) {
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
		      (XtCallbackProc) TextSecondaryWrapper,
		      (XtPointer) &insert_select, ds->time);
    } else {
      /*
       * Make selection request to replace the selection
       * with the insert selection.
       */
      XmTransferValue(ds->transfer_id, ((Atom) ds->location_data),
		      (XtCallbackProc) TextSecondaryWrapper,
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
_XmTextInstallTransferTrait(void)
{
  XmeTraitSet((XtPointer)xmTextWidgetClass, XmQTtransfer, 
	      (XtPointer) &TextTransfer);
}


/* leob fix for bug 4191799 */
Atom
_XmTextGetEncodingAtom(Widget w)
{
  int ret_status = 0;
  XTextProperty tmp_prop;
  char * tmp_string = "ABC";  /* these are characters in XPCS, so... safe */
  Atom encoding;
  _XmWidgetToAppContext(w);

  _XmAppLock(app);
  tmp_prop.value = NULL; /* just in case X doesn't do it */

  ret_status = XmbTextListToTextProperty(XtDisplay(w), &tmp_string, 1,
                                         (XICCEncodingStyle)XStdICCTextStyle,
                                         &tmp_prop);
  if (ret_status == Success)
    encoding = tmp_prop.encoding;
  else
    encoding = None;        
  if (tmp_prop.value != NULL) XFree((char *)tmp_prop.value);
  _XmAppUnlock(app);
  return(encoding);
}

