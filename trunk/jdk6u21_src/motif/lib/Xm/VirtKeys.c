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
static char rcsid[] = "$XConsortium: VirtKeys.c /main/16 1996/10/17 17:00:14 drk $"
#endif
#endif
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1990 MOTOROLA, INC. */

#define	GET_TOKEN1_START		1
#define	PARSE_TOKEN1			2
#define	GET_COLON				3
#define	GET_TOKEN2_START		4
#define	PARSE_TOKEN2			5
#define	SKIP_LINE				6


#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <X11/keysym.h>
#include <Xm/DisplayP.h>
#include <Xm/TransltnsP.h>
#include <Xm/AtomMgr.h>
#include <Xm/XmosP.h>
#include "MapEventsI.h"
#include "VirtKeysI.h"
#include "XmosI.h"
#include "XmI.h"
/*
#ifdef SUN_MOTIF
#include "_VirtKeysI.h"
#endif
*/

/* Added by Prabhat, Since XServer has KEYMAP-KEYSYM errors due
   to backward compatibility
 */
/*
#ifdef SUN_MOTIF*/
/*
static XContext ModMappingCache = NULL;
static XContext KeyMappingCache = NULL;

typedef struct _KeypadInfo{
        KeySym                  keysym;
        KeyCode*                keycodelist;
        Cardinal                keycount;
}       KeypadInfo;

static KeypadInfo keypad_table[] = {
    { XK_KP_0, NULL, 0 },
    { XK_KP_1, NULL, 0 },
    { XK_KP_2, NULL, 0 },
    { XK_KP_3, NULL, 0 },
    { XK_KP_4, NULL, 0 },
    { XK_KP_5, NULL, 0 },
    { XK_KP_6, NULL, 0 },
    { XK_KP_7, NULL, 0 },
    { XK_KP_8, NULL, 0 },
    { XK_KP_9, NULL, 0 },
    { XK_KP_Equal, NULL, 0 },
    { XK_KP_Multiply, NULL, 0 },
    { XK_KP_Add, NULL, 0 },
    { XK_KP_Separator, NULL, 0 },
    { XK_KP_Subtract, NULL, 0 },
    { XK_KP_Decimal, NULL, 0 },
    { XK_KP_Divide, NULL, 0 },
    { XK_KP_Space, NULL, 0 },
    { XK_KP_Tab, NULL, 0 },
    { XK_KP_Enter, NULL, 0 },
    { XK_KP_F1, NULL, 0 },
    { XK_KP_F2, NULL, 0 },
    { XK_KP_F3, NULL, 0 },
    { XK_KP_F4, NULL, 0 },
};

typedef struct _ModifierInfo{
        ModifierType            modifier_type;
        KeySym                  left_keysym;
        KeySym                  right_keysym;
        KeyCode                 left_keycode;
        KeyCode                 right_keycode;
        Modifiers               modifier;
} ModifierInfo;

static _XmConst ModifierInfo mod_info[] = {
   {Alt, XK_Alt_L,    XK_Alt_R,   (KeyCode)0, (KeyCode)0,    None },
   {Meta, XK_Meta_L,   XK_Meta_R,  (KeyCode)0, (KeyCode)0,    None },
   {Hyper, XK_Hyper_L,  XK_Hyper_R, (KeyCode)0, (KeyCode)0,    None },
   {Super, XK_Super_L,  XK_Super_R, (KeyCode)0, (KeyCode)0,    None },
   {NumLock, XK_Num_Lock, XK_Num_Lock, (KeyCode)0, (KeyCode)0,None },
   {ModeSwitch, XK_Mode_switch, XK_Mode_switch, (KeyCode)0, (KeyCode)0,None}
};
*/
/* These are from /usr/openwin/lib/X11/XKeysymDB. Are these defined
   elsewhere ? Prabhat *//*
#define SunCopy     0x1005FF72
#define SunPaste    0x1005FF74
#define SunCut      0x1005FF75
#define SunStop     0xFF69
#define SunAgain    0xFF66
#define SunProps    0x1005FF70
#define SunUndo     0xFF65
#define SunFront    0x1005FF71
#define SunOpen     0x1005FF73
#define SunFind     0xFF68

typedef struct _RebindInfo {
    KeySym     sourceKS;
    KeySym     targetKS;
} RebindInfo;

*/
/* This table contains keysyms which need to be converted in Sun Motif's
   virtual binding table in order to correct the X server's binding problem.
   sourceKS is what motif expects from the keycode and binds to a virtual
   keysym. targetKS is what the X server actually produces from a keycode.
   We need to do this because our X server's key binding is broken in order
   to keep things backwardly compatible. This should be removed when the
   X server's keycode-keysym binding is fixed. (Right now these sourceKS
   keysyms are defined as the third keysyms asscociated with their keycodes
   instead of the first.) *//*
static RebindInfo RebindInfoTable[] = {
    {SunStop,  XK_F11},
    {SunAgain, XK_F12},
    {SunProps, XK_F13},
    {SunUndo,  XK_F14},
    {SunFront, XK_F15},
    {SunCopy,  XK_F16},
    {SunOpen,  XK_F17},
    {SunPaste, XK_F18},
    {SunFind,  XK_F19},
    {SunCut,   XK_F20},
    {(KeySym) 0, (KeySym) 0}};
*/
/*#endif *//* SUN_MOTIF */

#ifdef __linux__
#define defaultFallbackBindings _XmVirtKeys_pcFallbackBindingString
#else
#define defaultFallbackBindings _XmVirtKeys_fallbackBindingString
#endif

#define BUFFERSIZE	2048
#define MAXLINE		256

/********    Static Function Declarations    ********/


static Boolean CvtStringToVirtualBinding(Display *dpy,
					 XrmValuePtr args,
					 Cardinal *num_args,
					 XrmValuePtr fromVal,
					 XrmValuePtr toVal,
					 XtPointer *closure_ret);

static XmVKeyBindingRec * FillBindingsFromDB(Display *dpy,
			                     XrmDatabase rdb,
			                     Cardinal num_keys,
                                             String bindStrings []);

static Boolean GetBindingsProperty(Display *display,
				   String property,
				   String *binding);
static void FindVirtKey(Display *dpy,
                        KeyCode keycode,
                        Modifiers modifiers,
                        Modifiers *modifiers_return,
                        KeySym *keysym_return);
static Modifiers EffectiveStdModMask(Display *dpy,
				     KeySym *kc_map,
				     int ks_per_kc);
static void LoadVendorBindings(Display *display,
			       char *path,
			       FILE *fp,
			       String *binding);


/* Solaris 2.6 motif diff bug 1206588 */
static _XmConst XmVKeyBindingRec nullBinding = { 0L, 0, 0L, -1 };

static void swap_tokens(const String, String *, String **, int *);

static int virtKeySymsCmp(const void *,
                          const void *);
/* END Solaris 2.6 motif diff bug 1206588 */


#ifdef SUN_MOTIF
static void DisplayDestroy(Widget wid,
                        XtPointer clientData,
                        XtPointer callData) ;
#endif /* SUN_MOTIF */

/********    End Static Function Declarations    ********/


