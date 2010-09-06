/*
 * @(#)ThreadDialog.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import
import java.awt.*;
import javax.swing.*;
import java.io.*;
//

public class ThreadDialog implements Runnable {
    
    Component parentComponent;
    Object message;
    String title;
    int messageType;
	
    public ThreadDialog (Component parentComponent, 
			 Object message, 
			 String title, 
			 int messageType) {
	this.parentComponent = parentComponent;
	this.message = message;
	this.title = title;
	this.messageType = messageType;
    }
	
    public void run() {
	JOptionPane.showMessageDialog(parentComponent,
				      message,
				      title,
				      messageType);
    }
}
