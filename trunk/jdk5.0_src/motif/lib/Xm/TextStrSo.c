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
static char rcsid[] = "$XConsortium: TextStrSo.c /main/14 1996/10/23 16:05:21 cde-osf $"
#endif
#endif
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <ctype.h>
#include <limits.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <Xm/AtomMgr.h>
#include <Xm/TextSelP.h>
#include <Xm/TextStrSoP.h>
#include <Xm/XmosP.h>
#include "XmI.h"		/* for _XmValidTimestamp() */
#include "TextI.h"
#include "TextStrSoI.h"

#ifdef SUN_CTL
#include "XmRenderTI.h"   /* For XmRendFontType */
#include "XmXOC.h"
#endif /* CTL */

/********    Static Function Declarations    ********/

static void AddWidget(XmTextSource source,
		      XmTextWidget tw);
static char * _XmStringSourceGetChar(XmSourceData data,
				     XmTextPosition position);
static int CountLines(XmTextSource source,
		      XmTextPosition start,
		      unsigned long length);
static void RemoveWidget(XmTextSource source,
			 XmTextWidget tw);
static void _XmStringSourceReadString(XmTextSource source,
				      int start,
				      XmTextBlock block);
static XmTextPosition ReadSource(XmTextSource source,
				 XmTextPosition position,
				 XmTextPosition last_position,
				 XmTextBlock block);
static XmTextStatus Replace(XmTextWidget initiator,
			    XEvent *event,
			    XmTextPosition *start,
			    XmTextPosition *end,
			    XmTextBlock block,
#if NeedWidePrototypes
			    int call_callbacks);
#else
                            Boolean call_callbacks);
#endif /* NeedsWidePrototypes */
static void ScanParagraph(XmSourceData data,
			  XmTextPosition *new_position,
			  XmTextScanDirection dir,
			  int ddir,
			  XmTextPosition *last_char);
static XmTextPosition Scan(XmTextSource source,
			   XmTextPosition pos,
			   XmTextScanType sType,
			   XmTextScanDirection dir,
			   int count,
#if NeedWidePrototypes
			   int include);
#else
                           Boolean include);
#endif /* NeedWidePrototypes */
static Boolean GetSelection(XmTextSource source,
			    XmTextPosition *left,
			    XmTextPosition *right);
static void SetSelection(XmTextSource source,
			 XmTextPosition left,
			 XmTextPosition right,
			 Time set_time);

/********    End Static Function Declarations    ********/

#define TEXT_INCREMENT 1024
#define TEXT_INITIAL_INCREM 64

/* Convert a stream of bytes into a char*, BITS16*, or wchar_t* array. 
 * Return number of characters created.
 *
 * If num_chars == 1, don't add a null terminator, else if a null terminator
 * is present on the byte stream, convert it and add it to the character
 * array;  Count returned does not include NULL terminator (just like strlen).
 *
 * This routine assumes that a BITS16 is two-bytes;
 * the routine must be modified if these assumptions are incorrect.
 */

/* ARGSUSED */
int
_XmTextBytesToCharacters(char * characters, 
			 char * bytes,
			 long  num_chars,  /* Wyoming 64-bit fix */ 
#if NeedWidePrototypes
			 int add_null_terminator,
#else
			 Boolean add_null_terminator,
#endif /* NeedWidePrototypes */
			 int max_char_size)
{
  unsigned char * tmp_bytes;
  int num_bytes; 
  int count=0;
  BITS16 *bits16_ptr, temp_bits16;
  wchar_t *wchar_t_ptr;
  
  
  /* If 0 characters requested, or a null pointer passed, dont do
   * anything... just return 0 characters converted.
   */
  
  if (num_chars == 0 || bytes == NULL) return 0;
  
  switch (max_char_size) {
  case 1: {
    (void) memcpy((void*)characters, (void*)bytes, num_chars);
    count = num_chars;
    break;
  } /* end case 1 */
  case 2: {
    bits16_ptr = (BITS16 *) characters;
    tmp_bytes = (unsigned char*) bytes;
    num_bytes = mblen((char*)tmp_bytes, max_char_size);
    if (num_bytes == -1) num_bytes = 1;
    for (temp_bits16 = 0; num_chars > 0 && num_bytes > 0; ) {
      if (num_bytes == 1) {
	temp_bits16 = (BITS16) *tmp_bytes++;
      } else {
	temp_bits16 = (BITS16) *tmp_bytes++;
	temp_bits16 = temp_bits16 << 8;
	temp_bits16 |= (BITS16) *tmp_bytes++;
      }
      *bits16_ptr = temp_bits16;
      count++;
      num_chars--;
      num_bytes = mblen((char*)tmp_bytes, max_char_size); 
      if (num_bytes == -1) num_bytes = 1;
      temp_bits16 = 0;
      bits16_ptr++;
    }
    /* if bytes is NULL terminated, characters should be too */
    if (add_null_terminator == True)
      *bits16_ptr = (BITS16) 0;  
    break;
  } /* end case 2 */
  default: {
    wchar_t_ptr = (wchar_t *)characters;
    count = mbstowcs(wchar_t_ptr, bytes, num_chars);
    if (count < 0)
       count = _Xm_mbs_invalid(wchar_t_ptr, bytes, num_chars);
    if (add_null_terminator == True && count >= 0)
      wchar_t_ptr[count] = (wchar_t)0;
    break;
  } /* end default */
  } /* end switch */
  
  return count;
}

/* Convert an array of char*, BITS16*, or wchar_t* into a stream of bytes.
 * Return the number of bytes placed into 'bytes'
 *
 * Null terminate the byte stream - caller better have alloc'ed enough space!
 */

/* ARGSUSED */
int
_XmTextCharactersToBytes(char * bytes,
			 char * characters,
			 long num_chars, /* Wyoming 64-bit fix */ 
			 long max_char_size) /* Wyoming 64-bit fix */ 
{
  unsigned char *temp_char;
  unsigned char *byte_ptr;
  int count = 0;
  int i, j;
  BITS16 *bits16_ptr, temp_bits16;
  wchar_t *wchars;

  if (num_chars == 0 || characters == 0) {
    *bytes = '\0';
    return 0;
  }
  
  switch (max_char_size) {
  case 1: {
    (void) memcpy((void*)bytes, (void*)characters, num_chars);
    count = num_chars;
    break;
  } /* end case 1 */
  case 2: {
    bits16_ptr = (BITS16 *) characters;
    byte_ptr = (unsigned char*) bytes;
    temp_char = (unsigned char*) XtMalloc (max_char_size);
    for (i = 0; i < num_chars && *bits16_ptr != 0; i++, bits16_ptr++) {
      temp_bits16 = *bits16_ptr;
      /* create an array of chars; char[max_char_size - 1] contains the 
       * low order byte */
      for (j = max_char_size - 1; j >= 0; j--) {
	temp_char[j] = (unsigned char)(temp_bits16 & 0377);
	temp_bits16 = temp_bits16 >> 8;
      }
      /* start with high order byte.  If any byte is 0, skip it. */
      for (j = 0; j < max_char_size; j++) { 
	if (temp_char[j] > 0) {
	  *byte_ptr = temp_char[j];
	  byte_ptr++; count++;
	}
      }
    }
    XtFree ((char*)temp_char);
    *byte_ptr = '\0';
    break;
  } /* end case 2 */
  default: {
    int nbytes;

    wchars = (wchar_t *)characters;
    for (i = 0; i < num_chars && *wchars != 0L; i++, wchars++) {
      nbytes = wctomb(bytes, *wchars);
      if (nbytes < 0)
      {
	nbytes = 1; /* illegal char */
	bytes[0] = *wchars;
      }
      count += nbytes;
      bytes += nbytes;
    }
    if (count >= 0)
      bytes = '\0';
    break;
  } /* end default */
  } /* end switch */
  return (count);    /* return the number of bytes placed in bptr */
}

char * 
_XmStringSourceGetString(XmTextWidget tw,
			 XmTextPosition from,
			 XmTextPosition to,
#if NeedWidePrototypes
			 int want_wchar)
#else
                         Boolean want_wchar)
#endif /* NeedWidePrototypes */
{
  char *buf;
  wchar_t *wc_buf;
  XmTextBlockRec block;
  int destpos;
  XmTextPosition pos, ret_pos;
  int return_val = 0;

  destpos = 0;
  if (!want_wchar) {
    /* NOTE: to - from could result in a truncated long. */
    buf = XtMalloc(((int)(to - from) + 1) * (int)tw->text.char_size);
    for (pos = from; pos < to; ) {
      pos = ReadSource(tw->text.source, pos, to, &block);
      if (block.length == 0)
	break;
      
      (void)memcpy((void*)&buf[destpos], (void*)block.ptr, block.length);
      destpos += block.length;
    }
    buf[destpos] = 0;
    return buf;
  } else { /* want buffer of wchar_t * data */
    /* NOTE: to - from could result in a truncated long. */
    buf = XtMalloc(((int)(to - from) + 1) * sizeof(wchar_t));
    wc_buf = (wchar_t *)buf;
    for (pos = from; pos < to; ) {
      ret_pos = ReadSource(tw->text.source, pos, to, &block);
      if (block.length == 0)
	break;
      
      /* NOTE: ret_pos - pos could result in a truncated long. */
      return_val =  mbstowcs(&wc_buf[destpos], block.ptr,
			     (unsigned int) (ret_pos - pos));
      if (return_val < 0)
         return_val = _Xm_mbs_invalid(&wc_buf[destpos], block.ptr,
                             (unsigned int) (ret_pos - pos));
      if (return_val > 0) destpos += return_val;
      pos = ret_pos;
    }
    wc_buf[destpos] = (wchar_t)0L;
    return ((char*)wc_buf);
  }
}


static void 
AddWidget(XmTextSource source,
	  XmTextWidget tw)
{
  XmSourceData data = source->data;
  data->numwidgets++;
  data->widgets = (XmTextWidget *)
    XtRealloc((char *) data->widgets,
	      (unsigned) (sizeof(XmTextWidget) * data->numwidgets));
  data->widgets[data->numwidgets - 1] = tw;
  
  if (data->numwidgets == 1)
    _XmTextSetHighlight((Widget) tw, 0, tw->text.last_position, 
		       XmHIGHLIGHT_NORMAL);
  else {
    tw->text.highlight.list = (_XmHighlightRec *)
      XtRealloc((char *) tw->text.highlight.list, 
		data->widgets[0]->text.highlight.maximum *
		sizeof(_XmHighlightRec));
    tw->text.highlight.maximum = data->widgets[0]->text.highlight.maximum;
    tw->text.highlight.number = data->widgets[0]->text.highlight.number;
    memmove((void *) tw->text.highlight.list, 
	    (void *) data->widgets[0]->text.highlight.list,
	    (size_t) data->widgets[0]->text.highlight.number *
	    sizeof(_XmHighlightRec));
  }
  
  
  if (data->hasselection && data->numwidgets == 1) {
    Time select_time = XtLastTimestampProcessed(XtDisplay((Widget)tw));
    if (!select_time) select_time = _XmValidTimestamp((Widget)tw);
    if (!XmePrimarySource((Widget) data->widgets[0], select_time)) {
      (*source->SetSelection)(source, 1, 0, select_time);
    } else {
      XmAnyCallbackStruct cb;
      
      data->prim_time = select_time;
      cb.reason = XmCR_GAIN_PRIMARY;
      cb.event = NULL;
      XtCallCallbackList ((Widget) data->widgets[0],
			  data->widgets[0]->text.gain_primary_callback,
			  (XtPointer) &cb);
    }
  }
}

