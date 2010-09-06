/*
 * @(#)AppletContainerCallback.java	1.10 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This code is located in the jnlp2 package - since it
 * is accessed directly from the Applet running in a sandbox it needs
 * to be outside the com.sun.javaws package
 */
package com.sun.jnlp;

import java.net.URL;
import java.awt.Dimension;

/** Callback interface for AppletContainer */
public interface AppletContainerCallback {
    /** show url in browser */
    public void showDocument(URL url);
    /** resize window with the relative amount specified */
    public void  relativeResize(Dimension delta);
}






