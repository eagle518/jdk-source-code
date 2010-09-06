/*
 * @(#)JDBMenuBar.java	1.11 03/12/19
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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Vector;
import java.util.List;

import com.sun.jdi.*;
import com.sun.tools.example.debug.bdi.*;

//### This is currently just a placeholder!

class JDBMenuBar extends JMenuBar {

    Environment env;

    ExecutionManager runtime;
    ClassManager classManager;
    SourceManager sourceManager;

    CommandInterpreter interpreter;

    JDBMenuBar(Environment env) {
	this.env = env;
	this.runtime = env.getExecutionManager();
	this.classManager = env.getClassManager();
	this.sourceManager = env.getSourceManager();
	this.interpreter = new CommandInterpreter(env, true);

	JMenu fileMenu = new JMenu("File");

	JMenuItem openItem = new JMenuItem("Open...", 'O');
	openItem.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		openCommand();
	    }
	});
	fileMenu.add(openItem);
	addTool(fileMenu, "Exit debugger", "Exit", "exit");

	JMenu cmdMenu = new JMenu("Commands");

	addTool(cmdMenu, "Step into next line", "Step", "step");
	addTool(cmdMenu, "Step over next line", "Next", "next");
	cmdMenu.addSeparator();

	addTool(cmdMenu, "Step into next instruction", 
                "Step Instruction", "stepi");
	addTool(cmdMenu, "Step over next instruction", 
                "Next Instruction", "nexti");
	cmdMenu.addSeparator();

	addTool(cmdMenu, "Step out of current method call", 
                "Step Up", "step up");
	cmdMenu.addSeparator();

	addTool(cmdMenu, "Suspend execution", "Interrupt", "interrupt");
	addTool(cmdMenu, "Continue execution", "Continue", "cont");
	cmdMenu.addSeparator();

	addTool(cmdMenu, "Display current stack", "Where", "where");
	cmdMenu.addSeparator();

	addTool(cmdMenu, "Move up one stack frame", "Up", "up");
	addTool(cmdMenu, "Move down one stack frame", "Down", "down");
	cmdMenu.addSeparator();

	JMenuItem monitorItem = new JMenuItem("Monitor Expression...", 'M');
	monitorItem.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		monitorCommand();
	    }
	});
        cmdMenu.add(monitorItem);

	JMenuItem unmonitorItem = new JMenuItem("Unmonitor Expression...");
	unmonitorItem.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		unmonitorCommand();
	    }
	});
        cmdMenu.add(unmonitorItem);

	JMenu breakpointMenu = new JMenu("Breakpoint");
	JMenuItem stopItem = new JMenuItem("Stop in...", 'S');
	stopItem.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		buildBreakpoint();
	    }
	});
	breakpointMenu.add(stopItem);
       
	JMenu helpMenu = new JMenu("Help");
	addTool(helpMenu, "Display command list", "Help", "help");
       
        this.add(fileMenu);
        this.add(cmdMenu);
//      this.add(breakpointMenu);
        this.add(helpMenu);
    }

    private void buildBreakpoint() {
        Frame frame = JOptionPane.getRootFrame();
        JDialog dialog = new JDialog(frame, "Specify Breakpoint");
        Container contents = dialog.getContentPane();
        Vector classes = new Vector();
        classes.add("Foo");
        classes.add("Bar");
        JList list = new JList(classes);
        JScrollPane scrollPane = new JScrollPane(list);
        contents.add(scrollPane);
        dialog.show();
        
    }

    private void monitorCommand() {
        String expr = (String)JOptionPane.showInputDialog(null, 
                           "Expression to monitor:", "Add Monitor",
                           JOptionPane.QUESTION_MESSAGE, null, null, null);
        if (expr != null) {
            interpreter.executeCommand("monitor " + expr);
        }
    }

    private void unmonitorCommand() {
        List monitors = env.getMonitorListModel().monitors();
        String expr = (String)JOptionPane.showInputDialog(null, 
                           "Expression to unmonitor:", "Remove Monitor",
                           JOptionPane.QUESTION_MESSAGE, null, 
                           monitors.toArray(), 
                           monitors.get(monitors.size()-1));
        if (expr != null) {
            interpreter.executeCommand("unmonitor " + expr);
        }
    }

    private void openCommand() {
	JFileChooser chooser = new JFileChooser();
	JDBFileFilter filter = new JDBFileFilter("java", "Java source code");
	chooser.setFileFilter(filter);
	int result = chooser.showOpenDialog(this);
	if (result == JFileChooser.APPROVE_OPTION) {
	    System.out.println("Chose file: " + chooser.getSelectedFile().getName());
	}
    }
    
    private void addTool(JMenu menu, String toolTip, String labelText, 
                         String command) {
	JMenuItem mi = new JMenuItem(labelText);
	mi.setToolTipText(toolTip);
	final String cmd = command;
	mi.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		interpreter.executeCommand(cmd);
	    }
	});
	menu.add(mi);
    }

}