/********************************<->***********************************/
static char * 
_XmStringSourceGetChar(XmSourceData data,
		       XmTextPosition position)       /* starting position */
{
  /* gap_size is the number of character in the gap, not number of bytes */
  register int gap_size;
  register XmTextPosition char_pos;
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  int char_size;
  
  if (tw->text.char_size > 1) {
    if (tw->text.char_size == 2)
      char_size = 2;
    else
      char_size = sizeof(wchar_t);
    char_pos = position * char_size;
    
    /* regardless of what it contains, data->ptr is treated as a char * ptr */
    if (data->ptr + char_pos < data->gap_start)
      return (&data->ptr[char_pos]);
    
    gap_size = (data->gap_end - data->gap_start) / char_size;
    if (position + gap_size >= data->maxlength)
      return ("");
    return (&data->ptr[(position + gap_size) * char_size]);
  } else {
    char_pos = position;
    /* regardless of what it contains, data->ptr is treated as a char * ptr */
    if (data->ptr + char_pos < data->gap_start)
      return (&data->ptr[char_pos]);
    
    gap_size = (data->gap_end - data->gap_start);
    if (char_pos + gap_size >= data->maxlength)
      return ("");
    return (&data->ptr[(char_pos + gap_size)]);
  }
}


/*DELTA: length IS NOW TREATED AS NUMBER OF CHARACTERS - CALLERS MUST CHANGE*/
static int 
CountLines(XmTextSource source,
	   XmTextPosition start,
	   unsigned long length)
{
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  int num_lines = 0;
  unsigned long seg_length;
  char *ptr;
  BITS16 *bits16_ptr, *bits16_gap_start, *bits16_gap_end;
  wchar_t *wchar_t_ptr, *wchar_t_gap_start, *wchar_t_gap_end;
  
  /* verify that the 'start' and 'length' parameters are reasonable */
  
  if (start + length > data->length)
    length = data->length - start;
  if (length == 0) return num_lines;
  
  seg_length = (data->gap_start - data->ptr) / (tw->text.char_size < 3 ?
						(int)tw->text.char_size :
						sizeof(wchar_t));
  
  /* make sure the segment length is not greater than the length desired */
  if (length < seg_length) seg_length = length;
  
  switch ((int)tw->text.char_size) {
  case 1: {
    /* setup the variables for the search of new lines before the gap */
    ptr = data->ptr + start;
    
    /* search up to gap */
    while (seg_length--) {
      if (*ptr++ == *(data->PSWC_NWLN)) ++num_lines;
    }
    
    /* check to see if we need more data after the gap */
    if ((int)length > data->gap_start - (data->ptr + start)) {
      if (data->gap_start - (data->ptr + start) > 0) /* if we searched
						      * before gap,
						      * adjust length */
	length -= data->gap_start - (data->ptr + start);
      ptr = data->gap_end;
      
      /* continue search till length is completed */
      while (length--) {
	if (*ptr++ == *(data->PSWC_NWLN)) ++num_lines;
      }
    }
    break;
  } /* end case 1 */
  case 2: {
    /* setup the variables for the search of new lines before the gap */
    bits16_ptr = (BITS16 *) data->ptr;
    bits16_gap_start = (BITS16 *) data->gap_start;
    bits16_gap_end = (BITS16 *) data->gap_end;
    bits16_ptr += start;
    
    /* search up to gap */
    while (seg_length--) {
      if (*bits16_ptr++ == *(BITS16 *)(data->PSWC_NWLN)) ++num_lines;
    }
    
    /* check to see if we need more data after the gap */
    if ((int)length > bits16_gap_start - ((BITS16 *)data->ptr + start)) {
      /* if we searched before gap, adjust length */
      if (bits16_gap_start - ((BITS16 *)data->ptr + start) > 0)
	length -= bits16_gap_start - ((BITS16 *)data->ptr + start);
      bits16_ptr = bits16_gap_end;
      
      /* continue search till length is completed */
      while (length--) {
	if (*bits16_ptr++ == *(BITS16 *)(data->PSWC_NWLN)) ++num_lines;
      }
    }
    break;
  } /* end case 2 */
  default: {
    /* setup the variables for the search of new lines before the gap */
    wchar_t_ptr = (wchar_t *) data->ptr;
    wchar_t_gap_start = (wchar_t *) data->gap_start;
    wchar_t_gap_end = (wchar_t *) data->gap_end;
    wchar_t_ptr += start;
    
    /* search up to gap */
    while (seg_length--) {
      if (*wchar_t_ptr++ == *(wchar_t *)(data->PSWC_NWLN)) ++num_lines;
    }
    
    /* check to see if we need more data after the gap */
    if ((int)length > wchar_t_gap_start - ((wchar_t *)data->ptr + start)) {
      /* if we searched before gap, adjust length */
      if (wchar_t_gap_start - ((wchar_t *)data->ptr + start) > 0)
	length -= wchar_t_gap_start - ((wchar_t *)data->ptr + start);
      wchar_t_ptr = wchar_t_gap_end;
      
      /* continue search till length is completed */
      while (length--) {
	if (*wchar_t_ptr++ == *(wchar_t *)(data->PSWC_NWLN)) ++num_lines;
      }
    }
    break;
  } /* end default */
  } /* end switch */
  return num_lines;
}

static void 
RemoveWidget(XmTextSource source,
	     XmTextWidget tw)
{
  XmSourceData data = source->data;
  int i;
  for (i=0; i<data->numwidgets; i++) {
    if (data->widgets[i] == tw) {
      XmTextPosition left, right;
      Boolean had_selection = False;
      Time select_time =
	XtLastTimestampProcessed(XtDisplay((Widget)tw));
      
      if (data->hasselection) {
	(*source->GetSelection)(source, &left, &right);
	(*source->SetSelection)(source, 1, -999, select_time);
	had_selection = True;
      }
      data->numwidgets--;
      data->widgets[i] = data->widgets[data->numwidgets];
      if (i == 0 && data->numwidgets > 0 && had_selection)
	(*source->SetSelection)(source, left, right, select_time);
      if (data->numwidgets == 0) _XmStringSourceDestroy(source);
      return;
    }
  }
}

Boolean *
_XmStringSourceGetPending(XmTextWidget tw)
{
  Boolean *pending;
  XmSourceData data = tw->text.source->data;
  int i;
  
  pending = (Boolean *)XtMalloc(data->numwidgets*sizeof(Boolean));
  for (i=0; i<data->numwidgets; i++) 
    pending[i] = ((XmTextWidget)data->widgets[i])->text.pendingoff;

  return pending;
}

void
_XmStringSourceSetPending(XmTextWidget tw,
			  Boolean *pending)
{
  XmSourceData data = tw->text.source->data;
  int i;
  
  if ((long)pending > 1)
    for (i=0; i<data->numwidgets; i++) 
      ((XmTextWidget)data->widgets[i])->text.pendingoff = pending[i];
  else
    for (i=0; i<data->numwidgets; i++) 
      ((XmTextWidget)data->widgets[i])->text.pendingoff = 
	(Boolean)(long)pending;
}

/*
 * Determines where to move the gap and calls memmove to move the gap.
 */
/********************************<->***********************************/
void 
_XmStringSourceSetGappedBuffer(XmSourceData data,
			       XmTextPosition position) /* starting position */
{
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  int count, char_size = (tw->text.char_size < 3 ? 
			  (int)tw->text.char_size :
			  sizeof(wchar_t));
  
  /* if no change in gap placement, return */
  if (data->ptr + (position * char_size) == data->gap_start)
    return;
  
  if (data->ptr + (position * char_size) < data->gap_start) {
    /* move gap to the left */
    count = data->gap_start - 
      (data->ptr + (position * char_size));
    memmove(data->gap_end - count, data->ptr + (position*char_size), count);
    data->gap_start -= count; /* ie, data->gap_start = position; */
    data->gap_end -= count;   /* ie, data->gap_end = position + gap_size; */
  } else {
    /* move gap to the right */
    count = (data->ptr + 
	     (position * char_size)) - data->gap_start;
    memmove(data->gap_start, data->gap_end, count);
    data->gap_start += count; /* ie, data->gap_start = position; */
    data->gap_end += count;   /* ie, data->gap_end = position + gap_size; */
  }
}

/********************************<->***********************************/
/* The only caller of this routine expects to get char* in block */
static void 
_XmStringSourceReadString(XmTextSource source,
			  int start,
			  XmTextBlock block)
{
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  int gap_size = data->gap_end - data->gap_start;
  int byte_start = start * (tw->text.char_size < 3 ?
			    (int)tw->text.char_size :
			    sizeof(wchar_t));
#ifdef SUN_CTL
  /* 
     This function handles three case:

     (1) the entire range requested falls before the gap
     (2) the entire range requested falls after the gap
     (3) the entire range requested straddles the gap

     Note that the second case could be expressed as:
        (data->ptr + byte_start >= data->gap_start)
  */
#endif /* CTL */
  if (data->ptr + byte_start + block->length <= data->gap_start)
    block->ptr = data->ptr + byte_start;
  else if (data->ptr + byte_start + gap_size >= data->gap_end)
    block->ptr = data->ptr + byte_start + gap_size;
  else {
    block->ptr = data->ptr + byte_start;
    block->length = data->gap_start - (data->ptr + byte_start);
  }
}

/* Caller wants block to contain char*; _XmStringSourceReadString provides
 * char*, BITS16* or wchar_t*; so we need to modify what it gives us.
 */