static XmConst XmVirtualKeysymRec virtualKeysyms[] =
{
  { XmVosfActivate,		osfXK_Activate	      },
  { XmVosfAddMode,		osfXK_AddMode	      },
  { XmVosfBackSpace,		osfXK_BackSpace	      },
  { XmVosfBackTab,		osfXK_BackTab	      }, /* Defunct */
  { XmVosfBeginData,		osfXK_BeginData	      }, /* Defunct */
  { XmVosfBeginLine,		osfXK_BeginLine	      },
  { XmVosfCancel,		osfXK_Cancel	      },
  { XmVosfClear,		osfXK_Clear	      },
  { XmVosfCopy,			osfXK_Copy	      },
  { XmVosfCut,			osfXK_Cut	      },
  { XmVosfDelete,		osfXK_Delete	      },
  { XmVosfDeselectAll,		osfXK_DeselectAll     },
  { XmVosfDown,			osfXK_Down	      },
  { XmVosfEndData,		osfXK_EndData	      }, /* Defunct */
  { XmVosfEndLine,		osfXK_EndLine	      },
  { XmVosfEscape,		osfXK_Escape	      }, /* Defunct */
  { XmVosfExtend,		osfXK_Extend	      }, /* Defunct */
  { XmVosfHelp,			osfXK_Help	      },
  { XmVosfInsert,		osfXK_Insert	      },
  { XmVosfLeft,			osfXK_Left	      },
  { XmVosfLeftLine,		osfXK_LeftLine	      }, /* X11R6 */
  { XmVosfMenu,			osfXK_Menu	      },
  { XmVosfMenuBar,		osfXK_MenuBar	      },
  { XmVosfNext,			osfXK_Next	      }, /* X11R6 */
  { XmVosfNextField,		osfXK_NextField	      }, /* Defunct */
  { XmVosfNextMenu,		osfXK_NextMenu	      }, /* Defunct */
  { XmVosfNextMinor,		osfXK_NextMinor	      }, /* X11R6 */
  { XmVosfPageDown,		osfXK_PageDown	      },
  { XmVosfPageLeft,		osfXK_PageLeft	      },
  { XmVosfPageRight,		osfXK_PageRight	      },
  { XmVosfPageUp,		osfXK_PageUp	      },
  { XmVosfPaste,		osfXK_Paste	      },
  { XmVosfPrevField,		osfXK_PrevField	      }, /* Defunct */
  { XmVosfPrevMenu,		osfXK_PrevMenu	      }, /* Defunct */
  { XmVosfPrimaryPaste,		osfXK_PrimaryPaste    },
  { XmVosfPrior,		osfXK_Prior	      }, /* X11R6 */
  { XmVosfPriorMinor,		osfXK_PriorMinor      }, /* X11R6 */
  { XmVosfQuickPaste,		osfXK_QuickPaste      },
  { XmVosfReselect,		osfXK_Reselect	      },
  { XmVosfRestore,		osfXK_Restore	      },
  { XmVosfRight,		osfXK_Right	      },
  { XmVosfRightLine,		osfXK_RightLine	      }, /* X11R6 */
  { XmVosfSelect,		osfXK_Select	      },
  { XmVosfSelectAll,		osfXK_SelectAll	      },
  { XmVosfSwitchDirection,	osfXK_SwitchDirection }, /* X11R6 */
  { XmVosfUndo,			osfXK_Undo	      },
  { XmVosfUp,			osfXK_Up	      }
};

static XmConst XmDefaultBindingStringRec fallbackBindingStrings[] =
{
  { "Acorn Computers Ltd", 
      _XmVirtKeys_acornFallbackBindingString },
  { "Apollo Computer Inc.",
      _XmVirtKeys_apolloFallbackBindingString },
  { "DECWINDOWS DigitalEquipmentCorp.",
      _XmVirtKeys_decFallbackBindingString },
  { "Data General Corporation  Rev 04",
      _XmVirtKeys_dgFallbackBindingString },
  { "Double Click Imaging, Inc. KeyX",
      _XmVirtKeys_dblclkFallbackBindingString },
  { "Hewlett-Packard Company",
      _XmVirtKeys_hpFallbackBindingString },
  { "International Business Machines",
      _XmVirtKeys_ibmFallbackBindingString },
  { "Intergraph Corporation",
      _XmVirtKeys_ingrFallbackBindingString },
  { "Megatek Corporation",
      _XmVirtKeys_megatekFallbackBindingString },
  { "Motorola Inc. (Microcomputer Division) ",
      _XmVirtKeys_motorolaFallbackBindingString },
  { "Silicon Graphics Inc.",
      _XmVirtKeys_sgiFallbackBindingString },
  { "Silicon Graphics",
      _XmVirtKeys_sgiFallbackBindingString },
  { "Siemens Munich by SP-4's Hacker Crew",
      _XmVirtKeys_siemensWx200FallbackBindingString },
  { "Siemens Munich (SP-4's hacker-clan)",
      _XmVirtKeys_siemens9733FallbackBindingString },
  { "X11/NeWS - Sun Microsystems Inc.",
      _XmVirtKeys_sunFallbackBindingString },
  { "Sun Microsystems, Inc.",
      _XmVirtKeys_sunFallbackBindingString },
  { "Tektronix, Inc.",
      _XmVirtKeys_tekFallbackBindingString },
  { "The XFree86 Project, Inc", 
      _XmVirtKeys_pcFallbackBindingString },
  { "Xi Graphics, Inc.", 
      _XmVirtKeys_pcFallbackBindingString }   
};

#ifdef SUN_MOTIF
static void
#ifdef _NO_PROTO
DisplayDestroy( wid, clientData, callData )
        Widget wid ;
        XtPointer clientData, callData;
#else
DisplayDestroy( Widget wid, XtPointer clientData, XtPointer callData )
#endif /* _NO_PROTO */
{
   KeypadInfo	*keypad_info = NULL;
   ModifierInfo *mods = NULL;
   int 		num = XtNumber(keypad_table), i;
   Display 	*dpy = XtDisplay(wid);

   if ((XFindContext(dpy, (XID)keypad_table, KeyMappingCache,
       (XPointer*)&keypad_info) == 0) && keypad_info != NULL ) {

      for (i = 0; i < num; i++)
          if (keypad_info[i].keycodelist != NULL)
             XtFree((char *)keypad_info[i].keycodelist);

      XtFree((char *) keypad_info);
      XDeleteContext(dpy, (XID)keypad_table, KeyMappingCache);
   }

   if ((XFindContext(dpy, (XID)mod_info, ModMappingCache,
      (XPointer*)&mods) == 0) && mods != NULL ) {
      XtFree((char *) mods);
      XDeleteContext(dpy, (XID)mod_info, ModMappingCache);
   }
}

static void
#ifdef _NO_PROTO
MappingEventHandler(w, client_data, event, cont_to_dispatch)
        Widget w;
        XtPointer client_data;
        XEvent *event;
        Boolean *cont_to_dispatch;
#else
MappingEventHandler(
        Widget w,
        XtPointer client_data,
        XEvent *event,
        Boolean *cont_to_dispatch)
#endif /* _NO_PROTO */
{
	if (event->xany.type != MappingNotify ||
	   event->xmapping.request == MappingPointer)
	return;

	if (event->xmapping.request == MappingModifier)
	   _XmGetModifierMapping(w);

	if (event->xmapping.request == MappingKeyboard)
	   _XmGetKPKeysymToKeycodeList(w);
}

