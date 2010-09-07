/*
 * @(#)XWindow.c	1.28 04/01/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Note that the contents of this file were taken from canvas.c 
 * in the old motif-based AWT. 
 */ 

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <ctype.h>

#include <jvm.h>
#include <jni.h>
#include <jlong.h>
#include <jni_util.h>

#include "sun_awt_X11_XWindow.h"

#include "awt_p.h"
#include "awt_GraphicsEnv.h"
#include "awt_AWTEvent.h"

#define XK_KATAKANA
#include <X11/keysym.h>     /* standard X keysyms */
#include <X11/DECkeysym.h>  /* DEC vendor-specific */
#include <X11/Sunkeysym.h>  /* Sun vendor-specific */
#include <X11/ap_keysym.h>  /* Apollo (HP) vendor-specific */
/*
 * #include <X11/HPkeysym.h>    HP vendor-specific
 * I checked HPkeysym.h into the workspace because although 
 * I think it will ship with X11R6.4.2 (and later) on Linux, 
 * it doesn't seem to be in Solaris 9 Update 2.  
 *
 * This is done not only for the hp keysyms, but also to 
 * give us the osf keysyms that are also defined in HPkeysym.h.
 * However, HPkeysym.h is missing a couple of osf keysyms,  
 * so I have #defined them below.
 */
#include "HPkeysym.h"   /* HP vendor-specific */

#include "java_awt_event_KeyEvent.h"
#include "java_awt_event_InputEvent.h"
#include "java_awt_event_MouseEvent.h"
#include "java_awt_event_MouseWheelEvent.h"
#include "java_awt_AWTEvent.h"

/*
 * Two osf keys are not defined in standard keysym.h,
 * /Xm/VirtKeys.h, or HPkeysym.h, so I added them below.
 * I found them in /usr/openwin/lib/X11/XKeysymDB
 */
#ifndef osfXK_Prior
#define osfXK_Prior 0x1004FF55
#endif
#ifndef osfXK_Next
#define osfXK_Next 0x1004FF56
#endif

jfieldID windowID;
jfieldID drawStateID;
jfieldID targetID;
jfieldID graphicsConfigID;

extern jobject currentX11InputMethodInstance;
extern Boolean awt_x11inputmethod_lookupString(XKeyPressedEvent *, KeySym *);

typedef struct KEYMAP_ENTRY {
    jint awtKey;
    KeySym x11Key;
    Boolean mapsToUnicodeChar;
    jint keyLocation;
} KeymapEntry;

/* NB: XK_R? keysyms are for Type 4 keyboards.
 * The corresponding XK_F? keysyms are for Type 5
 *
 * Note: this table must be kept in sorted order, since it is traversed
 * according to both Java keycode and X keysym.  There are a number of
 * keycodes that map to more than one corresponding keysym, and we need
 * to choose the right one.  Unfortunately, there are some keysyms that
 * can map to more than one keycode, depending on what kind of keyboard
 * is in use (e.g. F11 and F12).
 */