static XmTextPosition 
#ifdef SUN_CTL
NONCTLReadSource(XmTextSource source, /* another name for CTL version */
#else /* CTL */
ReadSource(XmTextSource source,
#endif /* CTL */
	   XmTextPosition position,       /* starting position */
	   XmTextPosition last_position,  /* The last position we're interested
					     in.  Don't return info about any
					     later positions. */
	   XmTextBlock block)            /* RETURN: text read in */
{
  XmTextPosition return_pos;
  int num_bytes;
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  int char_size = (tw->text.char_size < 3 ?
		   (int)tw->text.char_size :
		   sizeof(wchar_t));
  
  if (last_position > data->length) last_position = data->length;
  /* NOTE: the length calculation could result in a truncated long */
  block->length = (int)((last_position - position) * char_size);
  if (block->length < 0 ) block->length = 0;
  block->format = XmFMT_8_BIT;
  _XmStringSourceReadString(source, (int)position, block);
  
  if (block->length > 0) {
    if (data->old_length == 0) {
      data->value = (char *) 
	XtMalloc((unsigned)(block->length + 1) * (int)tw->text.char_size);
      data->old_length = block->length;
    } else if (block->length > data->old_length) {
      data->value = XtRealloc(data->value, (unsigned)
			      ((block->length + 1) * (int)tw->text.char_size));
      data->old_length = block->length;
    }
    
    if ((int)tw->text.char_size == 1) {
      return_pos = position + block->length;
    } else {
      return_pos = position + (block->length / char_size);
      num_bytes = _XmTextCharactersToBytes(data->value, block->ptr, 
					   block->length / char_size,
					   (int)tw->text.char_size);
      block->length = num_bytes;
      block->ptr = data->value;
    }
    return return_pos;
  } else
    return 0;
}

#ifdef SUN_CTL
/* Caller wants block to contain char*; _XmStringSourceReadString provides
 * char*, BITS16* or wchar_t*; so we need to modify what it gives us.
 */

/*
  The CTL change is to always return the range requested, jumping the gap
  and calling _XmTextCharactersToBytes twice if need be.

  NB: On closer reading, this whole architecture is flawed.  Namely, this 
  whole gapped buffer and block business is clearly not reentrant. block->ptr
  will be set either (1) to a location on either side of the gap in the gapped 
  buffer, if char_size == 1; or (2) to data->value, a block of heap memory
  that has had a portion of the gapped buffer copied into it via 
  _XmTextCharactersToBytes().

  To make matters worse, it's not going to be easy to move block->ptr to the
  heap, since block is usually/always on the stack somewhere up the chain of
  callers, meaning that there's no explicit destroy/free in the owning call 
  frame.

  We're not making anything any worse here, though by also forcing char_size
  == 1 to use data->value rather than a pointer directly into the gapped 
  buffer, what has been a benign race condition may become malign for some
  apps.
 */
static XmTextPosition 
_ReadSource(XmTextSource source,
	    XmTextPosition position,       /* starting position */
	    XmTextPosition last_position,  /* The last position we're interested
					      in.  Don't return info about any
					      later positions. */
	    XmTextBlock block,	           /* RETURN: text read in */
	    int data_value_offset)         /* so we can recurse across the gap */
{
  XmTextPosition return_pos;
  int num_bytes;
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  int char_size = (tw->text.char_size < 3 ?
		   (int)tw->text.char_size :
		   sizeof(wchar_t));
  
  if (last_position > data->length) 
    last_position = data->length;

  /* NOTE: the length calculation could result in a truncated long */
  block->length = (int)((last_position - position) * char_size);
  if (block->length < 0 ) 
    block->length = 0;
  block->format = XmFMT_8_BIT;
  /* NOTE: In the case where the requested range straddles the gap,
     block->length will have been reduced. */
  _XmStringSourceReadString(source, (int)position, block);
  
  if (block->length > 0) {
    if (data->old_length == 0) {
	  /* Old way was:
  	        XtMalloc((unsigned)(block->length + 1) * (int)tw->text.char_size);
	     But, that can't be right since length is already in bytes
	     */
      data->value = (char *) XtMalloc((unsigned)(block->length + 1));
      data->old_length = block->length;
    } else {
      if(data_value_offset + block->length > data->old_length) {
	  /* We're going to need to Realloc */
	data->old_length = data_value_offset + block->length;	      		

	/* Ditto previous comment about (... * tw->text.char_size) */
	data->value = XtRealloc(data->value, data->old_length + 1);
      }
    }

      /* Here we force the copy into data->value, even when char_size == 1,
	 so that we can do our appending of the second piece down below
	 if we're straddling the gap. The old way just set return_pos and
	 left block->ptr as is, pointing into the gapped buffer. 

	 _XmTextCharactersToBytes just turns into memcpy for char_size == 1.
	 Yes, this is slightly slower.
	 */
    return_pos    = position + (block->length / char_size);
    num_bytes     = _XmTextCharactersToBytes(data->value + data_value_offset, 
					     block->ptr, block->length / char_size,
					     (int)char_size); /* (int)tw->text.char_size); */
    block->length = num_bytes + data_value_offset;
    block->ptr    = data->value;
    
    /* This is the bulk of the CTL change: if we didn't get the whole range
       on the first try, go around for one more pass. */
    if(return_pos != last_position) {
	/* Recurse, copying the second segment of the overall range to 
	   be concatenated to the first in block->ptr = data->value. */
      return_pos = _ReadSource(source, return_pos, last_position, block, num_bytes);
	
      if(return_pos != last_position) {
	    /* This time we'd better have the whole 
	       requested range, if not, serious error */
	  XmeWarning((Widget)source->data->widgets[0], "-- fatal error in XmText:_ReadSource()\n");
      }
    }

    return return_pos;
  } else
    return 0;
}

static XmTextPosition 
CTLReadSource(XmTextSource source,
	   XmTextPosition position,       /* starting position */
	   XmTextPosition last_position,  /* The last position we're interested
					     in.  Don't return info about any
					     later positions. */
	   XmTextBlock block)            /* RETURN: text read in */
{
  return _ReadSource(source, position, last_position, block, 0);
}
#endif /* CTL */

#ifdef SUN_CTL
static XmTextPosition 
ReadSource(XmTextSource source,
	   XmTextPosition position,       /* starting position */
	   XmTextPosition last_position,  /* The last position we're interested
					     in.  Don't return info about any
					     later positions. */
	   XmTextBlock block)            /* RETURN: text read in */
{
  XmTextWidget tw = (XmTextWidget) source->data->widgets[0];
	
  if (TextW_LayoutActive(tw))
    return CTLReadSource(source, position, last_position, block);
  else
    return NONCTLReadSource(source, position, last_position, block);
}
#endif /* CTL */

void
_XmTextValidate(XmTextPosition * start,
		XmTextPosition * end,
		long maxsize) /* Wyoming 64-bit fix */ 
{
  if (*start < 0) *start = 0;
  if (*start > maxsize) {
    *start = maxsize;
  }
  if (*end < 0) *end = 0;
  if (*end > maxsize) {
    *end = maxsize;
  }
  
  if (*start > *end) { 
    XmTextPosition tmp; /* tmp variable for swapping positions */
    tmp = *end;    
    *end = *start;
    *start = tmp;
  }
}

Boolean 
_XmTextModifyVerify(XmTextWidget initiator,
		    XEvent *event,
		    XmTextPosition *start,
		    XmTextPosition *end,
		    XmTextPosition *cursorPos,	/* RETURN, may be NULL if not */
		    XmTextBlock block,
		    XmTextBlock newblock,	/* RETURN */
		    Boolean *freeBlock)
{
  register XmSourceData data = initiator->text.source->data;
  register long delta;
  register int block_num_chars;  /* number of characters in the block */
  XmTextPosition newInsert = initiator->text.cursor_position;
  XmTextVerifyCallbackStruct tvcb;
  XmTextVerifyCallbackStructWcs wcs_tvcb;
  XmTextBlockRecWcs wcs_newblock;
  
  *freeBlock = False;
  
  if (*start == *end && block->length == 0) return False;
  
  _XmTextValidate(start, end, data->length);
  
    newblock->length = block->length; 	/* RETURNed values */
    newblock->format = block->format; 
    newblock->ptr = block->ptr; 
    
    if (!initiator->text.modify_verify_callback &&
        !initiator->text.wcs_modify_verify_callback) 
    {
        /* we have neither callback, so do expensive 
        ** computation only if cursorPos is needed 
        */
        if (cursorPos)
  	{
			  /* Bug Id : 1217687/4128045/4154215 */
    	block_num_chars = TextCountCharacters((Widget)initiator, block->ptr, block->length);
    	*cursorPos = *start + block_num_chars;
  	}
      return True;
    }
  
    /* cursorPos may be needed to be returned. If not, and if the text is
    ** not editable, can drop out now
    */
    if (!cursorPos && !data->editable)
      return False;
  
    /* there is at least one callback, so block_num_chars is needed; 
    ** this is the potentially-expensive operation that we're trying to avoid 
    */
		      /* Bug Id : 1217687/4128045/4154215 */
    block_num_chars = TextCountCharacters((Widget)initiator, block->ptr, block->length);
    if (cursorPos) *cursorPos = *start + block_num_chars;
    
    if (!data->editable)	/* if cursorPos was needed, it's set; can now drop out */
      return False;
  
    /* we have at least one callback on editable text, and cursorPos may or may
    ** not need to be set. block_num_chars has been set, so perform some other 
    ** quick evaluations on whether or not the modify is reasonable. Then
    ** call one or both of the callbacks, and if necessary reset cursorPos.
    */
  
    delta = block_num_chars - (*end - *start);
    if (delta > 0 && (data->length + delta > data->maxallowed))
      return False;
  
  /* If both modify_verify and modify_verify_wcs are registered:
   *    - first call the char* callback, then
   *    - pass the modified data from the char* callback to the
   *      wchar_t callback.
   * If programmers set both callback lists, they get's what they asked for.
   */
  
  wcs_newblock.wcsptr = (wchar_t *)NULL;
  wcs_newblock.length = 0;
  
  /* If there are char* callbacks registered, call them. */
  if (initiator->text.modify_verify_callback) {
    /* Fill in the block to pass to the callback. */
    if (block->length) {
      newblock->ptr = (char *) XtMalloc(block->length + 1);
      *freeBlock = True;
      (void) memcpy((void*) newblock->ptr, (void*) block->ptr,
		    block->length);
      newblock->ptr[block->length] = '\0';
    }
    
    /* Call Verification Callback to indicate that text is being modified */
    tvcb.reason = XmCR_MODIFYING_TEXT_VALUE;
    tvcb.event = event;
    tvcb.currInsert = (XmTextPosition) (initiator->text.cursor_position);
    tvcb.newInsert = (XmTextPosition) (initiator->text.cursor_position);
    tvcb.startPos = *start;
    tvcb.endPos = *end;
    tvcb.doit = True;
    tvcb.text = newblock;
    XtCallCallbackList ((Widget) initiator,
			initiator->text.modify_verify_callback,
			(XtPointer) &tvcb);
    /* If doit flag is false, application wants to negate the action,
     * so free allocate space and return False.
     */
    if (!tvcb.doit) {
      if (newblock->ptr && newblock->ptr != block->ptr) 
	XtFree(newblock->ptr);
      *freeBlock = False;
      return False;
    } else {
      *start = tvcb.startPos;
      *end = tvcb.endPos;
      newInsert = tvcb.newInsert;
      _XmTextValidate (start, end, data->length);
      if (tvcb.text != newblock || tvcb.text->ptr != newblock->ptr) {
	newblock->length = tvcb.text->length;
	if (newblock->ptr && newblock->ptr != block->ptr) 
	  XtFree(newblock->ptr);
	*freeBlock = False;
	if (newblock->length) {
	  newblock->ptr = XtMalloc(newblock->length + 1);
	  *freeBlock = True;
	  (void)memcpy((void*)newblock->ptr, (void*)tvcb.text->ptr, 
		       tvcb.text->length);
	} else newblock->ptr = NULL;
      }
      newblock->format = tvcb.text->format;
		        /* Bug Id : 1217687/4128045/4154215 */
      block_num_chars = TextCountCharacters((Widget)initiator, newblock->ptr,
					       newblock->length);
      
      delta = block_num_chars - (*end - *start);
      
      if (delta > 0 && data->length + delta > data->maxallowed &&
	  (!UnderVerifyPreedit(initiator))) {
	if (newblock->ptr && newblock->ptr != block->ptr) 
	  XtFree(newblock->ptr);
	*freeBlock = False;
	return False;
      }
    }
  }  /* end if there are char* modify verify callbacks */
  
  if (initiator->text.wcs_modify_verify_callback) {
    wcs_newblock.wcsptr = (wchar_t *)XtMalloc((unsigned)sizeof(wchar_t) *
					      (newblock->length + 1));
    wcs_newblock.length = mbstowcs(wcs_newblock.wcsptr, newblock->ptr,
				   block_num_chars);
    if (wcs_newblock.length < 0)
       wcs_newblock.length = _Xm_mbs_invalid(wcs_newblock.wcsptr,
				newblock->ptr, block_num_chars);
    wcs_tvcb.reason = XmCR_MODIFYING_TEXT_VALUE;
    wcs_tvcb.event = event;
    wcs_tvcb.currInsert = initiator->text.cursor_position;
    wcs_tvcb.newInsert = initiator->text.cursor_position;
    wcs_tvcb.startPos = *start;
    wcs_tvcb.endPos = *end;
    wcs_tvcb.doit = True;
    wcs_tvcb.text = &wcs_newblock;
    
    XtCallCallbackList((Widget) initiator,
		       initiator->text.wcs_modify_verify_callback,
		       (XtPointer) &wcs_tvcb);
    if (!wcs_tvcb.doit) {
      if (newblock->ptr && newblock->ptr != block->ptr)
	XtFree(newblock->ptr);
      *freeBlock = False;
      if (wcs_newblock.wcsptr) XtFree((char*)wcs_newblock.wcsptr);
      return False;
    } else {
      *start = wcs_tvcb.startPos;
      *end = wcs_tvcb.endPos;
      newInsert = wcs_tvcb.newInsert;
      _XmTextValidate (start, end, data->length);
      /* use newblock as a temporary holder and put the char*
       * data there */
      if (newblock->ptr && newblock->ptr != block->ptr) {
	XtFree(newblock->ptr);
	newblock->ptr = NULL;
      }
      *freeBlock = False;
      if (wcs_tvcb.text->length) {
	newblock->ptr = (char*) XtMalloc((unsigned)
					 (1 + wcs_tvcb.text->length) *
					 (int)initiator->text.char_size);
	*freeBlock = True;
	wcs_tvcb.text->wcsptr[wcs_tvcb.text->length] = (wchar_t) 0L;
	/* NOTE: wcstombs returns a long which could be truncated */
	newblock->length = (int) wcstombs(newblock->ptr, 
					  wcs_tvcb.text->wcsptr,
					  (wcs_tvcb.text->length + 1) *
					  (int)initiator->text.char_size);
	if (newblock->length < 0)
           newblock->length = (int) _Xm_wcs_invalid(newblock->ptr,
                                                    wcs_tvcb.text->wcsptr,
                 (wcs_tvcb.text->length + 1) * (int)initiator->text.char_size);
      } else {
	newblock->ptr = NULL;
	newblock->length = 0;
      }
      
      block_num_chars = wcs_tvcb.text->length;
      delta = block_num_chars - (*end - *start);
      
      /* if the wcstombs found bad data, newblock->length is negative */
      if ((delta > 0 && data->length + delta > data->maxallowed &&
	  (!UnderVerifyPreedit(initiator))) ||
	  newblock->length < 0) {
	
	if (newblock->ptr && newblock->ptr != block->ptr)
	  XtFree(newblock->ptr);
	*freeBlock = False;
	if (wcs_newblock.wcsptr) XtFree((char*)wcs_newblock.wcsptr);
	return False;
      }
    }
    
    /* If we alloced space for the wcs_newblock, we need to clean it up */
    if (wcs_newblock.wcsptr) XtFree((char*)wcs_newblock.wcsptr);
    
  }  /* end if there are wide char modify verify callbacks */
  
  if (cursorPos)
  {
	  if (initiator->text.cursor_position != newInsert)	/* true only if we have callbacks */
	  {
	    if (newInsert > data->length + delta) {
	      *cursorPos = data->length + delta;
	    } else if (newInsert < 0) {
	      *cursorPos = 0;
	    } else {
	      *cursorPos = newInsert;
	    }
	  }
	  else
	    *cursorPos = *start + block_num_chars;
  }
  
  return True;
}

/*ARGSUSED*/
static XmTextStatus 
Replace(XmTextWidget initiator,
        XEvent * event,		/* unused */
        XmTextPosition *start,
        XmTextPosition *end,
        XmTextBlock block,
#if NeedWidePrototypes
        int call_callbacks)	/* unused */
#else
        Boolean call_callbacks)	/* unused */
