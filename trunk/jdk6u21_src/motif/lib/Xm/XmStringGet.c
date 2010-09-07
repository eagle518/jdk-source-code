/* $XConsortium: XmStringGet.c /main/6 1995/09/19 23:13:21 cde-sun $ */
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

#include "XmStringI.h"
#include "XmI.h"

XmStringComponentType 
XmStringGetNextComponent(
        XmStringContext context,
        char **text,
        XmStringTag *str_tag,
        XmStringDirection *direction,
        XmStringComponentType *unknown_tag,
        unsigned short *unknown_length,
        unsigned char **unknown_value )
{
  XmStringComponentType type;
  unsigned int  	len;
  XtPointer		val;
  
  _XmProcessLock();
  type = XmeStringGetComponent((_XmStringContext) context, True, True, &len, &val);

  /* Case on return type */
  /* Set appropriate return value and return. */
  switch (type)
    {
    case XmSTRING_COMPONENT_DIRECTION:
      *direction = *(XmStringDirection *)val;
      XtFree((char *)val);
      break;
    case XmSTRING_COMPONENT_TAG:
      *str_tag = (XmStringTag)val;
      break;
    case XmSTRING_COMPONENT_TEXT:
    case XmSTRING_COMPONENT_LOCALE_TEXT:
      *text = (char *)val;
      break;
    case XmSTRING_COMPONENT_SEPARATOR:
    case XmSTRING_COMPONENT_END:
      break;
    default:
      *unknown_tag = type;
      *unknown_length = len;
      *unknown_value = (unsigned char *)val; 
      type = XmSTRING_COMPONENT_UNKNOWN;
    }
  _XmProcessUnlock();
  return(type);
}
      
XmStringComponentType 
XmStringPeekNextComponent(XmStringContext context)
{
  unsigned int len;
  XtPointer    val;

  return XmeStringGetComponent((_XmStringContext) context, False, False, &len, &val);
}

/*
 * fetch the first text 'segment' of the external TCS that matches the given
 * char set.
 */
Boolean 
XmStringGetLtoR(
  XmString string,
  XmStringTag tag,
  char **text )
{
  XmStringContext context;
  char * t;
  XmStringTag c, curtag = NULL; 
  XmStringDirection d;
  Boolean s, is_local = FALSE, done = FALSE, is_default = FALSE;
  
  _XmProcessLock();
  if (!string) {
	_XmProcessUnlock();
	return(FALSE);
  }
  if (!tag) {
	_XmProcessUnlock();
	return (FALSE);
  }
  
  if ((tag == XmFONTLIST_DEFAULT_TAG) || 
      (strcmp(tag, XmFONTLIST_DEFAULT_TAG) == 0))
    is_local = TRUE; 
  
  *text = NULL;				  /* pre-condition result */
  
  if (!is_local)
    {
      if ((strcmp(tag, XmSTRING_DEFAULT_CHARSET) == 0))
	{
	  curtag = _XmStringGetCurrentCharset();
	  is_default = TRUE;
	}
      else curtag = tag;
    }
  
  XmStringInitContext (&context, string);
  
  while ( ! done)
    {
      if (XmStringGetNextSegment (context, &t, &c, &d, &s))
	{
	  if (c && ((d == XmSTRING_DIRECTION_L_TO_R) ||
		    (d == XmSTRING_DIRECTION_UNSET)) &&
	      (((is_local || is_default) && 
		((c == XmFONTLIST_DEFAULT_TAG) || 
		 (strcmp(c, XmFONTLIST_DEFAULT_TAG) == 0) ||
		 (strcmp(c, _XmStringGetCurrentCharset()) == 0))) ||
	       (curtag && (strcmp (c, curtag) == 0))))
	    {
	      *text = t;		  /* OK, pass text to caller */
	      done = TRUE;
	    }
	  else
	    XtFree (t);			  /* not this text */
	  
	  if (c)
	    XtFree (c);			  /* always dump charset */
	}
      else
	done = TRUE;
    }
  
  XmStringFreeContext (context);
  _XmProcessUnlock();
  return (*text != NULL);
}