KeymapEntry keymapTable[] =
{
    {java_awt_event_KeyEvent_VK_A, XK_a, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_B, XK_b, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_C, XK_c, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_D, XK_d, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_E, XK_e, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F, XK_f, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_G, XK_g, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_H, XK_h, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_I, XK_i, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_J, XK_j, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_K, XK_k, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_L, XK_l, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_M, XK_m, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_N, XK_n, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_O, XK_o, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_P, XK_p, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_Q, XK_q, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_R, XK_r, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_S, XK_s, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_T, XK_t, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_U, XK_u, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_V, XK_v, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_W, XK_w, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_X, XK_x, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_Y, XK_y, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_Z, XK_z, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* TTY Function keys */
    {java_awt_event_KeyEvent_VK_BACK_SPACE, XK_BackSpace, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_TAB, XK_Tab, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_TAB, XK_ISO_Left_Tab, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CLEAR, XK_Clear, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_ENTER, XK_Return, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_ENTER, XK_Linefeed, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAUSE, XK_Pause, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAUSE, XK_F21, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAUSE, XK_R1, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_SCROLL_LOCK, XK_Scroll_Lock, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_SCROLL_LOCK, XK_F23, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_SCROLL_LOCK, XK_R3, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_ESCAPE, XK_Escape, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Other vendor-specific versions of TTY Function keys */
    {java_awt_event_KeyEvent_VK_BACK_SPACE, osfXK_BackSpace, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CLEAR, osfXK_Clear, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_ESCAPE, osfXK_Escape, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Modifier keys */
    {java_awt_event_KeyEvent_VK_SHIFT, XK_Shift_L, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_LEFT},
    {java_awt_event_KeyEvent_VK_SHIFT, XK_Shift_R, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_RIGHT},
    {java_awt_event_KeyEvent_VK_CONTROL, XK_Control_L, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_LEFT},
    {java_awt_event_KeyEvent_VK_CONTROL, XK_Control_R, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_RIGHT},
    {java_awt_event_KeyEvent_VK_ALT, XK_Alt_L, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_LEFT},
    {java_awt_event_KeyEvent_VK_ALT, XK_Alt_R, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_RIGHT},
    {java_awt_event_KeyEvent_VK_META, XK_Meta_L, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_LEFT},
    {java_awt_event_KeyEvent_VK_META, XK_Meta_R, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_RIGHT},
    {java_awt_event_KeyEvent_VK_CAPS_LOCK, XK_Caps_Lock, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Misc Functions */
    {java_awt_event_KeyEvent_VK_PRINTSCREEN, XK_Print, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PRINTSCREEN, XK_F22, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PRINTSCREEN, XK_R2, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CANCEL, XK_Cancel, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_HELP, XK_Help, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_NUM_LOCK, XK_Num_Lock, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},

    /* Other vendor-specific versions of Misc Functions */
    {java_awt_event_KeyEvent_VK_CANCEL, osfXK_Cancel, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_HELP, osfXK_Help, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Rectangular Navigation Block */
    {java_awt_event_KeyEvent_VK_HOME, XK_Home, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_HOME, XK_R7, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_UP, XK_Page_Up, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_UP, XK_Prior, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_UP, XK_R9, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, XK_Page_Down, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, XK_Next, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, XK_R15, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_END, XK_End, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_END, XK_R13, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_INSERT, XK_Insert, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DELETE, XK_Delete, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Keypad equivalents of Rectangular Navigation Block */
    {java_awt_event_KeyEvent_VK_HOME, XK_KP_Home, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_PAGE_UP, XK_KP_Page_Up, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_PAGE_UP, XK_KP_Prior, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, XK_KP_Page_Down, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, XK_KP_Next, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_END, XK_KP_End, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_INSERT, XK_KP_Insert, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_DELETE, XK_KP_Delete, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},

    /* Other vendor-specific Rectangular Navigation Block */
    {java_awt_event_KeyEvent_VK_PAGE_UP, osfXK_PageUp, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_UP, osfXK_Prior, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, osfXK_PageDown, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PAGE_DOWN, osfXK_Next, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_END, osfXK_EndLine, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_INSERT, osfXK_Insert, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DELETE, osfXK_Delete, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Triangular Navigation Block */
    {java_awt_event_KeyEvent_VK_LEFT, XK_Left, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UP, XK_Up, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_RIGHT, XK_Right, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DOWN, XK_Down, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Keypad equivalents of Triangular Navigation Block */
    {java_awt_event_KeyEvent_VK_KP_LEFT, XK_KP_Left, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_KP_UP, XK_KP_Up, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_KP_RIGHT, XK_KP_Right, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_KP_DOWN, XK_KP_Down, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},

    /* Other vendor-specific Triangular Navigation Block */
    {java_awt_event_KeyEvent_VK_LEFT, osfXK_Left, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UP, osfXK_Up, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_RIGHT, osfXK_Right, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DOWN, osfXK_Down, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Remaining Cursor control & motion */
    {java_awt_event_KeyEvent_VK_BEGIN, XK_Begin, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_BEGIN, XK_KP_Begin, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},

    {java_awt_event_KeyEvent_VK_0, XK_0, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_1, XK_1, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_2, XK_2, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_3, XK_3, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_4, XK_4, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_5, XK_5, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_6, XK_6, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_7, XK_7, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_8, XK_8, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_9, XK_9, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_SPACE, XK_space, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_EXCLAMATION_MARK, XK_exclam, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_QUOTEDBL, XK_quotedbl, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_NUMBER_SIGN, XK_numbersign, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DOLLAR, XK_dollar, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_AMPERSAND, XK_ampersand, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_QUOTE, XK_apostrophe, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_LEFT_PARENTHESIS, XK_parenleft, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_RIGHT_PARENTHESIS, XK_parenright, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_ASTERISK, XK_asterisk, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PLUS, XK_plus, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_COMMA, XK_comma, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_MINUS, XK_minus, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PERIOD, XK_period, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_SLASH, XK_slash, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_COLON, XK_colon, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_SEMICOLON, XK_semicolon, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_LESS, XK_less, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_EQUALS, XK_equal, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_GREATER, XK_greater, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_AT, XK_at, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_OPEN_BRACKET, XK_bracketleft, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_BACK_SLASH, XK_backslash, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CLOSE_BRACKET, XK_bracketright, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CIRCUMFLEX, XK_asciicircum, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UNDERSCORE, XK_underscore, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_BACK_QUOTE, XK_grave, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_BRACELEFT, XK_braceleft, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_BRACERIGHT, XK_braceright, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_INVERTED_EXCLAMATION_MARK, XK_exclamdown, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Remaining Numeric Keypad Keys */
    {java_awt_event_KeyEvent_VK_NUMPAD0, XK_KP_0, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD1, XK_KP_1, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD2, XK_KP_2, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD3, XK_KP_3, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD4, XK_KP_4, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD5, XK_KP_5, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD6, XK_KP_6, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD7, XK_KP_7, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD8, XK_KP_8, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_NUMPAD9, XK_KP_9, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_SPACE, XK_KP_Space, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_TAB, XK_KP_Tab, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_ENTER, XK_KP_Enter, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_EQUALS, XK_KP_Equal, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_EQUALS, XK_R4, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_MULTIPLY, XK_KP_Multiply, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_MULTIPLY, XK_F26, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_MULTIPLY, XK_R6, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_ADD, XK_KP_Add, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_SEPARATOR, XK_KP_Separator, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_SUBTRACT, XK_KP_Subtract, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_SUBTRACT, XK_F24, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_DECIMAL, XK_KP_Decimal, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_DIVIDE, XK_KP_Divide, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_DIVIDE, XK_F25, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},
    {java_awt_event_KeyEvent_VK_DIVIDE, XK_R5, TRUE, java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD},

    /* Function Keys */
    {java_awt_event_KeyEvent_VK_F1, XK_F1, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F2, XK_F2, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F3, XK_F3, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F4, XK_F4, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F5, XK_F5, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F6, XK_F6, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F7, XK_F7, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F8, XK_F8, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F9, XK_F9, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F10, XK_F10, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F11, XK_F11, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F12, XK_F12, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Sun vendor-specific version of F11 and F12 */
    {java_awt_event_KeyEvent_VK_F11, SunXK_F36, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_F12, SunXK_F37, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* X11 keysym names for input method related keys don't always
     * match keytop engravings or Java virtual key names, so here we
     * only map constants that we've found on real keyboards.
     */
    /* Type 5c Japanese keyboard: kakutei */
    {java_awt_event_KeyEvent_VK_ACCEPT, XK_Execute, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    /* Type 5c Japanese keyboard: henkan */
    {java_awt_event_KeyEvent_VK_CONVERT, XK_Kanji, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    /* Type 5c Japanese keyboard: nihongo */
    {java_awt_event_KeyEvent_VK_INPUT_METHOD_ON_OFF, XK_Henkan_Mode, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    /* VK_KANA_LOCK is handled separately because it generates the
     * same keysym as ALT_GRAPH in spite of its different behavior.
     */

    {java_awt_event_KeyEvent_VK_COMPOSE, XK_Multi_key, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_ALT_GRAPH, XK_Mode_switch, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Editing block */
    {java_awt_event_KeyEvent_VK_AGAIN, XK_Redo, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_AGAIN, XK_L2, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UNDO, XK_Undo, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UNDO, XK_L4, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_COPY, XK_L6, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PASTE, XK_L8, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CUT, XK_L10, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_FIND, XK_Find, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_FIND, XK_L9, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PROPS, XK_L3, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_STOP, XK_L1, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Sun vendor-specific versions for editing block */
    {java_awt_event_KeyEvent_VK_AGAIN, SunXK_Again, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UNDO, SunXK_Undo, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_COPY, SunXK_Copy, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PASTE, SunXK_Paste, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CUT, SunXK_Cut, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_FIND, SunXK_Find, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PROPS, SunXK_Props, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_STOP, SunXK_Stop, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Apollo (HP) vendor-specific versions for editing block */
    {java_awt_event_KeyEvent_VK_COPY, apXK_Copy, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CUT, apXK_Cut, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PASTE, apXK_Paste, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Other vendor-specific versions for editing block */
    {java_awt_event_KeyEvent_VK_COPY, osfXK_Copy, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_CUT, osfXK_Cut, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_PASTE, osfXK_Paste, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_UNDO, osfXK_Undo, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Dead key mappings (for European keyboards) */
    {java_awt_event_KeyEvent_VK_DEAD_GRAVE, XK_dead_grave, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_ACUTE, XK_dead_acute, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CIRCUMFLEX, XK_dead_circumflex, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_TILDE, XK_dead_tilde, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_MACRON, XK_dead_macron, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_BREVE, XK_dead_breve, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_ABOVEDOT, XK_dead_abovedot, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_DIAERESIS, XK_dead_diaeresis, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_ABOVERING, XK_dead_abovering, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_DOUBLEACUTE, XK_dead_doubleacute, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CARON, XK_dead_caron, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CEDILLA, XK_dead_cedilla, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_OGONEK, XK_dead_ogonek, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_IOTA, XK_dead_iota, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_VOICED_SOUND, XK_dead_voiced_sound, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_SEMIVOICED_SOUND, XK_dead_semivoiced_sound, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Sun vendor-specific dead key mappings (for European keyboards) */
    {java_awt_event_KeyEvent_VK_DEAD_GRAVE, SunXK_FA_Grave, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CIRCUMFLEX, SunXK_FA_Circum, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_TILDE, SunXK_FA_Tilde, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_ACUTE, SunXK_FA_Acute, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_DIAERESIS, SunXK_FA_Diaeresis, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CEDILLA, SunXK_FA_Cedilla, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* DEC vendor-specific dead key mappings (for European keyboards) */
    {java_awt_event_KeyEvent_VK_DEAD_ABOVERING, DXK_ring_accent, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CIRCUMFLEX, DXK_circumflex_accent, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CEDILLA, DXK_cedilla_accent, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_ACUTE, DXK_acute_accent, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_GRAVE, DXK_grave_accent, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_TILDE, DXK_tilde, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_DIAERESIS, DXK_diaeresis, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    /* Other vendor-specific dead key mappings (for European keyboards) */
    {java_awt_event_KeyEvent_VK_DEAD_ACUTE, hpXK_mute_acute, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_GRAVE, hpXK_mute_grave, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_CIRCUMFLEX, hpXK_mute_asciicircum, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_DIAERESIS, hpXK_mute_diaeresis, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},
    {java_awt_event_KeyEvent_VK_DEAD_TILDE, hpXK_mute_asciitilde, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_STANDARD},

    {java_awt_event_KeyEvent_VK_UNDEFINED, NoSymbol, FALSE, java_awt_event_KeyEvent_KEY_LOCATION_UNKNOWN}
};

static Boolean
keyboardHasKanaLockKey()
{
    static Boolean haveResult = FALSE;
    static Boolean result = FALSE;

    int32_t minKeyCode, maxKeyCode, keySymsPerKeyCode;
    KeySym *keySyms, *keySymsStart, keySym;
    int32_t i;
    int32_t kanaCount = 0;

    // Solaris doesn't let you swap keyboards without rebooting,
    // so there's no need to check for the kana lock key more than once.
    if (haveResult) {
       return result;
    }

    // There's no direct way to determine whether the keyboard has
    // a kana lock key. From available keyboard mapping tables, it looks
    // like only keyboards with the kana lock key can produce keysyms
    // for kana characters. So, as an indirect test, we check for those.
    XDisplayKeycodes(awt_display, &minKeyCode, &maxKeyCode);
    keySyms = XGetKeyboardMapping(awt_display, minKeyCode, maxKeyCode - minKeyCode + 1, &keySymsPerKeyCode);
    keySymsStart = keySyms;
    for (i = 0; i < (maxKeyCode - minKeyCode + 1) * keySymsPerKeyCode; i++) {
        keySym = *keySyms++;
        if ((keySym & 0xff00) == 0x0400) {
            kanaCount++;
        }
    }
    XFree(keySymsStart);

    // use a (somewhat arbitrary) minimum so we don't get confused by a stray function key
    result = kanaCount > 10;
    haveResult = TRUE;
    return result;
}

static void
keysymToAWTKeyCode(KeySym x11Key, jint *keycode, Boolean *mapsToUnicodeChar,
  jint *keyLocation)
{
    int32_t i;

    // Solaris uses XK_Mode_switch for both the non-locking AltGraph
    // and the locking Kana key, but we want to keep them separate for
    // KeyEvent.
    if (x11Key == XK_Mode_switch && keyboardHasKanaLockKey()) {
        *keycode = java_awt_event_KeyEvent_VK_KANA_LOCK;
        *mapsToUnicodeChar = FALSE;
        *keyLocation = java_awt_event_KeyEvent_KEY_LOCATION_UNKNOWN;
        return;
    }

    for (i = 0;
         keymapTable[i].awtKey != java_awt_event_KeyEvent_VK_UNDEFINED;
         i++)
    {
        if (keymapTable[i].x11Key == x11Key) {
            *keycode = keymapTable[i].awtKey;
            *mapsToUnicodeChar = keymapTable[i].mapsToUnicodeChar;
            *keyLocation = keymapTable[i].keyLocation;
            return;
        }
    }

    *keycode = java_awt_event_KeyEvent_VK_UNDEFINED;
    *mapsToUnicodeChar = FALSE;
    *keyLocation = java_awt_event_KeyEvent_KEY_LOCATION_UNKNOWN;

    DTRACE_PRINTLN1("keysymToAWTKeyCode: no key mapping found: keysym = 0x%x", x11Key);
}

KeySym
awt_getX11KeySym(jint awtKey)
{
    int32_t i;

    if (awtKey == java_awt_event_KeyEvent_VK_KANA_LOCK && keyboardHasKanaLockKey()) {
        return XK_Mode_switch;
    }

    for (i = 0; keymapTable[i].awtKey != 0; i++) {
        if (keymapTable[i].awtKey == awtKey) {
            return keymapTable[i].x11Key;
        }
    }

    DTRACE_PRINTLN1("awt_getX11KeySym: no key mapping found: awtKey = 0x%x", awtKey);
    return NoSymbol;
}

/* Called from handleKeyEvent.  The purpose of this function is
 * to check for a list of vendor-specific keysyms, most of which
 * have values greater than 0xFFFF.  Most of these keys don't map
 * to unicode characters, but some do.
 *
 * For keys that don't map to unicode characters, the keysym
 * is irrelevant at this point.  We set the keysym to zero
 * to ensure that the switch statement immediately below
 * this function call (in adjustKeySym) won't incorrectly act
 * on them after the high bits are stripped off.
 *
 * For keys that do map to unicode characters, we change the keysym
 * to the equivalent that is < 0xFFFF
 */
static void
handleVendorKeySyms(XEvent *event, KeySym *keysym)
{
    KeySym originalKeysym = *keysym;

    switch (*keysym) {
        /* Apollo (HP) vendor-specific from <X11/ap_keysym.h> */
        case apXK_Copy:
        case apXK_Cut:
        case apXK_Paste:
        /* DEC vendor-specific from <X11/DECkeysym.h> */
        case DXK_ring_accent:         /* syn usldead_ring */
        case DXK_circumflex_accent:
        case DXK_cedilla_accent:      /* syn usldead_cedilla */
        case DXK_acute_accent:
        case DXK_grave_accent:
        case DXK_tilde:
        case DXK_diaeresis:
        /* Sun vendor-specific from <X11/Sunkeysym.h> */
        case SunXK_FA_Grave:
        case SunXK_FA_Circum:
        case SunXK_FA_Tilde:
        case SunXK_FA_Acute:
        case SunXK_FA_Diaeresis:
        case SunXK_FA_Cedilla:
        case SunXK_F36:                /* Labeled F11 */
        case SunXK_F37:                /* Labeled F12 */
        case SunXK_Props:
        case SunXK_Copy:
        case SunXK_Open:
        case SunXK_Paste:
        case SunXK_Cut:
        /* Other vendor-specific from HPkeysym.h */
        case hpXK_mute_acute:          /* syn usldead_acute */
        case hpXK_mute_grave:          /* syn usldead_grave */
        case hpXK_mute_asciicircum:    /* syn usldead_asciicircum */
        case hpXK_mute_diaeresis:      /* syn usldead_diaeresis */
        case hpXK_mute_asciitilde:     /* syn usldead_asciitilde */
        case osfXK_Copy:
        case osfXK_Cut:
        case osfXK_Paste:
        case osfXK_PageUp:
        case osfXK_PageDown:
        case osfXK_EndLine:
        case osfXK_Clear:
        case osfXK_Left:
        case osfXK_Up:
        case osfXK_Right:
        case osfXK_Down:
        case osfXK_Prior:
        case osfXK_Next:
        case osfXK_Insert:
        case osfXK_Undo:
        case osfXK_Help:
            *keysym = 0;
            break;
        /*
         * The rest DO map to unicode characters, so translate them
         */
        case osfXK_BackSpace:
            *keysym = XK_BackSpace;
            break;
        case osfXK_Escape:
            *keysym = XK_Escape;
            break;
        case osfXK_Cancel:
            *keysym = XK_Cancel;
            break;
        case osfXK_Delete:
            *keysym = XK_Delete;
            break;
        default:
            break;
    }

    if (originalKeysym != *keysym) {
        DTRACE_PRINTLN3("%s originalKeysym=0x%x, keysym=0x%x",
          "In handleVendorKeySyms:", originalKeysym, *keysym);
    }
}

/* Called from handleKeyEvent.
 * The purpose of this function is to adjust the keysym and XEvent
 * keycode for a key event.  This is basically a conglomeration of
 * bugfixes that require these adjustments.
 * Note that none of the keysyms in this function are less than 256. 
 */
static void
adjustKeySym(XEvent *event, KeySym *keysym)
{
    KeySym originalKeysym = *keysym;
    KeyCode originalKeycode = event->xkey.keycode;

    /* We have seen bits set in the high two bytes on Linux,
     * which prevents this switch statement from executing
     * correctly.  Strip off the high order bits.
     */
    *keysym &= 0x0000FFFF;

    switch (*keysym) {
        case XK_ISO_Left_Tab:        /* shift-tab on Linux */
            *keysym = XK_Tab; 
            break;
        case XK_KP_Decimal:
            *keysym = '.';
            break;
        case XK_KP_Add:
            *keysym = '+';
            break;
        case XK_F24:           /* NumLock off */
        case XK_KP_Subtract:   /* NumLock on */
            *keysym = '-';
            break;
        case XK_F25:           /* NumLock off */
        case XK_KP_Divide:     /* NumLock on */
            *keysym = '/';
            break;
        case XK_F26:           /* NumLock off */
        case XK_KP_Multiply:   /* NumLock on */
            *keysym = '*';
            break;
        case XK_KP_Equal:
            *keysym = '=';
            break;
        case XK_KP_0:
            *keysym = '0';
            break;
        case XK_KP_1:
            *keysym = '1';
            break;
        case XK_KP_2:
            *keysym = '2';
            break;
        case XK_KP_3:
            *keysym = '3';
            break;
        case XK_KP_4:
            *keysym = '4';
            break;
        case XK_KP_5:
            *keysym = '5';
            break;
        case XK_KP_6:
            *keysym = '6';
            break;
        case XK_KP_7:
            *keysym = '7';
            break;
        case XK_KP_8:
            *keysym = '8';
            break;
        case XK_KP_9:
            *keysym = '9';
            break;
        case XK_KP_Left:  /* Bug 4350175 */
            *keysym = XK_Left;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Up:
            *keysym = XK_Up;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Right:
            *keysym = XK_Right;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Down:
            *keysym = XK_Down;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Home:
            *keysym = XK_Home;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_End:
            *keysym = XK_End;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Page_Up:
            *keysym = XK_Page_Up;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Page_Down:
            *keysym = XK_Page_Down;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Begin:
            *keysym = XK_Begin;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Insert:
            *keysym = XK_Insert;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Delete:
            *keysym = XK_Delete;
            event->xkey.keycode = XKeysymToKeycode(awt_display, *keysym);
            break;
        case XK_KP_Enter:
            *keysym = XK_Linefeed;
            event->xkey.keycode = XKeysymToKeycode(awt_display, XK_Return);
            break;
        default:
            break;
    }

    if (originalKeysym != *keysym) {
        DTRACE_PRINTLN2("In adjustKeySym: originalKeysym=0x%x, keysym=0x%x",
          originalKeysym, *keysym);
    }
    if (originalKeycode != event->xkey.keycode) {
        DTRACE_PRINTLN2("In adjustKeySym: originalKeycode=0x%x, keycode=0x%x",
          originalKeycode, event->xkey.keycode);
    }
}

/* Called from handleKeyEvent.
 * The purpose of this function is to make some adjustments to keysyms
 * that have been found to be necessary when the NumLock mask is set.
 * They come from various bug fixes and rearchitectures.
 * This function is meant to be called when
 * (event->xkey.state & awt_NumLockMask) is TRUE.
 */
static void
handleKeyEventWithNumLockMask(XEvent *event, KeySym *keysym)
{
    KeySym originalKeysym = *keysym;

#ifndef __linux__
    /* The following code on Linux will cause the keypad keys
     * not to echo on JTextField when the NumLock is on. The
     * keysyms will be 0, because the last parameter 2 is not defined.
     * See Xlib Programming Manual, O'Reilly & Associates, Section
     * 9.1.5 "Other Keyboard-handling Routines", "The meaning of
     * the keysym list beyond the first two (unmodified, Shift or
     * Shift Lock) is not defined."
     */

    /* Translate again with NumLock as modifier. */
    /* ECH - I wonder why we think that NumLock corresponds to 2?
       On Linux, we've seen xmodmap -pm yield mod2 as NumLock,
       but I don't know that it will be for every configuration.
       Perhaps using the index (modn in awt_MToolkit.c:setup_modifier_map)
       would be more correct.
     */
    *keysym = XKeycodeToKeysym(event->xkey.display,
                               event->xkey.keycode, 2);
    if (originalKeysym != *keysym) {
        DTRACE_PRINTLN3("%s originalKeysym=0x%x, keysym=0x%x",
          "In handleKeyEventWithNumLockMask ifndef linux:",
          originalKeysym, *keysym);
    }
#endif

    /* Note: the XK_R? key assignments are for Type 4 kbds */
    switch (*keysym) {
        case XK_R13:
            *keysym = XK_KP_1;
            break;
        case XK_R14:
            *keysym = XK_KP_2;
            break;
        case XK_R15:
            *keysym = XK_KP_3;
            break;
        case XK_R10:
            *keysym = XK_KP_4;
            break;
        case XK_R11:
            *keysym = XK_KP_5;
            break;
        case XK_R12:
            *keysym = XK_KP_6;
            break;
        case XK_R7:
            *keysym = XK_KP_7;
            break;
        case XK_R8:
            *keysym = XK_KP_8;
            break;
        case XK_R9:
            *keysym = XK_KP_9;
            break;
        case XK_KP_Insert:
            *keysym = XK_KP_0;
            break;
        case XK_KP_Delete:
            *keysym = XK_KP_Decimal;
            break;
        case XK_R4:
            *keysym = XK_KP_Equal;  /* Type 4 kbd */
            break;
        case XK_R5:
            *keysym = XK_KP_Divide;
            break;
        case XK_R6:
            *keysym = XK_KP_Multiply;
            break;
        /*
         * Need the following keysym changes for Linux key releases.
         * Sometimes the modifier state gets messed up, so we get a
         * KP_Left when we should get a KP_4, for example.
         * XK_KP_Insert and XK_KP_Delete were already handled above.
         */
        case XK_KP_Left:
            *keysym = XK_KP_4;
            break;
        case XK_KP_Up:
            *keysym = XK_KP_8;
            break;
        case XK_KP_Right:
            *keysym = XK_KP_6;
            break;
        case XK_KP_Down:
            *keysym = XK_KP_2;
            break;
        case XK_KP_Home:
            *keysym = XK_KP_7;
            break;
        case XK_KP_End:
            *keysym = XK_KP_1;
            break;
        case XK_KP_Page_Up:
            *keysym = XK_KP_9;
            break;
        case XK_KP_Page_Down:
            *keysym = XK_KP_3;
            break;
        case XK_KP_Begin:
            *keysym = XK_KP_5;
            break;
        default:
            break;
    }

    if (originalKeysym != *keysym) {
        DTRACE_PRINTLN3("%s originalKeysym=0x%x, keysym=0x%x",
          "In handleKeyEventWithNumLockMask:", originalKeysym, *keysym);
    }
}


/* This function is called as the keyChar parameter of a call to 
 * awt_post_java_key_event.  It depends on being called after adjustKeySym. 
 *
 * This function just handles a few values where we know that the 
 * keysym is not the same as the unicode value.  For values that 
 * we don't handle explicitly, we just cast the keysym to a jchar.  
 * Most of the real mapping work that gets the correct keysym is handled 
 * in the mapping table, adjustKeySym, etc.  
 * 
 * XXX
 * Maybe we should enumerate the keysyms for which we have a mapping 
 * in the keyMap, but that don't map to unicode chars, and return 
 * CHAR_UNDEFINED?  Then use the buffer value from XLookupString 
 * instead of the keysym as the keychar when posting.  Then we don't 
 * need to test using mapsToUnicodeChar.  That way, we would post keyTyped 
 * for all the chars that generate unicode chars, including LATIN2-4, etc. 
 * Note: what does the buffer from XLookupString contain when 
 * the character is a non-printable unicode character like Cancel or Delete? 
 */ 
jchar 
keySymToUnicodeCharacter(KeySym keysym) { 
    jchar unicodeValue = (jchar) keysym;

    switch (keysym) {
      case XK_BackSpace:
      case XK_Tab:
      case XK_Linefeed:  
      case XK_Escape:
      case XK_Delete:
          /* Strip off highorder bits defined in xkeysymdef.h 
           * I think doing this converts them to values that 
           * we can cast to jchars and use as java keychars. 
           */ 
          unicodeValue = (jchar) (keysym & 0x007F);
          break;
      case XK_Return:
          unicodeValue = (jchar) 0x000a;  /* the unicode char for Linefeed */
          break;
      case XK_Cancel:
          unicodeValue = (jchar) 0x0018;  /* the unicode char for Cancel */
          break;
      default:
          break;
    }

    if (unicodeValue != (jchar)keysym) {
        DTRACE_PRINTLN3("%s originalKeysym=0x%x, keysym=0x%x",
          "In keysymToUnicode:", keysym, unicodeValue);
    }

    return unicodeValue;
}


void
awt_post_java_key_event(JNIEnv *env, jobject peer, jint id,
  jlong when, jint keyCode, jchar keyChar, jint keyLocation, jint state)
{
    JNU_CallMethodByName(env, NULL, peer, "postKeyEvent", "(IJICII)V", id,
        when, keyCode, keyChar, keyLocation, state);
} /* awt_post_java_key_event() */


/* Returns the keycode for the key event. 
 * 
 * TODO: write a wrapper function for XLookupKeysym that is analogous 
 * to the one in awt_InputMethod.c (awt_x11inputmethod_lookupString), 
 * and does error handling.  Note: Naoto told me that the code in that 
 * function is very old, and nobody is really sure how/why it works.  
 */
static int
handleKeyEvent(JNIEnv *env, jobject target, jobject peer, jint keyEventId,
               XEvent *event, Boolean post)
{
    KeySym keysym = NoSymbol;
    jint keycode = java_awt_event_KeyEvent_VK_UNDEFINED;
    Modifiers mods = 0;
    Boolean mapsToUnicodeChar = FALSE;
    jint keyLocation = java_awt_event_KeyEvent_KEY_LOCATION_UNKNOWN;
    XComposeStatus status_in_out;
    char buffer[20];
    int32_t bufsize = 20;
    int32_t charcount;

    DTRACE_PRINTLN4("Entered handleKeyEvent: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x",
      event->type, event->xkey.keycode, event->xkey.state, keysym);

    if (currentX11InputMethodInstance != NULL
        && keyEventId == java_awt_event_KeyEvent_KEY_PRESSED)
    {
        /* invokes XmbLookupString to get a committed string or keysym if any. */
        if (awt_x11inputmethod_lookupString((XKeyPressedEvent*)event, &keysym)) {
            //printf("XWindow.c:handleKeyEvent:return: 1\n"); fflush(stdout);
            return java_awt_event_KeyEvent_VK_UNDEFINED;
        }
    }

    /* Ignore the keysym found immediately above in
     * awt_x11inputmethod_lookupString; the methodology in that function
     * sometimes returns incorrect results.
     *
     * Get keysym without taking modifiers into account first.
     * This keysym is not necessarily for the character that was typed:
     * it is for the primary layer.  So, if $ were typed by pressing
     * shift-4, this call should give us 4, not $
     *
     * We only want this keysym so we can use it to index into the
     * keymapTable to get the Java keycode associated with the
     * primary layer key that was pressed.
     */
    keysym = XKeycodeToKeysym(event->xkey.display, event->xkey.keycode, 0);

    /* Linux: Sometimes the keysym returned is uppercase when CapsLock is
     * on and LockMask is not set in event->xkey.state.
     */
    if (keysym >= (KeySym) 'A' && keysym <= (KeySym) 'Z') {
        event->xkey.state |= LockMask;
        keysym = (KeySym) tolower((int32_t) keysym);
    }

    DTRACE_PRINTLN4("In handleKeyEvent: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x",
                    event->type, event->xkey.keycode, event->xkey.state, keysym);

    if (keysym == NoSymbol) {
        //printf("XWindow.c:handleKeyEvent:return: 2\n"); fflush(stdout);
        return java_awt_event_KeyEvent_VK_UNDEFINED;
    }

    /* One reason it is important to handle keysyms < 256 separately is that
     * some of them (e.g. XK_ssharp = 0x0df) don't have java keycodes defined,
     * so aren't mapped in keysymToAWTKeycode, but ought to generate KEY_TYPED
     * events anyway?  Not sure that makes sense since it doesn't cover LATIN2-4.  
     * Maybe it's just faster since we can ignore some cases?  
     * Note: do all keysyms < 256 generate unicode chars (e.g. XK_ordfeminine)? 
     * I think they do.  
     */
    if (keysym < (KeySym) 256) {
        keysymToAWTKeyCode(keysym, &keycode, &mapsToUnicodeChar, &keyLocation);

        charcount = XLookupString(&(event->xkey), buffer, bufsize-1, &keysym, &status_in_out);
        DTRACE_PRINTLN6("%s: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x, charcount=%d",
          "In handleKeyEvent", event->type, event->xkey.keycode,
          event->xkey.state, keysym, charcount);
        if (charcount > 0) {
            buffer[charcount] = '\0';
            DTRACE_PRINTLN1("buffer=%s", buffer);
        }
        /* XXX Need to do better error-handling here - see IM code */

        /* Linux: With caps lock on, chars echo lowercase. */
        if ((event->xkey.state & LockMask) &&
             (keysym >= (KeySym) 'a' && keysym <= (KeySym) 'z'))
        {
            keysym = (KeySym) toupper((int32_t) keysym);
        }

        if ((event->xkey.state & ControlMask)) {
            switch (keysym) {
                case '[':
                case ']':
                case '\\':
                case '_':
                    keysym -= 64;
                    break;
                default:
                    if (isalpha((int32_t) keysym)) {
                        keysym = (KeySym) tolower((int32_t) keysym) - 'a' + 1;
                    }
                    break;
            }
        }

        /* This is a dead branch of code - it can never be executed.  
         * We should check to see whether kana keys are being handled 
         * correctly, and either relocate or remove it.  
         */
        if (keysym >= (KeySym) XK_kana_fullstop &&
            keysym <= (KeySym) XK_semivoicedsound)
        {
            /*
             * 0xff61 is Unicode value of first XK_kana_fullstop.
             * We need X Keysym to Unicode map in post1.1 release
             * to support more intenational keyboard.
             */
            keysym = keysym - XK_kana_fullstop + 0xff61;
        }

        DTRACE_PRINTLN5("%s: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x",
          "In handleKeyEvent keysym<256 ", event->type, event->xkey.keycode,
          event->xkey.state, keysym);

        if (post) {
            awt_post_java_key_event(env, peer,
              keyEventId,
              event->xkey.time,
              keycode,
              keySymToUnicodeCharacter(keysym), 
              keyLocation,
              event->xkey.state);
        }

        if (post && keyEventId == java_awt_event_KeyEvent_KEY_PRESSED) {
            awt_post_java_key_event(env, peer,
                                    java_awt_event_KeyEvent_KEY_TYPED,
                                    event->xkey.time,
                                    java_awt_event_KeyEvent_VK_UNDEFINED,
                                    keySymToUnicodeCharacter(keysym), 
                                    java_awt_event_KeyEvent_KEY_LOCATION_UNKNOWN,
                                    event->xkey.state);
        }

        //printf("XWindow.c:handleKeyEvent:return: 3\n"); fflush(stdout);
        return keycode;
    } /* end keysym<256 */

    if (event->xkey.state & awt_NumLockMask) {
        handleKeyEventWithNumLockMask(event, &keysym);
    }

    /* The keysym here does not consider modifiers, so these results
     * are relevant to the KEY_PRESSED event only, not the KEY_TYPED
     */
    keysymToAWTKeyCode(keysym, &keycode, &mapsToUnicodeChar, &keyLocation);
    DTRACE_PRINTLN3("In handleKeyEvent: keysym=0x%x, AWTkeycode=0x%x, mapsToUnicodeChar=%d",
                    keysym, keycode, mapsToUnicodeChar);

    if (keycode == java_awt_event_KeyEvent_VK_UNDEFINED) {
        /* Rather than returning, which might filter characters such as a-grave (?), 
         * perhaps we should have a table based on keysymdef.h.  We could 
         * include only unicode chars (thus removing arrows, modifiers, etc.) 
         * and use it in keySymToUnicodeChar.  It would replace the 
         * mapsToUnicodeChar part of the keymap table entirely.  
         * The difference is that the new table would convert keysyms to 
         * unicode characters, while the current table converts them to keycodes. 
         */ 
        //printf("XWindow.c:handleKeyEvent:return: 4\n"); fflush(stdout);
        return java_awt_event_KeyEvent_VK_UNDEFINED;
    }

    handleVendorKeySyms(event, &keysym);
    adjustKeySym(event, &keysym);
    DTRACE_PRINT4("In handleKeyEvent: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x",
                  event->type, event->xkey.keycode, event->xkey.state, keysym);
    DTRACE_PRINTLN1(", AWTkeycode=0x%x", keycode);

    if (post) {
        awt_post_java_key_event(env, peer,
          keyEventId,
          event->xkey.time,
          keycode,
          (mapsToUnicodeChar ? keySymToUnicodeCharacter(keysym) 
                             : java_awt_event_KeyEvent_CHAR_UNDEFINED),
          keyLocation,
          event->xkey.state);
    }

    /* If this was a keyPressed event, we may need to post a
     * keyTyped event, too.  Otherwise, return.
     */
    if (keyEventId == java_awt_event_KeyEvent_KEY_RELEASED) {
        //printf("XWindow.c:handleKeyEvent:return: 5\n"); fflush(stdout);
        return keycode;
    }
    DTRACE_PRINTLN("This is a keyPressed event");

    /* Now get real keysym which looks at modifiers for keyTyped event. */
    charcount = XLookupString(&(event->xkey), buffer, bufsize-1, &keysym, &status_in_out);
    DTRACE_PRINTLN6("%s: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x, charcount=%d",
      "In handleKeyEvent", event->type, event->xkey.keycode,
      event->xkey.state, keysym, charcount);
    if (charcount > 0) {
        buffer[charcount] = '\0';
        DTRACE_PRINTLN1("buffer=%s", buffer);
    }
    /* XXX Need to do better error-handling here - see IM code */
    /* XXX should probably try to eliminate this second call to XLookupString.  */

    if (keysym == NoSymbol) {
        //printf("XWindow.c:handleKeyEvent:return: 6\n"); fflush(stdout);
        return keycode;
    }

    if (event->xkey.state & awt_NumLockMask) {
        handleKeyEventWithNumLockMask(event, &keysym);
    }

    /* Map the real keysym to a Java keycode */
    keysymToAWTKeyCode(keysym, &keycode, &mapsToUnicodeChar, &keyLocation);
    DTRACE_PRINTLN3("In handleKeyEvent: keysym=0x%x, AWTkeycode=0x%x, mapsToUnicodeChar=%d",
                    keysym, keycode, mapsToUnicodeChar);

    /* If it doesn't map to a Unicode character, don't post a keyTyped event */
    /* This seems a little bogus now: shouldn't the test be whether
     * charcount > 0 ?  Is charcount>0 when the character is unprintable 
     * (e.g. backspace, delete, cancel, escape)?  
     * Also, mapsToUnicodeChar really maps X keysyms to AWT keycodes, 
     * not to unicode chars.  That mechanism should really be removed.   
     * Will this prevent us from posting keyTyped events for LATIN-2, etc. ?
     */
    if (!mapsToUnicodeChar) {
        //printf("XWindow.c:handleKeyEvent:return: 7\n"); fflush(stdout);
        return keycode;
    }

    handleVendorKeySyms(event, &keysym);
    adjustKeySym(event, &keysym);
    DTRACE_PRINT4("In handleKeyEvent: type=%d, xkeycode=0x%x, xstate=0x%x, keysym=0x%x",
                  event->type, event->xkey.keycode, event->xkey.state, keysym);
    DTRACE_PRINTLN1(", AWTkeycode=0x%x", keycode);

    if (post) {
        awt_post_java_key_event(env, peer,
                                java_awt_event_KeyEvent_KEY_TYPED,
                                event->xkey.time,
                                java_awt_event_KeyEvent_VK_UNDEFINED,
                                keySymToUnicodeCharacter(keysym), 
                                java_awt_event_KeyEvent_KEY_LOCATION_UNKNOWN,
                                event->xkey.state);
    }

    //printf("XWindow.c:handleKeyEvent:return: 9\n"); fflush(stdout); 
    return keycode;
}


/*
 * Class:     sun_awt_X11_XWindow
 * Method:    nativeHandleKeyEvent
 * Signature: (Ljava/awt/Component;IJ)I
 */
JNIEXPORT jint JNICALL Java_sun_awt_X11_XWindow_nativeHandleKeyEvent
  (JNIEnv *env, jobject obj, jobject target, jint keyID, jlong ptr) {
    return handleKeyEvent(env, target, obj, keyID, (XEvent *) ptr, (Boolean)True);
}

/*
 * Class:     sun_awt_X11_XWindow
 * Method:    nativeGetKeyCode
 * Signature: (Ljava/awt/Component;IJ)I
 */
JNIEXPORT jint JNICALL Java_sun_awt_X11_XWindow_nativeGetKeyCode
  (JNIEnv *env, jobject obj, jobject target, jint keyID, jlong ptr) {
    return handleKeyEvent(env, target, obj, keyID, (XEvent *) ptr, (Boolean)False);
}

extern struct X11GraphicsConfigIDs x11GraphicsConfigIDs;

/*
 * Class:     Java_sun_awt_X11_XWindow_getNativeColor
 * Method:    getNativeColor
 * Signature  (Ljava/awt/Color;Ljava/awt/GraphicsConfiguration;)I
 */
JNIEXPORT jint JNICALL Java_sun_awt_X11_XWindow_getNativeColor
(JNIEnv *env, jobject this, jobject color, jobject gc_object) {
    AwtGraphicsConfigDataPtr adata;
    adata = (AwtGraphicsConfigDataPtr) JNU_GetLongFieldAsPtr(env, gc_object,
                                                    x11GraphicsConfigIDs.aData);
    return awtJNI_GetColorForVis(env, color, adata);
}

/* syncTopLevelPos() is necessary to insure that the window manager has in
 * fact moved us to our final position relative to the reParented WM window.
 * We have noted a timing window which our shell has not been moved so we
 * screw up the insets thinking they are 0,0.  Wait (for a limited period of
 * time to let the WM hava a chance to move us
 */
void syncTopLevelPos( Display *d, Window w, XWindowAttributes *winAttr ) {
    int32_t i = 0;
    do {
         XGetWindowAttributes( d, w, winAttr );
         /* Sometimes we get here before the WM has updated the
         ** window data struct with the correct position.  Loop
         ** until we get a non-zero position.
         */
         if ((winAttr->x != 0) || (winAttr->y != 0)) {
             break;
         }
         else {
             /* What we really want here is to sync with the WM,
             ** but there's no explicit way to do this, so we
             ** call XSync for a delay.
             */
             XSync(d, False);
         }
    } while (i++ < 50);
}

static Window getTopWindow(Window win, Window *rootWin)
{
    Window root=NULL, current_window=win, parent=NULL, *ignore_children=NULL;
    Window prev_window=NULL;
    unsigned int ignore_uint=0;
    Status status = 0;

    if (win == NULL) return NULL;
    do {
        status = XQueryTree(awt_display,
                            current_window,
                            &root,
                            &parent,
                            &ignore_children,
                            &ignore_uint);
        XFree(ignore_children);
        if (status == 0) return NULL;
        prev_window = current_window;
        current_window = parent;
    } while (parent != root);
    *rootWin = root;
    return prev_window;
}

JNIEXPORT jlong JNICALL Java_sun_awt_X11_XWindow_getTopWindow
(JNIEnv *env, jclass clazz, jlong win, jlong rootWin) {
    return getTopWindow((Window)win, (Window*)rootWin);
}

static void
getWMInsets
(Window window, int *left, int *top, int *right, int *bottom, int *border) {
    // window is event->xreparent.window
    Window topWin = None, rootWin = None, containerWindow = None;
    XWindowAttributes winAttr, topAttr;
    int screenX, screenY;
    topWin = getTopWindow(window, &rootWin);
    syncTopLevelPos(awt_display, topWin, &topAttr);
    // (screenX, screenY) is (0,0) of the reparented window
    // converted to screen coordinates.
    XTranslateCoordinates(awt_display, window, rootWin,
        0,0, &screenX, &screenY, &containerWindow);
    *left = screenX - topAttr.x - topAttr.border_width;
    *top  = screenY - topAttr.y - topAttr.border_width;
    XGetWindowAttributes(awt_display, window, &winAttr);
    *right  = topAttr.width  - ((winAttr.width)  + *left);
    *bottom = topAttr.height - ((winAttr.height) + *top);
    *border = topAttr.border_width;
}

JNIEXPORT void JNICALL Java_sun_awt_X11_XWindow_getWMInsets
(JNIEnv *env, jclass clazz, jlong window, jlong left, jlong top, jlong right, jlong bottom, jlong border) {
    getWMInsets((Window)window, (int*)left, (int*)top, (int*)right, (int*)bottom, (int*)border);
}

static void
getWindowBounds
(Window window, int *x, int *y, int *width, int *height) {
    XWindowAttributes winAttr;
    XSync(awt_display, False);
    XGetWindowAttributes(awt_display, window, &winAttr);
    *x = winAttr.x;
    *y = winAttr.y;
    *width = winAttr.width;
    *height = winAttr.height;
}

JNIEXPORT void JNICALL Java_sun_awt_X11_XWindow_getWindowBounds
(JNIEnv *env, jclass clazz, jlong window, jlong x, jlong y, jlong width, jlong height) {
    getWindowBounds((Window)window, (int*)x, (int*)y, (int*)width, (int*)height);
}

JNIEXPORT void JNICALL Java_sun_awt_X11_XWindow_setSizeHints
(JNIEnv *env, jclass clazz, jlong window, jlong x, jlong y, jlong width, jlong height) {
    XSizeHints *size_hints = XAllocSizeHints();
    size_hints->flags = USPosition | PPosition | PSize;
    size_hints->x = (int)x;
    size_hints->y = (int)y;
    size_hints->width = (int)width;
    size_hints->height = (int)height;
    XSetWMNormalHints(awt_display, (Window)window, size_hints);
    XFree((char*)size_hints);
}


JNIEXPORT void JNICALL
Java_sun_awt_X11_XWindow_initIDs
  (JNIEnv *env, jclass clazz)
{
   windowID = (*env)->GetFieldID(env, clazz, "window", "J");
   targetID = (*env)->GetFieldID(env, clazz, "target", "Ljava/awt/Component;");
   graphicsConfigID = (*env)->GetFieldID(env, clazz, "graphicsConfig", "Lsun/awt/X11GraphicsConfig;");
   drawStateID = (*env)->GetFieldID(env, clazz, "drawState", "I");
}

