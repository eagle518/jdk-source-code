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
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: XmStringSeg.c /main/7 1995/09/19 23:13:59 cde-sun $"
#endif
#endif

#include <Xm/XmosP.h>

#include "XmI.h"
#include "XmStringI.h"
#include "XmRenderTI.h"


/*
 * _XmStringGetSegment: A generalized version of XmStringGetNextSegment.
 *	Returns char_count, and allows explicit control over peeking
 *	and copying of data.  Interleaving of calls to _XmStringGetSegment()
 *	and XmeStringGetComponent() is supported.
 */
Boolean
_XmStringGetSegment(_XmStringContext   context, 
		    Boolean	       update_context,
		    Boolean	       copy_data,
		    XtPointer         *text, 
		    XmStringTag       *tag, 
		    XmTextType        *type, 
		    XmStringTag      **rendition_tags, 
		    unsigned int      *tag_count,
		    XmStringDirection *direction,
		    Boolean           *separator, 
		    unsigned char     *tabs,
		    short             *char_count,
		    XmDirection       *push_before,
		    Boolean	      *pop_after)
{
  XmStringTag*   	perm_rends = NULL;
  _XmStringContextRec	local_context_data;
  _XmStringContext	local_context = context;
  Boolean		result = FALSE;
  Boolean 		done;
  Boolean		new_renditions;
  XmStringComponentType ctype;
  unsigned int 		len;
  XtPointer 		val;
  
  /* Initialize the out parameters */
  if (text)           *text           = NULL;
  if (tag)            *tag            = NULL;
  if (type)           *type           = XmCHARSET_TEXT;
  if (rendition_tags) *rendition_tags = NULL;
  if (tag_count)      *tag_count      = 0;
  if (direction)      *direction      = _XmStrContDir(context);	/* for BC */
  if (separator)      *separator      = False;
  if (tabs)           *tabs           = 0;
  if (char_count)     *char_count     = 0;
  if (push_before)    *push_before    = 0;
  if (pop_after)      *pop_after      = False;
  
  /* No NULL pointers allowed. */
  if (! (context && text && tag && type && rendition_tags && tag_count
	 && direction && separator && tabs && char_count && push_before
	 && pop_after))
    return False;

  if (_XmStrContError(context))
    return False;

  /* Setup a writable context. */
  if (!update_context)
    {
      local_context = &local_context_data;
      _XmStringContextCopy(local_context, context);
    }

  /* N.B.: This code relies on the order of components from XmeStringGetComponent()! */
  done = new_renditions = FALSE;
  while (!done)
    {
      /* Peek at components before consuming them. */
      ctype = XmeStringGetComponent(local_context, FALSE, FALSE, &len, &val);
      switch (ctype)
	{
	case XmSTRING_COMPONENT_LAYOUT_PUSH:
	  if (*tabs || *text)
	    done = TRUE;
	  else
	    *push_before = *((XmDirection *) val);
	  break;

	case XmSTRING_COMPONENT_RENDITION_BEGIN:
	  if (*text)
	    done = TRUE;
	  else if (*tabs)
	    new_renditions = TRUE;
	  break;

	case XmSTRING_COMPONENT_CHARSET:
	case XmSTRING_COMPONENT_LOCALE:
	  if (*text)
	    done = TRUE;
	  else
	    *tag = (XmStringTag) val;
	  break;

	case XmSTRING_COMPONENT_TAB:
	  if (*text)
	    done = TRUE;
	  else
	    {
	      /* Save the renditions now. */
	      if ((*tag_count == 0) && _XmStrContRendCount(local_context))
		{
		  *tag_count = _XmStrContRendCount(local_context);
		  if (copy_data)
		    {
		      int tmp;
		      *rendition_tags = (XmStringTag *)
			XtMalloc(sizeof(XmStringTag) * *tag_count);
		      for (tmp = 0; tmp < *tag_count; tmp++)
			(*rendition_tags)[tmp] = 
			  XtNewString(_XmStrContRendTags(local_context)[tmp]);
		    }
		  else
		    {
		      perm_rends = (XmStringTag *)
			XtMalloc(sizeof(XmStringTag) * *tag_count);
		      memcpy((char*) perm_rends,
			     _XmStrContRendTags(local_context),
			     sizeof(XmStringTag) * *tag_count);
		      *rendition_tags = perm_rends;
		    }
		}

	      /* Return at the end of this line. */
	      (*tabs)++;
	      result = TRUE;
	    }
	  break;

	case XmSTRING_COMPONENT_DIRECTION:
	  if (*text)
	    done = TRUE;
	  else
	    *direction = *((XmStringDirection *) val);
	  break;

	case XmSTRING_COMPONENT_TEXT:
	case XmSTRING_COMPONENT_LOCALE_TEXT:
	case XmSTRING_COMPONENT_WIDECHAR_TEXT:
	  if (*text)
	    done = TRUE;
	  else if (*tabs && new_renditions)
	    {
	      /* Tabs had a different set of renditions than the text, */
	      /* so we can't return both tabs and text at once. */
	      done = TRUE;
	    }
	  else
	    {
	      *char_count = len;
	      *text = val;
	      
	      if (ctype == XmSTRING_COMPONENT_TEXT)
		*type = XmCHARSET_TEXT;
	      else if (ctype == XmSTRING_COMPONENT_LOCALE_TEXT)
		*type = XmMULTIBYTE_TEXT;
	      else if (ctype == XmSTRING_COMPONENT_WIDECHAR_TEXT)
		*type = XmWIDECHAR_TEXT;
	      else
		{ assert(FALSE); }
	      
	      /* Force a tag for backward compatibility with Motif 1.2 */
	      if (! *tag)
		*tag = _XmStrContTag(local_context);

	      result = TRUE;
	      
	      /* Save the renditions now. */
	      if ((*tag_count == 0) && _XmStrContRendCount(local_context))
		{
		  *tag_count = _XmStrContRendCount(local_context);
		  if (copy_data)
		    {
		      int tmp;
		      *rendition_tags = (XmStringTag*)
			XtMalloc(sizeof(XmStringTag) * *tag_count);
		      for (tmp = 0; tmp < *tag_count; tmp++)
			(*rendition_tags)[tmp] = 
			  XtNewString(_XmStrContRendTags(local_context)[tmp]);
		    }
		  else
		    {
		      perm_rends = (XmStringTag *)
			XtMalloc(sizeof(XmStringTag) * *tag_count);
		      memcpy((char*) perm_rends,
			     _XmStrContRendTags(local_context),
			     sizeof(XmStringTag) * *tag_count);
		      *rendition_tags = perm_rends;
		    }
		}
	    }
	  break;
	      
	case XmSTRING_COMPONENT_RENDITION_END:
	  break;
	      
	case XmSTRING_COMPONENT_LAYOUT_POP:
	  if (*tabs || *text)
	    {
	      /* We're almost done, so record this pop. */
	      *pop_after = TRUE;
	    }
	  else
	    {
	      /* We're ignoring this pop, so discard any recorded push. */
	      *push_before = 0;
	    }
	  break;

	case XmSTRING_COMPONENT_SEPARATOR:
	  if (*tabs || *text)
	    {
	      *separator = TRUE;
	      done = TRUE;
	    }
	  break;
	  
	case XmSTRING_COMPONENT_END:
	default:
	  done = TRUE;
	  break;
	}

      /* Consume the component if we aren't done. */
      if (!done)
	(void) XmeStringGetComponent(local_context, TRUE, FALSE, &len, &val);
    }

  if (copy_data && result)
    {
      /* Copy the tag. */
      if (*tag)
	*tag = XtNewString(*tag);

      /* Copy the text. */
      if (*text)
	{
	  char *tmp = XtMalloc(*char_count + sizeof(wchar_t));
	  memcpy(tmp, *text, *char_count);
	  bzero(tmp + *char_count, sizeof(wchar_t));

	  *text = (XtPointer) tmp;
	}
    }

  /* Free the local context. */
  if (local_context == &local_context_data)
    _XmStringContextFree(local_context);

  return result;
}