int
#ifdef _NO_PROTO
_XmTranslateKPKeySym (keysym, buffer, nbytes)
        KeySym keysym;
        char *buffer;
        int nbytes;
#else
_XmTranslateKPKeySym (KeySym keysym, char *buffer, int nbytes)
#endif /* _NO_PROTO */
{
	register unsigned char  c;
	unsigned long 		hiBytes;

	/* We MUST only be passed keypad (XK_KP_mumble) keysyms */
	/* if X keysym, convert to ascii by grabbing low 7 bits */
	hiBytes = keysym >> 8;

	if (keysym == XK_KP_Space)
		c = XK_space & 0x7F; /* patch encoding botch */
	else if (hiBytes == 0xFF)
		c = (unsigned char)keysym & 0x7F; /* Wyoming 64-bit fix */ 
	else
		c = (unsigned char)keysym & 0xFF; /* Wyoming 64-bit fix */ 
	buffer[0] = c;

	return 1;
}

void
#ifdef _NO_PROTO
_XmGetKPKeysymToKeycodeList(w)
Widget w;
#else
_XmGetKPKeysymToKeycodeList(Widget w)
#endif /* _NO_PROTO */
{
  int		i;
  int		num = XtNumber(keypad_table);
  Display	*dpy = XtDisplay(w);
  KeypadInfo	*keypad_info = NULL;

  if (KeyMappingCache == (XContext) NULL)
	KeyMappingCache = XUniqueContext();

  if (XFindContext(dpy, (XID)keypad_table,
     KeyMappingCache, (XPointer*)&keypad_info) == XCNOENT){
	keypad_info = (KeypadInfo *)XtCalloc(num, sizeof(KeypadInfo));
	for (i =0; i < num; i++)
		keypad_info[i].keysym = keypad_table[i].keysym;
	XtAddEventHandler(w, NoEventMask, True, MappingEventHandler, NULL);
	XtAddCallback(w, XmNdestroyCallback, DisplayDestroy, NULL);
	XSaveContext(dpy, (XID)keypad_table, KeyMappingCache,
			(XPointer)keypad_info);
  }

  if (keypad_info == NULL)
	return;

  for (i = 0; i < num; i++)
	if (keypad_info[i].keycodelist != NULL) {
		XtFree((char *)keypad_info[i].keycodelist);
		keypad_info[i].keycodelist = NULL;
		keypad_info[i].keycount = 0;
	}

  for (i = 0; i < num; i++)
	XtKeysymToKeycodeList(dpy, keypad_info[i].keysym,
		&keypad_info[i].keycodelist, &keypad_info[i].keycount);
}

Boolean
#ifdef _NO_PROTO
_XmIsKPKey(dpy, keycode, keysym_return)
Display *dpy;
KeyCode keycode;
KeySym  *keysym_return;
#else
_XmIsKPKey(Display *dpy, KeyCode keycode, KeySym *keysym_return)
#endif /* _NO_PROTO */
{
   int		i, j;
   KeypadInfo	*keypad_info = NULL;

   if (XFindContext(dpy, (XID)keypad_table, KeyMappingCache,
	(XPointer*)&keypad_info) || keypad_info == NULL)
      return False; /* previously returned no value */

   for (i = 0; i < XtNumber(keypad_table); i++)
	for (j = 0; j < keypad_info[i].keycount; j++)
		if (keypad_info[i].keycodelist[j] == keycode) {
			*keysym_return = keypad_info[i].keysym;
			return True;
                }
  return False;
}

void
#ifdef _NO_PROTO
_XmGetModifierMapping(w)
Widget w;
#else
_XmGetModifierMapping(Widget w)
#endif /* _NO_PROTO */
{
   int 			i, j, start_index;
   XModifierKeymap 	*modifier_map;
   static Modifiers 	mod_masks[] = { None, Mod1Mask, Mod2Mask,
                                        Mod3Mask, Mod4Mask, Mod5Mask };
   int			num = XtNumber(mod_info);
   Display		*dpy = XtDisplay(w);
   ModifierInfo		*mods = NULL;

   if (ModMappingCache == (XContext) NULL)
	ModMappingCache = XUniqueContext();

   if (XFindContext(dpy, (XID)mod_info,
	ModMappingCache, (XPointer*)&mods) == XCNOENT)  {
	mods = (ModifierInfo *)XtCalloc(num, sizeof(ModifierInfo));
	for (i =0; i < num; i++)
		memcpy((void *)&mods[i], (void *)&mod_info[i],
			sizeof(ModifierInfo));
        XSaveContext(dpy, (XID)mod_info, ModMappingCache, (XPointer)mods);
   }

   if ( mods == NULL)
      return;

   /*
    * Get the keycodes corresponding to the various keysyms
    */
   for (i = 0; i < num; i++) {
	mods[i].left_keycode = XKeysymToKeycode(dpy, mods[i].left_keysym);
	mods[i].right_keycode = XKeysymToKeycode(dpy,mods[i].right_keysym);
   }

   /*
    * Read the server's modifier mapping
    */
   modifier_map = XGetModifierMapping(dpy);

   /*
    * Skip shift, lock and control as they have their own 'standard'
    * modifier masks
    */
   start_index = modifier_map->max_keypermod * Mod1MapIndex;

   for (i = start_index; i < modifier_map->max_keypermod * 8; i++) {
	KeyCode kc = modifier_map->modifiermap[i];
	int this_mod = ((i - start_index) / modifier_map->max_keypermod) + 1;

	if (!kc) continue;

	for (j = 0; j < XtNumber(mod_info); j++) {
		if ((kc == mods[j].left_keycode ||
			kc == mods[j].right_keycode))
		mods[j].modifier = mod_masks[this_mod];
	}
   }
   XFreeModifiermap(modifier_map);
}

Modifiers
#ifdef _NO_PROTO
_XmGetModifierBinding(dpy, modifier_type)
Display *dpy;
ModifierType modifier_type;
#else
_XmGetModifierBinding(Display *dpy, ModifierType modifier_type)
#endif /* _NO_PROTO */
{
   ModifierInfo        *mods = NULL;

   if (XFindContext(dpy, (XID)mod_info, ModMappingCache, (XPointer*)&mods) ||
       mods == NULL)
      return 0; /* previously returned no value */

   return mods[modifier_type].modifier;
}
#endif /* SUN_MOTIF */
/* Phew!, prabhat */


