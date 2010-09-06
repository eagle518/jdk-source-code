/*
 * @(#)Tab.java	1.16 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;

import javax.swing.*;

public abstract class Tab extends JPanel {
    private String name;
    private Worker worker;

    protected VMPanel vmPanel;

    public Tab(VMPanel vmPanel, String name) {
	this.vmPanel = vmPanel;
	this.name = name;
    }

    abstract public void update();

    public synchronized void dispose() {
	if(worker != null)
	    worker.stopWorker();
	
	// Subclasses will override to clean up
    }

    protected VMPanel getVMPanel() {
	return vmPanel;
    }

    public synchronized void workerAdd(Runnable job) {
	if (worker == null) {
	    worker = new Worker(name+"-"+vmPanel.getConnectionName());
	    worker.start();
	}
	worker.add(job);
    }

    public Dimension getPreferredSize() {
	return new Dimension(700, 500);
    }
}
