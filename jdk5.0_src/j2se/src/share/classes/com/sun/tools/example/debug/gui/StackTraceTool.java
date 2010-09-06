/*
 * @(#)StackTraceTool.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.gui;

import java.io.*;
import java.util.*;
import java.util.List;  // Must import explicitly due to conflict with javax.awt.List

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

import com.sun.jdi.*;
import com.sun.tools.example.debug.bdi.*;

public class StackTraceTool extends JPanel {

    private Environment env;

    private ExecutionManager runtime;
    private ContextManager context;

    private ThreadInfo tinfo;

    private JList list;
    private ListModel stackModel;

    public StackTraceTool(Environment env) {

	super(new BorderLayout());

	this.env = env;
	this.runtime = env.getExecutionManager();
	this.context = env.getContextManager();
	
	stackModel = new DefaultListModel();  // empty
	
        list = new JList(stackModel);
	list.setCellRenderer(new StackFrameRenderer());

	JScrollPane listView = new JScrollPane(list);
	add(listView);

	// Create listener.
	StackTraceToolListener listener = new StackTraceToolListener();
	context.addContextListener(listener);
        list.addListSelectionListener(listener);

        //### remove listeners on exit!
    }

    private class StackTraceToolListener 
        implements ContextListener, ListSelectionListener 
    {

	// ContextListener

	// If the user selects a new current frame, display it in
	// this view.

	//### I suspect we handle the case badly that the VM is not interrupted.

	public void currentFrameChanged(CurrentFrameChangedEvent e) {
	    // If the current frame of the thread appearing in this
	    // view is changed, move the selection to track it.
            int frameIndex = e.getIndex();
            ThreadInfo ti = e.getThreadInfo();
	    if (e.getInvalidate() || tinfo != ti) {
                tinfo = ti;
                showStack(ti, frameIndex);
            } else {
		if (frameIndex < stackModel.getSize()) {
		    list.setSelectedIndex(frameIndex);
		    list.ensureIndexIsVisible(frameIndex);
		}
            }
	}

        // ListSelectionListener

        public void valueChanged(ListSelectionEvent e) {
            int index = list.getSelectedIndex();
            if (index != -1) {
                //### should use listener?
                try {
                    context.setCurrentFrameIndex(index);
                } catch (VMNotInterruptedException exc) {
                }
            }
        }
    }

    private class StackFrameRenderer extends DefaultListCellRenderer {

	public Component getListCellRendererComponent(JList list,
						      Object value,
						      int index,
						      boolean isSelected,
						      boolean cellHasFocus) {

	    //### We should indicate the current thread independently of the
	    //### selection, e.g., with an icon, because the user may change
	    //### the selection graphically without affecting the current 
            //### thread.

	    super.getListCellRendererComponent(list, value, index, 
                                               isSelected, cellHasFocus);
            if (value == null) {
                this.setText("<unavailable>");
            } else {                
                StackFrame frame = (StackFrame)value;
                Location loc = frame.location();
                Method meth = loc.method();
                String methName =
                    meth.declaringType().name() + '.' + meth.name();
                String position = "";
                if (meth instanceof Method && ((Method)meth).isNative()) {
                    position = " (native method)";
                } else if (loc.lineNumber() != -1) {
                    position = ":" + loc.lineNumber();
                } else {
                    long pc = loc.codeIndex();
                    if (pc != -1) {
                        position = ", pc = " + pc;
                    }
                }
                // Indices are presented to the user starting from 1, not 0.
                this.setText("[" + (index+1) +"] " + methName + position);
            }
	    return this;
	}
    }

    // Point this view at the given thread and frame.

    private void showStack(ThreadInfo tinfo, int selectFrame) {
	StackTraceListModel model = new StackTraceListModel(tinfo);
	stackModel = model;
	list.setModel(stackModel);
	list.setSelectedIndex(selectFrame);
	list.ensureIndexIsVisible(selectFrame);
    }

    private static class StackTraceListModel extends AbstractListModel {

	private final ThreadInfo tinfo;
	
	public StackTraceListModel(ThreadInfo tinfo) {
	    this.tinfo = tinfo;
	}
	
	public Object getElementAt(int index) {
	    try {
		return tinfo == null? null : tinfo.getFrame(index);
	    } catch (VMNotInterruptedException e) {
		//### Is this the right way to handle this?
		//### Would happen if user scrolled stack trace
		//### while not interrupted -- should probably
		//### block user interaction in this case.
		return null;
	    }
	}
	
	public int getSize() {
	    try {
		return tinfo == null? 1 : tinfo.getFrameCount();
	    } catch (VMNotInterruptedException e) {
		//### Is this the right way to handle this?
		return 0;
	    }
	}
    }
}
