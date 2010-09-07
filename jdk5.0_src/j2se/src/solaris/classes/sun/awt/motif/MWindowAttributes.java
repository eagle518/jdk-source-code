/*
 * @(#)MWindowAttributes.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;
import java.awt.Image;

class MWindowAttributes {
    static int NORMAL		= 0;
    static int ICONIC		= 1;
    static int MAXIMIZED	= 2;

    static int AWT_DECOR_NONE        = 0;
    static int AWT_DECOR_ALL         = 1;
    static int AWT_DECOR_BORDER      = 2;
    static int AWT_DECOR_RESIZEH     = 4;
    static int AWT_DECOR_TITLE       = 8;
    static int AWT_DECOR_MENU        = 0x10;
    static int AWT_DECOR_MINIMIZE    = 0x20;
    static int AWT_DECOR_MAXIMIZE    = 0x40;
    static int AWT_UNOBSCURED        = 0;   // X11 VisibilityUnobscured
    static int AWT_PARTIALLY_OBSCURED = 1;  // X11 VisibilityPartiallyObscured
    static int AWT_FULLY_OBSCURED    =  2;  // X11 VisibilityFullyObscured
    static int AWT_UNKNOWN_OBSCURITY = 3;

    boolean nativeDecor;
    boolean initialFocus;
    boolean isResizable;
    int initialState;
    int visibilityState; // updated by native X11 event handling code.
    String title;
    java.awt.Image icon;
    int	decorations;		// for future expansion to be able to 
				// specify native decorations
    
    private static native void initIDs();
    
    static {
	initIDs();
    }

    MWindowAttributes() {
        nativeDecor = false;
        initialFocus = false;
        isResizable = false;
        initialState = NORMAL;
	visibilityState = AWT_UNKNOWN_OBSCURITY;
	title = null;
	icon = null;
	decorations = 0;
    }   
}

