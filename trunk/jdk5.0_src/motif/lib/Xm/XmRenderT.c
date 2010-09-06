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
static char rcsid[] = "$XConsortium: XmRenderT.c /main/12 1996/10/23 15:02:38 cde-osf $"
#endif
#endif

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef __cplusplus
extern "C" { /* some 'locale.h' do not have prototypes (sun) */
#endif
#include <X11/Xlocale.h>
#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif /* __cplusplus */

#include <Xm/XmosP.h>		/* For ALLOCATE/DEALLOCATE_LOCAL */
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/Xresource.h>
#include <Xm/Display.h>		/* For XmGetXmDisplay */
#include <Xm/DisplayP.h>	/* For direct access to callback fields */
#include "MessagesI.h"
#include "XmI.h"
#include "XmRenderTI.h"
#include "XmStringI.h"
#include "XmTabListI.h"

#ifdef SUN_CTL
#include "XmXOC.h"
#endif /* CTL */

#define  SUCCESS                 1

/* Warning Messages */
#define NO_NULL_TAG_MSG			_XmMMsgXmRenderT_0000
#define NULL_DISPLAY_MSG      		_XmMMsgXmRenderT_0001
#define INVALID_TYPE_MSG      		_XmMMsgXmRenderT_0002
#define CONVERSION_FAILED_MSG 		_XmMMsgXmRenderT_0003
#define NULL_FONT_TYPE_MSG    		_XmMMsgXmRenderT_0004
#define NULL_LOAD_IMMEDIATE_MSG		_XmMMsgXmRenderT_0005

/* local macros */
#define GetHandle(type)		(type *)XtMalloc(sizeof(type))
#define FreeHandle(handle)	XtFree((char *)handle)
#define SetPtr(handle, ptr)	*(handle) = ptr
#define GetPtr(handle)		*(handle)
#define NameIsString(fontname) \
  (((fontname) != NULL) && ((fontname) != (String)XmAS_IS))
#define ListIsList(tablist) \
  (((tablist) != NULL) && \
   ((unsigned long)(tablist) != XmAS_IS)) /* Wyoming 64-bit fix */ 

/**********************************************************************
 *	      IMPORTANT NOTE: IMPLEMENTATION OF SHARING
 *
 *	Instances of XmRenderTable and XmRendition are shared via a
 *	reference counting mechanism. This comment provides a general
 *	overview of how this is done.
 *
 *	First, both rendertable and renditions are indirectly
 *	referenced via a handle mechanism.  See the GetHandle,
 *	FreeHandle, SetPtr and GetPtr macros above.  This allows
 *	either the handle to change without the underlying data
 * 	structure changing, or the underlying data structure to change
 *	without the handle changing. I will indicate below where this
 *	happens.
 *
 *	Second, the real data structure for rendertable and rendition
 *	contain a reference count.  This count is incremented on copy
 *	and decremented on free.  If a decrement produces a zero
 *	refcount, the actual memory is freed.  If an increment
 *	produces a zero refcount, then the refcount has overflowed.  The
 *	refcount is decremented, and new memory is allocated for a new
 *	copy. 
 *
 *	Finally, I have defined a terminology for the different types
 *	of "copying" that can done based on allocating a new handle or
 *	not and allocating a new data structure or not.  This probably
 *	conflicts with some other existing terminology, probably in
 *	object oriented programming.  Sorry about that.
 *
 *	Function:	Clone	Copy	Renew	Duplicate
 *			(Mutate)	(Update)
 *
 *	handle		new	new	old	old
 *	structure	new	inc	new	inc
 *			(changed)	(changed)
 *
 *	(changed) indicates that the data in the new structure has
 *	been changed from the data in the old structure.
 *
 *	I will use these terms as a short hand in describing the
 *	functions below.
 **********************************************************************/


/********    Static Function Declarations    ********/

static void CopyInto(XmRendition toRend, 
		     XmRendition fromRend); 
static void MergeInto(XmRendition toRend, 
		     XmRendition fromRend); 
static XmRendition CloneRendition(XmRendition rend); 
static XmRendition CopyRendition(XmRendition rend); 
static XmRendition RenewRendition(XmRendition rend); 
static void FreeRendition(XmRendition rend); 
static void RenditionWarning(char *tag, char *type,
			     char *message, Display *dpy);
static void CleanupResources(XmRendition rend, Boolean copy);
static void ValidateTag(XmRendition rend,
			XmStringTag dflt);
static void LoadAwtFont(XmRendition rend, Display *display);
static void ValidateAndLoadFont(XmRendition rend, Display *display);
static void SetRend(XmRendition to,
		    XmRendition from); 
static Boolean RendComplete(XmRendition rend);      
static void CopyFromArg(XtArgVal src,
			char *dst,
			unsigned int size);
static void CopyToArg(char *src,
		      XtArgVal *dst,
		      unsigned int size); 
static Cardinal GetNamesAndClasses(Widget w,
				   XrmNameList names,
				   XrmClassList classes); 
static XrmResourceList CompileResourceTable(XtResourceList resources,
					    Cardinal num_resources); 
static Boolean GetResources(XmRendition rend,
			    Display *dsp,
			    Widget wid,
			    String resname,
			    String resclass,
			    XmStringTag tag,
			    ArgList arglist,
			    Cardinal argcount);
static void SetDefault(XmRendition rend); 

#ifdef SUN_CTL
static void CTLGetSegInfo(XmRendition		rend, 
			  Position		xoffset, 
			  char			*text, 
			  int			text_length, 
			  Boolean		is_wchar,
			  Dimension		tab_width,
			  Dimension		**seg_widths, 
			  int			*total_num_segs, 
			  Position		**starting_xpos, 
			  Dimension		*block_length);
static int 
CTLRenditionSegPerCharExtents(XmRendition	rend, 
			      char		*seg, 
			      int		seg_len, 
			      Boolean		is_wchar, 
			      XtPointer		logical_array,
			      int		array_size, 
			      XtPointer		overall_logical,
			      int		*num_chars_return);
#endif /* CTL */

/********    End Static Function Declarations    ********/

/* Resource List. */

/************************************************************************/
/* N.B.:  The SetDefault procedure has a hardcoded list of all the	*/
/*	common resources.  Be sure to update it when adding resources.	*/
/************************************************************************/

#define DEFAULT_loadModel		XmAS_IS
#define DEFAULT_tag			XmS
#define DEFAULT_fontName		(String)XmAS_IS
#define DEFAULT_fontType		(XmFontType)XmAS_IS
#define DEFAULT_font			(XtPointer)XmAS_IS
#define DEFAULT_tabs			(XmTabList)XmAS_IS
#define DEFAULT_background		XmUNSPECIFIED_PIXEL
#define DEFAULT_foreground		XmUNSPECIFIED_PIXEL
#define DEFAULT_underlineType		XmAS_IS
#define DEFAULT_strikethruType		XmAS_IS
#define DEFAULT_backgroundState		XmAS_IS
#define DEFAULT_foregroundState		XmAS_IS
#ifdef SUN_CTL
#define DEFAULT_layoutIsCtl		(int)0
#define DEFAULT_layoutModifier          (String)NULL
#endif /* CTL */
#ifdef SUN_TBR
#define DEFAULT_TextBoundary		(XtPointer)NULL
#endif /* SUN_TBR */

static XtResource _XmRenditionResources[] = {
  { 
    XmNtag, XmCTag, XmRString,
    sizeof(XmStringTag), XtOffsetOf(_XmRenditionRec, tag),
    XmRImmediate, (XtPointer) DEFAULT_tag
  },
  { 
    XmNfontName, XmCFontName, XmRString,
    sizeof(String), XtOffsetOf(_XmRenditionRec, fontName),
    XmRImmediate, (XtPointer) DEFAULT_fontName
  },
  { 
    XmNfontType, XmCFontType, XmRFontType,
    sizeof(XmFontType), XtOffsetOf(_XmRenditionRec, fontType),
    XmRImmediate, (XtPointer) DEFAULT_fontType
  },
  { 
    XmNfont, XmCFont, XmRFontStruct,
    sizeof(XtPointer), XtOffsetOf(_XmRenditionRec, font),
    XmRImmediate, (XtPointer) DEFAULT_font
  },
  { 
    XmNloadModel, XmCLoadModel, XmRLoadModel,
    sizeof(unsigned char), XtOffsetOf(_XmRenditionRec, loadModel),
    XmRImmediate, (XtPointer) DEFAULT_loadModel
  },
  { 
    XmNtabList, XmCTabList, XmRTabList,
    sizeof(XmTabList), XtOffsetOf(_XmRenditionRec, tabs),
    XmRImmediate, (XtPointer) DEFAULT_tabs
  },
  { 
    XmNrenditionBackground, XmCRenditionBackground, XmRRenditionPixel,
    sizeof(Pixel), XtOffsetOf(_XmRenditionRec, background),
    XmRImmediate, (XtPointer) DEFAULT_background
  },
  { 
    XmNrenditionForeground, XmCRenditionForeground, XmRRenditionPixel,
    sizeof(Pixel), XtOffsetOf(_XmRenditionRec, foreground),
    XmRImmediate, (XtPointer) DEFAULT_foreground
  },
  { 
    XmNunderlineType, XmCUnderlineType, XmRLineType,
    sizeof(unsigned char), XtOffsetOf(_XmRenditionRec, underlineType),
    XmRImmediate, (XtPointer) DEFAULT_underlineType
  },
  { 
    XmNstrikethruType, XmCStrikethruType, XmRLineType,
    sizeof(unsigned char), XtOffsetOf(_XmRenditionRec, strikethruType),
    XmRImmediate, (XtPointer) DEFAULT_strikethruType
  },
  { 
    XmNforegroundState, XmCGroundState, XmRGroundState,
    sizeof(unsigned char), XtOffsetOf(_XmRenditionRec, foregroundState),
    XmRImmediate, (XtPointer) DEFAULT_foregroundState
  },
  { 
    XmNbackgroundState, XmCGroundState, XmRGroundState,
    sizeof(unsigned char), XtOffsetOf(_XmRenditionRec, backgroundState),
    XmRImmediate, (XtPointer) DEFAULT_backgroundState
  }
#ifdef SUN_CTL
  ,{ 
      XmNlayoutModifier, XmCLayoutModifier, XmRString,
      sizeof(String), XtOffsetOf(_XmRenditionRec, layoutModifier),
      XmRImmediate, (XtPointer) DEFAULT_layoutModifier
  }
#endif /* CTL */
};

static XmConst Cardinal _XmNumRenditionResources = 
	XtNumber(_XmRenditionResources);

/* Searches up widget hierarchy, quarkifying ancestor names and */
/* classes. */
static Cardinal
GetNamesAndClasses(Widget w, XrmNameList names, XrmClassList classes)
{
  Cardinal length, j;
  XrmQuark t;
  WidgetClass wc;

  /* Return null-terminated quark arrays, with length the number of
     quarks (not including NULL) */

  for (length = 0; w != NULL; w = (Widget)w->core.parent)
    {
      names[length] = w->core.xrm_name;
      wc = XtClass(w);
      /* KLUDGE KLUDGE KLUDGE KLUDGE */
      if (w->core.parent == NULL && XtIsApplicationShell(w)) {
	classes[length] =
	  ((ApplicationShellWidget) w)->application.xrm_class;
      } else classes[length] = wc->core_class.xrm_class;
      length++;
    }
  /* They're in backwards order, flop them around */
  for (j = 0; j < length/2; j++)
    {
      t = names[j];
      names[j] = names[length-j-1];
      names[length-j-1] = t;
      t = classes[j];
      classes[j] = classes[length-j-1];
      classes[length-j-1] = t;
    }
  names[length] = NULLQUARK;
  classes[length] = NULLQUARK;
  return length;
}						  /* GetNamesAndClasses */

/* Converts resource list to quarkified list. */
static XrmResourceList
CompileResourceTable(XtResourceList resources,
		     Cardinal num_resources)
{
  Cardinal		count;
  XrmResourceList	table, tPtr;
  XtResourceList	rPtr;
  
  tPtr = table = (XrmResourceList)XtMalloc(num_resources * sizeof(XrmResource));
  rPtr = resources;
  
  for (count = 0; count < num_resources; count++, tPtr++, rPtr++)
    {
      tPtr->xrm_name 		= XrmPermStringToQuark(rPtr->resource_name);
      tPtr->xrm_class 		= XrmPermStringToQuark(rPtr->resource_class);
      tPtr->xrm_type 		= XrmPermStringToQuark(rPtr->resource_type);
      tPtr->xrm_size		= rPtr->resource_size;
      tPtr->xrm_offset		= rPtr->resource_offset;
      tPtr->xrm_default_type 	= XrmPermStringToQuark(rPtr->default_type);
      tPtr->xrm_default_addr	= rPtr->default_addr;
    }
  return(table);
}

/* Does resource database lookup for arglist, filling in defaults from */
/* resource list as necessary. */
static Boolean
GetResources(XmRendition rend,
	     Display *dsp,
	     Widget wid,
	     String resname,
	     String resclass,
	     XmStringTag tag,
	     ArgList arglist,
	     Cardinal argcount)
{
  XrmName		names[100];
  XrmClass		classes[100];
  Cardinal		length = 0;
  static XrmQuarkList	quarks = NULL;
  static Cardinal	num_quarks = 0;
  static Boolean	*found = NULL;
  int			i, j;
  static XrmResourceList	table = NULL;
  static XrmQuark	QString;
  static XrmQuark	Qfont;
  static XrmQuark	QfontType;	/* Bug Id : 4107466 */
  static XrmQuark	QfontName;
  Boolean		foundFontName = False;
  Arg			*arg;
  XrmName		argName;
  XrmResource		*res;
  XrmDatabase		db = NULL;
  XrmHashTable   	stackSearchList[100];
  XrmHashTable    	*searchList = stackSearchList;
  unsigned int    	searchListSize = 100;
  Boolean		got_one = False;
  XrmValue		value;
  XrmQuark		rawType;
  XrmValue		convValue;
  Boolean		have_value, copied;
  int			loopnumbug = _XmNumRenditionResources;
#ifdef XTHREADS
  XtAppContext		app=NULL;

  if (wid)
      app = XtWidgetToApplicationContext(wid);
  else if (dsp)
      app = XtDisplayToApplicationContext(dsp);
  if (app)
      _XmAppLock(app);
  _XmProcessLock();
#endif
  /* Initialize quark cache */
  if (quarks == NULL) {
      quarks = (XrmQuark*)XtMalloc(_XmNumRenditionResources * sizeof(XrmQuark));
      num_quarks = _XmNumRenditionResources;
  }

  /* Initialize found */
  if (found == NULL)
      found = (Boolean*)XtMalloc(_XmNumRenditionResources * sizeof(Boolean));
  bzero(found, _XmNumRenditionResources * sizeof(Boolean));

  /* Compile names and classes. */
  if (wid != NULL)
      length = GetNamesAndClasses(wid, names, classes);
  
  names[length] = XrmStringToQuark(resname);
  classes[length] = XrmStringToQuark(resclass);
  length++;
  
  if (tag != NULL) {
      names[length] = XrmStringToQuark(tag);
      classes[length] = XrmPermStringToQuark(XmCRendition);
      length++;
  }

  names[length] = NULLQUARK;
  classes[length] = NULLQUARK;
  
  /* Cache arglist */
  if (num_quarks < argcount) {
      quarks = (XrmQuark *)XtRealloc((char *)quarks,
				     argcount * sizeof(XrmQuark));
      num_quarks = argcount;
  }
  for (i = 0; i < argcount; i++)
      quarks[i] = XrmStringToQuark(arglist[i].name);

  /* Compile resource description into XrmResourceList if not already done. */
  if (table == NULL) {
      table = CompileResourceTable(_XmRenditionResources,
				   _XmNumRenditionResources);
      QString = XrmPermStringToQuark(XtCString);
      Qfont = XrmPermStringToQuark(XmNfont);
      QfontType = XrmPermStringToQuark(XmNfontType);
      QfontName = XrmPermStringToQuark(XmNfontName);
  }
  
  /* Set resources from arglist. */
  for (arg = arglist, i = 0; i < argcount; arg++, i++) {
      
      argName = quarks[i];
      for (j = 0, res = table; j < loopnumbug; j++, res++) {
	  
	  if (res->xrm_name == argName) {
	      CopyFromArg((arg->value),
			  ((char *)GetPtr(rend) + res->xrm_offset),
			  res->xrm_size);
	      found[j] = TRUE;

	      if (res->xrm_name == QfontName) 
		  foundFontName = True;
	      break;
	  }
      }
  }
  
  /* DB query :: Get database */
  if (dsp != NULL) {
      
      if (wid != NULL)
	  db = XtScreenDatabase(XtScreenOfObject(wid));
      else
	  db = XtScreenDatabase(DefaultScreenOfDisplay(dsp));
      
      /* Get searchlist */
      while (!XrmQGetSearchList(db, names, classes, 
				searchList, searchListSize)) {
	  
	  if (searchList == stackSearchList) searchList = NULL;
	  searchList = (XrmHashTable *)XtRealloc((char*)searchList,
						 sizeof(XrmHashTable) * 
						 (searchListSize *= 2));
      }
  }
  
  /* Loop over table */
  for (j = 0, res = table; j < _XmNumRenditionResources; j++, res++) {

      if (!found[j]) {
	  
	  copied = False;
	  have_value = False;
	  
	  if ((db != NULL) &&
	      (XrmQGetSearchResource(searchList, (int)res->xrm_name, /* Wyoming 64-bit fix */ 
				     (int)res->xrm_class, &rawType, &value))) /* Wyoming 64-bit fix */ 
	    {
		/* convert if necessary */
		if (rawType != res->xrm_type) {
		  
		    if (wid != NULL) {
		      
			convValue.size = res->xrm_size;
			convValue.addr = (char *)GetPtr(rend) + res->xrm_offset;
			/*
			 * Check for special font case.
			 * Depending upon the fontType resource, try to convert
			 * to a FontSet, else to a FontStruct.
			 */

#ifdef SUN_CTL  /* fix for bug 4195719 - leob */
			if ((res->xrm_name == Qfont) && CTL_FONTTYPE(rend))
#else
			if ((res->xrm_name == Qfont) &&
			    (_XmRendFontType(rend) == XmFONT_IS_FONTSET))
#endif /* SUN_CTL */
			    copied = have_value =
				     XtConvertAndStore(wid, 
						       XrmQuarkToString(rawType),
						       &value,
						       "FontSet",
						       &convValue);
			else
			    copied = have_value =
				     XtConvertAndStore(wid, 
						       XrmQuarkToString(rawType),
						       &value,
						       XrmQuarkToString((int)res->xrm_type), /* Wyoming 64-bit fix */ 
						       &convValue);
		    }
		    else have_value = False;
		} 
		else have_value = True;
		
		/* Check for special font case */
		if (have_value)	{
		    
		    if (res->xrm_name == Qfont) {
			_XmRendFontName(rend) = value.addr;
			copied = True;
		    }
		    /* Bug Id : 4107466 Start */
		    /* fontType and fontName are reliant on each other so
		       applying code to set each resource back to default 
		       if only one of them is set. */
		    else if (res->xrm_name == QfontType) {
  			
			if ((unsigned long)_XmRendFontName(rend) == XmAS_IS) {
			    _XmRendFontType(rend) = XmAS_IS;
			    have_value = False;
			}
		    }
		    else if (res->xrm_name == QfontName) {
  			
			if (_XmRendFontType(rend) == XmAS_IS) {
			    _XmRendFontName(rend) = DEFAULT_fontName;
			    have_value = False;
			}
		    }
		}
	    }
	      
	  if (!got_one && have_value) got_one = True;

	  /* Set defaults */
	  if (!have_value) {
	      CopyFromArg((XtArgVal)(res->xrm_default_addr),
			  ((char *)GetPtr(rend) + res->xrm_offset),
			  res->xrm_size);
	      copied = True;
	  }
	  
	  /* Copy if needed */
	  if (!copied) {
	      if (res->xrm_type == QString)
		  *((String *)((char *)GetPtr(rend) + res->xrm_offset)) = value.addr;
	      else if (value.addr != NULL)
		  memcpy(((char *)GetPtr(rend) + res->xrm_offset),
			 value.addr, res->xrm_size);
	      else 
		  bzero(((char *)GetPtr(rend) + res->xrm_offset), res->xrm_size);
	  }
      }
  }
  if (searchList != stackSearchList) XtFree((char *)searchList);
  
#ifdef XTHREADS
  _XmProcessUnlock();
  if (app)
      _XmAppUnlock(app);
#endif
  return(got_one);
}


/* Sets all resources to defaults from resource list. */
static void
SetDefault(XmRendition rend)
{
    /* A more robust implementation of this routine would be to loop
     * over _XmRenditionResources and use CopyFromArg to reset values
     * in rend, but to improve performance we use direct assignments. */
    if (rend == NULL) return;
    
    /* Leave _XmRendFontOnly unchanged.	 */
    /* Leave _XmRendRefcount unchanged.	 */
    /* Leave _XmRendDisplay unchanged.	 */
    /* Leave _XmRendGC unchanged.	 */
    /* Leave _XmRendTags unchanged.	 */
    /* Leave _XmRendCount unchanged.	 */
    /* Leave _XmRendHadEnds unchanged.	 */
    _XmRendLoadModel(rend)	= DEFAULT_loadModel;
    _XmRendTag(rend)		= DEFAULT_tag;
    _XmRendFontName(rend)	= DEFAULT_fontName;
    _XmRendFontType(rend)	= DEFAULT_fontType;
    _XmRendFont(rend)		= DEFAULT_font;
    _XmRendTabs(rend)		= DEFAULT_tabs;
    _XmRendBG(rend)		= DEFAULT_background;
    _XmRendFG(rend)		= DEFAULT_foreground;
    _XmRendUnderlineType(rend)  = DEFAULT_underlineType;
    _XmRendStrikethruType(rend) = DEFAULT_strikethruType;
    _XmRendBGState(rend)	= DEFAULT_backgroundState;
    _XmRendFGState(rend)	= DEFAULT_foregroundState;
#ifdef SUN_CTL
    _XmRendLayoutIsCTL(rend)	= DEFAULT_layoutIsCtl;
    _XmRendLayoutModifier(rend) = DEFAULT_layoutModifier;
#endif /* CTL */
#ifdef SUN_TBR
    _XmRendTBR(rend)	        = DEFAULT_TextBoundary;
#endif /*SUN_TBR*/
}

/* Extern function to pick out display from rendertable. */
Display *
_XmRenderTableDisplay(XmRenderTable table)
{
    return(_XmRTDisplay(table));
}


/* Find a rendition in table with matching tag.  Call callback if not   */
/* found and callback available.  Fail if need_font is true and         */
/* rendition found does not provide font.                               */
XmRendition
_XmRenderTableFindRendition(XmRenderTable table,
                            XmStringTag   tag,
                            Boolean       cached_tag,
                            Boolean       need_font,
                            Boolean       call,
                            short *       index)
{
   int                     i, j;
   XmRendition             rend;
   Boolean                 hit=FALSE;
   XmDisplayCallbackStruct cb;
   XmDisplay               dsp;
   XmRenderTable           copy;

   if ((table == NULL) || (tag == NULL))
      return(NULL);

   for (;;)             /* (May have to try twice.)   */
    {
      for (i=0; i<_XmRTCount(table); i++)
       {
         rend = _XmRTRenditions(table)[i];

         if ((cached_tag) ?
             (_XmRendTag(rend) == tag) :
             (strcmp(_XmRendTag(rend), tag) == 0))
          {
            hit = TRUE;

            if ((_XmRendFont(rend) == NULL) &&
                NameIsString(_XmRendFontName(rend)))
             {
               if (_XmRendLoadModel(rend) == XmLOAD_DEFERRED)
                  _XmRendLoadModel(rend) = XmLOAD_IMMEDIATE;

               ValidateAndLoadFont(rend, _XmRendDisplay(rend));

               if (need_font && (_XmRendFont(rend) == NULL))
                  break;
             }

            if (index != NULL)
               *index = i;
            return rend;
          }
       }

      if (hit || !call)
         break;

      call = FALSE;
      if (_XmRTDisplay(table) != NULL)       /* Call callback */
       {
         dsp = (XmDisplay) XmGetXmDisplay(_XmRTDisplay(table));

         /* CR 7964: XtHasCallbacks is surprisingly expensive, */
         /* so we use a conservative approximation here. */
         if (dsp && dsp->display.noRenditionCallback)
          {
            copy = XmRenderTableCopy(table, NULL, 0);

            cb.reason = XmCR_NO_RENDITION;
            cb.event = NULL;
            cb.render_table = copy;
            cb.tag = tag;

            XtCallCallbackList((Widget)dsp, dsp->display.noRenditionCallback, &cb);

            if (cb.render_table != copy)
             {
               /* Callback mutated table.  Update table with */
               /* substitution and search again. */

               for (j=0; j<_XmRTCount(table); j++)
                {
                  FreeRendition(_XmRTRenditions(table)[j]);
                }

               if (_XmRTRefcountDec(table) == 0)
                {
                  XtFree((char *)GetPtr(table));
                }

               SetPtr(table, GetPtr(cb.render_table));
               FreeHandle(cb.render_table);
             }
            else
             {
               break;
             }
          }
         else
          {
            break;
          }
       }
      else
       {
         break;
       }
    }

   if (index != NULL)
      *index = -1;
   return NULL;
}


/* If to has resource unset and from has it set, set in to. */
static void
SetRend(XmRendition to,
	XmRendition from)
{
#ifdef SUN_CTL
    if (_XmRendTag(to) != XmS)
	_XmRendTag(to) = _XmStringCacheTag(_XmRendTag(from), XmSTRING_TAG_STRLEN);
#endif /* CTL */
  if (NameIsString(_XmRendFontName(from)) &&
      !NameIsString(_XmRendFontName(to)))
    _XmRendFontName(to) = _XmRendFontName(from);
  if ((_XmRendFontType(from) != XmAS_IS) &&
      (_XmRendFontType(to) == XmAS_IS))
    _XmRendFontType(to) = _XmRendFontType(from);
  if ((_XmRendLoadModel(from) != XmAS_IS) &&
      (_XmRendLoadModel(to) == XmAS_IS))
    _XmRendLoadModel(to) = _XmRendLoadModel(from);
  if ((_XmRendFont(from) != NULL) &&
      ((unsigned long)_XmRendFont(to) == XmAS_IS)) /* Wyoming 64-bit fix */ 
    _XmRendFont(to) = _XmRendFont(from);
  if (ListIsList(_XmRendTabs(from)) &&
      !ListIsList(_XmRendTabs(to)))
    _XmRendTabs(to) = _XmRendTabs(from);
  if ((_XmRendFG(from) != XmUNSPECIFIED_PIXEL) &&
      (_XmRendFG(to) == XmUNSPECIFIED_PIXEL))
    _XmRendFG(to) = _XmRendFG(from);
  if ((_XmRendBG(from) != XmUNSPECIFIED_PIXEL) &&
      (_XmRendBG(to) == XmUNSPECIFIED_PIXEL))
    _XmRendBG(to) = _XmRendBG(from);
  if ((_XmRendUnderlineType(from) != XmAS_IS) &&
      (_XmRendUnderlineType(to) == XmAS_IS))
    _XmRendUnderlineType(to) = _XmRendUnderlineType(from);
  if ((_XmRendStrikethruType(from) != XmAS_IS) &&
      (_XmRendStrikethruType(to) == XmAS_IS))
    _XmRendStrikethruType(to) = _XmRendStrikethruType(from);
#ifdef SUN_CTL
  _XmRendLayoutIsCTL(to) = _XmRendLayoutIsCTL(from);
  if ((_XmRendLayoutModifier(from) != NULL) &&
      (_XmRendLayoutModifier(to) == NULL))
      _XmRendLayoutModifier(to) = _XmRendLayoutModifier(from);
#endif /* CTL */
}

/* Check that all resources are not default values. */
static Boolean
RendComplete(XmRendition rend)
{
  return(((unsigned long)_XmRendFontName(rend) != XmAS_IS) && /* Wyoming 64-bit fix */
	 (_XmRendFontType(rend) != XmAS_IS) &&
	 (_XmRendLoadModel(rend) != XmAS_IS) &&
	 ((unsigned long)_XmRendFont(rend) != XmAS_IS) && /* Wyoming 64-bit fix */ 
	 ((unsigned long)_XmRendTabs(rend) != XmAS_IS) && /* Wyoming 64-bit fix */ 
	 (_XmRendFG(rend) != XmUNSPECIFIED_PIXEL) &&
	 (_XmRendBG(rend) != XmUNSPECIFIED_PIXEL) &&
	 (_XmRendUnderlineType(rend) != XmAS_IS) &&
	 (_XmRendStrikethruType(rend) != XmAS_IS));
}



/* Search rt for all renditions matching tags, successively merging */
/* resource values in scr rendition.                                */

XmRendition
_XmRenditionMerge(Display *      d,
                  XmRendition *  scr,
                  XmRendition    base_rend,
                  XmRenderTable  rt,
                  XmStringTag    base_tag,
                  XmStringTag *  tags,
#if NeedWidePrototypes
                  unsigned int   tag_count,
                  unsigned int   copy
#else
                  unsigned short tag_count,
                  Boolean        copy
#endif /* NeedWidePrototypes */
                 )
{
   XmRendition    rend, tmp;
   int            i;

   if (scr == NULL)
    {
      rend = XmRenditionCreate(NULL, XmS, NULL, 0); /* Create new */
    }
   else
    {
      rend = *scr;
      if (copy)
       {
         if (NameIsString(_XmRendFontName(rend)))
          {
            XtFree(_XmRendFontName(rend));
          }
         if (ListIsList(_XmRendTabs(rend)))
          {
            XmTabListFree(_XmRendTabs(rend));
          }
       }
      SetDefault(rend);         /* Reset state */
    }

   for (i=(tag_count-1); i>=0; i--)
    {
      tmp = _XmRenderTableFindRendition(rt, tags[i], TRUE, FALSE, TRUE, NULL);
      if (tmp == NULL)
         continue;

      SetRend(rend, tmp);
      if (RendComplete(rend))
         break;
    }

   if (!RendComplete(rend))
    {
      short    index;

      _XmRenderTableFindFallback(rt, base_tag, TRUE, &index, &tmp);
      if (tmp != NULL)
         SetRend(rend, tmp);
    }

   if (base_rend != NULL)
    {
      SetRend(rend, base_rend);

      if (_XmRendFGState(base_rend) == XmFORCE_COLOR)
         _XmRendFG(rend) = _XmRendFG(base_rend);

      if (_XmRendBGState(base_rend) == XmFORCE_COLOR)
         _XmRendBG(rend) = _XmRendBG(base_rend);
    }

   CleanupResources(rend, copy);

   return rend;
}


/****************
 * If the cached_tag flag is true, _XmRenderTableFindFallback assumes that the
 *   tag pointer is a pointer out of the (local) tag cache.
 *   Since XmRenditionCreate also uses tag pointers out of this cache,
 *   a string compare is avoided by simply comparing pointer values.
 ****************/
extern Boolean 
_XmRenderTableFindFallback(
        XmRenderTable rendertable,
        XmStringTag tag,
#if NeedWidePrototypes
        int cached_tag,
#else
        Boolean cached_tag,
#endif /* NeedWidePrototypes */
        short *indx,
	XmRendition *rend_ptr )
{   
  XmStringTag     search_cset = NULL;
  
  *indx = -1 ;

  if ((rendertable != NULL) && (_XmRTCount(rendertable) == 0))
    {
      *rend_ptr = NULL;
      return(FALSE);
    }
  
  if (rendertable != NULL)
    {   
      if (tag != NULL)
	{
	  if (cached_tag)			  /* No XmSTRING_DEFAULT_CHARSET */
	    {   
	      *rend_ptr = (XmRendition)
		_XmRenderTableFindRendition(rendertable, tag, TRUE, TRUE, FALSE,
					    indx);
	      if (*rend_ptr != NULL) return(TRUE);
	    }
	  else
	    {   
	      XmStringTag       curtag; 

	      if ((strcmp(tag, XmSTRING_DEFAULT_CHARSET) == 0))
		curtag = _XmStringGetCurrentCharset();
	      else curtag = tag;

	      *rend_ptr = (XmRendition)
		_XmRenderTableFindRendition(rendertable, curtag, FALSE, TRUE, FALSE,
					    indx);
	  
	      if (*rend_ptr != NULL) return(TRUE);
	    } 
      
	  /* Didn't find a match.  See if tag is one of the defaults
	     and search for the other. */
	  if (_XmStringIsCurrentCharset(tag))
	    {
	      search_cset = XmFONTLIST_DEFAULT_TAG;

	      *rend_ptr = (XmRendition)
		_XmRenderTableFindRendition(rendertable, search_cset, TRUE, 
					    TRUE, FALSE, indx);
	  
	      if (*rend_ptr != NULL) return(TRUE);
	    }
	  else if ((tag == XmFONTLIST_DEFAULT_TAG) ||
		   (strcmp(tag, XmFONTLIST_DEFAULT_TAG) == 0))
	    {
	      search_cset = _XmStringGetCurrentCharset();
	
	      *rend_ptr = (XmRendition)
		_XmRenderTableFindRendition(rendertable, search_cset, FALSE, 
					    TRUE, FALSE, indx);
	  
	      if (*rend_ptr != NULL) return(TRUE);
	    }
	}
        
      /* Otherwise pick up first font(set) if tag a default value. */
      if ((tag == NULL) ||
	  (tag == XmFONTLIST_DEFAULT_TAG) ||
	  (strcmp(tag, XmFONTLIST_DEFAULT_TAG) == 0) ||
	  _XmStringIsCurrentCharset(tag))
	return(_XmRenderTableFindFirstFont(rendertable, indx, rend_ptr));
    }
  *rend_ptr = NULL; 
  *indx = -1;
  return(FALSE);
}

extern Boolean
_XmRenderTableFindFirstFont(XmRenderTable rendertable,
			    short *indx,
			    XmRendition *rend_ptr)
{
    int i, f_idx = -1, fs_idx = -1;

    for (i = _XmRTCount(rendertable) - 1; i >= 0; i--) {
	*rend_ptr = _XmRTRenditions(rendertable)[i];
	if (_XmRendFont(*rend_ptr) != NULL)
	    if (_XmRendFontType(*rend_ptr) == XmFONT_IS_FONT) f_idx = i;
#ifdef SUN_CTL
	    else fs_idx = i;
#else /* CTL */
	else if (_XmRendFontType(*rend_ptr) == XmFONT_IS_FONTSET) fs_idx = i;
#endif /* CTL */
    }
    if (fs_idx >= 0) {
	*rend_ptr = _XmRTRenditions(rendertable)[fs_idx];
	*indx = fs_idx;
    }
    else if (f_idx >= 0) {
	*rend_ptr = _XmRTRenditions(rendertable)[f_idx];
	*indx = f_idx;
    }
    else {
	*rend_ptr = NULL;
	*indx = -1;
	return(FALSE);
    }
    return(TRUE); 
}

/* Put value of every resource in fromRend into toRend, copying where */
/* necessary. */
static void
CopyInto(XmRendition toRend,
	 XmRendition fromRend)
{
  _XmRendTag(toRend) = _XmStringCacheTag(_XmRendTag(fromRend),
					 XmSTRING_TAG_STRLEN);
  /* CR 7890 - the fontName might be XmAS_IS here - if so, we
   ** obviously don't want to do an XtNewString (implicit strcpy)
   */
  if (!NameIsString(_XmRendFontName(fromRend)))
    _XmRendFontName(toRend) = NULL;
  else
    _XmRendFontName(toRend) = XtNewString(_XmRendFontName(fromRend));
  _XmRendFontType(toRend) = _XmRendFontType(fromRend);
  _XmRendLoadModel(toRend) = _XmRendLoadModel(fromRend);
  _XmRendFont(toRend) = _XmRendFont(fromRend);
  _XmRendDisplay(toRend) = _XmRendDisplay(fromRend);
  
  if (!ListIsList(_XmRendTabs(fromRend)))
    _XmRendTabs(toRend) = NULL;
  else
    _XmRendTabs(toRend) = XmTabListCopy(_XmRendTabs(fromRend), 0, 0);
  _XmRendBG(toRend) = _XmRendBG(fromRend);
  _XmRendFG(toRend) = _XmRendFG(fromRend);
  _XmRendUnderlineType(toRend) = _XmRendUnderlineType(fromRend);
  _XmRendStrikethruType(toRend) = _XmRendStrikethruType(fromRend);
#ifdef SUN_CTL
  _XmRendLayoutIsCTL(toRend) = _XmRendLayoutIsCTL(fromRend);
  if (_XmRendLayoutModifier(fromRend) != NULL)
      _XmRendLayoutModifier(toRend) = XtNewString(_XmRendLayoutModifier(fromRend));
#endif /* CTL */
}

/* As above, except only change resources in toRend that are default. */
static void
MergeInto(XmRendition toRend,
	 XmRendition fromRend)
{
  _XmRendTag(toRend) = _XmStringCacheTag(_XmRendTag(fromRend),
					 XmSTRING_TAG_STRLEN);
  if ((_XmRendFontName(toRend) == NULL) &&
      NameIsString(_XmRendFontName(fromRend)))
    _XmRendFontName(toRend) = XtNewString(_XmRendFontName(fromRend));
  if (_XmRendFontType(toRend) == XmAS_IS)
    _XmRendFontType(toRend) = _XmRendFontType(fromRend);
  if (_XmRendLoadModel(toRend) == XmAS_IS)
    _XmRendLoadModel(toRend) = _XmRendLoadModel(fromRend);
  if (_XmRendFont(toRend) == NULL)
    _XmRendFont(toRend) = _XmRendFont(fromRend);
#ifdef SUN_CTL
  _XmRendLayoutIsCTL(toRend) = _XmRendLayoutIsCTL(fromRend);
  _XmRendLayoutModifier(toRend) = NULL;
  if (_XmRendLayoutModifier(fromRend) != NULL)
      _XmRendLayoutModifier(toRend) = XtNewString(_XmRendLayoutModifier(fromRend));
#endif /* CTL */
  if (!ListIsList(_XmRendTabs(toRend)) && 
      ListIsList(_XmRendTabs(fromRend)))
    _XmRendTabs(toRend) = XmTabListCopy(_XmRendTabs(fromRend), 0, 0);
  if (_XmRendBG(toRend) == XmUNSPECIFIED_PIXEL)
    _XmRendBG(toRend) = _XmRendBG(fromRend);
  if (_XmRendFG(toRend) == XmUNSPECIFIED_PIXEL)
    _XmRendFG(toRend) = _XmRendFG(fromRend);
  if (_XmRendUnderlineType(toRend) == XmAS_IS)
    _XmRendUnderlineType(toRend) = _XmRendUnderlineType(fromRend);
  if (_XmRendUnderlineType(toRend) == XmAS_IS)
    _XmRendStrikethruType(toRend) = _XmRendStrikethruType(fromRend);
}

/* Make a Clone--new handle and new data structure--of a rendition. */
static XmRendition
CloneRendition(XmRendition rend)
{
  _XmRendition 	copy;
  XmRendition	copy_handle;
  
  if (rend == NULL) return(NULL);
  
  copy = (_XmRendition)XtMalloc(sizeof(_XmRenditionRec));
  bzero((char*)copy, sizeof(_XmRenditionRec));
  copy_handle = GetHandle(_XmRendition);
  SetPtr(copy_handle, copy);
  
  _XmRendFontOnly(copy_handle) = FALSE;
  _XmRendRefcount(copy_handle) = 1;
  
  CopyInto(copy_handle, rend);
  return(copy_handle);
}

/* Set the old handle to point to a new data structure. */
static XmRendition
RenewRendition(XmRendition rend)
{
  _XmRendition copy;
  
  if (rend == NULL) return(NULL);
  
  copy = (_XmRendition)XtMalloc(sizeof(_XmRenditionRec));
  memcpy((char *)copy, (char *)GetPtr(rend), sizeof(_XmRenditionRec));
  SetPtr(rend, copy);
  
  _XmRendFontOnly(rend) = FALSE;
  _XmRendRefcount(rend) = 1;
  
  return(rend);
}

/* Allocate a new handle which points to the old data structure with */
/* an incremented refcount. */
static XmRendition
CopyRendition(XmRendition rend)
{
  XmRendition	copy;
  
  if (rend == NULL) return(NULL);

  if (_XmRendRefcountInc(rend) == 0)
    {
      _XmRendRefcountDec(rend);
      return(CloneRendition(rend));
    }
  else 
    {
      copy = GetHandle(_XmRendition);
      SetPtr(copy, GetPtr(rend));
      return(copy);
    }
}


/* Make a copy of a rendition, *including* the "scratch" info (tags,
 * GC, hadEnds).
 * Shared indicates whether or not this is a shared copy.
 */
XmRendition
_XmRenditionCopy(XmRendition rend,
		 Boolean shared)
{
  XmRendition toRend;
  int i;

  if (rend == NULL) return(NULL);

  if (shared) toRend = CopyRendition(rend);
  else toRend = CloneRendition(rend);

  /* If we had to clone, copy the 'scratch' info. */
  if (*toRend != *rend)
    {
      _XmRendGC(toRend) = _XmRendGC(rend);
      _XmRendTagCount(toRend) = _XmRendTagCount(rend);
      _XmRendHadEnds(toRend) = _XmRendHadEnds(rend);
      _XmRendTags(toRend) =
	(XmStringTag *)XtMalloc(sizeof(XmStringTag) * _XmRendTagCount(rend));
      for (i = 0; i < _XmRendTagCount(rend); i++)
	_XmRendTags(toRend)[i] = _XmRendTags(rend)[i];
    }
  
  return(toRend);
}

/* Creates new rendertable, adding any new renditions. */
/* Mutate rendertable.  Copy renditions. */
XmRenderTable
XmRenderTableAddRenditions(XmRenderTable oldtable, 
                           XmRendition *renditions,
                           Cardinal rendition_count,
                           XmMergeMode merge_mode)
{
   int            i, next;
   int            count = rendition_count;
   XmRendition    rend, match;
   _XmRenderTable table;
   XmRenderTable  newtable, tmptable = NULL;
   Boolean *      matches;
   short          idx;
   XtAppContext   app=NULL;

   if ((renditions == NULL) || (rendition_count == 0))
    {
      return oldtable;
    }

#ifdef XTHREADS
   if (_XmRendDisplay(renditions[0]))
    {
      app = XtDisplayToApplicationContext(_XmRendDisplay(renditions[0]));
    }
   if (app)
    {
      _XmAppLock(app);
    }
   else
    {
      _XmProcessLock();
    }
#endif
   if (oldtable == NULL)                  /* If no table as yet, malloc one,  */
    {
      table = (_XmRenderTable)XtMalloc(sizeof(_XmRenderTableRec) +
               (sizeof(XmRendition) * (rendition_count - RENDITIONS_IN_STRUCT)));
      newtable = GetHandle(_XmRenderTable);
      SetPtr(newtable, table);

      _XmRTCount(newtable) = rendition_count;
      _XmRTDisplay(newtable) = NULL;
      _XmRTRefcount(newtable) = 1;
      
      for (i = 0; i < rendition_count; i++)
       {                                  /* and copy in the renditions.      */
         _XmRTRenditions(newtable)[i] = CopyRendition(renditions[i]);
         if (_XmRTDisplay(newtable) == NULL)
          {
            _XmRTDisplay(newtable) = _XmRendDisplay(renditions[i]);
          }
       }
    }
   else                                   /* Else already have a table. In    */
    {                                     /* cases where additional entries   */
                                          /* have to be added, a new table is */
                                          /* allocated, and the old one wiped.*/
      matches = (Boolean *)ALLOCATE_LOCAL(rendition_count * sizeof(Boolean));
      bzero(matches, rendition_count * sizeof(Boolean));

      for (i=0; i<rendition_count; i++)   /* Loop to merge duplicate entries. */
       {
         rend = renditions[i];

         match = _XmRenderTableFindRendition(oldtable, _XmRendTag(rend),
                                             TRUE, FALSE, FALSE, &idx);
         if ((match != NULL) && (merge_mode != XmDUPLICATE))
          {                            /* Merge renditions.                */
            switch (merge_mode)
             {
               case XmMERGE_REPLACE:
                  FreeRendition(match);
                  _XmRTRenditions(oldtable)[idx] = CopyRendition(rend);
                  break;

               case XmSKIP:
                  break;

               case XmMERGE_OLD:
                  if (_XmRendRefcount(match) > 1) 
                   {
                     _XmRTRenditions(oldtable)[idx] = CloneRendition(match);
                     FreeRendition(match);
                     match = _XmRTRenditions(oldtable)[idx];
                   }          
                  MergeInto(match, rend);
                  break;

               case XmMERGE_NEW:
                  _XmRTRenditions(oldtable)[idx] = CloneRendition(rend);
                  MergeInto(_XmRTRenditions(oldtable)[idx], match);
                  FreeRendition(match);
                  break;

               default:
                  /* Note that memory is leaked here, but it is assumed that  */
                  /* since calls to this routine are internal, getting here   */
                  /* would be quite a serious bug.                            */

                  printf("NYI");
                  break;
             }

            matches[i] = TRUE;
            --count;
          }
       }

      if (count > 0)
       {
         table = (_XmRenderTable)XtMalloc(sizeof(_XmRenderTableRec) +
                  (sizeof(XmRendition) *
                  (_XmRTCount(oldtable) + count - RENDITIONS_IN_STRUCT)));
         newtable = GetHandle(_XmRenderTable);
         SetPtr(newtable, table);

         _XmRTDisplay(newtable) = _XmRTDisplay(oldtable);
         _XmRTRefcount(newtable) = 1;

         for (i=0; i<_XmRTCount(oldtable); i++)
          {                               /* First copy old Renditions,    */
            _XmRTRenditions(newtable)[i] = CopyRendition(_XmRTRenditions(oldtable)[i]);
          }

         next = _XmRTCount(oldtable);
         for (i=0; i<rendition_count; i++)   /* and then the new ones that */
          {                                  /* found no match.            */
            if (!matches[i])
             {
               _XmRTRenditions(newtable)[next] = CopyRendition(renditions[i]);
               ++next;
             }
          }

         _XmRTCount(newtable) = _XmRTCount(oldtable) + count;

         XmRenderTableFree(oldtable);
       }
      else                             /* Otherwise just return new handle */
       {                               /* to old table.                    */
         newtable = oldtable;
       }

      DEALLOCATE_LOCAL((char *)matches);
    }

#ifdef XTHREADS
   if (app)
    {
      _XmAppUnlock(app);
    }
   else
    {
      _XmProcessUnlock();
    }
#endif
   return newtable;
}


/* Remove matching renditions. */
/* Mutates oldtable, decrements removed renditions. */
XmRenderTable
XmRenderTableRemoveRenditions(XmRenderTable oldtable,
			      XmStringTag *tags,
			      int tag_count)
{
  XmRenderTable ret_val;
#ifdef XTHREADS
  XtAppContext  app=NULL;

  if (_XmRTDisplay(oldtable))
	app = XtDisplayToApplicationContext(_XmRTDisplay(oldtable));
  if (app) {
    _XmAppLock(app);
  }
  else {
    _XmProcessLock();
  }
#endif
  ret_val = _XmRenderTableRemoveRenditions(oldtable, tags,tag_count,
				FALSE, XmFONT_IS_FONT, NULL);
#ifdef XTHREADS
  if (app) {
     _XmAppUnlock(app);
  }
  else {
     _XmProcessUnlock();
  }
#endif
  return ret_val;
}

/* Remove matching renditions. */
/* Mutates oldtable, decrements removed renditions. */
/* If chk_font TRUE, checks that font and type also match. */
XmRenderTable
_XmRenderTableRemoveRenditions(XmRenderTable oldtable,
			       XmStringTag *tags,
			       int tag_count,
#if NeedWidePrototypes
			       int chk_font,
#else
			       Boolean chk_font,
#endif /* NeedWidePrototypes */
			       XmFontType type,
			       XtPointer font)
{
  int			i, j;
  int			count;
  _XmRenderTable	table;
  XmRenderTable		newtable = NULL;

  if ((oldtable == NULL) || (tags == NULL) || (tag_count == 0))
    return(oldtable);
  
  count = 0;
  
  if (_XmRTRefcount(oldtable) > 1)
    {
      /* Allocate new table */
      table = (_XmRenderTable)
	XtMalloc(sizeof(_XmRenderTableRec) +
		 (sizeof(XmRendition) *
		  (_XmRTCount(oldtable) - RENDITIONS_IN_STRUCT)));

      newtable = GetHandle(_XmRenderTable);
      SetPtr(newtable, table);

      _XmRTDisplay(newtable) = _XmRTDisplay(oldtable);
      _XmRTRefcount(newtable) = 1;

      /* Move old Renditions. */
      for (i = 0; i < _XmRTCount(oldtable); i++)
	_XmRTRenditions(newtable)[i] = _XmRTRenditions(oldtable)[i];
      _XmRTCount(newtable) = _XmRTCount(oldtable);
      
      FreeHandle(oldtable);
      
      oldtable = newtable;
    }
  /* Iterate over renditions */
  for (i = 0; i < _XmRTCount(oldtable); i++)
    {
      /* Match against tags */
      for (j = 0; j < tag_count; j++)
	{
	  if ((strcmp(_XmRendTag(_XmRTRenditions(oldtable)[i]),
		      tags[j]) == 0) &&
	      (!chk_font ||
	       ((font == _XmRendFont(_XmRTRenditions(oldtable)[i])) &&
		(type == _XmRendFontType(_XmRTRenditions(oldtable)[i])))))
	    {
	      FreeRendition(_XmRTRenditions(oldtable)[i]);
	      _XmRTRenditions(oldtable)[i] = NULL;
	      break;
	    }
	}
      if (_XmRTRenditions(oldtable)[i] != NULL) 
	{
	  if (count != i)
	    _XmRTRenditions(oldtable)[count] = _XmRTRenditions(oldtable)[i];

	  count++;
	}
    }
  if (count == 0)
    /* No renditions left. Return NULL. */
    {
      XmRenderTableFree(oldtable);
      return(NULL);
    }
  else if (count < _XmRTCount(oldtable))
    {
      /* Realloc table */
      table = (_XmRenderTable)XtRealloc((char *)*oldtable,
					sizeof(_XmRenderTableRec) +
					(sizeof(XmRendition) *
					 (count - RENDITIONS_IN_STRUCT)));
      if (newtable == NULL) 
	{
	  newtable = GetHandle(_XmRenderTable);
	  FreeHandle(oldtable);
	}      
      SetPtr(newtable, table);

      _XmRTCount(newtable) = count;
  
      return(newtable);
    }
  return(oldtable);
}

static void
CopyFromArg(XtArgVal src, char *dst, unsigned int size)
{
  if (size > sizeof(XtArgVal))
    memcpy((char *)dst, (char *)src, (size_t)size);
  else {
    union {
      long	longval;
      int	intval; /* Added,Fix for Bug 4120977, sometimes size==4. */
      short	shortval;
      char	charval;
      char*	charptr;
      XtPointer	ptr;
    } u;
    char *p = (char*)&u;
    if      (size == sizeof(long))	    u.longval = (long)src;
    /* Added next line, fix for Bug 4120977, sometimes size==4. */
    else if (size == sizeof(int))	    u.intval = (int)src;
    else if (size == sizeof(short))	    u.shortval = (short)src;
    else if (size == sizeof(char))	    u.charval = (char)src;
    else if (size == sizeof(XtPointer))	    u.ptr = (XtPointer)src;
    else if (size == sizeof(char*))	    u.charptr = (char*)src;
    else				    p = (char*)&src;

    memcpy((char *)dst, p, (size_t)size);
  }
} /* CopyFromArg */

static void
CopyToArg(char *src, XtArgVal *dst, unsigned int size)
{
  if ((void *)(*dst) == NULL) {
    /* old GetValues semantics (storing directly into arglists) are bad,
     * but preserve for compatibility as long as arglist contains NULL.
     */
    if	    (size == sizeof(long))	   *dst = (XtArgVal)*(long*)src;
    /* Added next line, fix for Bug 4120977, sometimes size==4. */
    else if (size == sizeof(int))    *dst = (XtArgVal)*(int*)src;
    else if (size == sizeof(short))    *dst = (XtArgVal)*(short*)src;
    else if (size == sizeof(char))	   *dst = (XtArgVal)*(char*)src;
    else if (size == sizeof(XtPointer)) *dst = (XtArgVal)*(XtPointer*)src;
    else if (size == sizeof(char*))    *dst = (XtArgVal)*(char**)src;
    else if (size == sizeof(XtArgVal)) *dst = *(XtArgVal*)src;
    else memcpy((char*)dst, (char*)src, (size_t)size);
  }
  else {
    /* proper GetValues semantics: argval is pointer to destination */
    if	(size == sizeof(long))	   *((long*)*dst) = *(long*)src;
    /* Added next line, fix for Bug 4120977, sometimes size==4. */
    else if (size == sizeof(int))    *((int*)*dst) = *(int*)src;
    else if (size == sizeof(short))    *((short*)*dst) = *(short*)src;
    else if (size == sizeof(char))	   *((char*)*dst) = *(char*)src;
    else if (size == sizeof(XtPointer)) *((XtPointer*)*dst) = *(XtPointer*)src;
    else if (size == sizeof(char*))    *((char**)*dst) = *(char**)src;
    else if (size == sizeof(XtArgVal)) *((XtArgVal*)*dst)= *(XtArgVal*)src;
    else memcpy((char *)*dst, (char *)src, (size_t)size);
  }
} /* CopyToArg */


/* Copies renditions matching tags to a new table. */
/* If all renditions copied then duplicate rendertable, duplicate */
/* renditions.  Otherwise, mutate rendertable, duplicate renditions. */
XmRenderTable
XmRenderTableCopy(XmRenderTable table, XmStringTag *tags, int tag_count)
{
   XmRenderTable     rt;
   _XmRenderTable    t;
   int               i, j, count, size;
   XmRendition       rend;
   XtAppContext      app=NULL;
   u_char            new_table_created=FALSE;


   if (table == NULL)
    {
      return((XmRenderTable)NULL);
    }

#ifdef XTHREADS
  if (_XmRTDisplay(table))
     app = XtDisplayToApplicationContext(_XmRTDisplay(table));
  if (app) {
     _XmAppLock(app);
  }
  else {
     _XmProcessLock();
  }
#endif

   count = 0;

   if (tags == NULL)
    {
      tag_count = 0;
    }

   if ((_XmRTRefcountInc(table) == 0) || tag_count)
    {                                     /* Create new table if it is likely */
                                          /* to be different to the old one,  */
                                          /* or on ref count overflow.        */
      _XmRTRefcountDec(table);

      if (tag_count > 0)
       {
         size = (sizeof(_XmRendition) * (tag_count - RENDITIONS_IN_STRUCT));
       }
      else
       {
         size = (sizeof(_XmRendition) * (_XmRTCount(table)-RENDITIONS_IN_STRUCT));
       }
      size = (size < 0) ? 0 : size;

      t = (_XmRenderTable)XtMalloc(sizeof(_XmRenderTableRec) + size);
      rt = GetHandle(_XmRenderTable);
      SetPtr(rt, t);
      _XmRTRefcount(rt) = 1;
      new_table_created = TRUE;
    }

   if (!new_table_created)
    {
      rt = GetHandle(_XmRenderTable);
      SetPtr(rt, GetPtr(table));
    }
   else
    {
      XmRendition    match;
      int            nbr_entries=_XmRTCount(table);
      u_char         take_this_one;


      for (i=0; i<nbr_entries; i++)
       {
         match = 0;
         if (tag_count)
          {
            take_this_one = FALSE;
            for (j=0; j<tag_count; j++)
             {
               if (strcmp(_XmRendTag(_XmRTRenditions(table)[i]), tags[j]) == 0)
                {
                  take_this_one = TRUE;
                  if (count == (tag_count-1))
                   {                      /* If all tags have been matched    */
                     i = nbr_entries;     /* then table will have been built  */
                                          /* at the end of the "i" loop.      */
                   }
                  break;                  /* (So we stop checking renditions.)*/
                }
             }
          }
         else
          {
            take_this_one = TRUE;
          }

         if (take_this_one)
          {
            _XmRTRenditions(rt)[i] = CopyRendition(_XmRTRenditions(table)[i]);
            ++count;
          }
       }

      if (tag_count && (tag_count != count))
       {                                  /* If not all tags were matched,    */
                                          /* adjust table to smaller size.    */
         t = (_XmRenderTable)XtRealloc((char *)t,
               sizeof(_XmRenderTableRec) + (sizeof(XmRendition) *
                  (count - RENDITIONS_IN_STRUCT)));
         SetPtr(rt, t);
       }
      _XmRTCount(rt) = count;
      _XmRTDisplay(rt) = _XmRTDisplay(table);
    }

#ifdef XTHREADS
   if (app)
    {
      _XmAppUnlock(app);
    }
   else
    {
      _XmProcessUnlock();
    }
#endif
   return rt;
}


/* Decrement rendertable, free if refcount is zero.  XmRenditionFree */
/* renditions. */
void
XmRenderTableFree(XmRenderTable table)
{
   int   i;


   if (table == NULL || *table == NULL)
      return;

   _XmProcessLock();
   if (_XmRTRefcountDec(table) == 0)
    {
      for (i=0; i<_XmRTCount(table); i++)
       {
         FreeRendition(_XmRTRenditions(table)[i]);
       }
      XtFree((char *)GetPtr(table));
    }
   FreeHandle(table);
   _XmProcessUnlock();
}


/* Get list of tags of all renditions in table. */
int
XmRenderTableGetTags(XmRenderTable table, 
		     XmStringTag **tag_list)
{
  int i, ret_val;
  XtAppContext          app = NULL;

  if (table == NULL)
    {
      *tag_list = NULL;
      return(0);
    }

  app = XtDisplayToApplicationContext(_XmRTDisplay(table));
  _XmAppLock(app);
  *tag_list = 
    (XmStringTag *)XtMalloc(sizeof(XmStringTag) * _XmRTCount(table));

  for (i = 0; i < _XmRTCount(table); i++)
      (*tag_list)[i] = 
	XtNewString(_XmRendTag(_XmRTRenditions(table)[i]));
      
  ret_val = _XmRTCount(table);
  _XmAppUnlock(app);
  return ret_val;
}

/* Returns copy of matching rendition. */
XmRendition
XmRenderTableGetRendition(XmRenderTable table,
			  XmStringTag tag)
{
  XmRendition ret_val;
  _XmDisplayToAppContext(_XmRTDisplay(table));

  _XmAppLock(app);
  ret_val = CopyRendition(_XmRenderTableFindRendition(table, tag,
			FALSE, FALSE, FALSE, NULL));
  _XmAppUnlock(app);
  return ret_val;
}

/* Returns array of copies of matching renditions. */
XmRendition *
XmRenderTableGetRenditions(XmRenderTable table,
			   char **tags,
			   Cardinal tag_count)
{
  XmRendition	rend, *rends;
  int		i, count;
  XtAppContext  app = NULL;

  if ((table == NULL) || (tags == NULL) || (tag_count == 0))
      return(NULL);
 
#ifdef XTHREADS
  if (_XmRTDisplay(table))
  {
     app = XtDisplayToApplicationContext(_XmRTDisplay(table));
     _XmAppLock(app);
  }
#endif
  rends = (XmRendition *)XtMalloc(tag_count * sizeof(XmRendition));
  
  count = 0;
  for (i = 0; i < tag_count; i++)
    {
      rend = _XmRenderTableFindRendition(table, tags[i],
					 FALSE, FALSE, FALSE, NULL);
      if (rend != NULL)
	{
	  rends[count] = CopyRendition(rend);
	  count++;
	}
    }
  
  if (count < tag_count)
    rends = (XmRendition *)XtRealloc((char *)rends, count * sizeof(XmRendition));
  
#ifdef XTHREADS
  if (app) {
     _XmAppUnlock(app);
  }
#endif
  return(rends);
}

/* Wrapper for calling XtWarning functions. */
static void
RenditionWarning(char *tag,
		 char *type,
		 char *message,
     		 Display *dpy)
{
  char *params[1];
  Cardinal num_params = 1 ;
  Display *d;

  /* the MotifWarningHandler installed in VendorS.c knows about
     this convention */
  params[0] = XME_WARNING;

  if (dpy)
     d = dpy;
  else
     d = _XmGetDefaultDisplay();
  if (d)
    XtAppWarningMsg (XtDisplayToApplicationContext(d),
		     tag, type, "XmRendition", 
		     message, params, &num_params);
  else XtWarning(message);
}

/* Replace XmAS_IS and copy as necessary. */
static void
CleanupResources(XmRendition rend,
		 Boolean copy)
{
  if ((unsigned long)_XmRendFont(rend) == XmAS_IS)  /* Wyoming 64-bit fix */ 
    _XmRendFont(rend) = NULL;
  else if (_XmRendFontType(rend) == XmAS_IS)
    _XmRendFontType(rend) = XmFONT_IS_FONT;

  if (((unsigned long)_XmRendFontName(rend) == XmAS_IS) || /* Wyoming 64-bit fix */ 
      (strcmp(_XmRendFontName(rend), XmSXmAS_IS) == 0))
    _XmRendFontName(rend) = NULL;
  else if (copy)
    _XmRendFontName(rend) = XtNewString(_XmRendFontName(rend));
  
  if ((unsigned long)_XmRendTabs(rend) == XmAS_IS) /* Wyoming 64-bit fix */ 
    _XmRendTabs(rend) = NULL;
  else if (copy)
    _XmRendTabs(rend) = XmTabListCopy(_XmRendTabs(rend), 0, 0);
}


/* Emit warning and set default if tag is NULL. */
static void
ValidateTag(XmRendition rend,
	    XmStringTag dflt)
{
  if (_XmRendTag(rend) == NULL)
    {
      RenditionWarning(_XmRendTag(rend), "NO_NULL_TAG",
	NO_NULL_TAG_MSG, _XmRendDisplay(rend));
      _XmRendTag(rend) = _XmStringCacheTag(dflt, XmSTRING_TAG_STRLEN);
    }
}

#ifdef SUN_CTL
/*
 * Make sure that FontSet is created for AWT in cases where
 * a preexisting rendition is passed into CreateRendition
 */
static void
LoadAwtFont(XmRendition rend, Display *display)
{
  XtPointer   font;
  Boolean result = False;
  XmXOCFuncRec *fnRec = (XmXOCFuncRec*)malloc(sizeof(XmXOCFuncRec));
  
  _XmRendDisplay(rend) = display;
  
  if (ctlLocale(fnRec)) {
    String lo_modifier = _XmRendLayoutModifier(rend);
    
    font = _XmRendFont(rend);
    font = (XtPointer)XmCreateXmXOC((XOC)font, lo_modifier, fnRec);
    result = font ? True : False; /* compiler doesn't like "(Boolean)font" */
    _XmRendLayoutIsCTL(rend) = result;
  }
  
  if (!_XmRendLayoutIsCTL(rend))
    free(fnRec);
  else
    _XmRendFont(rend) = font;
}
#endif


/* Make sure all the font related resources make sense together and */
/* then load the font specified by fontName if necessary. */
static void
ValidateAndLoadFont(XmRendition rend, Display *display)
{
   XrmString         locale;
   XtPointer         font;
   XrmValue          args[2];
   Cardinal          num_args = 0;
   XrmValue          fromVal;
   XrmValue          toVal;
   Boolean           result=False;
#ifdef SUN_CTL
   XmXOCFuncRec *    fnRec=(XmXOCFuncRec*)malloc(sizeof(XmXOCFuncRec));
#endif

   _XmRendDisplay(rend) = display;

   if (_XmRendLoadModel(rend) != XmLOAD_DEFERRED)
    {
      XmDisplay                  dsp=NULL;
      XmDisplayCallbackStruct    cb;

      if ((_XmRendFont(rend) == NULL) && (_XmRendFontName(rend) != NULL))
       {
         if (_XmRendFontType(rend) != XmAS_IS)
          {
            if (display == NULL)
             {
               RenditionWarning(_XmRendTag(rend), "NULL_DISPLAY",
                                NULL_DISPLAY_MSG, NULL);
               return;
             }
            args[0].addr = (XPointer) &display;
            args[0].size = sizeof(Display*);
            num_args++;
#ifdef SUN_CTL
            if (!NameIsString(_XmRendFontName(rend)))
             {
               char *   s=(char*)XmDEFAULT_FONT;
               _XmRendFontName(rend) = XtNewString(s);
             }
#endif
            fromVal.addr = _XmRendFontName(rend);
            fromVal.size = strlen(_XmRendFontName(rend));
            toVal.addr = (XPointer) &font;
            toVal.size = sizeof (XtPointer);

            switch (_XmRendFontType(rend))
             {
              case XmFONT_IS_FONT:
               result = XtCallConverter(display, XtCvtStringToFontStruct,
                                        args, num_args, &fromVal, &toVal, NULL);
               break;
#ifdef SUN_CTL
              case XmFONT_IS_XOC:
#endif
              case XmFONT_IS_FONTSET:
               locale = XrmQuarkToString(XrmStringToQuark(setlocale(LC_ALL, NULL)));
               args[1].addr = (XPointer)&locale;
               args[1].size = sizeof(XrmString);
               num_args++;

               result = XtCallConverter(display, XtCvtStringToFontSet, args,
                                        num_args, &fromVal, &toVal, NULL);
#ifdef SUN_CTL
               if (result && ctlLocale(fnRec))
                {
                  String lo_modifier = _XmRendLayoutModifier(rend);

                  font = (XtPointer)XmCreateXmXOC((XOC)font, lo_modifier, fnRec);
                  result = font ? True : False; /* compiler doesn't like "(Boolean)font" */
                  _XmRendLayoutIsCTL(rend) = result;
                }

               if (!_XmRendLayoutIsCTL(rend))
                {
                  free(fnRec);
                }
#endif
               break;

              default:
               RenditionWarning(_XmRendTag(rend), "INVALID_TYPE",
                                INVALID_TYPE_MSG, _XmRendDisplay(rend));

               break;
             }

            if (!result)
             {

               if (display != NULL)
                {
                  dsp = (XmDisplay) XmGetXmDisplay(display);
                  cb.reason = XmCR_NO_FONT;
                  cb.event = NULL;
                  cb.rendition = rend;
                  cb.font_name = _XmRendFontName(rend);

                  /* We must know for sure whether there are any  */
                  /* callbacks, so we have to use XtHasCallbacks. */

                  if (XtHasCallbacks((Widget)dsp, XmNnoFontCallback) ==
                        XtCallbackHasSome)
                   {
                     XtCallCallbackList((Widget)dsp,
                                        dsp->display.noFontCallback, &cb);
                     return;
                   }
                }
               RenditionWarning(_XmRendTag(rend), "CONVERSION_FAILED",
                                CONVERSION_FAILED_MSG, _XmRendDisplay(rend));
             }
            else
             {
               _XmRendFont(rend) = font;
             }
          }
         else
          {
            RenditionWarning(_XmRendTag(rend), "NULL_FONT_TYPE",
                             NULL_FONT_TYPE_MSG, _XmRendDisplay(rend));
          }
       }
      else
       {
         if ((_XmRendLoadModel(rend) == XmLOAD_IMMEDIATE) &&
             (_XmRendFont(rend) == NULL) &&
             (_XmRendFontName(rend) == NULL)
            )
          {
            RenditionWarning(_XmRendTag(rend), "NULL_LOAD_IMMEDIATE",
                             NULL_LOAD_IMMEDIATE_MSG, _XmRendDisplay(rend));
          }
#ifdef SUN_CTL        
         free(fnRec);
#endif
       }
    }
}


/* Create new rendition. */
XmRendition
XmRenditionCreate(Widget widget,
		  XmStringTag tag,
		  ArgList arglist,
		  Cardinal argcount)
{
  XmRendition  ret_val;
  XtAppContext app=NULL;

  /* If cannot applock, assume its an internal call (from
   *	fontlist code, etc.) and already process locked.
   */
  if (widget)
	app = XtWidgetToApplicationContext(widget);
  if (app) {
	_XmAppLock(app);
  }
  else {
	_XmProcessLock();
  }
  ret_val = _XmRenditionCreate(NULL, widget, XmS, XmCRenderTable,
			    tag, arglist, argcount, NULL);
  if (app) {
	_XmAppUnlock(app);
  }
  else {
	_XmProcessUnlock();
  }

  return ret_val;
}

/* Internal function.  Called from XmRenditionCreate, resource */
/* converter, and Mrm create function. */
XmRendition
_XmRenditionCreate(Display *display,
		   Widget widget,
		   String resname,
		   String resclass,
		   XmStringTag tag,
		   ArgList arglist,
		   Cardinal argcount,
		   Boolean *in_db)
{
    XmRendition		rend;
    _XmRendition	rend_int;
    Boolean		result;
  
    if ((display == NULL) && (widget != NULL))
	display = XtDisplayOfObject(widget);

    if ((tag != NULL) &&
	(tag != XmFONTLIST_DEFAULT_TAG) &&
	(strcmp(tag, XmSTRING_DEFAULT_CHARSET) == 0))
	tag = _XmStringGetCurrentCharset();
    
    /* Allocate rendition. */
    rend_int = (_XmRendition)XtMalloc(sizeof(_XmRenditionRec));
    bzero((char*)rend_int, sizeof(_XmRenditionRec));
    rend = GetHandle(_XmRendition);
    SetPtr(rend, rend_int);
    
    _XmRendRefcount(rend) = 1;
    /* For now, FontOnly renditions aren't implemented. */
    _XmRendFontOnly(rend) = FALSE;
    
    /* X resource DB query */
    result = GetResources(rend, display, widget, resname, resclass, tag, arglist, argcount);
    
    if (in_db != NULL) *in_db = result;
    
    if (tag == NULL) {
	if (result == FALSE) {
	    XtFree((char *)rend_int);
	    FreeHandle(rend);
	    return(NULL);
	}
	else tag = _MOTIF_DEFAULT_LOCALE;
    }
    
    _XmRendTag(rend) = _XmStringCacheTag(tag, XmSTRING_TAG_STRLEN);
    
    /* Cleanup and validate resources. */
    CleanupResources(rend, TRUE);
    ValidateTag(rend, XmS);

#ifdef SUN_CTL
    if ((_XmRendFont(rend) != NULL) &&
	((_XmRendFontType(rend) == XmFONT_IS_FONTSET) ||
	 (_XmRendFontType(rend) == XmFONT_IS_XOC)))
      LoadAwtFont(rend, display);
    else
#endif
      ValidateAndLoadFont(rend, display);
#ifdef SUN_TBR
    /*
     * We try to create TBR for this rendition no matter what so that
     * the TBR object will be available in any case for the locale with
     * a TBR locale module; if there is no TBR module for the current locale,
     * XmCreateXmTBR() will return "NULL".
     */
    _XmRendTBR(rend) = (XtPointer)XmCreateXmTBR();
#endif /*SUN_TBR*/
    return(rend);
}

/* Mrm create function for rendertables. */
/*ARGSUSED*/
Widget
_XmCreateRenderTable(Widget parent,
		     String name, /* unused */
		     ArgList arglist, /* unused */
		     Cardinal argcount)	/* unused */
{
  XmRenderTable 	newtable;
  _XmRenderTable	table;
  
  /* Malloc new table */
  table = (_XmRenderTable)XtMalloc(sizeof(_XmRenderTableRec));
  newtable = GetHandle(_XmRenderTable);
  SetPtr(newtable, table);
  _XmRTCount(newtable) = 0;
  _XmRTDisplay(newtable) = XtDisplay(parent);
  
  return((Widget)newtable);
}

/* Mrm create function for renditions. */
Widget
_XmCreateRendition(Widget parent,
		   String name,
		   ArgList arglist,
		   Cardinal argcount)
{
    XmRenderTable	rt = (XmRenderTable)parent;
    _XmRenderTable	table;
    XmRendition		rend;
  
    table = GetPtr(rt);
  
    rend = _XmRenditionCreate(_XmRTDisplay(rt), NULL, XmS, XmCRenderTable,
			      name, arglist, argcount, NULL);
  
    /* Ignore repeats */
    if (_XmRenderTableFindRendition(rt, _XmRendTag(rend),
				    TRUE, FALSE, FALSE, NULL) 
	!= NULL)
    {
	FreeRendition(rend);
	return((Widget)NULL);
    }
    
    table = (_XmRenderTable) XtRealloc((char *)table,
				       sizeof(_XmRenderTableRec) +
				       (sizeof(XmRendition) * ((_XmRTCount(rt) + 1) - RENDITIONS_IN_STRUCT)));
    SetPtr(rt, table);
    
    /* Copy new rendition. */
    _XmRTRenditions(rt)[_XmRTCount(rt)] = CopyRendition(rend);
    _XmRTCount(rt)++;
    
    return((Widget)rend);
}


/* Free a rendition handle and decrement the _XmRenditionRec's */
/* ref count. If the ref count goes to 0, then free the        */
/* _XmRenditionRec as well.                                    */

static void
FreeRendition(XmRendition rendition)
{
   if (rendition)
    {
      if (_XmRendRefcountDec(rendition) == 0)
       {
         if (NameIsString(_XmRendFontName(rendition)))
          {
            XtFree(_XmRendFontName(rendition));
          }
         if (ListIsList(_XmRendTabs(rendition)))
          {
            XmTabListFree(_XmRendTabs(rendition));
          }
         if (_XmRendTagCount(rendition) != 0)
          {
            XtFree((char *)_XmRendTags(rendition));
          }
#ifdef SUN_CTL
         if (_XmRendLayoutIsCTL(rendition))
          {
            XtPointer   font=(XtPointer)_XmRendXOC(rendition);
	    XtPointer	font_ret;
            font_ret = (XtPointer)XmDestroyXmXOC((XOC)_XmRendFont(rendition));
            _XmRendFont(rendition) = font_ret;
          }
         if (_XmRendLayoutModifier(rendition))
          {
            XtFree((char*)_XmRendLayoutModifier(rendition));
          }
#endif /* CTL */
#ifdef SUN_TBR
	 if (_XmRendIsTBR(rendition))
	  {
	    XmDestroyXmTBR((XmTBR)_XmRendTBR(rendition)); 
	     _XmRendTBR(rendition)= (XtPointer)NULL;
	  }
#endif /*SUN_TBR*/
         if (_XmRendFont(rendition) &&
             ((_XmRendFontType(rendition) != XmFONT_IS_FONT) &&
              (_XmRendFontType(rendition) != XmFONT_IS_FONTSET)
             )
            )
          {
            XtFree(_XmRendFont(rendition));
          }
         _XmRendFont(rendition) = 0;
         XtFree((char *)GetPtr(rendition));
       }
      FreeHandle(rendition);
    }
}


void
XmRenditionFree(XmRendition rendition)
{
  XtAppContext app;

  if (rendition == NULL) return;

  _XmProcessLock();
  FreeRendition(rendition);
  _XmProcessUnlock();
}

/* Get resource values from rendition. */
void
XmRenditionRetrieve(XmRendition rendition,
		    ArgList arglist,
		    Cardinal argcount)
{
  int			i, j;
  Arg			*arg;
  XtResource		*res;
  char			*as_is = (char *)XmAS_IS;
  
  if (rendition == NULL) return;

  _XmProcessLock();
  /* Get resources */
  for (i = 0; i < argcount; i++)
    {
      arg = &(arglist[i]);
      
      for (j = 0; j < _XmNumRenditionResources; j++)
	{
	  res = &(_XmRenditionResources[j]);

	  if (strcmp(res->resource_name, arg->name) == 0)
	    {
	      /* CR 7890: Font hook - if there's a fontName but the 
	      ** font hasn't been fetched yet, now's a good time to 
	      ** get it - if the caller wants to use the font to, say, 
	      ** compute font metrics for layout (as CSText does), it won't
	      ** like to get NULL back
	      */
	      if (strcmp(res->resource_name, XmNfont) == 0)
		{
		  if ((_XmRendFont(rendition) == NULL) &&
		      (_XmRendFontName(rendition) != NULL))
		    {
		      if (_XmRendLoadModel(rendition) == XmLOAD_DEFERRED)
			_XmRendLoadModel(rendition) = XmLOAD_IMMEDIATE;
		      ValidateAndLoadFont(rendition, _XmRendDisplay(rendition));
		    }
		  if (_XmRendFont(rendition) == NULL)
		    CopyToArg((char*)&as_is, &(arg->value), sizeof(char*));
		  else CopyToArg(((char *)GetPtr(rendition) + 
				  res->resource_offset),
				 &(arg->value),
				 res->resource_size);
		}
	      else if (((strcmp(res->resource_name, XmNfontName) == 0) &&
		       (_XmRendFontName(rendition) == NULL)) ||
		       ((strcmp(res->resource_name, XmNtabList) == 0) &&
		       (_XmRendTabs(rendition) == NULL)))
		CopyToArg((char*)&as_is, &(arg->value), sizeof(char*));
	      else CopyToArg(((char *)GetPtr(rendition) + res->resource_offset),
			     &(arg->value),
			     res->resource_size);
	      break;
	    }
	}
    }
    _XmProcessUnlock();
}


/************************************************************************
*                                                                       *
*  XmRenditionUpdate                                                    *
*     Set new resources in a rendition. Renew rendition if              *
*     necessary, then update resources.                                 *
*                                                                       *
************************************************************************/
void
XmRenditionUpdate(XmRendition rendition, ArgList arglist, Cardinal argcount)
{
   XmStringTag    oldtag;
   char *         oldname;
   XtPointer      oldfont;
   XmTabList      oldtabs;
   int            i, j;
   XtResource *   res;
   Arg *          arg;
   Display *      display=_XmGetDefaultDisplay();
   Boolean        can_free;
   XtAppContext   app=NULL;

   if (rendition == NULL)
      return;

#ifdef XTHREADS
   if (_XmRendDisplay(rendition))
    {
      app = XtDisplayToApplicationContext(_XmRendDisplay(rendition));
      _XmAppLock(app);
    }
   if (_XmRendDisplay(rendition) && (_XmRendDisplay(rendition) != display))
    {
      display = _XmRendDisplay(rendition);
    }
#endif
   /* Save old values to check for dependencies and free memory. */
   oldtag = _XmRendTag(rendition);
   oldname = _XmRendFontName(rendition);
   oldfont = _XmRendFont(rendition);
   oldtabs = _XmRendTabs(rendition);
   can_free = TRUE;

   /* New memory if needed. */
   if (_XmRendRefcount(rendition) > 1)
    {
      _XmRendRefcountDec(rendition);
      RenewRendition(rendition);
      can_free = FALSE;
    }

   for (i=0; i<argcount; i++)
    {
      arg = &(arglist[i]);

      for (j=0; j<_XmNumRenditionResources; j++)
       {
         res = &(_XmRenditionResources[j]);

         if (strcmp(res->resource_name, arg->name) == 0)
          {
            CopyFromArg((arg->value),
                     ((char *)GetPtr(rendition) + res->resource_offset),
                        res->resource_size);
            break;
          }
       }
    }

   CopyInto(rendition, rendition);
   if (!can_free)
    {
      _XmRendFont(rendition) = NULL;
#ifdef	SUN_TBR
      /*
       * Once the "rendition" is renewed by using RenewRendition() at above,
       * it is a "clone" that need to have a separate everything including
       * the TBR object. By setting the "rendition->XmTBRObject" to NULL
       * here, we get a new TBR object for this renewed "rendition" at
       * below in this function. Otherwise, the cloned rendition will possibly
       * have and point to a freed TBR object handle and thus a segmentation
       * fault can happen.
       *
       * If there was no "renew", we don't need to get a new TBR object
       * handle by the way.
       */
      _XmRendTBR(rendition) = (XtPointer)NULL;
#endif	/* SUN_TBR */
    }


   /** Validate resources **/

   /* CR 7890 - handle cases of fontName == NULL and fontName == XmAS_IS */

   /* If fontName changed but not font, NULL font so it's updated. 
   ** (first make sure we won't crash on the strcmp) */

   if (NameIsString(oldname) && NameIsString(_XmRendFontName(rendition)))
    {
      if (strcmp(oldname, _XmRendFontName(rendition)) != 0)
       {
         if (oldfont == _XmRendFont(rendition))
            _XmRendFont(rendition) = NULL;
       }
      if (can_free)
         XtFree(oldname);
    }

   /* Also handle the case where we started with a NULL fontName and
   ** had a real fontName specified */
   else if ((oldname == NULL) && NameIsString(_XmRendFontName(rendition)))
    {
      if (oldfont == _XmRendFontName(rendition))
         _XmRendFont(rendition) = NULL;
    }

   if (_XmRendFont(rendition) == (XtPointer)XmAS_IS)
      _XmRendFont(rendition) = NULL;

   if ((oldtabs != _XmRendTabs(rendition)) && can_free)
      XmTabListFree(oldtabs);

   ValidateTag(rendition, oldtag);

   ValidateAndLoadFont(rendition, display);

#ifdef SUN_CTL
   if (CTL_FONTTYPE(rendition) && _XmRendLayoutIsCTL(rendition))
    {
      if (_XmRendLayoutModifier(rendition))
         XSetOCValues((XOC)_XmRendFont(rendition), XNLayoutModifier,
                       _XmRendLayoutModifier(rendition), NULL);
      _XmRendLayoutModifier(rendition) = ((XmXOC)_XmRendFont(rendition))->layout_modifier;
    }
#endif /* CTL */

#ifdef SUN_TBR
    /* Was this rendition renewed at RenewRendition() at above? */
    if (_XmRendTBR(rendition) == (XtPointer)NULL) {
      _XmRendTBR(rendition) = (XtPointer)XmCreateXmTBR();
    }
#endif /*SUN_TBR*/

#ifdef XTHREADS
   if (app)
      _XmAppUnlock(app);
#endif
}


/*****************************************************************************/
/* XmRenderTableCvtToProp takes a rendertable and converts it to             */
/* an ascii string in the following format:				     */
/* tag : char*								     */
/* font : either fontid (integer) or [ fontid, fontid ... fontid ] or -1     */
/* tablist : [ tab1, ... tabn ] or -1					     */
/* background : pixel or -1						     */
/* foreground : pixel or -1						     */
/* underlineType : integer (from enum in Xm.h ) or -1			     */
/* strikethruType : integer (from enum in Xm.h ) or -1			     */
/* 									     */
/* example:								     */
/* "tag, font, tablist, background, foreground, underlineType, 		     */
/*  strikethruType\n							     */
/* bold, 10000031, -1, -1, -1, -1, -1\n					     */
/* underline, 10000029, -1, -1, -1, -1, -1\n				     */
/* default, 10000029, [ 1.234 1 0 0, 2.43 2 0 2], 1, 2, 0, 0\n		     */
/* japanese, [10000029, 10000030], -1, -1, -1, -1, -1"			     */
/* 									     */
/* The first line gives a complete list of the attributes by name.	     */
/* on the destination side,  attributes which are not understood	     */
/* or are outdated can be ignored.  The conversion of each rendition	     */
/* passes a single "line" which contains the fields in order.		     */
/*****************************************************************************/

/* Note that this MUST be in the same order as the output conversion
   below!! */
static XmConst char *CVTproperties[] = {
  XmNtag,
  XmNfont,
  XmNtabList,
  XmNbackground,
  XmNforeground,
  XmNunderlineType,
  XmNstrikethruType,
  NULL,
  };

/* Must be big enough to take all the above strings concatenated with
   commas separating them */
static char CVTtransfervector[256];
static int CVTtvinited = 0;

/* Use this macro to encapsulate the code that extends the output
   buffer as needed */
#define CVTaddString(dest, src, srcsize)\
{\
   if ((chars_used + srcsize) > allocated_size) {\
     allocated_size *= 2;\
     buffer = XtRealloc(buffer, allocated_size);\
   }\
   strcat(buffer, src);\
   chars_used += srcsize;\
}

/*ARGSUSED*/
unsigned int
XmRenderTableCvtToProp(Widget widget, /* unused */
		       XmRenderTable table,
		       char **prop_return)
{
  int i;
  int allocated_size = 256;
  size_t chars_used = 0, size; /* Wyoming 64-bit fix */ 
  char *buffer;
  char *str;
  XmRendition rendition;
  _XmWidgetToAppContext(widget);

  _XmAppLock(app);
  buffer = XtMalloc(allocated_size);

  _XmProcessLock();
  if (CVTtvinited == 0) {
    CVTtvinited = 1;
    strcpy(CVTtransfervector, "");
    for(i = 0; CVTproperties[i] != NULL; i++) {
      strcat(CVTtransfervector, CVTproperties[i]);
      strcat(CVTtransfervector, ",");
    }
    strcat(CVTtransfervector, "\n");
  }
  
  /* Copy the transfer vector into the output buffer. */
  strcpy(buffer, CVTtransfervector);
  chars_used = strlen(buffer);
  _XmProcessUnlock();

  /* Now iterate over the list of renditions */
  for(i = 0; i < _XmRTCount(table); i++) {
    char temp[2048];

    rendition = _XmRTRenditions(table)[i];
    sprintf(temp, "\"%s\", ", _XmRendTag(rendition));
    size = strlen(temp);
    CVTaddString(buffer, temp, size);

    if (_XmRendFontType(rendition) == XmAS_IS)
      str = "-1, ";
    else {
      sprintf(temp, "%d \"%s\" %d,", _XmRendFontType(rendition),
	      _XmRendFontName(rendition), _XmRendLoadModel(rendition));
      str = temp;
    }
    size = strlen(str);
    CVTaddString(buffer, str, size);

    if ((unsigned long)_XmRendTabs(rendition) == XmAS_IS || /* Wyoming 64-bit fix */ 
	_XmRendTabs(rendition) == NULL)
      str = "-1, ";
    else {
      _XmTab tab;
      _XmTabList tlist;
      int number;
      strcpy(temp, "[ ");
      tlist = (_XmTabList) _XmRendTabs(rendition);
      number = tlist -> count;
      tab = (_XmTab) tlist -> start;
      while(number > 0) {
	sprintf(temp, "%s %f %d %d %d, ", temp, tab -> value, 
		tab -> units, tab -> alignment, tab -> offsetModel);
	tab = (_XmTab) tab -> next;
	number--;
      }
      strcat(temp, " ], ");
      str = temp;
    }
    size = strlen(str);
    CVTaddString(buffer, str, size);

    if (_XmRendBG(rendition) == XmAS_IS)
      str = "-1, ";
    else {
      sprintf(temp, "%d, ", _XmRendBG(rendition));
      str = temp;
    }
    size = strlen(str);
    CVTaddString(buffer, str, size);

    if (_XmRendFG(rendition) == XmAS_IS)
      str = "-1, ";
    else {
      sprintf(temp, "%d, ", _XmRendFG(rendition));
      str = temp;
    }
    size = strlen(str);
    CVTaddString(buffer, str, size);

    if (_XmRendUnderlineType(rendition) == XmAS_IS)
      str = "-1, ";
    else {
      sprintf(temp, "%d, ", _XmRendUnderlineType(rendition));
      str = temp;
    }
    size = strlen(str);
    CVTaddString(buffer, str, size);

    if (_XmRendStrikethruType(rendition) == XmAS_IS)
      str = "-1, ";
    else {
      sprintf(temp, "%d, ", _XmRendStrikethruType(rendition));
      str = temp;
    }
    size = strlen(str);
    CVTaddString(buffer, str, size);
    CVTaddString(buffer, "\n", size);
  }

  /* Return the converted rendertable string */
  *prop_return = buffer;

  _XmAppUnlock(app);
  /* chars_used is always the size - the NULL terminator */
  return(chars_used + 1);
}

typedef enum {   T_NL, T_INT, T_FLOAT, T_SEP, 
		 T_OPEN, T_CLOSE, T_STR, T_EOF } TokenType;

typedef struct _TokenRec {
  TokenType	type;
  int		integer;
  float		real;
  char		*string;
} TokenRec, *Token;


#ifndef XTHREADS
static TokenRec reusetoken;
#endif

static Token
ReadToken(char *string, int *position)
{
#ifdef XTHREADS
  TokenRec reusetoken;
  Token new_token = &reusetoken;
#else
  Token new_token = &reusetoken;
#endif
  int pos = *position;
  int count;

  /* Skip whitespace but not newlines */
  while (isspace(string[pos]) && ! (string[pos] == '\n'))
    pos++;

  /* Select token type */
  switch(string[pos]) {
  case '\0':
    new_token -> type = T_EOF;
    break;
  case '\n': 
    new_token -> type = T_NL;
    pos++;
    break;
  case ',':
    new_token -> type = T_SEP;
    pos++;
    break;
  case '[':
    new_token -> type = T_OPEN;
    pos++;
    break;
  case ']':
    new_token -> type = T_CLOSE;
    pos++;
    break;
  case '"': /* String result */
    count = 1;
    while (string[pos + count] != '"' &&
	   string[pos + count] != '\0')
      count++; /* Scan for end of string */
    new_token -> type = T_STR;
    new_token -> string = NULL;
    count -= 1;
    if (count > 0) {
      new_token -> string = (char*) XtMalloc(count + 1);
      strncpy(new_token -> string, &string[pos + 1], count);
      pos += count + 2; /* Move past end quote */
      new_token -> string[count] = 0; /* Null terminate */
    }
    break;
  default:
    if (isalpha(string[pos])) /* String result */
      {
	char temp[80];
	int count;
	for(count = 0; 
	    isalpha(string[pos + count]) && count < 79;
	    count++) temp[count] = string[pos + count];
	temp[count] = 0;
	pos += count;
	new_token -> type = T_STR;
	new_token -> string = XtNewString(temp);
      }
    else
      {
	/* start converting a float number.  If it is exactly integer
	   then we return an int,  otherwise return a float */
	double result;
	int intresult;
	char *newpos;
	result=strtod(&(string[pos]), &newpos);
	intresult= (int) result;
	pos = newpos - string;
	if (((double) intresult) == result) /* Integer result */
	  {
	    new_token -> type = T_INT;
	    new_token -> integer = intresult;
	  }
	else
	  {
	    new_token -> type = T_FLOAT;
	    new_token -> real = (float) result;
	  }
      }
  }

  *position = pos;
  return(new_token);
}

/*ARGSUSED*/
XmRenderTable
XmRenderTableCvtFromProp(Widget w, 
			 char *prop,
			 unsigned int len) /* unused */
{
  XmRenderTable new_rt;
  XmRendition rendition;
  XmRendition *rarray;
  int rarray_count, rarray_max;
  /* These must both be big enough for the number of passed parameters */
  char *items[20];
  char *name;
  Arg args[20];
  /* This must be big enough to hold all the strings returned by
     readtoken */
  char *freelater[5];
  int scanpointer, j, count, freecount, i;
  Token token;
  _XmWidgetToAppContext(w);

  _XmAppLock(app);
  new_rt = NULL;
  scanpointer = 0;
  rarray_max = 10;
  rarray_count = 0;
  rarray = (XmRendition *) XtMalloc(sizeof(XmRendition) * rarray_max);
  name = "";

  for(j = 0; j < 20; j++) items[j] = NULL;
  /* Read the list of items */
  for(j = 0; j < 20; ) {
    token = ReadToken(prop, &scanpointer);
    if (token -> type == T_NL) break;
    if (token -> type == T_STR) {
      items[j] = token -> string;
      j++;
    }
  }
  
  j = -1;
  count = 0;
  freecount = 0;
  while(True) {
    token = ReadToken(prop, &scanpointer);
    /* We skip the separators */
    while(token -> type == T_SEP &&
	  token -> type != T_EOF)
      token = ReadToken(prop, &scanpointer);
    if (token -> type == T_EOF) goto finish;

    j++; /* Go to next item in items array */
	  
    if (items[j] == NULL) {
      /* End of line processing.  Scan for NewLine */
      while(token -> type != T_NL &&
	    token -> type != T_EOF)
	token = ReadToken(prop, &scanpointer);
      /* Store rendition */
      rendition = XmRenditionCreate(w, name, args, count);
      name = "";
      count = 0;
      /* Reset index into namelist */
      j = -1;
      /* Free temp strings returned by ReadToken */
      for(i = 0; i < freecount; i++) XtFree(freelater[i]);
      freecount = 0;
      /* Record rendition in array */
      if (rarray_count >= rarray_max) {
	/* Extend array if necessary */
	rarray_max += 10;
	rarray = (XmRendition *) XtRealloc((char*) rarray, 
					   sizeof(XmRendition) * rarray_max);
      }
      if (token -> type == T_EOF) goto finish;
      rarray[rarray_count] = rendition;
      rarray_count++;
    } else if (strcmp(items[j], XmNtag) == 0) {
      /* Next item should be a string with the name of the new
	 rendition to create */
      if (token -> type == T_STR) {
	name = token -> string;
	freelater[freecount] = token -> string; freecount++;
      } else {
	goto error;
      }
    } else if (strcmp(items[j], XmNfont) == 0) {
      /* If the next item is a number then we have a font
	 id,  otherwise we are reading in a fontset */
      if (token -> type != T_INT) goto error;
      if (token -> integer != -1) { /* AS IS */ 
	XtSetArg(args[count], XmNfontType, token -> integer); count++;
	token = ReadToken(prop, &scanpointer);
	if (token -> type != T_STR) goto error;
	XtSetArg(args[count], XmNfontName, token -> string); count++;
	freelater[freecount] = token -> string; freecount++;
	token = ReadToken(prop, &scanpointer);
	if (token -> type != T_INT) goto error;
	XtSetArg(args[count], XmNloadModel, token -> integer); count++;
      }
    } else if (strcmp(items[j], XmNtabList) == 0) {
      /* This starts with an OPEN then a number of
	 FLOAT INT INT INT then CLOSE and SEP */
      if (token -> type == T_INT) { /* Should be AS IS */
	if (token -> integer != -1) goto error;
      } else if (token -> type == T_OPEN) {
	float value;
	int units, align;
	XmOffsetModel model;
	XmTabList tablist;
	XmTab tabs[1];

	tablist = NULL;
	token = ReadToken(prop, &scanpointer);
	while(token -> type != T_CLOSE) {
	  if (token -> type != T_FLOAT &&
	      token -> type != T_INT) goto error;
	  if (token -> type == T_FLOAT)
	    value = token -> real;
	  else
	    value = (float) token -> integer;
	  token = ReadToken(prop, &scanpointer);
	  if (token -> type != T_INT) goto error;
	  units = token -> integer;
	  token = ReadToken(prop, &scanpointer);
	  if (token -> type != T_INT) goto error;
	  align = token -> integer;
	  token = ReadToken(prop, &scanpointer);
	  if (token -> type != T_INT) goto error;
	  model = (XmOffsetModel) token -> integer;
	  tabs[0] = XmTabCreate(value, units, model, align, NULL);
	  tablist = XmTabListInsertTabs(tablist, tabs, 1, 1000);
	  XtFree((char*) tabs[0]);
	  /* Go to next separator to skip unknown future values */
	  while(token -> type != T_SEP) 
	    token = ReadToken(prop, &scanpointer);
	  if (token -> type == T_SEP)
	    token = ReadToken(prop, &scanpointer);
	}
	XtSetArg(args[count], XmNtabList, tablist); count++;
      } else 
	goto error;
    } else if (strcmp(items[j], XmNbackground) == 0) {
      if (token -> type != T_INT) goto error;
      if (token -> type != -1) {
	XtSetArg(args[count], XmNrenditionBackground, token -> integer); count++;
      }
    } else if (strcmp(items[j], XmNforeground) == 0) {
      if (token -> type != T_INT) goto error;
      if (token -> type != -1) {
	XtSetArg(args[count], XmNrenditionForeground, token -> integer); count++;
      }
    } else if (strcmp(items[j], XmNunderlineType) == 0) {
      if (token -> type != T_INT) goto error;
      if (token -> type != -1) {
	XtSetArg(args[count], XmNunderlineType, token -> integer); count++;
      }
    } else if (strcmp(items[j], XmNstrikethruType) == 0) {
      if (token -> type != T_INT) goto error;
      if (token -> type != -1) {
	XtSetArg(args[count], XmNstrikethruType, token -> integer); count++;
      }
    }
  }

 finish:
  new_rt = XmRenderTableAddRenditions(new_rt, rarray, rarray_count, XmMERGE_REPLACE);
  for (i = 0; i < rarray_count; i++) XmRenditionFree(rarray[i]);
  _XmAppUnlock(app);
  return(new_rt);

 error:
  /* Free temp strings returned by ReadToken */
  for(i = 0; i < freecount; i++) XtFree((char*) freelater[i]);
  freecount = 0;
  goto finish;
}

#ifdef SUN_CTL
/*********************************************************************************
*  All of the new CTL internal functions are below.
*********************************************************************************/

/* Need to XmStackAlloc for real implementation */
#define MAX_ARRAY_SIZE		CTL_MAX_BUF_SIZE
#define MAX_LINE_HIGHLIGHTS	256
#define Half(x) 		(x >> 1)

/* The following macros have "value" parameters */
#define GET_NEXT_TAB_STOP(offset, tab_width) \
	((offset)/(tab_width) * (tab_width) + (tab_width))
	       
/* Let the compiler do the dirty work of optimization */
#define GET_NEXT_SEGMENT(text, max_text_length, is_wchar, i_o_counter) \
	while (*i_o_counter < max_text_length && !IS_TAB(STR_IPTR(text,	*i_o_counter, is_wchar), is_wchar)) \
		(*i_o_counter)++;

static void
GetClipRect(Widget w, XRectangle *rect)
{
    Dimension w_margin_width, w_margin_height, w_shadow_thickness; 
    Dimension w_highlight_thickness, w_width, w_height;
    Dimension margin_width, margin_height;
    
    XtVaGetValues(w, XmNmarginWidth, &w_margin_width, 
		  XmNmarginHeight, &w_margin_height,
		  XmNshadowThickness, &w_shadow_thickness,
		  XmNhighlightThickness, &w_highlight_thickness,
		  XmNwidth, &w_width,
		  XmNheight, &w_height,
		  NULL);
    
    margin_width = w_margin_width + w_shadow_thickness + w_highlight_thickness;
    margin_height = w_margin_height + w_shadow_thickness + w_highlight_thickness;
    
    if (margin_width < w_width)
	rect->x = margin_width;
    else
	rect->x = w_width;
    
    if (margin_height < w_height)
	rect->y = margin_height;
    else
	rect->y = w_height;
    
    if ((int) (2 * margin_width) < (int) w_width)
	rect->width = (int) w_width - (2 * margin_width);
    else
	rect->width = 0;
    
    if ((int) (2 * margin_height) < (int) w_height)
	rect->height = (int) w_height - (2 * margin_height);
    else
	rect->height = 0;
}

static void 
SetMarginGC(Widget w, GC gc)
{
    XRectangle clipRect;
    
    GetClipRect(w, &clipRect);
    XSetClipRectangles(XtDisplay(w), gc, 0, 0, &clipRect, 1, Unsorted);
}

static void 
SetNormGC(Widget w, GC gc)
{
    unsigned long valueMask = (GCForeground | GCBackground | GCFunction);
    Pixel         fg, bg;
    XGCValues     values;
    
    XtVaGetValues(w, XmNbackground, &bg, XmNforeground, &fg, NULL);
    values.function   = GXcopy;
    values.background = bg;
    values.foreground = fg;
    XChangeGC(XtDisplay(w), gc, valueMask, &values);
}

static void 
SetReverseVideoGC(Widget w, GC gc)
{
    unsigned long valueMask = (GCForeground | GCBackground | GCFunction);
    Pixel         fg, bg;
    XGCValues     values;
    
    XtVaGetValues(w, XmNbackground, &bg, XmNforeground, &fg, NULL);
    values.function   = GXxor;
    values.foreground = bg ^ fg;
    values.background = 0;
    XChangeGC(XtDisplay(w), gc, valueMask, &values);
}

static void 
SetInvGC(Widget w, GC gc)
{
    unsigned long valueMask = (GCForeground | GCBackground);
    Pixel         fg, bg;
    XGCValues     values;
    
    XtVaGetValues(w, XmNbackground, &bg, XmNforeground, &fg, NULL);
    values.background = fg;
    values.foreground = bg;
    XChangeGC(XtDisplay(w), gc, valueMask, &values);
}


/*****************************************************************
 * Description Gets the Escapement of the text segment		 *
 * Inputs      text segment, text segment length, wide char flag *
 * Output      Escapement of the segment			 *
 *****************************************************************/
static Dimension 
CTLRenditionSegEscapement(XmRendition	rend, 
			  char		*seg, 
			  int		seg_len,
			  Boolean	is_wchar)
{
    Dimension   seg_escapement;
    XFontStruct *f = _XmRendFont(rend);
    
    switch (_XmRendFontType(rend)) {
	case XmFONT_IS_XOC :
	case XmFONT_IS_FONTSET :
	    if (is_wchar)
		seg_escapement = XwcTextEscapement((XFontSet)f, (wchar_t*)seg, seg_len);
	    else
		seg_escapement = XmbTextEscapement((XFontSet)f, (char*)seg, seg_len);
	    break;
	case XmFONT_IS_FONT :
	    if (is_wchar)
		XmeWarning(NULL, "Error::CTLRenditionSegEscapement NYI for wchar\n");
	    else
		seg_escapement =  XTextWidth(f, seg, seg_len);
	    break;
    }
    return seg_escapement;
}


/************************************************************************
*                                                                       *
*  Description                                                          *
*     Gets the Segment information of the segments in the given text.   *
*     The segments are separated by the tabs. In addition, account is   *
*     taken of the highlighting data, so that the segments may in       *
*     turn be sub-divided if the highlighting characteristics change    *
*     within a segment.                                                 *
*   Inputs                                                              *
*     Rendition, xoffset (to find the tab stops), text, text_length,    *
*     is_wchar, and tab_width                                           *
*   Outputs (not necessarily in order)                                  *
*     Number of segments, array of segwidths, array of x starting       *
*     positions of each segment, array of char lengths of each segment, *
*     array of highlighting data for each seg, totalblocklength.        *
*                                                                       *
*     Any of the output params could be NULL                            *
*                                                                       *
*   Note1:                                                              *
*     Allocates memory for all 4 output arrays. The caller has to       *
*     ensure that this memory is freed.                                 *
*   Note2:                                                              *
*     "length" always indicates the length in pixels and "len"          *
*     indicates length in characters.                                   *
*     "width" is in pixels.                                             *
*                                                                       *
************************************************************************/
static void
CTLRenditionSegInfo( XmRendition          rend,
                     char *               text,
                     int                  text_len,
                     Boolean              is_wchar,
                     Dimension            tab_width,     /* tab width in pixels              */
                     _XmHighlightData *   hl_data,
                     Boolean              istext_rtaligned,
                     Dimension **         seg_widths,    /* segment widths in pixels         */
                     int *                total_num_segs,
                     Position **          starting_xpos, /* starting seg positions in pixels */
                     Dimension *          block_length,  /* complete text length in pixels   */
                     ushort **            seg_lens,      /* segment widths in chars          */
                     u_char **            seg_hilites,   /* segment highlighting modes       */
                     XSegment **          hl_areas,      /* Used to return hl areas.         */
                     u_char **            hl_area_modes, /* segment highlighting modes       */
                     int *                nbr_hls
                   )
{
   int               num_segs=0, curr_seg_num=0,
                     hl_index;
   int               i, lasti;
   int               text_len_left=text_len;
   int               nbr_hl_areas=0;
   Dimension         seg_escapement;
   ushort            next_hl_break_posn;
   ushort            first_hl_break_posn;
   Position          xoffset=0, layout_offset, ctl_correction;
   XmHighlightMode   hl_mode=XmHIGHLIGHT_NORMAL;
   Position *        starting_xpos_arr=NULL;    /* These are local copies for the   */
   Dimension *       seg_widths_arr=NULL;       /* outputs and will be copied to    */
   Dimension         overall_length=0;          /* the output params at the end.    */
   ushort *          seg_lens_arr=NULL;
   u_char *          seg_hl_arr=NULL;

   ushort            curr_seg_len;
   char *            seg_start_posn;
   ushort            layout_segs[MAX_ARRAY_SIZE];  /* These are the starting positions */
                                                   /* of the non-tab segments, without */
                                                   /* regard to the highlighting       */
                                                   /* breaks that may occur.           */
   XSegment          logical_segts[MAX_ARRAY_SIZE];
   XSegment *        hl_rects_arr=0,
            *        xrp;
   u_char *          hl_modes_arr;
   int               hl_rects_index=0;
   int               seg_index;
   Boolean           font_or_fontset;
   XSegment          overall_logical_seg;
   u_char            is_tab, was_tab, update_xoffset=TRUE;


   if (text_len < 0) return;

   font_or_fontset = CTL_FONTTYPE(rend) && !_XmRendLayoutIsCTL(rend);

   first_hl_break_posn = -1,                    /* Set the max possible changeover  */
   first_hl_break_posn >>= 1;                   /* for case where there is only one */
                                                /* highlighting for the whole line. */
   if (hl_data)
    {
      nbr_hl_areas = hl_data->number;
      if (nbr_hl_areas > 1)
       {
         first_hl_break_posn = hl_data->list[1].position;
       }
      if (nbr_hl_areas)
       {
         hl_mode = hl_data->list[0].mode;
       }
    }

   next_hl_break_posn = first_hl_break_posn;
   seg_index = 0, was_tab = TRUE;
   for (i=0, hl_index=0; i<text_len; )          /* Loop 1st to find the nbr of segments.  */
    {
      is_tab = FALSE;
      if (IS_TAB(STR_IPTR(text, i, is_wchar), is_wchar))
       {
         is_tab = TRUE;
         layout_segs[seg_index++] = i;          /* Record tabs as layout segments too.    */
         i++;
       }
      else
       {
         if (was_tab)
          {
            layout_segs[seg_index++] = i;
          }
         while((i < text_len) && (i < next_hl_break_posn))
          {
            if (IS_TAB(STR_IPTR(text, i, is_wchar), is_wchar))
             {
               break;
             }
            i++;
          }
       }
      if (i == next_hl_break_posn)
       {
         hl_index++;
         if (hl_index == (nbr_hl_areas-1))
          {
            next_hl_break_posn = -1;
            next_hl_break_posn >>= 1;
          }
         else
          {
            next_hl_break_posn = hl_data->list[hl_index+1].position;
          }
       }
      was_tab = is_tab;
      num_segs++;
    }
   layout_segs[seg_index] = text_len;           /* (Inserted here only as a marker.)   */


   /* Allocate memory for the arrays (if needed)   */

   if (starting_xpos)
    {
      starting_xpos_arr = (Position*)XtMalloc(num_segs * sizeof(Position));
    }
   if (seg_widths)
    {
      seg_widths_arr = (Dimension*)XtMalloc(num_segs * sizeof(Dimension));
    }
   if (seg_lens)
    {
      seg_lens_arr = (ushort*)XtMalloc(num_segs * sizeof(ushort));
    }
   if (seg_hilites)
    {
      seg_hl_arr = (u_char*)XtMalloc(num_segs);
    }
   if (hl_areas)
    {
      hl_rects_arr = (XSegment*)XtMalloc(num_segs * sizeof(XSegment) * 3);
      hl_modes_arr = (u_char *)XtMalloc(num_segs * 3);
      xrp = hl_rects_arr;
    }

   if ((seg_widths && !seg_widths_arr)       ||
       (starting_xpos && !starting_xpos_arr) ||
       (seg_lens && !seg_lens_arr)           ||
       (seg_hilites && !seg_hl_arr)          ||
       (hl_areas && (!hl_rects_arr || !hl_modes_arr))
      )
    {
      XmeWarning(NULL, "Error::CTLRenditionSegInfo():No Memory\n");
      return;
    }

   next_hl_break_posn = first_hl_break_posn;
   was_tab = TRUE, seg_index = 0;
   for (i=0, lasti=0, curr_seg_num=0, hl_index=0; i<text_len; curr_seg_num++)
    {                                        /* Loop again to process each segment.    */
      is_tab = FALSE;
      ctl_correction = 0;

      if (IS_TAB(STR_IPTR(text, i, is_wchar), is_wchar))
       {
         is_tab = TRUE;
         seg_index++;
         if (!was_tab)
          {
            seg_index++;
          }
         i++;
       }
      else
       {
         while((i < text_len) && (i < next_hl_break_posn))
          {
            if (IS_TAB(STR_IPTR(text, i, is_wchar), is_wchar))
             {
               break;
             }
            i++;
          }
       }

      if (i == next_hl_break_posn)
       {
         hl_index++;
         if (hl_index == (nbr_hl_areas-1))
          {
            next_hl_break_posn = -1;
            next_hl_break_posn >>= 1;
          }
         else
          {
            next_hl_break_posn = hl_data->list[hl_index+1].position;
          }
       }

      curr_seg_len = i - lasti;
      if (is_tab)
       {
         layout_offset = xoffset;
         seg_escapement = GET_NEXT_TAB_STOP(xoffset, tab_width) - xoffset;
         if (hl_mode == XmHIGHLIGHT_SELECTED)
          {
            hl_modes_arr[hl_rects_index] = hl_mode;
            hl_rects_arr[hl_rects_index].x1 = xoffset;
            hl_rects_arr[hl_rects_index].x2 = xoffset + seg_escapement;
            hl_rects_index++;
          }
       }
      else if (font_or_fontset)
       {
         layout_offset = xoffset;
         seg_start_posn = is_wchar ?   (char*)((wchar_t*)text + lasti) :
                                       text + lasti;
         seg_escapement =  CTLRenditionSegEscapement(rend, seg_start_posn,
                                                     curr_seg_len, is_wchar);
       }

      /* That's taken care of for a visual layout. If the layout is logical,  */
      /* however, then the xoffset & width of highlighted areas may have to   */
      /* be adjusted because of active and/or right-  to-left layout.         */
      /* This is what the next section does.                                  */

      else
       {
         int   j=lasti-layout_segs[seg_index];  /* Index into "logical_segts" array to */
                                                /* get values for current char.        */
         int   esc_left,                        /* Used to mark R&L edges of current   */
               esc_right;                       /* seg, so that escapement can be      */
                                                /* calculated.                         */
         int   ii, jj;

         if (was_tab)                           /* If this is a new layout segment,    */
          {                                     /* calculate its char extents.         */
            int   seg_width=layout_segs[seg_index+1] - layout_segs[seg_index];
            int   num_chars_return;

            CTLRenditionSegPerCharExtents(rend, STR_IPTR(text, lasti, is_wchar),
                                          seg_width, is_wchar, logical_segts,
                                          seg_width, (XtPointer)&overall_logical_seg,
                                          &num_chars_return
                                         );
            layout_offset = xoffset;
          }

         if (logical_segts[j].x1 < logical_segts[j].x2)
          {
            esc_left = logical_segts[j].x1;
            esc_right = logical_segts[j].x2;
          }
         else
          {
            esc_left = logical_segts[j].x2;
            esc_right = logical_segts[j].x1;
          }

         for (ii=0; ii<curr_seg_len; ii++, j++)
          {
            Position *  pp1,                 /* Current rectangle's leftmost  */
                     *  pp2;                 /* & rightmost values, resp.     */

            if (logical_segts[j].x1 < logical_segts[j].x2)
             {
               pp1 = &logical_segts[j].x1;
               pp2 = &logical_segts[j].x2;
             }
            else
             {
               pp2 = &logical_segts[j].x1;
               pp1 = &logical_segts[j].x2;
             }

            if (esc_left > *pp1)    esc_left = *pp1;
            if (esc_right < *pp2)   esc_right = *pp2;

            *pp1 += layout_offset;           /* Convert these into rectangles */
            *pp2 += layout_offset;           /* relative to the whole line's  */
                                             /* layout.                       */

            if (xrp &&
                ((hl_mode == XmHIGHLIGHT_SELECTED) ||
                 (hl_mode == XmHIGHLIGHT_SECONDARY_SELECTED)
                )
               )
             {
               for (jj=0, xrp=hl_rects_arr; jj<hl_rects_index; jj++, xrp++)
                {
                  if ((*pp1 == xrp->x2) &&   /* If new rect is right adjacent */
                      (hl_modes_arr[jj] == hl_mode)
                     )
                   {                         /* to an existing rect, then     */
                     xrp->x2 = *pp2;         /* extend existing one.          */
                     break;
                   }
                  else if ((*pp2 == xrp->x1) &&
                           (hl_modes_arr[jj] == hl_mode)
                          )
                   {                         /* Likewise for left adjacency.  */
                     xrp->x1 = *pp1;
                     break;
                   }
                }
               if (jj == hl_rects_index)     /* (Case where no adjacency      */
                {                            /*  found.)                      */
                  hl_rects_index++;
                  xrp->x1 = *pp1;
                  xrp->x2 = *pp2;
                  hl_modes_arr[jj] = hl_mode;
                }
             }
          }
         ctl_correction = esc_left;
         seg_escapement = esc_right - esc_left;
       }

      if (seg_widths)      seg_widths_arr[curr_seg_num] = seg_escapement;
      if (seg_lens)        seg_lens_arr[curr_seg_num] = curr_seg_len;
      if (starting_xpos)   starting_xpos_arr[curr_seg_num] = layout_offset + ctl_correction;
      if (seg_hilites)     seg_hl_arr[curr_seg_num] = hl_mode,
                           hl_mode = hl_data->list[hl_index].mode;

      xoffset += seg_escapement;
      overall_length += seg_escapement;
      lasti = i;
      was_tab = is_tab;
    }


   /* That's it. Now simply set up the output parameters.   */

   if (seg_widths)      *seg_widths = seg_widths_arr;
   if (seg_lens)        *seg_lens = seg_lens_arr;
   if (starting_xpos)   *starting_xpos = starting_xpos_arr;
   if (seg_hilites)     *seg_hilites = seg_hl_arr;
   if (total_num_segs)  *total_num_segs = num_segs;
   if (block_length)    *block_length = overall_length;
   if (hl_areas)        *hl_areas = hl_rects_arr,
                        *hl_area_modes = hl_modes_arr,
                        *nbr_hls = hl_rects_index;
}


/*******************************************************************************
 * Description	: Return the Escapement of text passed. Handles tabs also.
 * Inputs	: Rendition, text
 * Output	: Escapement of the text in pixels
 * Algo		: While more segments {
 *		    Get the next segment
 *		      if tab segment 
 *			compute tabsegment pixel length based on next tab stop
 *		      else compute the text segment
 *		    update the total segment escapement
 *  		  }
 * Note1	: Can be done by a call to CTLRenditionSegInfo. But this
 * 		  implementation is more effeicient.
 ********************************************************************************/
Dimension 
_XmRenditionEscapement(XmRendition    rend,
		       char          *text,
		       size_t         text_len,
		       Boolean        is_wchar,
		       Dimension      tab_width)
{
    Position      offset = 0;
    Dimension     ret_escapement = 0;
    
    if (text_len <= 0) return (Dimension)0;
    
    while (text_len > 0) {
	Dimension  seg_escapement;
	Dimension  new_offset;
	int        seg_len = 0;
	
	if (IS_TAB(text, is_wchar)) { /*Handle the tab segment */
	    new_offset     = GET_NEXT_TAB_STOP(offset, tab_width);
	    seg_escapement = new_offset - offset;
	    seg_len        = 1;
	}
	else { /* or handle the text segment */
	    GET_NEXT_SEGMENT(text, text_len, is_wchar, &seg_len);
	    seg_escapement = CTLRenditionSegEscapement(rend, text, seg_len, is_wchar);
	    new_offset     = offset + seg_escapement;
	}
	/* set the iteration params */
	offset          =  new_offset;
	text_len       -= seg_len;
	text            = STR_IPTR(text, seg_len, is_wchar);
	ret_escapement += seg_escapement;
    }
    return ret_escapement;
}

/**************************************************************************** 
 * Description
 * 	This function computes the Segment/Rectangle Per Char for the text
 * 	segment passed. Note: This can't handle the tab
 * Input : Rend, segment, segmentlength, flag of wide char
 * Output: logical array, overall array, number of characters for
 *         which rectangles/segments are calculated.
 * Algo  : Depending on the rendition type call the respective
 *         PerCharExtents funtions
 *****************************************************************************/
int 
CTLRenditionSegPerCharExtents(XmRendition    rend, 
			      char          *seg, 
			      int            seg_len, 
			      Boolean        is_wchar, 
			      XtPointer      logical_array,
			      int            array_size, 
			      XtPointer      overall_logical,
			      int           *num_chars_return)
{
    int status;
    
    if (seg_len <= 0) return 0;
    
    switch (_XmRendFontType(rend)) {
	case XmFONT_IS_XOC :
	case XmFONT_IS_FONTSET :
	    if (_XmRendLayoutIsCTL(rend)) {
		XOC      xoc = (XOC)_XmRendFont(rend);
		XSegment overall_ink;
		XSegment *ink_array, ink_array_cache[MAX_ARRAY_SIZE];
		XSegment dummy_overall_logical;
		
		ink_array = (XSegment*)XmStackAlloc((array_size * sizeof(XSegment)), ink_array_cache);

		if (!overall_logical)
		    overall_logical = &dummy_overall_logical;
		
		memset((char*)logical_array, 0, (sizeof(XSegment) * array_size));
		status = XocTextPerCharExtents(xoc, 
					       seg, 
					       is_wchar, 
					       seg_len,
					       ink_array, 
					       (XSegment*)logical_array, 
					       array_size, 
					       num_chars_return, 
					       &overall_ink,
					       (XSegment*)overall_logical);
	    }
	    else { /* Not a CTL Locale */
		XFontSet   fontset = (XFontSet)_XmRendFont(rend);
		XRectangle overall_ink;
		XRectangle *ink_array, ink_array_cache[MAX_ARRAY_SIZE];
		XRectangle dummy_overall_logical;
		
		ink_array = (XRectangle*)XmStackAlloc((array_size*sizeof(XRectangle)), ink_array_cache);
		memset((char*)logical_array, 0, (sizeof(XRectangle) * array_size));
		if (!overall_logical)
		    overall_logical = &dummy_overall_logical;
		if (is_wchar)
		    status = XwcTextPerCharExtents(fontset, 
						   (wchar_t*)seg, 
						   seg_len,
						   ink_array, 
						   (XRectangle*)logical_array, 
						   array_size, 
						   num_chars_return, 
						   &overall_ink, 
						   (XRectangle*)overall_logical);
		else
		    status = XmbTextPerCharExtents(fontset, 
						   seg, 
						   seg_len, 
						   ink_array, 
						   (XRectangle*)logical_array, 
						   array_size, 
						   num_chars_return, 
						   &overall_ink,
						   (XRectangle*)overall_logical);
	    }
	    break;
	case XmFONT_IS_FONT :
	    if (is_wchar)
		XmeWarning(NULL, "ERROR:CTLRenditionSegPerCharExtents() is NYI\n");
	    else {
		XRectangle overall_ink;
		XRectangle ink_array[MAX_ARRAY_SIZE]; 
		XRectangle dummy_overall_logical;
		
		if (!overall_logical)
		    overall_logical = &dummy_overall_logical;
		bzero ((void *)logical_array, sizeof(XRectangle) * array_size);
		status =  _XFontStructTextPerCharExtents(_XmRendFont(rend), 
							 seg, 
							 seg_len, 
							 ink_array,
							 (XRectangle*)logical_array, 
							 array_size, 
							 num_chars_return, 
							 &overall_ink, 
							 (XRectangle*)overall_logical);
	    }
	    break;
    }
    if (!status)
	XmeWarning(NULL, "Error::CTLRenditionSegPerCharExtents\n");
    return status;
}


/******************************************************************************
 * Description:
 *     This function computes the Segment/Rectangle Per Char for the text
 *     (the text may include tabs also) passed
 *     Input : Rend, text
 *     Output: logical array, overall array, number of characters for
 *             which rectangles/segments are calculated.
 *     Assumptions:
 *             i)  The logical_array is allocated and is of size array_size
 *             ii) non zero tabwidth
 *     Algo:
 *          Get Segment info i.e segment starting pos etc.
 *          while more segments {
 *              get next segment;
 *             if rend is font or fontset then
 *          1) allocate the logical rectangles
 *          2) get percharextents for the current segment using
 *             CTLRenditionSegPerCharExtents
 *              else
 *          1) allocate the logicalsegments
 *          2) get percharextents for the current segment using
 *             CTLRenditionSegPerCharExtents
 *              update percharextent of current segment w.r.t whole text
 *          }
 *****************************************************************************/
int
_XmRenditionTextPerCharExtents(  XmRendition rend,
                                 char *      string,
                                 int         string_len,
                                 Boolean     is_wchar,
                                 XtPointer   logical_array,
                                 int         array_size,
                                 int *       num_chars_return,
                                 Position    xoffset,
                                 Dimension   tab_width,
                                 Boolean     istext_rtaligned,
                                 XtPointer   overall_logical_return)
{
   int               font_ascent;
   int               line_height;
   int               font_descent;
   int               curr_seg_num=0;
   int               num_segs=0;
   int               status=SUCCESS;
   Position *        starting_xpos=NULL;
   Dimension         block_length=0;         /* pixel length of the complete text */
   Dimension *       seg_widths=NULL;
   XFontSetExtents * fs_extents=NULL;
   Boolean           font_or_fontset=True;
   Position          seg_start_x;


   if (string_len > MAX_ARRAY_SIZE)
    {
      XmeWarning(NULL, "Unable to Handle the Huge line for CTL processing");
      return SUCCESS;
    }

   if ((string_len <= 0) || !CTL_FONTTYPE(rend))
    {
      return SUCCESS;         /* Work For Fontset & XOC Only */
    }

   *num_chars_return = 0;

   CTLRenditionSegInfo(rend, string, string_len, is_wchar, tab_width, 0, istext_rtaligned,
                       &seg_widths, &num_segs, &starting_xpos, &block_length,
                       0, 0, 0, 0, 0);

   /* used to fill the vertical components of tab : Presumes fontset */
   fs_extents = XExtentsOfFontSet((XFontSet)_XmRendFont(rend));
   font_ascent  = -fs_extents->max_logical_extent.y;
   font_descent = fs_extents->max_logical_extent.height + fs_extents->max_logical_extent.y;
   line_height  = fs_extents->max_logical_extent.height;

   if (CTL_FONTTYPE(rend) && !_XmRendLayoutIsCTL(rend))
    {
      font_or_fontset = True;
    }
   else
    {
      font_or_fontset = False;
    }

   while (string_len > 0)
    {
      int curr_seg_len = 0;

      if (IS_TAB(string, is_wchar))       /* handle a tab         */
       {
         if (font_or_fontset == True)     /* NON CTL Fontset Code */
          {
            XRectangle *rect_logical_array = (XRectangle *)logical_array;

            /* compute the rectangle for the tab character */
            rect_logical_array->width = seg_widths[curr_seg_num];
            if (istext_rtaligned)
             {
               rect_logical_array->x = block_length - starting_xpos[curr_seg_num];
             }
            else
             {
               rect_logical_array->x = starting_xpos[curr_seg_num];
             }

            rect_logical_array->y      = -font_ascent;
            rect_logical_array->height = line_height;
          }
         else                             /* CTL Code             */
          {
            XSegment *xsegment_logical_array = (XSegment*)logical_array;

            /* compute the xsegment for the tab charater */
            xsegment_logical_array->x1 = starting_xpos[curr_seg_num];
            xsegment_logical_array->x2 = starting_xpos[curr_seg_num] + seg_widths[curr_seg_num];

            if (istext_rtaligned)
             {
               xsegment_logical_array->x1 = block_length - xsegment_logical_array->x1;
               xsegment_logical_array->x2 = block_length - xsegment_logical_array->x2;
             }
            xsegment_logical_array->y1 = -font_ascent;
            xsegment_logical_array->y2 = font_descent;
          }
         *num_chars_return += 1;
         curr_seg_len     += 1;
       }
      else                                /* Not tab, so its a text segment   */
       {
         int   i;
         int   curr_chars_return=0;

         /* Get the logical array for the text segment */
         GET_NEXT_SEGMENT(string, string_len, is_wchar, &curr_seg_len);
         status = CTLRenditionSegPerCharExtents(rend, string, curr_seg_len, is_wchar,
                                                logical_array, curr_seg_len,
                                                overall_logical_return,
                                                &curr_chars_return);
         if (!status)
          {
            XmeWarning(NULL, "Error:CTLRenditionSegPerCharExtents\n");
            return SUCCESS;
          }

         /* find the x of starting position of this segment in the text passed */
         if (istext_rtaligned)
          {
            seg_start_x = block_length - seg_widths[curr_seg_num] - starting_xpos[curr_seg_num];
          }
         else
          {
            seg_start_x = starting_xpos[curr_seg_num];
          }

         /* update the individual char's x (starting pos) which
          * is now w.r.t current segment w.r.t the complete text
          */
         if (font_or_fontset)
          {
            XRectangle *rect_logical_array = (XRectangle *)logical_array;
            /* it is the responsibility of CTLRenditionSegPerCharExtents to return
               num_chars_return >= curr_seg_len */
            for (i = 0; i < curr_seg_len; i++)
             {
               rect_logical_array[i].x += seg_start_x;
             }
          }
         else                             /* fonttype == XOC      */
          {
            XSegment *xsegment_logical_array = (XSegment *) logical_array;
            for (i=0; i<curr_seg_len; i++)
             {
               xsegment_logical_array[i].x1 += seg_start_x;
               xsegment_logical_array[i].x2 += seg_start_x;
             }
          }
         *num_chars_return += curr_chars_return;
       }

      /* set the loop parameters here */
      curr_seg_num++;
      string_len -= curr_seg_len;
      string = STR_IPTR(string, curr_seg_len, is_wchar);
      if (font_or_fontset)
       {
         logical_array = (XtPointer)((XRectangle*)logical_array + curr_seg_len);
       }
      else
       {
         logical_array = (XtPointer)((XSegment*)logical_array + curr_seg_len);
       }
    }

   if (seg_widths)
      XtFree ((char *) seg_widths);
   if (starting_xpos)
      XtFree ((char *) starting_xpos);
   return status;
}


/* gives the bounding rectangle of the rects */
static void
CTLGetOverallRectangle(XRectangle *logical_rects,
		       int         array_size,
		       XRectangle *overall_rect)
{
    int i;
    
    overall_rect->x      = logical_rects[0].x;
    overall_rect->y      = logical_rects[0].y;
    overall_rect->height = logical_rects[0].height;
    overall_rect->width  = 0;
    
    for (i = 0; i < array_size; i++)
	overall_rect->width += logical_rects[i].width;
}

/* This method is invoked when the ToggleOverstrike action changes
 * the mode to overstrike, and sets the cursor width to the current
 * character width. The default character width is to take care of the
 * cursor width beyond the text. Also note that in bidirectional text
 * last character in the line could fall in the middle of the visual
 * text. So we have to keep the cursor width so that it won't highlight
 * the wrong character. Presumes fontset always
 */
/***************************************************************************
 * Description Gets the XSegment of the character text[pos]
 * Inputs:     rendition, pos, text, text_len, tab_width, and w_char flag
 * output:     XSegment bounding the text[pos] character.
 * returns:    Returns the return value of _XmRenditionTextPerCharExtents
 ***************************************************************************/
int 
_XmRenditionPosCharExtents(XmRendition       rend,
			   XmTextPosition    pos,
			   char             *text,
			   size_t            text_len,
			   Boolean           is_wchar,
			   int               tab_width,
			   Boolean           istext_rtaligned,
			   XSegment         *char_segment)
{
    int        status;
    int        num_chars_return  = 0;
    int        default_char_width = XmbTextEscapement((XFontSet)_XmRendFont(rend), "0", 1)/2;
    XRectangle char_rect;
    
    if (text_len <= 0) {
	char_segment->x1 = 0;
	char_segment->x2 = default_char_width;
	return 1;
    }
    
    /* get the perchar extents for the whole text */   
    switch (_XmRendFontType(rend)) {
	case XmFONT_IS_XOC :
	case XmFONT_IS_FONTSET :
	    if (_XmRendLayoutIsCTL(rend)) {
		XSegment  *logical_array, logical_array_cache[MAX_ARRAY_SIZE];
		
		logical_array = (XSegment*)XmStackAlloc((text_len*sizeof(XSegment)),
							logical_array_cache);
		status = _XmRenditionTextPerCharExtents(rend,
							text,
							text_len, 
							is_wchar,
							logical_array,
							text_len, /* array_size */
							&num_chars_return,
							0,
							tab_width,
							istext_rtaligned,
							NULL);
		if (!status)
		    XmeWarning(NULL, "Error::_XmRenditionCharEscapement\n");
		/* when the pos is at newlinechar or beyond the text we have
		 * to return segment of default character length which comes later. */
		if (pos < text_len)
		    *char_segment = *(logical_array+pos);
		else {
		    /* Ensure that the Cursor doesn't highlight some other
		     * character when it is past the last character,  (in 
		     * bidirectional text).	     */
		    Boolean l_to_r = logical_array[text_len - 1].x2 >
				     logical_array[text_len - 1].x1;
		    
		    if (l_to_r) {
			char_segment->x1 = logical_array[text_len - 1].x2;
			char_segment->x2 = logical_array[text_len - 1].x2 
					   + default_char_width;
		    }
		    else {
			char_segment->x1 = logical_array[text_len - 1].x2;
			char_segment->x2 = logical_array[text_len - 1].x2 
					   - default_char_width;
		    }
		}
	    }
	    else {
		XRectangle *log_array, log_array_cache[MAX_ARRAY_SIZE];

		log_array = (XRectangle*)XmStackAlloc((text_len*sizeof(XRectangle)),
						      log_array_cache);
		status = _XmRenditionTextPerCharExtents(rend, 
							text, 
							text_len, 
							is_wchar, 
							(XtPointer)log_array, 
							text_len, /* array_size */
							&num_chars_return,
							0, 
							tab_width, 
							istext_rtaligned,
							NULL);
		if (!status)
		    XmeWarning(NULL, "Error::_XmRenditionCharEscapement\n");
		/* when the pos is at newlinechar or beyond the text we have
		 * to return segment of default character length which comes later. */
		if (pos < text_len) {
		    char_rect = log_array[pos];
		    _XRectangleToXSegment(True, char_segment, &char_rect);
		}
		else {
		    /* fill the default char segment */
		    char_segment->x1 = log_array[text_len- 1].x + 
				       log_array[text_len - 1].width;
		    char_segment->x2 = char_segment->x1 + default_char_width;
		}
	    }
	    break;
	case XmFONT_IS_FONT : /* Should never get executed */
	    XmeWarning(NULL, "Error::_XmRenditionPosCharExtents NYI\n");
	    break;
    }
    return 1;
}

/***********************************************************************
 * Description
 *    Gets the xpos where the string[index] falls
 * Inputs: text etc. and edge (end, start etc). 
 * returns: The xpos in pixels of the string[index] character in the text
 *************************************************************************/
Dimension			/* returns an escapement from offset */
_XmRenditionPosToEscapement(XmRendition 	 rend,
			    Position 		 xoffset, /* for tabbing */
			    char 		*string,
			    Boolean 		 is_wchar,
			    XmTextPosition 	 index,
			    XmTextPosition 	 len,
			    Dimension 		 tab_width,
			    XmEDGE 		 edge,
			    unsigned char        mode,
			    Boolean              istext_rtaligned )
{
    int        array_size        = len;
    int        num_chars_return  = 0;
    Status     status;
    Dimension  ret_escapement    = 0;
    
    if (len > MAX_ARRAY_SIZE) {
	 XmeWarning(NULL, "Unable to Handle the Huge line for CTL processing\n");
	 return (Dimension)0;
    }

    if (len <= 0) 
	return (Dimension)0;
    
    switch (_XmRendFontType(rend)) {
	case XmFONT_IS_XOC :
	case XmFONT_IS_FONTSET :
	    if (_XmRendLayoutIsCTL(rend)) {		
		XSegment *logical_array, logical_array_cache[MAX_ARRAY_SIZE];
		XSegment *seg;
		
		logical_array = (XSegment*)XmStackAlloc((array_size*sizeof(XSegment)), logical_array_cache);
		status = _XmRenditionTextPerCharExtents(rend,
							string,
							len, 
							is_wchar,
							logical_array,
							array_size,
							&num_chars_return,
							xoffset,
							tab_width,
							istext_rtaligned,
							NULL);
		if (!status) {
		    XmeWarning(NULL, "Error::_XmRenditionPosToEscapement\n");
		    return (Dimension)0;
		}
		
		if (index >= len) {
		    if (mode == XmEDIT_LOGICAL) {
			seg = logical_array + (len -1);
			edge = XmEDGE_END;
		    }
		    else { /* mode == XmEDIT_VISUAL */
			/* Find the visual last segment */
			int i=0;
			XSegment *max_segment = logical_array;
			
			for (i = 1; i < len; i++) {
			    if (MAX(logical_array[i].x1, logical_array[i].x2) >
				MAX(max_segment->x1,max_segment->x2))
				max_segment = logical_array + i;
			}
			seg = max_segment;
			edge = XmEDGE_RIGHT;
		    }
		}
		else if ((index == len - 1) && index) { /* end of line*/
		    if (mode == XmEDIT_LOGICAL) {
			seg = logical_array + (index - 1);
			edge = XmEDGE_END;
		    }
		    else { /* mode == XmEDIT_VISUAL */
			/* Find the visual last segment */
			int i=0;
			XSegment *max_segment = logical_array;
			
			for (i = 1; i < len; i++) {
			    if (MAX(logical_array[i].x1, logical_array[i].x2) >
				MAX(max_segment->x1,max_segment->x2))
				max_segment = logical_array + i;
			}
			seg = max_segment;
			edge = XmEDGE_RIGHT;
		    }
		}
		else
		    seg = logical_array + index;
		
		switch (edge) {
		    case XmEDGE_LEFT:
			ret_escapement = MIN(seg->x1, seg->x2);
			break;
		    case XmEDGE_BEG:
			ret_escapement = seg->x1;
			break;
		    case XmEDGE_RIGHT:
			ret_escapement = MAX(seg->x1, seg->x2);
			break;
		    case XmEDGE_END:
			ret_escapement = seg->x2;
			break;
		}
	    }
	    else {		
		XRectangle *log_array, log_array_cache[MAX_ARRAY_SIZE];
		XRectangle *rect; 
		
		log_array = (XRectangle*)XmStackAlloc((array_size*sizeof(XRectangle)), log_array_cache);
		status = _XmRenditionTextPerCharExtents(rend,
							string,
							len, 
							is_wchar,
							(XtPointer)log_array,
							array_size,
							&num_chars_return,
							xoffset,
							tab_width,
							istext_rtaligned,
							NULL);
		if (!status) {
		    XmeWarning(NULL, "_XmRenditionTextPerCharExtents error\n");
		    return ret_escapement;
		}
		
		/* Bug 4110732, if index is zero the return escapement is incorrect as
		 * index -1 = -1, resulting in reference to a bad area in log_array	   */
		if (index >= len) { 
		    rect = log_array + len - 1; 
		    edge = XmEDGE_END; 
		} 
		else if ((index == len - 1) && (index != 0)) { 
		    rect = log_array + index - 1; 
		    edge = XmEDGE_END; 
		} 
		else 
		    rect = log_array + index; 
		
		if ((edge == XmEDGE_LEFT) || (edge == XmEDGE_BEG)) 
		    ret_escapement = rect->x; 
		else 
		    ret_escapement = rect->x + rect->width; 
	    }
	    break;
	case XmFONT_IS_FONT :
	    XmeWarning(NULL, "RenditionPosToEscapement NYI\n");
	    break;
    }
    return ret_escapement;
}

/* This is a common subroutine for XmFONT_IS_FONT and XmFONT_IS_FONTSET
   in _XmRenditionEscapementToPos */
XmTextPosition 
_FindEscapement(Position           escapement, 
		XmTextPosition     len, 
		XRectangle        *logical_array,
		XmEDGE             edge)
{
    XmTextPosition i;

    if (len <= 0) return 0;

    for (i = 0; i < len; i++) {
	Position left_side  = logical_array[i].x;
	int      width      = logical_array[i].width;
	Position right_side = left_side + width;
	
	if ((left_side <= escapement) && (escapement < right_side)) {
	    /* x is in the current rectangle -- find the desired edge */
	    if ((edge == XmEDGE_RIGHT) ||
		(edge == XmEDGE_END) ||
		((edge == XmEDGE_NEAREST) && ((escapement - left_side) > Half(width)))) 
		i++;
	    break;		/* we're done, break out of for () */
	}
    }
    return i;
}

/* Given the position in the given text returns the cell end of the
/* position in which the pos character falls 
 */
#define ISVOWEL(prop) (!(((prop) & DISPLAYCELL_MASK) || ((prop) & 020)))

XmTextPosition
_XmRenditionPosToCellEnd(XmRendition 	 rend,
			 char 		*text, 
			 XmTextPosition  len,
			 Boolean 	 is_wchar,
			 XmTextPosition  pos)
{
    char	  *glyph_string;
    int		  num_glyphs;
    unsigned char *inp_str_props, inpstr_cache[CTL_CACHE_SIZE];
    
    if (len <= 0) 
	return pos;

    inp_str_props = (unsigned char*)XmStackAlloc(len, inpstr_cache);
    XocTextInfo((XOC)_XmRendFont(rend),
		text,
		is_wchar, 
		len,
		inp_str_props,
		NULL,
		NULL);
    
    while (pos < len && ISVOWEL(inp_str_props[pos]))
	pos++;
    XmStackFree((char*)inp_str_props, inpstr_cache);
    return pos;
}

/***************************************************************************
 *  Description Get pos(in chars) of the character which falls at escapement
 *  Inputs:     Escapement
 *  Output:     The position of the character in the line
 ***************************************************************************/
XmTextPosition
_XmRenditionEscapementToPos(XmRendition 	 rend,
			    Position 		 offset,
			    Position 		 escapement,
			    char 		*text,
			    XmTextPosition 	 len,
			    Boolean 		 is_wchar,
			    Dimension 		 tab_width,
			    XmEDGE 		 edge,
			    XmCURSOR_DIRECTION 	*ret_cursor_dir,
			    Boolean              istext_rtaligned)
{
    int            array_size        = len;
    int            num_chars_return  = 0;
    Status         status;
    XmTextPosition ret_pos           = 0;
    
    if (escapement < 0)
	escapement = 0;
    
    switch (_XmRendFontType(rend)) {
	case XmFONT_IS_XOC :
	case XmFONT_IS_FONTSET :
	    if (_XmRendLayoutIsCTL(rend)) {
		XSegment  *logical_array, logical_array_cache[MAX_ARRAY_SIZE];
		int       i;
		
		logical_array = (XSegment*)XmStackAlloc((array_size*sizeof(XSegment)), logical_array_cache);
		status = _XmRenditionTextPerCharExtents(rend,
							text,
							len,
							is_wchar,
							(XtPointer) logical_array,
							array_size,
							&num_chars_return,
							offset, 
							tab_width,
							istext_rtaligned,
							NULL);
		if (!status) { 
		    XmeWarning((Widget)NULL, "Error :: _XmRenditionEscapementToPos\n");
		    return 0;
		}
		
		/* Change the logical_array for text when it is rightaligned */
		if (istext_rtaligned) {
		    int           i;
		    Dimension     block_length;
		    
		    block_length = _XmRenditionEscapement(rend, text, len, is_wchar, tab_width);
		    for (i = 0; i < len; i++) {
			logical_array[i].x1 = block_length - logical_array[i].x1;
			logical_array[i].x2 = block_length - logical_array[i].x2;	
		    }
		}
		
		/*++ Doesn't yet handle overlapping rectangles. */
		for (i = 0; i < len; i++) {
		    Position  left_side  = logical_array[i].x1;
		    Position  right_side = logical_array[i].x2;
		    Boolean   rtl_char   = (right_side < left_side);
		    int       width;
		    
		    if (rtl_char) {
			right_side = left_side;
			left_side  = logical_array[i].x2;
		    }
		    
		    width = right_side - left_side;
		    
		    if ((left_side <= escapement) && (escapement < right_side)) {
			/* x is in the current rectangle -- find the desired edge */
			Boolean right_half = (escapement - left_side) > Half(width);
			
			if ((edge == XmEDGE_END) || (edge == XmEDGE_BEG && (right_half ^ rtl_char)) ||
			    /* If in right half of LTR char, or in left half of RTL,
			       then we want the position after this glyph, not before it */
			    ((edge == XmEDGE_NEAREST) && (right_half ^ rtl_char)) ||
			    ((edge == XmEDGE_RIGHT)   && !rtl_char))
			    i++;
			
			/* Note: the XmEDGE_LEFT ==> mode == Visual */
			if (edge == XmEDGE_LEFT && right_half) {
			    /* find which character falls to right */
			    int j;
			    for (j = 0; j < len; j++)
				if (i != j)
				    if (right_side == logical_array[j].x1 ||
					right_side == logical_array[j].x2)
					break;
			    i = j;
			}
			break;		/* we're done, break out of for () */
		    }
		}
		
		/* If caller wants the cursor direction, compute it */
		if (ret_cursor_dir) {
		    XSegment *prev = (i > 0)   ? logical_array + (i - 1) : NULL;
		    XSegment *next = (i < len) ? logical_array + i       : NULL;
		    
		    if (prev == NULL) {
			if (next->x2 < next->x1)
			    *ret_cursor_dir = XmCURSOR_DIRECTION_B_R;
			else
			    *ret_cursor_dir = XmCURSOR_DIRECTION_B_L;
		    }
		    else if (next == NULL) {
			if (prev->x2 < prev->x1)
			    *ret_cursor_dir = XmCURSOR_DIRECTION_R_E;
			else
			    *ret_cursor_dir = XmCURSOR_DIRECTION_L_E;
		    }
		    else if (prev->x2 < prev->x1) {
			if (next->x2 < next->x1)
			    *ret_cursor_dir = XmCURSOR_DIRECTION_R_R;
			else
			    *ret_cursor_dir = XmCURSOR_DIRECTION_R_L;
		    }
		    else {
			if (next->x2 < next->x1)
			    *ret_cursor_dir = XmCURSOR_DIRECTION_L_R;
			else
			    *ret_cursor_dir = XmCURSOR_DIRECTION_L_L;
		    }
		}
		ret_pos = i;
		/*++ We need to be a little more sophisticated about caching the
		  > end-of-line case, since the last glyph could come anywhere in
		  the logical_array. */
	    }
	    else {
		XRectangle *log_array, log_array_cache[MAX_ARRAY_SIZE];
		
		log_array = (XRectangle*)XmStackAlloc((array_size*sizeof(XRectangle)), log_array_cache);
		status = _XmRenditionTextPerCharExtents(rend,
							text,
							len,
							is_wchar,
							(XtPointer)log_array,
							array_size,
							&num_chars_return,
							offset,
							tab_width,
							istext_rtaligned,
							NULL); 
		if (!status)
		    XmeWarning(NULL, "Error::_XmRenditionEscapementToPos\n");
		
		ret_pos = _FindEscapement(escapement, array_size, log_array, edge);
		if (ret_cursor_dir)
		    *ret_cursor_dir = XmCURSOR_DIRECTION_L_L;
	    }
	    break;
	case XmFONT_IS_FONT :
	    XmeWarning((Widget)NULL, "_XmRenditionEscapementToPos NYI\n");
	    break;
    }
    
    /* This is a fix to include all the vowels */
    /* Note1: This will just eliminate the posibility of placing the
       cursor in the middle of a cell */
    /* Note 2: This will work for RTL locales but the transforming the
       whole line is not the convention which we are following. So in the
       spirit we should have a check of ltr & cell based locale for the
       following code and the function which provides the check for cell
       base locale should come from XmXOC.*/
    
    if (len <= 0)
	ret_pos = 0;
    else {
	if (_XmRendLayoutIsCTL(rend))
	    ret_pos = _XmRenditionPosToCellEnd(rend, text, len, is_wchar, ret_pos);
	else
	    ret_pos = 0;
    }
    return ret_pos;
}


_XmRenditionHighlightSegment(Display* d, Window w, Widget widget, GC gc,
                             Position x, Position y, int font_ascent,
                             Dimension seg_width, Dimension seg_height,
                             u_char hl_mode
                            )
{
   if (hl_mode == XmHIGHLIGHT_SELECTED)
    {
      SetReverseVideoGC(widget, gc);
      XFillRectangle(d, w, gc, x, y-font_ascent, seg_width, seg_height);
    }
   else if (hl_mode == XmHIGHLIGHT_SECONDARY_SELECTED)
    {
      SetNormGC(widget, gc);
      XFillRectangle(d, w, gc, x, y, seg_width, 1);
    }
}


/****************************************************************************
 *  Description: Draws the Text segment (which does't have a tab )
 *  Input:       rendition, display, window, gc, x, y (baseline), text to draw
 *  
 *****************************************************************************/
void
_XmRenditionDrawSegment(XmRendition    rend,
                        Display *      d,
                        Window         w,
                        Widget         widget,
                        XmTextPart *   tpp,
                        GC             gc,
                        Position       x,
                        Position       y,
                        char *         draw_text,
                        unsigned int   seg_len,
                        unsigned int   seg_length,
                        Boolean        is_wchar,
                        Boolean        image,
                        u_char         hl_mode
                       )
{
   u_long      valueMask=GCBackground | GCForeground;
   XGCValues   values;


   if (hl_mode == XmHIGHLIGHT_COLOR_1)
    {
      _Setup_hl1(widget, tpp, d, XtScreen(widget));
      values.foreground = tpp->highlightColor1->pixel;
      values.background = tpp->inner_widget->core.background_pixel;
      XChangeGC(d, gc, valueMask, &values);
    }
   else if (hl_mode == XmHIGHLIGHT_COLOR_2)
    {
      _Setup_hl2(widget, tpp, d, XtScreen(widget));
      values.foreground = tpp->highlightColor2->pixel;
      values.background = tpp->inner_widget->core.background_pixel;
      XChangeGC(d, gc, valueMask, &values);
    }

   if (_XmRendFontType(rend) == XmFONT_IS_FONT)
    {
      if (is_wchar)
       {
         XmeWarning(NULL, "Error::_XmRenditionDrawSegment NYI\n");
       }
      else
       {
         if (image)
            XDrawImageString(d, w, gc, x, y, draw_text, seg_len);
         else
            XDrawString(d, w, gc, x, y, draw_text, seg_len);
       }
    }
   else
    {
      if (is_wchar)
       {
         if (image)
            XwcDrawImageString(d, w, (XFontSet)_XmRendFont(rend), gc, x, y,
                               (wchar_t*)draw_text, seg_len);
         else
            XwcDrawString(d, w, (XFontSet)_XmRendFont(rend), gc, x, y,
                          (wchar_t*)draw_text, seg_len);
       }
      else
       {
         if (image)
            XmbDrawImageString(d, w, (XFontSet)_XmRendFont(rend), gc, x, y,
                               draw_text, seg_len);
         else
            XmbDrawString(d, w, (XFontSet)_XmRendFont(rend), gc, x, y,
                          draw_text, seg_len);
       }
    }

   if ((hl_mode == XmHIGHLIGHT_COLOR_1) ||
       (hl_mode == XmHIGHLIGHT_COLOR_2)
      )
    {
      XFillRectangle(d, w, gc, x, y, seg_length, 1);
    }
}


/***************************************************************************
 * Desc:  Draws the text in a line on the widget
 * Input  Text to draw etc.
 * output:The end x pos (pixel) till which the drawing is done.
 * Algorithm for Draw 
 * While more to draw
 *      1. Get the current segment
 *      2. Clear the segment boudning rectangle
 *      3. Draw the segment
 * GetPerCharExtents for the text
 * Draw the Highlighting regions
 *****************************************************************************/
int
_XmRenditionDraw(XmRendition rend, Widget widget, GC gc, XmTextPart* tpp,
                 Position x, Position y, char* draw_text, uint text_len,
                 Boolean is_wchar, Boolean is_editable, Boolean image,
                 _XmHighlightData* hl_data, Dimension tab_width,
                 Boolean istext_rtaligned
                )
{
   Status   status;
   int      overall_width,
            i, j;

   if (text_len > MAX_ARRAY_SIZE)
    {
      XmeWarning(NULL, "Unable to Handle the line for CTL processing");
      return x;
    }

   if (text_len <= 0)
    {
      return x;
    }

   if (!hl_data)
    {
      image = True;
    }


   /* Here is the actual drawing of the text; done segment by segment   */

   if (hl_data)
    {
      Display *   d=XtDisplay(widget);
      Window      w=XtWindow(widget);
      Position    seg_start_x;
      char *      rem_text=draw_text,
           *      lookahead_ptr;
      Boolean     more_to_draw=True;

      Dimension * seg_widths=NULL;
      int         num_segs=0; 
      Position *  starting_xpos=NULL;
      Dimension   block_length=0, seg_width;
      ushort *    char_seg_widths=NULL;
      u_char *    seg_hilites=NULL;

      XtPointer   log_array;
      Boolean     font_or_fontset=True;
      XRectangle  logical_rects[MAX_ARRAY_SIZE];
      XSegment    logical_segts[MAX_ARRAY_SIZE];
      XRectangle  overall_logical_rect;
      XSegment    overall_logical_seg;
      XSegment *  hl_rects=0;
      u_char *    hl_modes=0;
      int         nbr_hl_rects=0;
      int         num_chars_return = 0;

      ushort      char_seg_width;

      int               font_ascent, line_height;
      Widget            widget=XtWindowToWidget(d, w);
      XFontSetExtents * fs_extents=XExtentsOfFontSet((XFontSet)_XmRendFont(rend));


      /* get the font ascent and line_height so that we can get the top left  */
      /* most (starting ) pointing of the segment rectangles.                 */

      font_ascent = -fs_extents->max_logical_extent.y;
      line_height = fs_extents->max_logical_extent.height;

      CTLRenditionSegInfo(rend, draw_text, text_len, is_wchar, tab_width,
                          hl_data, istext_rtaligned, &seg_widths, &num_segs,
                          &starting_xpos, &block_length, &char_seg_widths,
                          &seg_hilites, &hl_rects, &hl_modes, &nbr_hl_rects
                         );
      overall_width = block_length;

      /* Clear the overall bounding rectangle.     */

      if(!image)
       {
         SetInvGC(widget, gc);
         XFillRectangle(d, w, gc, x, y - font_ascent, block_length, line_height);
       }

      for (i=0; i<num_segs; i=j)
       {
         Boolean  is_tab, lookahead_is_tab;
         int      lookahead_index,
                  leftmost_xpos;
         u_char   hl_mode, lookahead_hl_mode;


         is_tab = IS_TAB(rem_text, is_wchar);
         leftmost_xpos = starting_xpos[i];

         /* Now consolidate adjacent segments that have NORMAL,   */
         /* SELECTED or SECONDARY_SELECTED highlighting.          */

         hl_mode = seg_hilites[i];
         seg_width = seg_widths[i];
         char_seg_width = char_seg_widths[i];

         j = i+1;
         if (!is_tab &&
             ((seg_hilites[i] == XmHIGHLIGHT_NORMAL) ||
              (seg_hilites[i] == XmHIGHLIGHT_SELECTED) ||
              (seg_hilites[i] == XmHIGHLIGHT_SECONDARY_SELECTED)
             )
            )
          {
            lookahead_index = char_seg_width;
            while (j < num_segs)
             {
               lookahead_hl_mode = seg_hilites[j];
               lookahead_ptr = STR_IPTR(rem_text, lookahead_index, is_wchar);
               lookahead_is_tab = IS_TAB(lookahead_ptr, is_wchar);
               if (!lookahead_is_tab &&
                   ((lookahead_hl_mode == XmHIGHLIGHT_NORMAL) ||
                    (lookahead_hl_mode == XmHIGHLIGHT_SELECTED) ||
                    (lookahead_hl_mode == XmHIGHLIGHT_SECONDARY_SELECTED)
                   )
                  )
                {
                  seg_width += seg_widths[j];
                  lookahead_index += char_seg_widths[j];
                  if (starting_xpos[j] < leftmost_xpos)
                   {
                     leftmost_xpos = starting_xpos[j];
                   }
                  j++;
                }
               else
                {
                  break;
                }
             }
            char_seg_width = lookahead_index;   /* Work out the width of the layout */
                                                /* segment.                         */
          }

         seg_start_x = x + leftmost_xpos;       /* get the seg_start_x  */

         if (!is_tab)
          {
            SetNormGC(widget, gc);
            SetMarginGC(widget, gc);
            _XmRenditionDrawSegment(rend, d, w, widget, tpp, gc, seg_start_x, y,
                                    rem_text, char_seg_width, seg_width,
                                    is_wchar, image, hl_mode
                                   );
          }
         rem_text = STR_IPTR(rem_text, char_seg_width, is_wchar);
       }

      for (i=0; i<nbr_hl_rects; i++)         /* Run through all the highlight    */
       {                                     /* segments & draw them.            */
         _XmRenditionHighlightSegment(d, w, widget, gc, x+hl_rects[i].x1, y,
                                      font_ascent, hl_rects[i].x2-hl_rects[i].x1,
                                      line_height, hl_modes[i]
                                     );
       }

      if (starting_xpos)      XtFree((char *)starting_xpos);
      if (seg_widths)         XtFree((char *)seg_widths);
      if (char_seg_widths)    XtFree((char *)char_seg_widths);
      if (seg_hilites)        XtFree((char *)seg_hilites);
      if (hl_rects)           XtFree((char *)hl_rects);
      if (hl_modes)           XtFree((char *)hl_modes);
    }

   SetMarginGC(widget, gc);
   SetNormGC(widget, gc);
   return (x + overall_width);
}
#endif /* CTL */
