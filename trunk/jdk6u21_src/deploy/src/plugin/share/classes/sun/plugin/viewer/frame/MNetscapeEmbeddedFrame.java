/*
 * @(#)MNetscapeEmbeddedFrame.java	1.7 03/10/21
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.frame;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

/* The MNetscapeEmbeddedFrame holds the Panel, that holds the applet or JavaBeans.
 * The plugin is handed a native window object, embedded in the browser window
 * - so we extend EmbeddedFrame for this purpose.
 */
public class MNetscapeEmbeddedFrame extends sun.awt.motif.MEmbeddedFrame
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

    public MNetscapeEmbeddedFrame() {
	super();

	setLayout(new BorderLayout());
	setBackground(Color.white);
	addWindowListener(this);
    }

    // handle should be a valid Motif widget.
    public MNetscapeEmbeddedFrame(int handle, boolean supportsXEmbed) {
	super((long)handle, supportsXEmbed);

	setLayout(new BorderLayout());
	setBackground(Color.white);
	addWindowListener(this);
    }
}



