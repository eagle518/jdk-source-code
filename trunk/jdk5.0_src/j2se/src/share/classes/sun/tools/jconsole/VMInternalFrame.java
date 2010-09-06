/*
 * @(#)VMInternalFrame.java	1.8 04/05/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

import javax.swing.*;
import javax.swing.Timer;
import javax.swing.border.*;
import javax.swing.event.*;


public class VMInternalFrame extends JInternalFrame {
    private VMPanel vmPanel;

    public VMInternalFrame(VMPanel vmPanel) {
	super("", true, true, true, true);

	this.vmPanel = vmPanel;
	getContentPane().add(vmPanel, BorderLayout.CENTER);
	pack();

	String str = vmPanel.getConnectionName();
	if (str.equals("localhost:0")) {
	    str = Resources.getText("Monitoring Self", str);
	}
	setTitle(str);
    }

    public VMPanel getVMPanel() {
	return vmPanel;
    }
}
