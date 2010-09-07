/*
 *  @(#)LaunchErrorDialog.java	1.67 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.URL;
import java.util.*;
import java.security.GeneralSecurityException;
import com.sun.javaws.Main;
import com.sun.javaws.Globals;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.util.JavawsConsoleController;
import com.sun.deploy.util.ConsoleWindow;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.Environment;
import com.sun.deploy.net.DownloadException;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.UITextArea;


public class LaunchErrorDialog {
    
    private static JPanel getDetailPanel(Throwable throwable) {

	JPanel detailPanel = new JPanel() {
	    public Dimension getPreferredSize() {
		return new Dimension(480, 300);
	    }
	};
        detailPanel.setLayout(new BorderLayout());

        JNLPException jnlpException = null;
        DownloadException dlException = null;
	Throwable wrappedThrowable = null;

        if (throwable instanceof JNLPException) {
	    jnlpException = (JNLPException)throwable;
            wrappedThrowable = jnlpException.getWrappedException();
        } else if (throwable instanceof DownloadException) {
            dlException = (DownloadException)throwable;
            wrappedThrowable = dlException.getWrappedException();
        } else if (throwable instanceof ExitException) {
	    wrappedThrowable = ((ExitException)throwable).getException();
	}
        
        JTabbedPane tabPane = new JTabbedPane();
        detailPanel.add(tabPane, BorderLayout.CENTER);

	tabPane.setBorder(BorderFactory.createEmptyBorder(8,0,0,0));
        
        String errorKind = getErrorCategory(throwable);
        
	JPanel intro = new JPanel(new BorderLayout());
	JLabel label = new JLabel(ResourceManager.getString(
				"launcherrordialog.error.label"));
	Font bold = label.getFont().deriveFont(Font.BOLD);
	label.setFont(bold);
	JPanel labelPanel = new JPanel(new BorderLayout());
	labelPanel.add(label, BorderLayout.NORTH);
	labelPanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 12, 0));
	intro.add(labelPanel, BorderLayout.WEST);
	
	String msg = getErrorDescription(throwable);
	JTextArea ta = new UITextArea(msg);
	intro.add(ta, BorderLayout.CENTER);

        detailPanel.add(intro, BorderLayout.NORTH);
        
        // Get source for LaunchFile that got error, 
	// and the one that for the application (if they are different)
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
		errorLaunchDescSource = 
		    JNLPException.getDefaultLaunchDesc().getSource();
	    }
        }
        // Get the main LaunchDesc
        if (JNLPException.getDefaultLaunchDesc() != null) {
	    mainLaunchDescSource = 
		JNLPException.getDefaultLaunchDesc().getSource();
        }
        
        // Only show main if it is different than the error one
        if (mainLaunchDescSource != null && 
	    mainLaunchDescSource.equals(errorLaunchDescSource)) {
	    mainLaunchDescSource = null;
        }
        
        // Pane 2: Launch File w/ error
        if (errorLaunchDescSource != null) {
	    JTextArea tp12 = new JTextArea();
	    tp12.setFont(ResourceManager.getUIFont());
	    tp12.setEditable(false);
	    tp12.setLineWrap(true);
	    tp12.setText(filter(errorLaunchDescSource));
	    tabPane.add(ResourceManager.getString(
		"launcherrordialog.jnlpTab"), new JScrollPane(tp12));
        }
        
        // Pane 3: Main Launch File if different
        if (mainLaunchDescSource != null) {
	    JTextArea tp12 = new JTextArea();
	    tp12.setFont(ResourceManager.getUIFont());
	    tp12.setEditable(false);
	    tp12.setLineWrap(true);
	    tp12.setText(filter(mainLaunchDescSource));
	    tabPane.add(ResourceManager.getString(
		"launcherrordialog.jnlpMainTab"), new JScrollPane(tp12));
        }
        
        // Pane 4: Exception
        if (throwable != null) {
	    JTextArea tp3 = new JTextArea();
	    tp3.setFont(ResourceManager.getUIFont());
	    tp3.setEditable(false);
	    tp3.setLineWrap(true);
	    tp3.setWrapStyleWord(false);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    throwable.printStackTrace(pw);
	    tp3.setText(sw.toString());
	    tabPane.add(ResourceManager.getString(
		"launcherrordialog.exceptionTab"), new JScrollPane(tp3));
        }
        
        // Pane 5: Wrapped Exception
        if (wrappedThrowable != null) {
	    JTextArea tp3 = new JTextArea();
	    tp3.setFont(ResourceManager.getUIFont());
	    tp3.setEditable(false);
	    tp3.setLineWrap(true);
	    tp3.setWrapStyleWord(false);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    wrappedThrowable.printStackTrace(pw);
	    tp3.setText(sw.toString());
	    tabPane.add(ResourceManager.getString(
		"launcherrordialog.wrappedExceptionTab"), new JScrollPane(tp3));
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

	    tabPane.add(ResourceManager.getString(
		"launcherrordialog.consoleTab"), new JScrollPane(text));
	}
	return detailPanel;
    }

    /** this filter function takes a jnlp file as a string,
     * however, in the case the jnlp file is complete garbage, this can hang
     * when setting this as the text in a JTextArea, so if jnlp file is a
     * very long string, use only the first 10K
     */
    private static String filter(String jnlpString) {
        if (jnlpString.length() > 10240) {
            return (jnlpString.substring(0, 10239) + 
                "\njnlp file truncated after 10K\n") ;
        }
        return jnlpString;
    }
    
    /** Shows an error message for the given exception. The error box gets
     *  automatically configured based on the exception. The error dialog
     *  knows how to handle JNLPException's and its subtype.
     */
    public static void show(final Component owner, final Throwable throwable, 
			    boolean exit)
    {
        com.sun.javaws.ui.SplashScreen.hide(); // Just in case

        // Just print details to System.err for debugging
        System.err.println("#### Java Web Start Error:");
        System.err.println("#### " + getMessage(throwable));

        boolean showit = !Globals.TCKHarnessRun && !Environment.isSilentMode();
        if (showit) {
            DeploySysAction action = new DeploySysAction() {
                public Object execute() {
		    try {
	                String message = null;
	                String errorKind = getErrorCategory(throwable);

	                if (throwable instanceof JNLPException) {
                            message = 
				((JNLPException) throwable).getBriefMessage();
	                }
                        if (message == null) {
                            if (Environment.isImportMode()) {
                                if (Environment.isInstallMode()) {
                                message = ResourceManager.getString
                                    ("launcherrordialog.uninstall.brief.message");   
                                } else {
		                message = ResourceManager.getString
                                    ("launcherrordialog.import.brief.message");
                                }
                            } else {
		                message = ResourceManager.getString
                                    ("launcherrordialog.brief.message");
                            }
	                }
                        AppInfo ainfo = ((getLaunchDesc() == null) ? 
			    new AppInfo() : getLaunchDesc().getAppInfo());

	                String btn1String = ResourceManager.getString(
			    "launcherrordialog.brief.ok");
	                String btn2String = ResourceManager.getString(
			    "launcherrordialog.brief.details");
	                String title = ResourceManager.getString(
		                "error.default.title",errorKind);
                
	                UIFactory.showErrorDialog(owner, 
                                                  ainfo, 
                                                  title, 
                                                  message, 
                                                  null, 
                                                  btn1String, 
                                                  btn2String, 
                                                  null, 
                                                  getDetailPanel(throwable),
						  null);
	            } catch (Exception ee) {
		        Trace.ignored(ee);
		    };
                    return null;
                }
            };
            DeploySysRun.execute(action, null);
        }

        if (exit) {
            try {
                Main.systemExit(0); 
            } catch (ExitException ee) { 
                Trace.println("systemExit: "+ee, TraceLevel.BASIC);
                Trace.ignoredException(ee);
            }
        }
    }

    /** Compute the broad describtion of the exception */
    static private String getErrorCategory(Throwable e) {
        String errorKind = ResourceManager.getString(
	    "launch.error.category.unexpected");
        // Comput brief message
        if (e instanceof JNLPException) {
	    // JNLPExceptions are easy - the know there category already
	    JNLPException je = (JNLPException)e;
	    errorKind = je.getCategory();
        } else if (e instanceof SecurityException || 
		   e instanceof GeneralSecurityException) {
	    errorKind = ResourceManager.getString(
		"launch.error.category.security");
        } else if (e instanceof java.lang.OutOfMemoryError) {
	    errorKind = ResourceManager.getString(
		"launch.error.category.memory");
	} else if (e instanceof DownloadException) {
            errorKind = ResourceManager.getString(
		"launch.error.category.download");
        }
        return errorKind;
    }
    
    /** Compute the detailed describtion of the exception */
    static private String getErrorDescription(Throwable e) {
        String errorDescription = getMessage(e);
        if (errorDescription == null) {
	    // Create generic error messaage based on exception type
	    errorDescription = ResourceManager.getString(
		"launcherrordialog.genericerror", e.getClass().getName());
        }
        return errorDescription;
    }
   
    static private String getMessage (Throwable t) {
	if (t instanceof Exception) {
	    return t.getMessage();
	}
	return t.getClass().getName() + ": " + t.getMessage();
    }
    
    private static LaunchDesc getLaunchDesc() {
        return  JNLPException.getDefaultLaunchDesc();
    }
    
}