Boolean 
_XmStringGetNextSegment(
        _XmStringContext context,
        XmStringTag *tag,
        XmStringDirection *direction,
        char **text,
        short *char_count,
        Boolean *separator )
{
  Boolean       result;
  XmTextType    type;
  XmStringTag * rendition_tags;
  unsigned int  tag_count;
  unsigned char tabs;
  XmDirection   push_before;
  Boolean	pop_after;

  /* Get all the fields and discard the ones we don't want. */
  result = _XmStringGetSegment(context, True, True, (XtPointer*) text, tag,
		       &type, &rendition_tags, &tag_count, direction,
		       separator, &tabs, char_count, &push_before, &pop_after);
  if (result) {
    if (rendition_tags)
      {
	while (tag_count-- > 0)
	  XtFree((char*) rendition_tags[tag_count]);
	XtFree((char*) rendition_tags);
      }
    
    if (type == XmWIDECHAR_TEXT && *text) {
      /* must convert (this should be done in segment's locale instead) */
      long len; /* Wyoming 64-bit fix */ 
      wchar_t *wtext = (wchar_t *)(*text);

      /* should be enough */
      len = ((*char_count)*MB_CUR_MAX)/sizeof(wchar_t);

      *text = (char *)XtMalloc(len+1);
      *char_count = wcstombs(*text, wtext, len);
      if ((*char_count) == (size_t)-1)
	 *char_count = _Xm_wcs_invalid(*text, wtext, len);
      (*text)[*char_count] = '\0';
      XtFree((char *)wtext);
    } 
  }
	  
  return result;
}

/*
 * fetch the next 'segment' of the external TCS
 */
Boolean 
XmStringGetNextSegment(XmStringContext context,
		       char **text,
		       XmStringTag *tag,
		       XmStringDirection *direction,
		       Boolean *separator )
{
  short char_count;
  Boolean ret_val;
  
  _XmProcessLock();
  ret_val = _XmStringGetNextSegment((_XmStringContext)context, 
		 tag, direction, text, &char_count, separator);
  _XmProcessUnlock();
  return ret_val;
}
