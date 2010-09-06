/*
 * @(#)XNetscapeEmbeddedFrame.java	1.1 02/10/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.frame;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

/* The XNetscapeEmbeddedFrame holds the Panel, that holds the applet or JavaBeans.
 * The plugin is handed a native window object, embedded in the browser window
 * - so we extend EmbeddedFrame for this purpose.
 */
public class XNetscapeEmbeddedFrame extends sun.awt.X11.XEmbeddedFrame
					  implements WindowListener
{

    public void windowActivated(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowClosing(WindowEvent e) 
    {
	// Remove window listener
	removeWindowListener(this);
	
	// Remove all components
	removeAll();

	// Dispose the frame
	dispose();
    }
    public void windowDeactivated(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}

    public XNetscapeEmbeddedFrame() {
	super();
        init();
    }

    // handle should be a valid Motif widget.
    public XNetscapeEmbeddedFrame(int handle) {
	super(handle);
        init();
    }

    public XNetscapeEmbeddedFrame(long handle, boolean supportsXEmbed) {
	super(handle, supportsXEmbed);
        init();
    }

    private void init() {
	setLayout(new BorderLayout());
	setBackground(Color.white);
	addWindowListener(this);
    }
}



