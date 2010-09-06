/*
 *  @(#)LaunchErrorDialog.java	1.46 04/03/23
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.security.GeneralSecurityException;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.util.JavawsConsoleController;
import com.sun.deploy.util.ConsoleWindow;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;

public class LaunchErrorDialog extends JDialog {
    
    private LaunchErrorDialog(Frame owner, Throwable exception) {
        super(owner, true);
        
        JNLPException jnlpException = null;
        if (exception instanceof JNLPException) {
	    jnlpException = (JNLPException)exception;
        }
        
        JTabbedPane tabPane = new JTabbedPane();
        getContentPane().setLayout(new BorderLayout());
        getContentPane().add("Center", tabPane);

	tabPane.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
        
        String errorKind = getErrorCategory(exception);
        setTitle(ResourceManager.getString("launcherrordialog.title", errorKind));
        
        String title = getLaunchDescTitle();
        String vendor = getLaunchDescVendor();
	String msg;
	if (Globals.isImportMode()) {
	    msg= ResourceManager.getString("launcherrordialog.import.errorintro");
	} else {
	    msg= ResourceManager.getString("launcherrordialog.errorintro");
	}
        if (title != null) {
	    msg += ResourceManager.getString("launcherrordialog.errortitle", title);
        }
        if (vendor != null) {
	    msg += ResourceManager.getString("launcherrordialog.errorvendor", vendor);
        }
        msg += ResourceManager.getString("launcherrordialog.errorcategory", errorKind);
        msg += getErrorDescription(exception);
        
        // Pane 1: General
        JTextArea tp1 = new JTextArea();
	tp1.setFont(ResourceManager.getUIFont());
        tp1.setEditable(false);
        tp1.setLineWrap(true);
        tp1.setText(msg);
        tabPane.add(ResourceManager.getString("launcherrordialog.generalTab"), new JScrollPane(tp1));
        
        
        // Get source for LaunchFile that got error, and the one that for the application (if
        // they are different)
        String errorLaunchDescSource = null;
        String mainLaunchDescSource = null;
        
        // Get LaunchDesc for JNLP file that caused the error
        if (jnlpException != null) {
	    errorLaunchDescSource = jnlpException.getLaunchDescSource();
	    if (errorLaunchDescSource == null) {
		LaunchDesc ld = jnlpException.getDefaultLaunchDesc();
		if (ld != null) {
			errorLaunchDescSource = ld.getSource();
		}
	    }
        } else {
	    // Just get the default one
	    if (JNLPException.getDefaultLaunchDesc() != null) {
		errorLaunchDescSource = JNLPException.getDefaultLaunchDesc().getSource();
	    }
        }
        // Get the main LaunchDesc
        if (JNLPException.getDefaultLaunchDesc() != null) {
	    mainLaunchDescSource = JNLPException.getDefaultLaunchDesc().getSource();
        }
        
        // Only show main if it is different than the error one
        if (mainLaunchDescSource != null && mainLaunchDescSource.equals(errorLaunchDescSource)) {
	    mainLaunchDescSource = null;
        }
        
        // Pane 2: Launch File w/ error
        if (errorLaunchDescSource != null) {
	    JTextArea tp12 = new JTextArea();
	    tp12.setFont(ResourceManager.getUIFont());
	    tp12.setEditable(false);
	    tp12.setLineWrap(true);
	    tp12.setText(errorLaunchDescSource);
	    tabPane.add(ResourceManager.getString("launcherrordialog.jnlpTab"), new JScrollPane(tp12));
        }
        
        // Pane 3: Main Launch File if different
        if (mainLaunchDescSource != null) {
	    JTextArea tp12 = new JTextArea();
	    tp12.setFont(ResourceManager.getUIFont());
	    tp12.setEditable(false);
	    tp12.setLineWrap(true);
	    tp12.setText(mainLaunchDescSource);
	    tabPane.add(ResourceManager.getString("launcherrordialog.jnlpMainTab"), new JScrollPane(tp12));
        }
        
        // Pane 4: Exception
        if (exception != null) {
	    JTextArea tp3 = new JTextArea();
	    tp3.setFont(ResourceManager.getUIFont());
	    tp3.setEditable(false);
	    tp3.setLineWrap(true);
	    tp3.setWrapStyleWord(false);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exception.printStackTrace(pw);
	    tp3.setText(sw.toString());
	    tabPane.add(ResourceManager.getString("launcherrordialog.exceptionTab"),
			new JScrollPane(tp3));
        }
        
        // Pane 5: Wrapped Exception
        if (jnlpException != null && jnlpException.getWrappedException() != null) {
	    JTextArea tp3 = new JTextArea();
	    tp3.setFont(ResourceManager.getUIFont());
	    tp3.setEditable(false);
	    tp3.setLineWrap(true);
	    tp3.setWrapStyleWord(false);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    jnlpException.getWrappedException().printStackTrace(pw);
	    tp3.setText(sw.toString());
	    tabPane.add(ResourceManager.getString("launcherrordialog.wrappedExceptionTab"),
			new JScrollPane(tp3));
        }
	// Pane 6: Console
	Document doc = null;
	ConsoleWindow cw = JavawsConsoleController.getInstance().getConsole();
	if (cw != null) {
	    doc = cw.getTextArea().getDocument();
	}
	if (doc != null) {
	    JTextArea text = new JTextArea(doc);
	    text.setFont(ResourceManager.getUIFont());

	    tabPane.add(ResourceManager.getString("launcherrordialog.consoleTab"),
                        new JScrollPane(text));
	}
        // Add Abort button
        JButton abortButton = new JButton(
	    ResourceManager.getString("launcherrordialog.abort"));
	abortButton.setMnemonic(
	    ResourceManager.getVKCode("launcherrordialog.abortMnemonic"));
        Box box = new Box(BoxLayout.X_AXIS);
        box.add(Box.createHorizontalGlue());
        box.add(abortButton);
        box.add(Box.createHorizontalGlue());
        getContentPane().add("South", box);
        getRootPane().setDefaultButton(abortButton);

        abortButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			setVisible(false);
		    }
		});
        
        addWindowListener(new WindowAdapter() {
		    public void windowClosing(WindowEvent e) {
			setVisible(false);
		    }
		});
        pack();
        setSize(450, 300);
        
        // Center the window
        Rectangle size = getBounds();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        size.width = Math.min(screenSize.width, size.width);
        size.height = Math.min(screenSize.height, size.height);
        setBounds(
		     (screenSize.width - size.width) / 2,
		     (screenSize.height - size.height) / 2,
		     (size.width), (size.height));
    }
    
    /** Shows an error message for the given exception. The error box gets
     *  automatically configured based on the exception. The error dialog
     *  knows how to handle JNLPException's and its subtype.
     */
    public static void show(final Frame owner, final Throwable e, boolean exit)
    {
	try {
	    SwingUtilities.invokeAndWait(new Runnable() {
	        public void run() {
		    showWarning(owner, e);
	        }
	    });
	} catch (Exception ee) {};

	if (exit) {
            Main.systemExit(0); 
	}
    }

    private static void showWarning(Frame owner, Throwable e) 
    {	
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    SplashScreen.hide(); // Just in case

	    // Just print details to System.err (this is mostly for debugging and testing)
	    System.err.println("#### Java Web Start Error:");
	    System.err.println("#### " + e.getMessage());
	    boolean showit = !Globals.TCKHarnessRun && 
			     (!Globals.isSilentMode() || Main.isViewer());

	    if (showit && wantsDetails(owner, e)) {
		LaunchErrorDialog led = new LaunchErrorDialog(owner, e);
		led.setVisible(true);
	    }
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }
    
    /** Compute the broad describtion of the exception */
    static private String getErrorCategory(Throwable e) {
        String errorKind = ResourceManager.getString("launch.error.category.unexpected");
        // Comput brief message
        if (e instanceof JNLPException) {
	    // JNLPExceptions are easy - the know there category already
	    JNLPException je = (JNLPException)e;
	    errorKind = je.getCategory();
        } else if (e instanceof SecurityException || e instanceof GeneralSecurityException) {
	    errorKind = ResourceManager.getString("launch.error.category.security");
        } else if (e instanceof java.lang.OutOfMemoryError) {
	    errorKind = ResourceManager.getString("launch.error.category.memory");
	}
        return errorKind;
    }
    
    /** Compute the detailed describtion of the exception */
    static private String getErrorDescription(Throwable e) {
        // The error messages should already be localized. We should probably handle
        // non-JNLP exceptions in a different way
        String errorDescription = e.getMessage();
        if (errorDescription == null) {
	    // Create generic error messaage based on exception type
	    errorDescription = ResourceManager.getString("launcherrordialog.genericerror", e.getClass().getName());
        }
        return errorDescription;
    }
    
    private static String getLaunchDescTitle() {
        LaunchDesc ld = JNLPException.getDefaultLaunchDesc();
        return (ld == null) ? null : ld.getInformation().getTitle();
    }
    
    private static String getLaunchDescVendor() {
        LaunchDesc ld = JNLPException.getDefaultLaunchDesc();
        return (ld == null) ? null : ld.getInformation().getVendor();
    }
    
    
    /**
     * Shows the user that we were unable to launch the specified application.
     * Will return true if the user wants details on what went wrong.
     */
    private static boolean wantsDetails(Frame owner, Throwable exception) {
        String message = null;
	String errorKind = getErrorCategory(exception);
        
	if (exception instanceof JNLPException) {
	    message = ((JNLPException) exception).getBriefMessage();
	}
	if (message == null) {
            if (getLaunchDescTitle() == null) {
		if (Globals.isImportMode()) {
		    message = ResourceManager.getString
			("launcherrordialog.import.brief.message");
		} else {
		    message = ResourceManager.getString
			("launcherrordialog.brief.message");
		}
            } else {
		if (Globals.isImportMode()) {
		    message = ResourceManager.getString
			("launcherrordialog.import.brief.messageKnown",  getLaunchDescTitle());
		} else {
		    message = ResourceManager.getString
			("launcherrordialog.brief.messageKnown", getLaunchDescTitle());
		}
            }
	}
        String[] options = new String[] {
		ResourceManager.getString("launcherrordialog.brief.ok"),
	    ResourceManager.getString("launcherrordialog.brief.details")
        };
        int retValue = DialogFactory.showOptionDialog (
		owner, DialogFactory.ERROR_MESSAGE, message, 
		ResourceManager.getString("launcherrordialog.brief.title", errorKind),
		options, options[0]);
        if (retValue == 1) {
	    return true;
        }
        return false;
    }
}

