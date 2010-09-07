/*
 * @(#)WColor.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.Color;

/*
 * This helper class maps Windows system colors to AWT Color objects.
 */
class WColor {

    static final int WINDOW_BKGND = 1;	// COLOR_WINDOW
    static final int WINDOW_TEXT  = 2;	// COLOR_WINDOWTEXT
    static final int FRAME        = 3;	// COLOR_WINDOWFRAME
    static final int SCROLLBAR    = 4;	// COLOR_SCROLLBAR
    static final int MENU_BKGND   = 5;	// COLOR_MENU
    static final int MENU_TEXT    = 6;  // COLOR MENUTEXT
    static final int BUTTON_BKGND = 7;  // COLOR_3DFACE or COLOR_BTNFACE
    static final int BUTTON_TEXT  = 8;  // COLOR_BTNTEXT
    static final int HIGHLIGHT    = 9;  // COLOR_HIGHLIGHT

    static native Color getDefaultColor(int index);
}
