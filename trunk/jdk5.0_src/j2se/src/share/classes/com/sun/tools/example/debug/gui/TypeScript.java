/*
 * @(#)TypeScript.java	1.8 03/12/19
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

public class TypeScript extends JPanel {

    private JTextArea history;
    private JTextField entry;
    
    private JLabel promptLabel;

    private JScrollBar historyVScrollBar;
    private JScrollBar historyHScrollBar;

    private boolean echoInput = false;
    private boolean nlPending = false;

    private static String newline = System.getProperty("line.separator");

    public TypeScript(String prompt) {
	this(prompt, true);
    }
    
    public TypeScript(String prompt, boolean echoInput) {
	this.echoInput = echoInput;

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	//setBorder(new EmptyBorder(5, 5, 5, 5));

	history = new JTextArea(0, 0);
	history.setEditable(false);
	JScrollPane scroller = new JScrollPane(history);
	historyVScrollBar = scroller.getVerticalScrollBar();
	historyHScrollBar = scroller.getHorizontalScrollBar();

	add(scroller);

	JPanel cmdLine = new JPanel();
	cmdLine.setLayout(new BoxLayout(cmdLine, BoxLayout.X_AXIS));
	//cmdLine.setBorder(new EmptyBorder(5, 5, 0, 0));

	promptLabel = new JLabel(prompt + " ");
	cmdLine.add(promptLabel);
	entry = new JTextField();
//### Swing bug workaround.
entry.setMaximumSize(new Dimension(1000, 20));
	cmdLine.add(entry);
	add(cmdLine);
    }

    /******
    public void setFont(Font f) {
	entry.setFont(f);
	history.setFont(f);
    }
    ******/

    public void setPrompt(String prompt) {
	promptLabel.setText(prompt + " ");
    }

    public void append(String text) {
	history.append(text);
	historyVScrollBar.setValue(historyVScrollBar.getMaximum());
	historyHScrollBar.setValue(historyHScrollBar.getMinimum());
    }

    public void newline() {
	history.append(newline);
	historyVScrollBar.setValue(historyVScrollBar.getMaximum());
	historyHScrollBar.setValue(historyHScrollBar.getMinimum());
    }

    public void flush() {}

    public void addActionListener(ActionListener a) {
	entry.addActionListener(a);
    }

    public void removeActionListener(ActionListener a) {
	entry.removeActionListener(a);
    }

    public String readln() {
	String text = entry.getText();
	entry.setText("");
	if (echoInput) {
	    history.append(">>>");
	    history.append(text);
	    history.append(newline);
	    historyVScrollBar.setValue(historyVScrollBar.getMaximum());
	    historyHScrollBar.setValue(historyHScrollBar.getMinimum());
	}
	return text;
    }
}