#endif
{
  register XmSourceData data = initiator->text.source->data;
  register int i;
  register long delta;
  register int block_num_chars;  /* number of characters in the block */
  int gap_size;
  int old_maxlength;
  int char_size = (initiator->text.char_size < 3 ?
		   (int)initiator->text.char_size :
		   sizeof(wchar_t));
  
  if (*start == *end && block->length == 0) return EditReject;
  
  _XmTextValidate(start, end, data->length);
  
		    /* Bug Id : 1217687/4128045/4154215 */
  block_num_chars = TextCountCharacters((Widget)initiator, block->ptr, block->length);
  delta = block_num_chars - (*end - *start);
  
  if (!data->editable ||
      (delta > 0 && data->length + delta > data->maxallowed &&
	(!UnderVerifyPreedit(initiator))))
    return EditError;
  
  /**********************************************************************/
  
  initiator->text.output->DrawInsertionPoint(initiator,
					     initiator->text.cursor_position,
					     off);
  
  /* Move the gap to the editing position (*start). */
  _XmStringSourceSetGappedBuffer(data, *start);
  
  for (i=0; i<data->numwidgets; i++) {
    _XmTextDisableRedisplay(data->widgets[i], TRUE);
    if (data->hasselection)
      _XmTextSetHighlight((Widget)data->widgets[i], data->left, 
			 data->right, XmHIGHLIGHT_NORMAL);
  }
  
  old_maxlength = data->maxlength;
  if (data->length + delta >= data->maxlength) {
    int gap_start_offset, gap_end_offset;
    
    while (data->length + delta >= data->maxlength) {
      if (data->maxlength < TEXT_INCREMENT)
	data->maxlength *= 2;
      else
	data->maxlength += TEXT_INCREMENT;
    }
    
    gap_start_offset = data->gap_start - data->ptr;
    gap_end_offset = data->gap_end - data->ptr;
    data->ptr = XtRealloc(data->ptr, (unsigned) 
			  ((data->maxlength) * char_size));
    data->gap_start = data->ptr + gap_start_offset;
    data->gap_end = data->ptr + gap_end_offset +
      (char_size * (data->maxlength - old_maxlength));
    if (gap_end_offset != (old_maxlength * char_size))
      memmove(data->gap_end, data->ptr + gap_end_offset, 
	      (char_size * old_maxlength) - gap_end_offset);
    /* Do something to move the allocated space into the buffer */
  } 
  
  /* NOTE: the value of delta could be truncated by cast to int. */
  data->length += (int) delta;
  
  if (data->hasselection && *start < data->right && *end > data->left) {
    if (*start <= data->left) {
      if (*end < data->right) {
	data->left = *end; /* delete encompasses left half of the
			      selection so move left endpoint */
      } else {
	data->right = data->left; /* delete encompasses the selection
				     so set selection to NULL */
      }
    } else {
      if (*end >= data->right) {
	data->right = *start; /* delete encompasses the right half of the
				 selection so move right endpoint */
      } else {
	data->right -= (*end - *start); /* delete is completely within the
					   selection so shrink the selection */
      }
    }
  }
  
  /* delete data */
  gap_size = data->gap_end - data->gap_start;
  /* expand the end of the gap to the right */
  if ((data->ptr + gap_size + (*end * char_size)) > data->gap_end)
    data->gap_end += ((*end - *start) * char_size);
  
  /* add data */
  /* copy the data into the gap_start and increment the gap start pointer */
  /* convert data from char* to characters and copy into the gapped buffer */
  if ((int)initiator->text.char_size == 1) {
    for (i=0; i < block->length; i++) {
      /* if (data->gap_start == data->gap_end) break; */
      *data->gap_start++ = block->ptr[i];
    }
    
  } else {
    data->gap_start += char_size * 
      _XmTextBytesToCharacters(data->gap_start,
			       &block->ptr[0], 
			       block_num_chars, False,
			       (int)initiator->text.char_size);
  }
  
  if (data->hasselection && data->left != data->right) {
    if (*end <= data->left) {
      data->left += delta;
      data->right += delta;
    }
    if (data->left > data->right)
      data->right = data->left;
  }
  
  for (i=0; i<data->numwidgets; i++) {
    _XmTextInvalidate(data->widgets[i], *start, *end, delta);
    _XmTextUpdateLineTable((Widget) data->widgets[i], *start,
			   *end, block, True);
    if (data->hasselection)
      _XmTextSetHighlight((Widget)data->widgets[i], data->left, 
			 data->right, XmHIGHLIGHT_SELECTED);
    
    _XmTextEnableRedisplay(data->widgets[i]);
  }
  initiator->text.output->DrawInsertionPoint(initiator,
					     initiator->text.cursor_position, 
					     on);
  
  if (data->maxlength != TEXT_INITIAL_INCREM &&
      ((data->maxlength > TEXT_INCREMENT &&
	data->length <= data->maxlength - TEXT_INCREMENT) ||
       data->length <= data->maxlength >> 1)) {
    /* Move the gap to the last position. */
    _XmStringSourceSetGappedBuffer(data, data->length);
    
    data->maxlength = TEXT_INITIAL_INCREM;
    
    while (data->length >= data->maxlength) {
      if (data->maxlength < TEXT_INCREMENT)
	data->maxlength *= 2;
      else
	data->maxlength += TEXT_INCREMENT;
    }
    
    data->ptr = XtRealloc(data->ptr, (unsigned) 
			  ((data->maxlength) * char_size));
    data->gap_start = data->ptr + (data->length * char_size);
    data->gap_end = data->ptr + ((data->maxlength - 1) * char_size);
  }
  
  return EditDone;
}