/*ARGSUSED*/
static Boolean 
CvtStringToVirtualBinding(Display    *dpy,
			  XrmValuePtr args,
			  Cardinal   *num_args,
			  XrmValuePtr fromVal,
			  XrmValuePtr toVal,
			  XtPointer  *closure_ret )
{
    char 	 *str = (char *)fromVal->addr;
    XKeyEvent	  event;
    int		  count, tmp;
    int		 *eventTypes;
    KeySym       *keysyms;
    unsigned int *modifiers;
    int		  j, ct;
    int		  codes_per_sym;
    KeyCode	  minK;
    Modifiers	  used_mods;


    /* solaris 2.6 motif diff bug 1206588, 4044583 */
    if (str)
    {
        Boolean firstBracket = True;
        int i;

        /* Replace brackets with '_' since we had to do this when
           we filled the Xrm database. (Brackets are not legal in
           an X resource name. */
        for(i=0; i<strlen(str); i++)
            if ((str[i]=='_') && (firstBracket))
            {
                str[i] = '<';
                firstBracket = False;
            }
            else if (str[i] == '_')
            {
                str[i] = '>';
                break;
            }
    }
    /* END solaris 2.6 motif diff bug 1206588, 4044583 */


    /* Lookup codes_per_sym, and let Xt cache the result instead of */
    /* always downloading a new copy with XGetKeyboardMapping(). */
    /* This also initializes Xt's per-display data structures, so */
    /* we can use XtTranslateKey() instead of XLookupString(). */
    (void) XtGetKeysymTable(dpy, &minK, &codes_per_sym);

    count = _XmMapKeyEvents(str, &eventTypes, &keysyms, &modifiers);
    if (count > 0)
      {
	Boolean fini;

	for (tmp = 0; tmp < count; tmp++)
	  {
	    fini = False;  

	    /*
	     * Here's a nasty bit of code. If some vendor defines one of
	     * the standard modifiers to be the mode switch mod, the
	     * keysym returned by XtTranslateKey is the mode-shifted 
	     * one. This may or may not be bogus, but we have to live
	     * with it :-(. Soo, we need to translate the keysym to a
	     * keycode, then ask someone to translate the combo for us.
	     */
	    event.display = dpy;
	    event.keycode = XKeysymToKeycode(dpy, keysyms[tmp]);
	    
	    /*
	     * In case the guy specifies a symbol that is modified (like
	     * HP's Del which is <shift><escape>), we'll find the implied
	     * modifers and 'OR' it together with the explicitly stated
	     * modifers.
	     */
	    event.state = 0;
	    if (XKeycodeToKeysym(dpy, event.keycode, 0) != keysyms[tmp])
	      for (j = 1; j < codes_per_sym; j++)
		if (XKeycodeToKeysym(dpy, event.keycode, j) == keysyms[tmp])
		  {

                  /* 
		   * Gross Hack for Hobo keyboard .. 
		   * Assumptions: 
		   * 	1. Hobo keyboard has XK_Return  as the first entry
		   *		and XK_KP_Enter as the 4th entry in its
		   *            keycode to keysym key map.
		   *	2. This fix is only designed to work for the precise 
		   *            combination of the Sun server with vendor 
		   *            string "Sun Microsystems, Inc." and the Hobo 
		   *            keyboard, as the fix assumes knowledge of the 
		   *            server keycode to keysym key map.
		   */

		    if ((keysyms[tmp] == XK_KP_Enter) &&
                        (j == 4) &&
                        (XKeycodeToKeysym(dpy, event.keycode, 0) 
			    == XK_Return) &&
			(strcmp("Sun Microsystems, Inc.", ServerVendor(dpy)) 
			 == 0)) 
		    {
			fini = True;
		    } else
			event.state = 1 << (j-1);

		    break;
		  }

	    if (!fini) {
		event.state |= modifiers[tmp];
		XtTranslateKey(dpy, event.keycode, event.state, &used_mods,
			       keysyms + tmp);
	    }
	  }
	
	/* Fail if insufficient storage was provided. */
	if ((toVal->addr != NULL) &&
	    (toVal->size < sizeof(XmKeyBindingRec) * count))
	  {
	    toVal->size = sizeof(XmKeyBindingRec) * count;

	    XtFree((char*) eventTypes);
	    XtFree((char*) keysyms);
	    XtFree((char*) modifiers);
	    return False;
	  }

	/* Allocate storage if none was provided. */
	toVal->size = sizeof(XmKeyBindingRec) * count;
	if (toVal->addr == NULL)
	  toVal->addr = XtMalloc(toVal->size);

	/* Copy the data. */
	for (tmp = 0; tmp < count; tmp++)
	  {
	    ((XmKeyBinding) toVal->addr)[tmp].keysym = keysyms[tmp];
	    ((XmKeyBinding) toVal->addr)[tmp].modifiers = modifiers[tmp];
	  }

	XtFree((char*) eventTypes);
	XtFree((char*) keysyms);
	XtFree((char*) modifiers);
	return True;
      }

    /* The value supplied could not be converted. */
    XtDisplayStringConversionWarning(dpy, str, XmRVirtualBinding);
    return False;
}

static XmVKeyBindingRec *
FillBindingsFromDB(Display       *dpy,
		   XrmDatabase    rdb,
		   Cardinal      num_keys,
                   String bindStrings [])
{
  XmVKeyBindingRec  *virtBinding, *keys;
  XrmName 	    xrm_name[2];
  XrmClass 	    xrm_class[2];
  XrmRepresentation rep_type;
  XrmValue 	    value;
  Cardinal	    vk_num;
  XrmQuark	    XmQVirtualBinding = XrmPermStringToQuark(XmRVirtualBinding);
  XrmQuark	    XmQString         = XrmPermStringToQuark(XmRString);
  Cardinal	    i;
  XmIndexedVirtualKeysymRec indexVirtSyms [XtNumber (virtualKeysyms) ];
  XmIndexedVirtualKeysym virtKey;



  /* Solaris 2.6 motif diff bug 1206588, 4044583 */
  /* Get a list of virtual keysyms sorted alphabetically. Keep track
     of the original index. */
  for (i = 0; i < XtNumber(virtualKeysyms); i++)
  {
     indexVirtSyms [i].virtRec = &virtualKeysyms [i];
     indexVirtSyms [i].index = i;
  }
  qsort ( (void *) indexVirtSyms, XtNumber(virtualKeysyms),
  sizeof (*indexVirtSyms), virtKeySymsCmp);

  xrm_class[0] = XrmPermStringToQuark(XmRString);
  xrm_class[1] = 0;

  keys = (XmVKeyBinding) XtMalloc(sizeof(XmVKeyBindingRec) * num_keys);


  for (virtBinding = keys, i = 0; i < num_keys; virtBinding++, i++)
  {
      xrm_name[0] = XrmStringToQuark(bindStrings [i]);
      xrm_name[1] = 0;
      if (XrmQGetResource(rdb, xrm_name, xrm_class, &rep_type, &value ))
      {
          if (rep_type == XmQString) 
          {
              XmIndexedVirtualKeysymRec keyrec;
              XmVirtualKeysymRec keyvrec;
              keyrec.virtRec = &keyvrec;
              keyrec.virtRec->name = (String) value.addr;

              if ( (virtKey = (XmIndexedVirtualKeysym) bsearch
                        ( (void *) &keyrec, (void *) indexVirtSyms,
                        XtNumber(virtualKeysyms), sizeof (*indexVirtSyms),
                        virtKeySymsCmp) ) )
              {
                  XrmValue toVal;
                  toVal.addr = (XPointer)virtBinding;
                  toVal.size = sizeof(XmVKeyBindingRec);
                  value.addr = (XPointer)bindStrings [i];
                  value.size = strlen(bindStrings [i] + 1);
                  if (!XtCallConverter(dpy,
                                          CvtStringToVirtualBinding,
                                          NULL,
                                          0,
                                          &value,
                                          &toVal,
                                          (XtCacheRef*)NULL))
                            *virtBinding = nullBinding;
                  else
                  {
                            virtBinding->index = virtKey->index;
                            virtBinding->virtkey = virtKey->virtRec->keysym;
                  }
               }
          }
          else
               *virtBinding = nullBinding;
      }
      else
          *virtBinding = nullBinding;
   }

   return(keys);
  /* END Solaris 2.6 motif diff bug 1206588, 4044583 */
}


