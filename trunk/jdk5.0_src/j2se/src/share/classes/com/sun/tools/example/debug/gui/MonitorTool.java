/*
 * @(#)MonitorTool.java	1.8 03/12/19
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

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

import com.sun.jdi.*;
import com.sun.tools.example.debug.bdi.*;
import com.sun.tools.example.debug.expr.ExpressionParser;
import com.sun.tools.example.debug.expr.ParseException;

public class MonitorTool extends JPanel {

    private ExecutionManager runtime;
    private ContextManager context;

    private JList list;

    public MonitorTool(Environment env) {
	super(new BorderLayout());
	this.runtime = env.getExecutionManager();
	this.context = env.getContextManager();
	
        list = new JList(env.getMonitorListModel());
	list.setCellRenderer(new MonitorRenderer());

	JScrollPane listView = new JScrollPane(list);
	add(listView);

	// Create listener.
	MonitorToolListener listener = new MonitorToolListener();
        list.addListSelectionListener(listener);
        //### remove listeners on exit!
    }

    private class MonitorToolListener implements ListSelectionListener {
        public void valueChanged(ListSelectionEvent e) {
            int index = list.getSelectedIndex();
            if (index != -1) {
            }
        }
    }

    private Value evaluate(String expr) throws ParseException,
                                            InvocationException, 
                                            InvalidTypeException,
                                            ClassNotLoadedException,
                                            IncompatibleThreadStateException {
        ExpressionParser.GetFrame frameGetter = 
            new ExpressionParser.GetFrame() {
                public StackFrame get() 
                    throws IncompatibleThreadStateException 
                {
                    try {
                        return context.getCurrentFrame();
                    } catch (VMNotInterruptedException exc) {
                        throw new IncompatibleThreadStateException();
                    }
                }
            };
        return ExpressionParser.evaluate(expr, runtime.vm(), frameGetter);
    }

    private class MonitorRenderer extends DefaultListCellRenderer {

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
                String expr = (String)value;
                try {
                    Value result = evaluate(expr);
                    this.setText(expr + " = " + result);
                } catch (ParseException exc) {
                    this.setText(expr + " ? " + exc.getMessage());
                } catch (IncompatibleThreadStateException exc) {
                    this.setText(expr + " ...");
                } catch (Exception exc) {
                    this.setText(expr + " ? " + exc);
                }                    
            }
	    return this;
	}
    }
}