#define Increment(data, position, direction)\
{\
  if (direction == XmsdLeft) {\
    if (position > 0) \
      position--;\
  } else {\
    if (position < data->length)\
      position++;\
  }\
}

#define Look(data, position, dir) \
    ((dir == XmsdLeft) \
      ? ((position) ? _XmStringSourceGetChar(data, position - 1) \
	            : NULL) \
      : ((position == data->length) ? NULL \
	                            : _XmStringSourceGetChar(data, position)))
   
static void 
ScanParagraph(XmSourceData data,
	      XmTextPosition *new_position,
	      XmTextScanDirection dir,
	      int ddir,
	      XmTextPosition *last_char)
{
  Boolean found = False;
  XmTextPosition position = *new_position;
  char mb_char[1 + MB_LEN_MAX];
  char * c;
  
  while (position >= 0 && position <= data->length) {
    /* DELTA: Look now returns a pointer */
    /* DELTA: EFFECIENCY: LEAVE AS SHORT*, INT*, ... COMPARE TO PSWC_NWLN */
    c = Look(data, position, dir);
    (void) _XmTextCharactersToBytes(mb_char, c, 1, 
				    (int)data->widgets[0]->text.char_size);
    if (mb_char && *mb_char == '\n') {
      /* DELTA: Look now returns a pointer */
      c = Look(data, position + ddir, dir);
      (void) _XmTextCharactersToBytes(mb_char, c, 1,
				      (int)data->widgets[0]->text.char_size);
      while (mb_char && isspace((unsigned char)*mb_char)) {
	if (*mb_char == '\n') {
	  found = True;
	  while (mb_char && isspace((unsigned char)*mb_char)) {
	    /* DELTA: Look now returns a pointer */
	    c = Look(data, position + ddir, dir);
	    (void) _XmTextCharactersToBytes(mb_char, c, 1,
					(int)data->widgets[0]->text.char_size);
	    Increment(data, position, dir);
	  }
	  break;
	}
	/* DELTA: Look now returns a pointer */
	c = Look(data, position + ddir, dir);
	(void) _XmTextCharactersToBytes(mb_char, c, 1,
					(int)data->widgets[0]->text.char_size);
	Increment(data, position, dir);
	/* BEGIN 3145 fix -- Do not bypass a nonspace character */
    /* Bug Id : 4199568, ensure that C is not Null before checking with Isspace */
	if (c && !isspace((unsigned char)*c))
	  *last_char = (position) + ddir;
	/* END 3145 */
      }
      if (found) break;
    } else if (mb_char && !isspace((unsigned char)*mb_char)) {
      *last_char = (position) + ddir;
    }
    
    if(((dir == XmsdRight) && (position == data->length)) ||
       ((dir == XmsdLeft) && (position == 0)))
      break;
    Increment(data, position, dir);
  }
  
  *new_position = position;
}
  /*  Function that uses external locale dependent text boundary libarary to find 
   *  the Text Boundary 
   */
#ifdef SUN_TBR
static XmTextPosition 
_XmFindTB(XmTextSource        source,
          XmTextPosition      pos,
	  TBRScanCondition    scan_cond,
          XmTextScanDirection dir,
	  Boolean             inverse)
{ 
  XmTextPosition position = pos;
  int i;
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget)data->widgets[0];
  Boolean is_wchar = tw->text.char_size > 1;	
  char *string = NULL;
  long string_len;
  XmTextBlockRec block;
  XmTextPosition new_pos; 
   wchar_t *tmp_wc = NULL;

   if (position > 0 && dir == XmsdLeft &&
       (scan_cond != TBR_WhiteSpace)) /*in backward we need to check the prev char*/
     position --;

   if(((dir == XmsdRight) && (position == data->length)) || 
      ((dir == XmsdLeft) && (position < 0))) /*Nothing to do, we are in the edge*/
     return(position);

   _ReadSource(source, 0 ,data->length , &block,0);
   string = block.ptr;
   string_len = block.length;
   
  if (!is_wchar) {
    string = (char*) XtMalloc(string_len + 1);
    strncpy(string, block.ptr, string_len);
  }
  else {    
    tmp_wc         = (wchar_t*)XtMalloc((string_len + 1) * sizeof(wchar_t));
    string_len     = mbstowcs(tmp_wc, block.ptr, string_len); /* block.ptr is not null-terminated */      
      if (string_len == -1)
	  string_len = _Xm_mbs_invalid(tmp_wc, block.ptr, string_len); 
      tmp_wc[string_len] = (wchar_t)0;
      string = (char*)tmp_wc;
  }

    if( string_len > 0){
      new_pos= XmStrScanForTB((XmTBR)_XmRendTBR(tw->text.output->data->rendition), 
			      string, string_len, is_wchar, position,
			      dir, scan_cond, inverse);     

      if(new_pos >= 0 && new_pos <= data->length){/*TB found*/
      if ((dir == XmsdRight && new_pos > position ) ||
	    (dir == XmsdLeft  && new_pos < position)) /*wrong direction values*/
	    position = new_pos;     
      }
      else { /*No TB found (return the max/min values)*/
	position = (scan_cond==TBR_WhiteSpace)? -1: (dir== XmsdRight)? data->length : 0;
      }
    }	  		  
    else 
      position = (dir== XmsdRight)? data->length : 0;
    if(string)
      XtFree(string);  
  return(position);
}

#endif /*SUN_TBR*/

#ifdef SUN_CTL
Boolean
_XmCTLGetLine(XmTextWidget       tw,
	      int                pos,
	      XmTextPosition    *line_start,
	      XmTextPosition    *next_line_start,
	      char             **text,
	      int               *text_len)
{
  LineTableExtra extra;
  XmTextBlockRec block;
  XmTextPosition linestart;
  XmTextPosition nextlinestart;
    
  OutputData     data       = tw->text.output->data;
  LineNum        line       = PosToAbsLine(tw, pos);
  int      count;
  wchar_t *tmp_wc = NULL;
  Boolean is_wchar;
  
  if (line == NOLINE || line > tw->text.total_lines || pos < 0)
      return False;
  
  CTLLineInfo(tw, line, &linestart);
  CTLLineInfo(tw, line+1, &nextlinestart);
    
  if (nextlinestart == PASTENDPOS) {
      nextlinestart = tw->text.last_position;
  }
  *line_start = linestart;
  *next_line_start = nextlinestart;

  if (nextlinestart > linestart) /* if zero-length, no need to fetch the line */
      (void)(*tw->text.source->ReadSource)(tw->text.source, linestart, nextlinestart, &block);
  else {
    *text_len = 0;
    *text = NULL;
    return True;
  }

  /* ReadSource() fills block with mbs.  But for our purposes we
     need wcs when char_size > 1 and rendition is XmFONT_IS_XOC,
     since the position from which we are scanning is a character
     position, not a byte position. */
  count     = block.length;
  is_wchar  = (tw->text.char_size > 1);
  if (!is_wchar) {
      *text = (char*) XtMalloc(count + 1);
      strncpy(*text, block.ptr, count);
      *text_len = count;
  } 
  else if (_XmRendLayoutIsCTL(data->rendition)) {
      count     = nextlinestart - linestart;
      tmp_wc    = (wchar_t*)XtMalloc((count + 1) * sizeof(wchar_t));
      count     = mbstowcs(tmp_wc, block.ptr, count); /* block.ptr is not null-terminated */
      
      if (count == -1)
	  count = _Xm_mbs_invalid(tmp_wc, block.ptr, nextlinestart - linestart); /* fix for bug 4277497 - leob */
      tmp_wc[count] = (wchar_t)0;
      
      *text = (char*)tmp_wc;
      *text_len = count;
  }
  return True;
}

XmTextPosition
_XmTextVisualConstScan(XmTextSource source,
		       XmTextPosition pos,
		       int         posType)
{
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget)data->widgets[0];
  OutputData output_data = tw->text.output->data; 
  XFontStruct *f = _XmRendFont(output_data->rendition);
  XFontSet fontset = (XFontSet)f;
  
  char *text;
  int text_len;
  Boolean is_wchar = tw->text.char_size > 1;
  
  XmTextPosition line_start, next_line_start;
  XmTextPosition new_pos;
  
  Boolean line_exists = _XmCTLGetLine(tw, pos, &line_start, &next_line_start, &text,
				      &text_len);
  if (!line_exists)
    return pos;
  
  if (posType == LINE_START)
    XocVisualScan(fontset, text, is_wchar, text_len, LINE_START, XocCONST_POS,
		  XmSELECT_POSITION, XmsdLeft,True,
		  &new_pos);
  return (new_pos + line_start);
}

XmTextPosition 
_XmTextVisualScan(XmTextSource source,
	      XmTextPosition pos,
	      XmTextScanType sType,
	      XmTextScanDirection dir,
	      int count,
#if NeedWidePrototypes
	      int include)
#else 
	      Boolean include)