static Boolean 
GetBindingsProperty(Display *display,
		    String   property,
		    String  *binding)
{
  char		*prop = NULL;
  Atom		actual_type;
  int		actual_format;
  unsigned long	num_items;
  unsigned long	bytes_after;


  if ( binding == NULL ) 
    return False;

  XGetWindowProperty (display, 
		      RootWindow(display, 0),
		      XInternAtom(display, property, FALSE),
		      0, (long)1000000,
		      FALSE, XA_STRING,
		      &actual_type, &actual_format,
		      &num_items, &bytes_after,
		      (unsigned char **) &prop);

  if ((actual_type != XA_STRING) ||
      (actual_format != 8) || 
      (num_items == 0))
    {
      if (prop != NULL) 
	XFree(prop);
      return False;
    }
  else
    {
      *binding = prop;
      return True;
    }
}
	   
/*
 * This routine is called by the XmDisplay Initialize method to set
 * up the virtual bindings table, XtKeyProc, and event handler.
 */
void 
_XmVirtKeysInitialize(Widget widget)
{
   XmDisplay   xmDisplay=(XmDisplay) widget;
   Display *   dpy=XtDisplay(xmDisplay);
   XrmDatabase keyDB;
   String      bindingsString;
   String      fallbackString=NULL;
   String      swappedbindingsString=NULL;
   String *    bindingStringsNames;
   int         nbindingStrings, i;
   Boolean     needXFree=False;

   if (!XmIsDisplay(widget))
      return;

   bindingsString = xmDisplay->display.bindingsString;
   xmDisplay->display.lastKeyEvent = XtNew(XKeyEvent);
   memset((void *)(xmDisplay->display.lastKeyEvent), 0, sizeof(XKeyEvent));


   if (bindingsString == NULL)         /* If XmNdefaultVirtualBindings not set,  */
    {                                  /* try _MOTIF_BINDINGS                    */
      if (GetBindingsProperty(XtDisplay(xmDisplay), "_MOTIF_BINDINGS",
                              &bindingsString) == True)
       {
         needXFree = True;
       }
      else if (GetBindingsProperty(XtDisplay(xmDisplay), "_MOTIF_DEFAULT_BINDINGS",
                                   &bindingsString) == True)
       {
         needXFree = True;
       }
      else                             /* else property not set, find a useful   */
       {                               /* fallback                               */
         _XmVirtKeysLoadFallbackBindings(XtDisplay(xmDisplay), &fallbackString);
         bindingsString = fallbackString;
       }
    }

   swap_tokens(bindingsString, &swappedbindingsString, &bindingStringsNames,
               &nbindingStrings);

   XtSetTypeConverter(XmRString, XmRVirtualBinding, CvtStringToVirtualBinding,
                      NULL, 0, XtCacheNone, (XtDestructor)NULL);

   keyDB = XrmGetStringDatabase(swappedbindingsString);

   xmDisplay->display.num_bindings = nbindingStrings;
   xmDisplay->display.bindings = FillBindingsFromDB(XtDisplay(xmDisplay), keyDB,
                                                    xmDisplay->display.num_bindings,
                                                    bindingStringsNames);

   XrmDestroyDatabase(keyDB);
   if (needXFree)
      XFree(bindingsString);
   if (fallbackString)
      XtFree(fallbackString);

   for (i=0; i<nbindingStrings; i++)
      XtFree (bindingStringsNames[i]);
   XtFree((char*)bindingStringsNames);
   XtFree(swappedbindingsString);


   XtSetKeyTranslator(dpy, (XtKeyProc)XmTranslateKey);
}


/*
 * This routine is called by the XmDisplay Destroy method to free
 * up the virtual bindings table.
 */
void 
_XmVirtKeysDestroy(Widget widget)
{
  XmDisplay xmDisplay = (XmDisplay) widget;

  XtFree((char*)xmDisplay->display.lastKeyEvent);
  XtFree((char*)xmDisplay->display.bindings);
}