#endif /* NeedWidePrototypes */
{
  char *text;
  int text_len=0;
    
  XmTextPosition line_start = 0;
  XmTextPosition next_line_start = 0;
  XmTextPosition new_pos;
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget)data->widgets[0];
  OutputData output_data = tw->text.output->data;
  Boolean is_wchar = (tw->text.char_size > 1);
  
  XFontStruct *f = _XmRendFont(output_data->rendition);
  XFontSet fontset = (XFontSet)f;
  Boolean line_exists;
  
  if (pos == tw->text.last_position)
    if(dir == XmsdRight)
      return tw->text.last_position;
    
  line_exists = _XmCTLGetLine(tw, pos, &line_start, &next_line_start, &text, &text_len);
  /* return if the line doesn't exits */
  if (!line_exists || (text_len <  0))
    return pos;

  if (text_len == 0) {
    if (dir == XmsdRight) {
      if (pos < tw->text.last_position)
	return pos + 1;
      else
	return tw->text.last_position;
    } else { /* dir == XmsdLeft */
      if (line_start > 0)
	return line_start - 1;
      else
	return pos;
    }
  }
  /* the xoc functions doesn't handle the new line character, so for
     /* scanning strip of the newline char (logically)
      */
  if (IS_NEWLINE(STR_IPTR(text, text_len - 1, is_wchar), is_wchar))
    text_len--;
    /* let the position be is relative to the current line */
  pos -= line_start;
    
  switch (sType) {
    case XmSELECT_POSITION: 
    {
      int result;
      result = XocVisualScan(fontset, text, is_wchar, text_len,
			     pos, XocRELATIVE_POS,
			     XmSELECT_POSITION, dir, True, &new_pos);
      if (dir == XmsdLeft) {
	if (result == AT_VISUAL_LINE_START) {
	  if (line_start > 0)
	    new_pos = line_start - 1;
	  else {
	      new_pos = line_start + pos;
	  }
	} else
	  new_pos += line_start;
      } else {/* dir == XmsdRight */
	if (result == AT_VISUAL_LINE_END) {
	  if (next_line_start >= tw->text.last_position)
	    new_pos = next_line_start;
	  else {
	    XtFree((char*)text); 
	    line_exists = _XmCTLGetLine(tw, next_line_start, &line_start, &next_line_start, &text,
					&text_len);
	    if (line_exists && text_len > 0) {
	      XocVisualScan(fontset, text, is_wchar, text_len, LINE_START, XocCONST_POS,
			    XmSELECT_POSITION,dir, True, &new_pos);
	      
	      new_pos += line_start;
	    }
	  } 
	} else
	  new_pos += line_start;
      }
      break;
    }
    case XmSELECT_WORD:
    {
      if (dir == XmsdLeft) {
	int result;
	
	result = XocVisualScan(fontset, text, is_wchar, text_len,
			       pos, XocRELATIVE_POS,
			       XmSELECT_WORD, XmsdLeft, include, &new_pos);
	if ((result == AT_VISUAL_LINE_START || result == NO_WORD) && line_start > 0) {
	  char *pl_text;
	  int pl_text_len;
	  XmTextPosition pl_next_line_start, pl_line_start;
	  
	  do {
	    /* Get Previous Line (Abbreviation pl) */
	    line_exists = _XmCTLGetLine(tw, line_start-1, &pl_line_start, &pl_next_line_start, &pl_text,
					&pl_text_len);
	    if (line_exists)
	      result = XocVisualScan(fontset, pl_text, is_wchar,
				     pl_text_len, LAST_WORD,
				     XocCONST_POS,
				     XmSELECT_WORD,
				     XmsdLeft, include,
				     &new_pos);
	    if (pl_text)
	      XtFree((char*)pl_text);
	    line_start = pl_line_start;
	  } while (result == NO_WORD && pl_line_start >0);
	  
	  if (result == NO_WORD && pl_line_start == 0)
	    new_pos = 0;
	} /* if result == AT_VISUAL ... */
	new_pos += line_start;
      }
      else {/* dir == XmsdRight */
	int result;
		
	result = XocVisualScan(fontset, text, is_wchar, text_len,
			       pos, XocRELATIVE_POS,
			       XmSELECT_WORD, XmsdRight, include, &new_pos);
	if ((result == AT_VISUAL_LINE_END || result == NO_WORD) &&
	    next_line_start < tw->text.last_position) {
	  char *nl_text;
	  int nl_text_len;
	  XmTextPosition nl_next_line_start = line_start, nl_line_start;
	  
	  do {
	    /* GetNextLine */
	    line_exists = _XmCTLGetLine(tw, next_line_start, &nl_line_start, &nl_next_line_start, &nl_text,
					&nl_text_len);
	    if (line_exists)
	      result = XocVisualScan(fontset, nl_text, is_wchar,
				     nl_text_len, FIRST_WORD,
				     XocCONST_POS,
				     XmSELECT_WORD,
				     XmsdRight, include,
				     &new_pos); 
	    if (nl_text)
	      XtFree((char*) nl_text);
	    next_line_start = nl_next_line_start;
	  } while(result == NO_WORD && next_line_start < tw->text.last_position);

	  if (next_line_start >= tw->text.last_position && result == NO_WORD)
	    new_pos = tw->text.last_position;
	  else
	    new_pos += nl_line_start;
	} else
	  new_pos += line_start;
      }
      break;
    }
    
    case XmSELECT_LINE:
      XocVisualScan(fontset, text, is_wchar, text_len, pos,
		    XocRELATIVE_POS, XmSELECT_LINE, dir, include, &new_pos);
      new_pos += line_start;
      break;
      
    case XmSELECT_ALL:
      if (dir == XmsdLeft)
	new_pos = _XmTextVisualConstScan(source, 0, LINE_START);
      else
	new_pos = tw->text.last_position;
      break;
    case XmSELECT_CELL:
    {
      int result;
      result = XocCellScan(fontset, text, is_wchar, text_len, pos, dir, &new_pos);
      if (dir == XmsdLeft) {
	if (result == LINE_START) {
	  if (line_start > 0)
	    new_pos = line_start - 1;
	  else
	    new_pos = pos;
	} else
	  new_pos += line_start;
      } else {/* dir == XmsdRight */
	if (result == LINE_END)	{
	  if (next_line_start >= tw->text.last_position)
	      new_pos = tw->text.last_position;
	  else
	    new_pos = next_line_start;
	} else
	  new_pos += line_start;
      }
      break;
    }
      default: XmeWarning(NULL, "No selection type:TextStrSo.c _XmTextVisualScan \n");
  }
  XtFree ((char*)text);
  return new_pos;
}

int _XmTextFindCell(XmTextSource source,
		    XmTextPosition pos,
		    XmTextScanDirection dir,
		    XmTextPosition *start_pos,
		    XmTextPosition *end_pos)
{
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget)data->widgets[0];
  OutputData output_data = tw->text.output->data;
  
  char *text = NULL;
  int text_len=0;
  Boolean is_wchar = tw->text.char_size > 1;
    
  int result;
  XmTextPosition next_line_start = 0;
  XmTextPosition line_start = 0;
  
  XFontStruct *f = _XmRendFont(output_data->rendition);
  XFontSet fontset = (XFontSet)f;
  
  Boolean line_exists = _XmCTLGetLine(tw, pos, &line_start, &next_line_start, &text, &text_len);
  
  if (!line_exists) {
    if (text)
      XtFree(text);
    return NO_CELL;
  }
  if (text_len == 0) {
    if (line_start > 0) {/* There is something to delete */
      if (dir == XmsdRight) {
	*start_pos = pos;
	*end_pos = pos + 1;
      }
      else {/* dir == XmdLeft */
	*start_pos = pos - 1;
	*end_pos = pos;
      }
      return 0;
    }
    return LINE_END;
  }
  
  if (pos == next_line_start - 1 && dir == XmsdRight) {
    *start_pos = pos;
    *end_pos = pos + 1;
    return 0;
  }
  if (pos == line_start && dir == XmsdLeft) {
    if (line_start > 0)	{
      *start_pos = line_start - 1;
      *end_pos = line_start;
    } else
      *start_pos = *end_pos = line_start;
    return 0;
  }
  
  if (text_len > 0)
    if (IS_NEWLINE(STR_IPTR(text, text_len - 1, is_wchar), is_wchar))
      text_len--;
    
  result = XocFindCell(fontset, text, is_wchar, text_len, pos - line_start, dir, start_pos, end_pos);
  *start_pos += line_start;
  *end_pos += line_start;
  if(text)
    XtFree(text);
  return result;
}

int
_XmTextFindVisualWord(XmTextSource source,
		      XmTextPosition pos,
		      XmTextScanDirection dir,
		      XmTextPosition *word_char_pos_list,
		      int *num_chars,
		      XmTextPosition *new_pos)
{
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget)data->widgets[0];
  OutputData output_data = tw->text.output->data;
  
  char *text = NULL;
  int text_len=0;
  Boolean is_wchar = tw->text.char_size > 1;
  
  int result;
  XmTextPosition next_line_start = 0;
  XmTextPosition line_start = 0;
  
  XFontStruct *f = _XmRendFont(output_data->rendition);
  XFontSet fontset = (XFontSet)f;
  
  Boolean line_exists = _XmCTLGetLine(tw, pos, &line_start, &next_line_start, &text, &text_len);
  
  if (!line_exists) {
    if (text)
      XtFree(text);
    *num_chars = 0;
    *new_pos = pos;
    return NO_WORD;
  } else if (text_len > 0)
    if (IS_NEWLINE(STR_IPTR(text, text_len - 1, is_wchar), is_wchar))
      text_len--;
    
  result = XocFindVisualWord(fontset,
			     text,
			     is_wchar,
			     text_len,
			     XocRELATIVE_POS,
			     pos - line_start,
			     dir,
			     word_char_pos_list,
			     num_chars,
			     new_pos);
  
  if (dir == XmsdLeft) {
    if ((result == AT_VISUAL_LINE_START || result == NO_WORD) && line_start > 0) {
      word_char_pos_list[0] = line_start - 1;
      *num_chars = 1;
      XocVisualScan(fontset, text, is_wchar, text_len, LINE_START, XocCONST_POS,
		    XmSELECT_POSITION, dir, True,
		    new_pos);
    } else {
      int i;
      for (i = 0; i < *num_chars; i++)
	  word_char_pos_list[i] += line_start;
    }
    *new_pos += line_start;
  }
  else {/* dir == XmsdRight */
    if ((result == AT_VISUAL_LINE_END || result == NO_WORD) && next_line_start < tw->text.last_position) {
      word_char_pos_list[0] = next_line_start - 1;
      *num_chars = 1;
      
      if (next_line_start >= tw->text.last_position) {
	XtFree((char*)text);
	return AT_VISUAL_LINE_END;
      }
	     
      /* find the new_pos */
      {
	XmTextPosition nl_line_start, nl_next_line_start;
	char *nl_text;
	int   nl_text_len;
	XmTextPosition new_char_pos;
	
	line_exists = _XmCTLGetLine(tw, next_line_start,
				    &nl_line_start, &nl_next_line_start, &nl_text,
				    &nl_text_len);

	if(!line_exists) {
	  XmeWarning((Widget)tw, "Fatal error in _XmTextFindVisualWord\n");
	  if (nl_text)
	    XtFree((char*)nl_text);
	}
	
	if (nl_text_len > 0)
	  if (IS_NEWLINE(STR_IPTR(nl_text, nl_text_len - 1, is_wchar), is_wchar))
	    nl_text_len--;
	XocVisualScan(fontset, nl_text, is_wchar, nl_text_len, LINE_START, XocCONST_POS,
		      XmSELECT_POSITION,dir, True, &new_char_pos);
	XtFree(nl_text);
	
	*new_pos = nl_line_start + new_char_pos - 1;
      }
    } else {
      int i;
      for (i = 0; i < *num_chars; i++)
	word_char_pos_list[i] += line_start;
      *new_pos += line_start;
    }
  }

  XtFree(text);
}
#endif /* CTL */


static XmTextPosition 
Scan(XmTextSource source,
     XmTextPosition pos,
     XmTextScanType sType,
     XmTextScanDirection dir,
     int count,
#if NeedWidePrototypes
     int include)
#else
     Boolean include)
#endif /* NeedWidePrototypes */
{
  register long whiteSpace = -1;
  register XmTextPosition position = pos;
  register int i;
  XmTextPosition temp;
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget)data->widgets[0];
  char * c;
  BITS16 * bits16_ptr;
  wchar_t * wchar_t_ptr;
  char mb_char[1 + MB_LEN_MAX];
  Boolean start_is_mb, cur_is_mb;  /* False == 1-byte char, else multi-byte */
  int num_bytes = 0;
  int ddir = (dir == XmsdRight) ? 1 : -1;
  
  switch (sType) {
  case XmSELECT_POSITION: 
    if (!include && count > 0)
      count -= 1;
    for (i = 0; i < count; i++) {
      Increment(data, position, dir);
    }
    break;
  case XmSELECT_WHITESPACE: 
  case XmSELECT_WORD:
#ifdef SUN_TBR
  /* Fix for bug 4304604 - leob */
  if (tw->text.output->data->rendition && _XmRendIsTBR(tw->text.output->data->rendition)){
       /* use external locale dependent text boundary libarary to find the WordBoundary */  
       TBRScanCondition scan_cond = TBR_WordBoundary;
       char * c;
       for (i = 0; i < count; i++) { 
            XmTextPosition newpos= ( position > 0 && dir == XmsdLeft)?position-1: position;
            whiteSpace = -1;        
            if (_XmFindTB( source, newpos,TBR_WhiteSpace , dir, False) > 0){ /*check if the current pos is space*/
               whiteSpace = position;
               if (!include)
                  break; /*no need to continue search*/          
             }           
            newpos = _XmFindTB( source, position, scan_cond , dir, False);           
            if (newpos==position && ((dir == XmsdRight && newpos < data->length  ) ||
	                             (dir == XmsdLeft  && newpos > 0))){ 
               /*didn't move: still over the first char of the word, incremet and scan again*/
               position+=ddir;
               position=_XmFindTB( source, position, scan_cond , dir, False);
            }
            else
              position=newpos; 
          
            if (whiteSpace < 0) { 
               whiteSpace = position; /*got the first edge of the current word*/
               position+=ddir;/*check the next edge*/
               if (!include)
                  break; /*no need to continue search*/  
             }
            else
              break; /*this edge follows a space char(s), stop searching*/
               
            if ((dir == XmsdLeft && position > 0) ||
                (dir == XmsdRight && position < data->length)){ /*scan for the next edge*/
                position = _XmFindTB( source, position, scan_cond, dir, False); /*this should return what we need*/
             }
            else
	      position= (position < 0 )? 0: (position > data->length)? data->length:position;

        }

        if (!include) {
          if(whiteSpace < 0 && dir == XmsdRight)
             whiteSpace = data->length;
	  else if(whiteSpace < 0 && dir == XmsdLeft)
              whiteSpace =0;
          position = whiteSpace;
        }
      break; 
    }  
#endif /*SUN_TBR*/

    if (tw->text.char_size == 1) {
      char * c;
      for (i = 0; i < count; i++) {
	whiteSpace = -1;
	while (position >= 0 && position <= data->length) {
	  c = Look(data, position, dir);
	  if (c && isspace((unsigned char)*c)) {
	    if (whiteSpace < 0) whiteSpace = position;
	  } else if (whiteSpace >= 0)
	    break;
	  position += ddir;
	}
      }
    } else {
      for (i = 0; i < count; i++) {
	whiteSpace = -1;
	num_bytes = _XmTextCharactersToBytes(mb_char,
					     Look(data, position, dir),
					     1,(int)tw->text.char_size);
	start_is_mb = (num_bytes < 2 ? False : True);
	while (position >= 0 && position <= data->length) {
	  num_bytes = _XmTextCharactersToBytes(mb_char,
					       Look(data, position, dir),
					       1, (int)tw->text.char_size);
	  cur_is_mb = (num_bytes < 2 ? False : True);
	  if (!cur_is_mb && mb_char && 
	      isspace((unsigned char)*mb_char)) {
	    if (whiteSpace < 0) whiteSpace = position;
	  } else if ((sType == XmSELECT_WORD) &&
		     (start_is_mb ^ cur_is_mb)) {
	    if (whiteSpace < 0) whiteSpace = position;
	    break;
	  } else if (whiteSpace >= 0)
	    break;
	  position += ddir;
	}
      }
    }
    if (!include) {
      if(whiteSpace < 0 && dir == XmsdRight)
	whiteSpace = data->length;
      position = whiteSpace;
    }
    break;
  case XmSELECT_LINE: 
#ifdef SUN_TBR
    /* fix for bug 4304604 - leob */
    if (tw->text.output->data->rendition && _XmRendIsTBR(tw->text.output->data->rendition)){
       /* use external locale dependent text boundary libarary to find the LineBoundary */
       TBRScanCondition scan_cond = TBR_LineBreakCharacter;    
       for (i = 0; i < count; i++) { 
           position = _XmFindTB( source, position, scan_cond, dir, False);            
           if(((dir == XmsdRight) && (position == data->length)) || 
	      ((dir == XmsdLeft) && (position == 0)))
	      break;       
           if (i + 1 != count)
               Increment(data, position, dir);  
         }
     if (include) {
           Increment(data, position, dir); 
        }
       break;      
     }  
#endif /*SUN_TBR*/
    for (i = 0; i < count; i++) {
      while (position >= 0 && position <= data->length) {
	/* DELTA: Look now returns a pointer */
	if ((int)tw->text.char_size == 1) {
	  c = Look(data, position, dir);
	  if ((c == '\0') || (*c == *data->PSWC_NWLN))
	    break;
	}
	else if ((int)tw->text.char_size == 2) {
	  bits16_ptr = (BITS16 *) Look(data, position, dir);
	  if ((bits16_ptr == NULL) ||
	      (*bits16_ptr == *(BITS16 *)data->PSWC_NWLN))
	    break;
	}
	else { /* MB_CUR_MAX == 3 or 4 or more */
	  wchar_t_ptr = (wchar_t *) Look(data, position, dir);
	  if ((wchar_t_ptr == NULL) ||
	      (*wchar_t_ptr == *(wchar_t *)data->PSWC_NWLN))
	    break;
	}
	
	if(((dir == XmsdRight) && (position == data->length)) || 
	   ((dir == XmsdLeft) && (position == 0)))
	  break;
	Increment(data, position, dir);
      }
      if (i + 1 != count)
	Increment(data, position, dir);
    }
    if (include) {
      /* later!!!check for last char in file # eol */
      Increment(data, position, dir);
    }
    break;
  case XmSELECT_PARAGRAPH: 
#ifdef SUN_TBR
    /* fix for bug 4304604 - leob */
    if (tw->text.output->data->rendition && _XmRendIsTBR(tw->text.output->data->rendition)){
       /* use external locale dependent text boundary libarary to find the ParagraphBoundary */
       TBRScanCondition scan_cond = TBR_ParagraphBoundary;    
       for (i = 0; i < count; i++) { 
            /*get the first non break to start the search */
            position = _XmFindTB( source, position, scan_cond, dir, True);
            /* start the search */   
            position = _XmFindTB( source, position, scan_cond, dir, False);
           if((dir == XmsdRight) && (position < data->length)){ /*find the first char in paragraph*/
              position = _XmFindTB( source, position, scan_cond, dir, True);
            }
           if(((dir == XmsdRight) && (position == data->length)) || 
              ((dir == XmsdLeft) && (position == 0)))
               break;	             
	   if (i + 1 != count)
	      Increment(data, position, dir);
         }
       if (include) {
         Increment(data, position, dir);
        }
       break;     
    }  
#endif /*SUN_TBR*/
    /* Muliple paragraph scanning is not guarenteed to work. */
    for (i = 0; i < count; i++) {
      XmTextPosition start_position = position; 
      XmTextPosition last_char = position; 
      
      /* if scanning forward, check for between paragraphs condition */
      if (dir == XmsdRight) {
	/* DELTA: Look now returns a pointer */
	c = Look(data, position, dir);
	(void) _XmTextCharactersToBytes(mb_char, 
					Look(data, position, dir), 
					1, (int)tw->text.char_size);
	/* if is space, go back to first non-space */
	while (mb_char && isspace((unsigned char)*mb_char)) {
	  if (position > 0)
	    position--;
	  else if (position == 0)
	    break;
	  (void) _XmTextCharactersToBytes(mb_char, 
					  Look(data, position, dir),
					  1, (int)tw->text.char_size);
	}
      }
      
      temp = position;
      ScanParagraph(data, &temp, dir, ddir, &last_char);
      position = temp;
      
      /*
       * If we are at the beginning of the paragraph and we are
       * scanning left, we need to rescan to find the character
       * at the beginning of the next paragraph.
       */  
      if (dir == XmsdLeft) {
	/* If we started at the beginning of the paragraph, rescan */
	if (last_char == start_position) {
	  temp = position;
	  ScanParagraph(data, &temp, dir, ddir, &last_char);
	}
	/*
	 * Set position to the last non-space 
	 * character that was scanned.
	 */
	position = last_char;
      }
      
      if (i + 1 != count)
	Increment(data, position, dir);
      
    }
    if (include) {
      Increment(data, position, dir);
    }
    break;
  case XmSELECT_ALL: 
    if (dir == XmsdLeft)
      position = 0;
    else
      position = data->length;
  }
  if (position < 0) position = 0;
  if (position > data->length) position = data->length;
  return(position);
}

static Boolean 
GetSelection(XmTextSource source,
	     XmTextPosition *left,
	     XmTextPosition *right)
{
  XmSourceData data = source->data;
  
  if (data->hasselection && data->left <= data->right && data->left >= 0) {
    *left = data->left;
    *right = data->right;
    return True;
  } else {
    *left = *right = 0;
    data->hasselection = False;
    data->take_selection = True;
  }
  return False;
}

static void 
SetSelection(XmTextSource source,
	     XmTextPosition left,
	     XmTextPosition right, /* if right == -999, then  we're in 
				      LoseSelection, so don't call 
				      XtDisownSelection.*/
	     Time set_time)
{
  XmSourceData data = source->data;
  XmTextWidget tw;
  int i;
  int oldleft, oldright;
  
  if (!XtIsRealized((Widget)data->widgets[0]) ||
      (left > right && !data->hasselection)) return;
  
  if (left < 0) left = right = 0;
  
  for (i=0; i<data->numwidgets; i++) {
    tw = (XmTextWidget)(data->widgets[i]);
    (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position,
					   off);
    _XmTextDisableRedisplay(data->widgets[i], FALSE);
    if (data->hasselection)
      _XmTextSetHighlight((Widget)data->widgets[i], data->left, 
			 data->right, XmHIGHLIGHT_NORMAL);
    
    data->widgets[i]->text.output->data->refresh_ibeam_off = True;
  }
  
  oldleft = data->left;
  oldright = data->right;
  data->left = left;
  data->right = right;
  if (data->numwidgets > 0) {
    Widget widget = (Widget) data->widgets[0];
    tw = (XmTextWidget)widget;
    if (!set_time) set_time = _XmValidTimestamp(widget);
    
    if (left <= right) {
      if (data->take_selection || (oldleft == oldright && left != right)) {
	if (!XmePrimarySource(widget, set_time)) {
	  (*source->SetSelection)(source, 1, 0, set_time);
	} else {
	  XmAnyCallbackStruct cb;
	  
	  data->prim_time = set_time;
	  data->hasselection = True;
	  data->take_selection = False;
	  
	  cb.reason = XmCR_GAIN_PRIMARY;
	  cb.event = NULL;
	  XtCallCallbackList ((Widget) data->widgets[0], 
			      data->widgets[0]->text.gain_primary_callback,
			      (XtPointer) &cb);
	}
      }
      if (data->hasselection && data->left < data->right) {
	for (i=0; i<data->numwidgets; i++) {
	  _XmTextSetHighlight((Widget) data->widgets[i], data->left,
			     data->right, XmHIGHLIGHT_SELECTED);
	}
      }
      if (left == right) tw->text.add_mode = False;
    } else {
      if (right != -999)
	XtDisownSelection(widget, XA_PRIMARY, set_time);
      data->hasselection = False;
      data->take_selection = True;
      tw->text.add_mode = False;
    }
  }
  for (i=0; i<data->numwidgets; i++) {
    tw = (XmTextWidget)(data->widgets[i]);
    _XmTextEnableRedisplay(data->widgets[i]);
    (*tw->text.output->DrawInsertionPoint)(tw, tw->text.cursor_position,
					   on);
  }
}