static void 
FindVirtKey(Display *dpy,
	    KeyCode keycode,
	    Modifiers modifiers,
	    Modifiers *modifiers_return,
	    KeySym *keysym_return )
{
  Cardinal      i;
  XmDisplay     xmDisplay = (XmDisplay) XmGetXmDisplay( dpy);
  XmVKeyBinding keyBindings = xmDisplay->display.bindings;
  KeyCode       min_kcode;
  int           ks_per_kc;
  KeySym       *ks_table = XtGetKeysymTable( dpy, &min_kcode, &ks_per_kc);
  KeySym       *kc_map = &ks_table[(keycode - min_kcode) * ks_per_kc];
  Modifiers     EffectiveSMMask = EffectiveStdModMask( dpy, kc_map, ks_per_kc);
  /* Solaris 2.6 motif diff bug 4085003 - 1 line */
  Modifiers effective_Xt_modifiers ;
  
  /* Bug Id : 4106529, numeric keypad not working */
  #ifdef SUN_MOTIF
      Modifiers num_lock, mode_switch;
  #endif /* SUN_MOTIF */
  Modifiers eventMods = (Modifiers)(xmDisplay->display.lastKeyEvent->state);

  /* Get the modifiers from the actual event */
  Modifiers VirtualStdMods = 0;
  Modifiers StdModMask;


/* START Bug Id : 4106529, numeric keypad not working */
#ifdef SUN_MOTIF

    num_lock = _XmGetModifierBinding(dpy, NumLock);
    mode_switch = _XmGetModifierBinding(dpy, ModeSwitch);

    *modifiers_return |= num_lock;
    if((num_lock & eventMods) &&
        !(~num_lock & ~LockMask & ~mode_switch & eventMods))
        (void)_XmIsKPKey(dpy, xmDisplay->display.lastKeyEvent->keycode,
                         keysym_return);
#endif /* SUN_MOTIF */
/* END Bug Id : 4106529, numeric keypad not working */

  for (i = 0; i < xmDisplay->display.num_bindings; i++)
    {
      unsigned j = ks_per_kc;
      KeySym vks = keyBindings[i].keysym;
      
      if (vks)
	{
	  while (j--)
	    {
	      /* Want to walk through keymap (containing all possible
	       * keysyms generated by this keycode) to compare against
	       * virtual key keysyms.  Any keycode that can possibly
	       * generate a virtual keysym must be sure to return all
	       * modifiers that are in the virtual key binding, since
	       * this means that those modifiers are now part of the
	       * "standard modifiers" for this keycode.  (A "standard
	       * modifier" is a modifier that can affect which keysym
	       * is generated from a particular keycode.)
	       */
	      if ((j == 1)  &&  (kc_map[j] == NoSymbol))
		{   
		  KeySym uc, lc;
		  
		  XtConvertCase( dpy, kc_map[0], &lc, &uc);
		  if ((vks == lc)  ||  (vks == uc))
		    VirtualStdMods |= keyBindings[i].modifiers;
		  break;
		} 
	      else if (vks == kc_map[j])
		{
		  /* The keysym generated by this keycode can possibly
		   * be influenced by the virtual key modifier(s), so must
		   * add the modifier(s) associated with this virtual
		   * key to the returned list of "standard modifiers".
		   * The Intrinsics requires that the set of modifiers
		   * returned by the keyproc is constant for a given
		   * keycode.
		   */
		  VirtualStdMods |= keyBindings[i].modifiers;
		  break;
		}
	    } 
	}
    }
  
  /* Don't want to return standard modifiers that do not
   * impact the keysym selected for a particular keycode,
   * since this blocks matching of translation productions
   * which use ":" style translations with the returned
   * standard modifier in the production.  The ":" style
   * of production is essential for proper matching of
   * Motif translations (PC numeric pad, for instance).
   *
   * Recent fixes to the Intrinsics should have included this
   * change to the set of standard modifiers returned from
   * the default key translation routine, but could not be
   * done for reasons of backwards compatibility (which is
   * not an issue for Xm, since we do not export this facility).
   * So, we do the extra masking here after the return from
   * the call to XtTranslateKey.
   */
  *modifiers_return &= EffectiveSMMask;
  /* Solaris 2.6 motif diff bug 4085003 - 1 line */
  effective_Xt_modifiers = *modifiers_return;
  
  /* Modifiers present in the virtual binding table for the
   * keysyms associated with this keycode, which are or might
   * have been used to change the keysym generated by this
   * keycode (to a virtual keysym), must be included in the
   * returned set of standard modifiers.  Remember that "standard
   * modifiers" for a keycode are those modifiers that can affect
   * which keysym is generated by that keycode.
   */
  *modifiers_return |= VirtualStdMods;
  
  /* Effective standard modifiers that are set in the event
   * will be 0 in the following bit mask, which will be used
   * to collapse conflicting modifiers in the virtual
   * key binding table, as described below.
   */
  /* Bug : 4106529, added eventMods */
  StdModMask = ~(modifiers & eventMods & EffectiveSMMask);
  
  for (i = 0; i < xmDisplay->display.num_bindings; i++)
    {
      XmVKeyBinding currBinding = &keyBindings[i];
      KeySym vks = currBinding->keysym;

        Modifiers overloaded_mods = currBinding->modifiers
                                                     & effective_Xt_modifiers ;
        /* Solaris 2.6 motif diff bug 4085003 8 lines */
        KeySym effective_keysym;
        KeySym uc, lc;

        XtConvertCase( dpy, vks, &lc, &uc) ;
        if(    lc != uc    )
          {
            overloaded_mods |= (ShiftMask | LockMask) & effective_Xt_modifiers;
          }


        /* Note that when a virtual key binding table uses a modifier
         * that is treated by the Intrinsics as a standard modifier,
         * we have a semantic collision.  What to do?  Must disambiguate
         * in favor of one or the other.  The VTS assumes that any
         * specified virtual binding table modifier dominates, so even
         * though this is not the appropriate answer in terms of
         * consistency with the Xt keyboard model, we let the virtual
         * key table modifier dominate and reprocess the Xt translation
         * without the virtually preempted modifier to get the keysym
         * for use in matching the rhs of the virtual keysym table.
         */
        if(    overloaded_mods    )
          {
            Modifiers dummy ;
            XtTranslateKey( dpy, keycode, (modifiers & ~overloaded_mods),
                                                   &dummy, &effective_keysym) ;
          }
        else
          {
            effective_keysym = *keysym_return ;
          }
        /* END Solaris 2.6 motif diff bug 4085003 */
      
      /* The null binding should not be interpreted as a match
       * keysym is zero (e.g. pre-edit terminator)
       */
      /* Those modifiers that are effective standard modifiers and
       * that are set in the event will be ignored in the following
       * conditional (i.e., the state designated in the virtual key
       * binding will not be considered), since these modifiers have
       * already had their affect in the determination of the value
       * of *keysym_return.  This allows matching that is consistent
       * with industry-standard interpretation for keys such as
       * those of the PC-style numeric pad.  This apparent loss of
       * binding semantics is an unavoidable consequence of specifying
       * a modifier in the virtual binding table that is already being
       * used to select one of several keysyms associated with a
       * particular keycode (usually as printed on the keycap).
       * The disambiguation of the collapsing of key bindings
       * is based on "first match" in the virtual key binding table.
       */
      /* Solaris 2.6 motif diff bug 4085003 - 1 line */
      if (vks && (vks == effective_keysym) &&
	  ((currBinding->modifiers & StdModMask) ==
	   (modifiers & eventMods & VirtualStdMods & StdModMask))) /* Bug 4106529, added eventMods */
	{
          /* Solaris 2.6 motif diff bug 1206588, 4044583 1 line */
	  *keysym_return = virtualKeysyms[currBinding->index].keysym;
	  break;
	}
    }
}

static Modifiers
EffectiveStdModMask(Display *dpy,
		    KeySym *kc_map,
		    int ks_per_kc)
{
  /* This routine determines which set of modifiers can possibly
   * impact the keysym that is generated by the keycode associated
   * with the keymap passed in.  The basis of the algorithm used
   * here is described in section 12.7 "Keyboard Encoding" of the
   * R5 "Xlib - C Language X Interface" specification.
   */
  KeySym uc;
  KeySym lc;
  
  /* Since the group modifier can be any of Mod1-Mod5 or Control, we will
   * return all of these bits if the group modifier is found to be effective.
   * Lock will always be returned (for backwards compatibility with
   * productions assuming that Lock is always a "don't care" modifier
   * for non-alphabetic keys).  Shift will be returned unless it has
   * no effect on the selection of keysyms within either group.
   */
  Modifiers esm_mask = (Mod5Mask | Mod4Mask | Mod3Mask | Mod2Mask | Mod1Mask |
			ControlMask | LockMask | ShiftMask);
  switch (ks_per_kc)
    {
    default:	/* CR 8799: Ignore non-standard modifier groups. */
    case 4:
      if (kc_map[3] != NoSymbol)
	{
	  /* The keysym in position 4 is selected when the group
	   * modifier is set and the Shift (or Lock) modifier is
	   * set, so both Shift/Lock and the group modifiers are
	   * all "effective" standard modifiers.
	   */
	  break;
	} 
    case 3:
      if (kc_map[2] == NoSymbol)
	{
	  /* Both Group 2 keysyms are NoSymbol, so the group
	   * modifier has no effect; only Shift and Lock remain
	   * as possible effective modifiers.
	   */
	  esm_mask = ShiftMask | LockMask;
	}
      else
	{
	  XtConvertCase( dpy, kc_map[2], &lc, &uc);
	  if (lc != uc)
	    {   
	      /* The Group 2 keysym is case-sensitive, so group
	       * modifiers and Shift/Lock modifiers are effective.
	       */
	      break;
	    }
	} 
      /* At this fall-through, the group modifier bits have been
       * decided, while the case is still out on Shift/Lock.
       */
    case 2:
      if (kc_map[1] != NoSymbol)
	{
	  /* Shift/Lock modifier selects keysym from Group 1,
	   * so leave those bits in the mask.  The group modifier
	   * was determined above, so leave those bits in the mask.
	   */
	  break;
	} 
    case 1:
      if (kc_map[0] != NoSymbol)
	{
	  XtConvertCase( dpy, kc_map[0], &lc, &uc);
	  if (lc != uc)
	    {
	      /* The Group 1 keysym is case-sensitive, so Shift/Lock
	       * modifiers are effective.
	       */
	      break;
	    }
	} 
      /* If we did not break out of the switch before this, then
       * the Shift modifier is not effective; mask it out.
       */
      esm_mask &= ~ShiftMask;
    case 0:
      break;
    } 

  return esm_mask;
}