/* Public routines. */

void
_XmTextValueChanged(XmTextWidget initiator,
		    XEvent *event)
{
  XmAnyCallbackStruct cb;
  
  cb.reason = XmCR_VALUE_CHANGED;
  cb.event = event;
  if (initiator->text.value_changed_callback)
    XtCallCallbackList((Widget)initiator,
		       initiator->text.value_changed_callback, (XtPointer)&cb);
}

XmTextSource 
_XmStringSourceCreate(char *value,
#if NeedWidePrototypes
		      int is_wchar)
#else
                      Boolean is_wchar)
#endif /* NeedWidePrototypes */
{
    /* Bug Id : 1217687/4128045/4154215 */
    /* Changed to wrapper function for StringSourceCreate()
     * StringSourceCreate is exactly the same as _XmStringSourceCreate()
     * except for extra parameter widget 
     */
    return (StringSourceCreate((Widget)NULL, value, is_wchar));
}

/* Bug Id : 1217687/4128045/4154215 */
/* Exactly the same as _XmStringSourceCreate() was except with Widget 
 * parameter so that TextCountCharacters() can have widget passed to
 * it for error setting if illegal byte sequence detected and data 
 * truncation has occurred */
XmTextSource 
StringSourceCreate(Widget w,
		      char *value,
#if NeedWidePrototypes
		      int is_wchar)
#else
                      Boolean is_wchar)
#endif /* NeedWidePrototypes */
{
  XmTextSource source;
  XmSourceData data;
  int num_chars;
  char newline = '\n';
  wchar_t *wc_value;
  char * tmp_value;
  int char_size, max_char_size;
  int ret_value = 0;

  source = (XmTextSource) XtMalloc((unsigned) sizeof(XmTextSourceRec));
  data = source->data = (XmSourceData)
    XtMalloc((unsigned) sizeof(XmSourceDataRec));
  source->AddWidget = AddWidget;
  source->CountLines = CountLines;
  source->RemoveWidget = RemoveWidget;
  source->ReadSource = ReadSource;
  source->Replace = (ReplaceProc) Replace;
  source->Scan = Scan;
  source->GetSelection = GetSelection;
  source->SetSelection = SetSelection;
  
  data->source = source;
  switch (MB_CUR_MAX) {
  case 1: case 2: 
    max_char_size = char_size = MB_CUR_MAX;
    break;
  case 0: 
    max_char_size = char_size = 1;
    break;
  default: 
    max_char_size = MB_CUR_MAX;
    char_size = sizeof(wchar_t);
  }
  
  if (is_wchar) {
    for (num_chars = 0, wc_value = (wchar_t*)value; 
	 wc_value[num_chars] != 0L;
	 num_chars++)
      /*EMPTY*/;
    
    data->maxlength = TEXT_INITIAL_INCREM;
    
    while ((num_chars + 1) >= data->maxlength) {
      if (data->maxlength < TEXT_INCREMENT)
	data->maxlength *= 2;
      else
	data->maxlength += TEXT_INCREMENT;
    }
    
    data->old_length = 0;
    data->ptr = XtMalloc((unsigned)((data->maxlength) * char_size));
    tmp_value = XtMalloc((unsigned)((num_chars + 1) * max_char_size));
    ret_value = wcstombs(tmp_value, wc_value, (num_chars + 1) * max_char_size);
    if (ret_value < 0)
       ret_value = _Xm_wcs_invalid(tmp_value, wc_value,
					(num_chars + 1) * max_char_size);
    data->value = NULL;  /* Scratch area for block->ptr conversions */
    /* Doesnt include NULL */
    data->length = _XmTextBytesToCharacters(data->ptr, tmp_value, num_chars,
					      False, max_char_size);
    XtFree(tmp_value);
  } else {
    if (value)
		  /* Bug Id : 1217687/4128045/4154215 */
      num_chars = TextCountCharacters(w, value, strlen(value));
    else
      num_chars = 0;
    data->maxlength = TEXT_INITIAL_INCREM;
    
    while ((num_chars + 1) >= data->maxlength) {
      if (data->maxlength < TEXT_INCREMENT)
	data->maxlength *= 2;
      else
	data->maxlength += TEXT_INCREMENT;
    }
    
    data->old_length = 0;
    data->ptr = XtMalloc((unsigned)((data->maxlength) * char_size));
    
    data->value = NULL;  /* Scratch area for block->ptr conversions */
    data->length = _XmTextBytesToCharacters(data->ptr, value, num_chars,
					    False, max_char_size);
  }
  
  data->PSWC_NWLN = (char *) XtMalloc(char_size);
  _XmTextBytesToCharacters(data->PSWC_NWLN, &newline, 1, False, 
			   max_char_size);
  
  data->numwidgets = 0;
  data->widgets = (XmTextWidget *) XtMalloc((unsigned) sizeof(XmTextWidget));
  data->hasselection = False;
  data->take_selection = True;
  data->left = data->right = 0;
  data->editable = True;
  data->maxallowed = INT_MAX;
  data->gap_start = data->ptr + (data->length * char_size);
  data->gap_end = data->ptr + ((data->maxlength - 1) * char_size);
  data->prim_time = 0;
  return source;
}

void 
_XmStringSourceDestroy(XmTextSource source)
{
  XtFree((char *) source->data->ptr);
  XtFree((char *) source->data->value);
  XtFree((char *) source->data->widgets);
  XtFree((char *) source->data->PSWC_NWLN);
  XtFree((char *) source->data);
  XtFree((char *) source);
  source = NULL;
}

char *
_XmStringSourceGetValue(XmTextSource source,
#if NeedWidePrototypes
			int want_wchar)
#else
                        Boolean want_wchar)
#endif /* NeedWidePrototypes */
{
  XmSourceData data = source->data;
  XmTextWidget tw = (XmTextWidget) data->widgets[0];
  XmTextBlockRec block;
  int length = 0;
  XmTextPosition pos = 0;
  XmTextPosition ret_pos = 0;
  XmTextPosition last_pos = 0;
  char * temp;
  wchar_t * wc_temp;
  int return_val;

  if (!want_wchar) {
    if (data->length > 0)
      temp = (char *) XtMalloc((unsigned)
			       (data->length + 1) * (int)tw->text.char_size);
    else
      return(XtNewString(""));
    
    last_pos = (XmTextPosition) data->length;
    
    while (pos < last_pos) {
      ret_pos = ReadSource(source, pos, last_pos, &block);
      if (block.length == 0)
	break;
      
      (void)memcpy((void*)&temp[length], (void*)block.ptr, block.length);
      length += block.length;
      pos = ret_pos;
    }
    temp[length] = '\0';
    return (temp);
  } else {
    
    if (data->length > 0)
      wc_temp = (wchar_t*)XtMalloc((unsigned)
				   (data->length+1) * sizeof(wchar_t));
    else {
      wc_temp = (wchar_t*)XtMalloc((unsigned) sizeof(wchar_t));
      wc_temp[0] = (wchar_t)0L;
      return (char*) wc_temp;
    }
    
    last_pos = (XmTextPosition) data->length;
    
    while (pos < last_pos) {
      ret_pos = ReadSource(source, pos, last_pos, &block);
      if (block.length == 0)
	break;
      
      /* NOTE: ret_pos - pos could result in a truncated long. */
      return_val = mbstowcs(&wc_temp[length], block.ptr, (int)(ret_pos - pos));
      if (return_val < 0)
         return_val = _Xm_mbs_invalid(&wc_temp[length], block.ptr,
					(int)(ret_pos - pos));
      if (return_val > 0) length += return_val;
      pos = ret_pos;
    }
    wc_temp[length] = (wchar_t)0L;
    return ((char*)wc_temp);
  }
}

void 
_XmStringSourceSetValue(XmTextWidget tw,
			char *value)
{
  XmTextSource source = tw->text.source;
  XmSourceData data = source->data;
  Boolean editable, freeBlock;
  int maxallowed;
  XmTextBlockRec block, newblock;
  XmTextPosition fromPos = 0; 
  XmTextPosition toPos = data->length;
  
  (*source->SetSelection)(source, 1, 0,
			  XtLastTimestampProcessed(XtDisplay(tw)));
  block.format = XmFMT_8_BIT;
  block.length = strlen(value);
  block.ptr = value;
  editable = data->editable;
  maxallowed = data->maxallowed;
  data->editable = TRUE;
  data->maxallowed = INT_MAX;
  XtFree((char*)(tw->text.url_highlight.list));
  tw->text.url_highlight.list = NULL;
  tw->text.url_highlight.number = 0;
  tw->text.url_highlight.maximum = 0;

  /* don't do any unnecessary work */
  if ((value) && (*value))
      _XmTextSetHighlight((Widget)tw, 0, tw->text.last_position, 
			  XmHIGHLIGHT_NORMAL);
  
  if (_XmTextModifyVerify(tw, NULL, &fromPos, &toPos,
			  NULL, &block, &newblock, &freeBlock)) {
    (void)(source->Replace)(tw, NULL, &fromPos, &toPos, &newblock, False);
    if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
    _XmTextValueChanged(tw, NULL);
  }
  
  data->editable = editable;
  data->maxallowed = maxallowed;
}


Boolean 
_XmStringSourceHasSelection(XmTextSource source)
{
  return source->data->hasselection;
}

Boolean 
_XmStringSourceGetEditable(XmTextSource source)
{
  return source->data->editable;
}

void 
_XmStringSourceSetEditable(XmTextSource source,
#if NeedWidePrototypes
			   int editable)
#else
                           Boolean editable)
#endif /* NeedWidePrototypes */
{
  source->data->editable = editable;
}

int 
_XmStringSourceGetMaxLength(XmTextSource source)
{
  return source->data->maxallowed;
}

void 
_XmStringSourceSetMaxLength(XmTextSource source,
			    int max)
{
  source->data->maxallowed = max;
}