void
XmTranslateKey(Display     *dpy,
#if NeedWidePrototypes
	       unsigned int keycode,
#else
	       KeyCode      keycode,
#endif /* NeedWidePrototypes */
	       Modifiers    modifiers,
	       Modifiers   *modifiers_return,
	       KeySym      *keysym_return )
{
  _XmDisplayToAppContext(dpy);
  _XmAppLock(app);
  XtTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return);

  FindVirtKey(dpy, keycode, modifiers, modifiers_return, keysym_return);
  _XmAppUnlock(app);
}

int
XmeVirtualToActualKeysyms(Display      *dpy,
			  KeySym        virtKeysym,
			  XmKeyBinding *actualKeyData)
{
  int           matches;
  Cardinal      index;
  XmDisplay     xmDisplay = (XmDisplay)XmGetXmDisplay (dpy);
  XmVKeyBinding keyBindings = xmDisplay->display.bindings;
  _XmDisplayToAppContext(dpy);
  
  _XmAppLock(app);
  /* Initialize the return parameters. */
  *actualKeyData = NULL;

  /* Count the number of matches. */
  matches = 0;
  for (index = 0; index < xmDisplay->display.num_bindings; index++)
    if (keyBindings[index].virtkey == virtKeysym)
      matches++;

  /* Allocate the return array. */
  if (matches > 0)
    {
      *actualKeyData = (XmKeyBinding) 
	XtMalloc(matches * sizeof(XmKeyBindingRec));

      matches = 0;
      for (index = 0; index < xmDisplay->display.num_bindings; index++)
	if (keyBindings[index].virtkey == virtKeysym)
	  {
	    (*actualKeyData)[matches].keysym = keyBindings[index].keysym;
	    (*actualKeyData)[matches].modifiers = keyBindings[index].modifiers;
	    matches++;
	  }
    }

  _XmAppUnlock(app);
  return matches;
}

Boolean 
_XmVirtKeysLoadFileBindings(char   *fileName,
			    String *binding )
{
  FILE *fileP;
  size_t offset = 0; /* Wyoming 64-bit fix */ 
  size_t count; /* Wyoming 64-bit fix */ 
  
  if ((fileP = fopen (fileName, "r")) != NULL) 
    {
      *binding = NULL;
      do {
	*binding = XtRealloc (*binding, offset + BUFFERSIZE);
	count = fread (*binding + offset, 1, BUFFERSIZE, fileP); /* Wyoming 64-bit fix */ 
	offset += count;
      } while (count == BUFFERSIZE);
      (*binding)[offset] = '\0';
      
      /* trim unused buffer space */
      *binding = XtRealloc (*binding, offset + 1);
      
      fclose (fileP);
      return True;
    }

  return False;
}

static void 
LoadVendorBindings(Display *display,
		   char    *path,
		   FILE    *fp,
		   String  *binding )
{
  char buffer[MAXLINE];
  char *bindFile;
  char *vendor;
  char *vendorV;
  char *ptr;
  char *start;
  
  vendor = ServerVendor(display);
  vendorV = XtMalloc (strlen(vendor) + 20); /* assume rel.# is < 19 digits */
  sprintf (vendorV, "%s %d", vendor, VendorRelease(display));
  
  while (fgets (buffer, MAXLINE, fp) != NULL) 
    {
      ptr = buffer;
      while (*ptr != '"' && *ptr != '!' && *ptr != '\0') 
	ptr++;
      if (*ptr != '"') 
	continue;

      start = ++ptr;
      while (*ptr != '"' && *ptr != '\0') 
	ptr++;
      if (*ptr != '"') 
	continue;

      *ptr = '\0';
      if ((strcmp (start, vendor) == 0) || (strcmp (start, vendorV) == 0)) 
	{
	  ptr++;
	  while (isspace((unsigned char)*ptr) && *ptr) 
	    ptr++;
	  if (*ptr == '\0') 
	    continue;

	  start = ptr;
	  while (!isspace((unsigned char)*ptr) && *ptr != '\n' && *ptr)
	    ptr++;
	  *ptr = '\0';

	  bindFile = _XmOSBuildFileName (path, start);
	  if (_XmVirtKeysLoadFileBindings (bindFile, binding)) 
	    {
	      XtFree (bindFile);
	      break;
	    }
	  XtFree (bindFile);
	}
    }

  XtFree (vendorV);
}

int 
_XmVirtKeysLoadFallbackBindings(Display	*display,
				String	*binding )
{
  XmConst XmDefaultBindingStringRec *currDefault;
  int i;
  FILE *fp;
  char *homeDir;
  char *fileName;
  char *bindDir;
  static XmConst char xmbinddir_fallback[] = XMBINDDIR_FALLBACK;
  
  *binding = NULL;
  
  /* Load .motifbind - necessary, if mwm and xmbind are not used */
  homeDir = XmeGetHomeDirName();
  fileName = _XmOSBuildFileName(homeDir, MOTIFBIND);
  _XmVirtKeysLoadFileBindings(fileName, binding);
  XtFree(fileName);
  
  /* Look for a match in the user's xmbind.alias */
  if (*binding == NULL) 
    {
      fileName = _XmOSBuildFileName (homeDir, XMBINDFILE);
      if ((fp = fopen (fileName, "r")) != NULL) 
	{
	  LoadVendorBindings (display, homeDir, fp, binding);
	  fclose (fp);
	}
      XtFree (fileName);
    }
  
  if (*binding != NULL) 
    {
      /* Set the user property for future Xm applications. */
      XChangeProperty (display, RootWindow(display, 0),
		       XInternAtom (display, XmS_MOTIF_BINDINGS, False),
		       XA_STRING, 8, PropModeReplace,
		       (unsigned char *)*binding, (int)strlen(*binding)); /* Wyoming 64-bit fix */ 
      return 0;
    }
  
  /* Look for a match in the system xmbind.alias */
  if (*binding == NULL) 
    {
      if ((bindDir = getenv(XMBINDDIR)) == NULL)
	bindDir = (char*) xmbinddir_fallback;
      fileName = _XmOSBuildFileName (bindDir, XMBINDFILE);
      if ((fp = fopen (fileName, "r")) != NULL) 
	{
	  LoadVendorBindings (display, bindDir, fp, binding);
	  fclose (fp);
	}
      XtFree (fileName);
    }
  
  /* Check hardcoded fallbacks (for 1.1 bc) */
  if (*binding == NULL) 
    {
      for (i = 0, currDefault = fallbackBindingStrings;
	   i < XtNumber(fallbackBindingStrings);
	   i++, currDefault++) 
	{
	  if (strcmp(currDefault->vendorName, ServerVendor(display)) == 0) 
	    {
	      *binding = XtMalloc (strlen (currDefault->defaults) + 1);
	      strcpy (*binding, currDefault->defaults);
	      break;
	    }
	}
    }
  
  /* Use generic fallback bindings */
  if (*binding == NULL) 
    {
      *binding = XtMalloc (strlen (defaultFallbackBindings) + 1);
      strcpy (*binding, defaultFallbackBindings);
    }
  
  /* Set the fallback property for future Xm applications */
  XChangeProperty (display, RootWindow(display, 0),
		   XInternAtom (display, XmS_MOTIF_DEFAULT_BINDINGS, False),
		   XA_STRING, 8, PropModeReplace,
		   (unsigned char *)*binding, (int)strlen(*binding));
  
  return 0;
}


static void
swap_tokens(const String	source,
				String *			destination,
				String **		keyStrings,
				int *				nkeyStrings)
{
   String   ptr, dest, output_string, token1_start;
   size_t   input_length, output_length, max_output_length,
            token1_length, token2_length, token2_offset;
   int      nbr_bindingstrings, max_bindingstrings,
            nbr_chars_to_xfer;
   String * bindingstrings_ptr;
   char     c, copy_char, copy_tokens, parse_state;


   *keyStrings = NULL;
   *nkeyStrings = 0;
   input_length = strlen(source);
   max_output_length = input_length << 1;
   output_length = 0;
   nbr_bindingstrings = 0;
   output_string = dest = (char *)XtMalloc(max_output_length+1);
   bindingstrings_ptr = (String *)XtMalloc(max_output_length);
   max_bindingstrings = max_output_length >> 2;
   copy_char = FALSE;
   copy_tokens = FALSE;

   for (ptr=source, parse_state=GET_TOKEN1_START; *ptr; ptr++)
    {
      c = *ptr;

      switch(parse_state)
       {
        case GET_TOKEN1_START:

         if ((c == '\n') || (c == ' ') || (c == '\t'))
          {
            continue;
          }
         else if (c == '!')
          {
            parse_state = SKIP_LINE;
          }
         else
          {
            token1_start = ptr;
            parse_state = PARSE_TOKEN1;
          }
         break;


        case PARSE_TOKEN1:

         if (c == '\n')
          {
            parse_state = GET_TOKEN1_START;
          }
         else if ((c == ' ') || (c == '\t'))
          {
            token1_length = ptr - token1_start;
            parse_state = GET_COLON;
          }
         else if (c == ':')
          {
            token1_length = ptr - token1_start;
            parse_state = GET_TOKEN2_START;
          }
         break;


        case GET_COLON:

         if (c == '\n')
          {
            parse_state = GET_TOKEN1_START;
          }
         else if (c == ':')
          {
            parse_state = GET_TOKEN2_START;
          }
         break;


        case GET_TOKEN2_START:

         if (c == '\n')
          {
            parse_state = GET_TOKEN1_START;
          }
         else if ((c != ' ') && (c != '\t') && (c != ','))
          {
            ptr--;
            parse_state = PARSE_TOKEN2;
            token2_offset = dest - output_string;
            token2_length = 0;
          }
         break;


        case PARSE_TOKEN2:

         if ((c == '>') || (c == '<'))
          {
            c = '_';
          }

         if ((c == ' ') || (c == '\t'))
          {
            break;
          }
         else if (c == ',')
          {
            copy_tokens = TRUE;
            parse_state = GET_TOKEN2_START;
          }
         else if (c == '\n')
          {
            copy_tokens = TRUE;
            parse_state = GET_TOKEN1_START;
          }
         else
          {
            copy_char = TRUE;
            token2_length++;
          }
         break;


        case SKIP_LINE:

         if (c != '\n')
          {
            break;
          }


        default:

         parse_state = GET_TOKEN1_START;
         break;
       }

      nbr_chars_to_xfer = 0;

      if (copy_char)
       {
         nbr_chars_to_xfer = 1;
       }
      else if (copy_tokens)
       {
         nbr_chars_to_xfer = token1_length + 2;
       }

      if (nbr_chars_to_xfer)
       {
         if ((output_length+nbr_chars_to_xfer) > max_output_length)
          {
            max_output_length += input_length;
            output_string = (char *)XtRealloc(output_string, max_output_length);
            if (!output_string)
             {
               int   i;

               for (i=0; i<nbr_bindingstrings; i++)
                {
                  XtFree(bindingstrings_ptr[i]);
                }
               XtFree((char *)bindingstrings_ptr);
               *nkeyStrings = 0;
               return;
             }
            else
             {
               dest = output_string + output_length;
             }
          }
       }

      if (copy_char)
       {
         *dest++ = c;
         output_length++;
         copy_char = FALSE;
       }
      else if (copy_tokens)
       {
         char *   cp;

         *dest++ = ':';
         memcpy(dest, token1_start, token1_length);
         dest += token1_length;
         *dest++ = '\n';
         output_length += token1_length + 2;

         if ((nbr_bindingstrings < max_bindingstrings) &&
             (cp = (char *)XtMalloc(token2_length + 1)))
          {
            bindingstrings_ptr[nbr_bindingstrings] = cp;
            nbr_bindingstrings++;
            memcpy(cp, output_string+token2_offset, token2_length);
            cp[token2_length] = 0;
          }
         copy_tokens = FALSE;
       }
    }

   *dest = 0;
   *destination = output_string;
   *keyStrings = bindingstrings_ptr;
   *nkeyStrings = nbr_bindingstrings;
}


static int
virtKeySymsCmp(const void *e1, const void *e2)
{
    return (strcmp ( ( (XmIndexedVirtualKeysymRec *)e1)->virtRec->name,
        ( (XmIndexedVirtualKeysymRec *)e2)->virtRec->name) );
}

/* END Solaris 2.6 motif diff bug 1206588, 4044583 */


/* START Bug Id : 4106529, numeric key pad */
/************************************************************************
 *
 *  _XmVirtKeysHandler
 *
 *  This handler provides all kind of magic. It is added to all widgets.
 *
 ************************************************************************/
/* ARGSUSED */
void
_XmVirtKeysHandler(
        Widget widget,
        XtPointer client_data,
        XEvent *event,
        Boolean *dontSwallow )
{
    XmDisplay xmDisplay = (XmDisplay)XmGetXmDisplay (XtDisplay (widget) );
#ifdef SUN_MOTIF
    Modifiers eventMods = (Modifiers)(xmDisplay->display.lastKeyEvent->state);
#endif /* SUN_MOTIF */
    KeyCode keycode;

    if (widget->core.being_destroyed)
      {
          *dontSwallow = False;
          return;
      }

#ifdef SUN_MOTIF
    if((event->type == KeyPress || event->type == KeyRelease)) {
        *(xmDisplay->display.lastKeyEvent) = *((XKeyEvent *)event);
        if((Modifiers)(((XKeyEvent *)event)->state) != eventMods)
                XtSetKeyTranslator (XtDisplay(widget), (XtKeyProc)XmTranslateKey);
    }
#else
    switch( event->type ) {
      case KeyPress:
        *(xmDisplay->display.lastKeyEvent) = *((XKeyEvent *)event);

        /*
         * if keycode is tagged as a modified virtual key, reset
         * the Xt translation manager cache.
         */
        keycode = ((XKeyEvent *)event)->keycode;
        if ((xmDisplay->display.keycode_tag[keycode/8] & (1 << (keycode % 8)))
                                                                        != 0) {
            XtSetKeyTranslator (XtDisplay(widget), (XtKeyProc)XmTranslateKey);
        }
        break;
    }
#endif
}

/* END Bug Id : 4106529, numeric key pad */
