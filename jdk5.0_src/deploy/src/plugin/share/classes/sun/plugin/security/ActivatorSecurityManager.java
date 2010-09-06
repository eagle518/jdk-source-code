/*
 * @(#)ActivatorSecurityManager.java	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import sun.awt.AppContext;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.Trace;
import sun.plugin.util.PluginSysUtil;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DialogFactory;

/** 
 * The Activator security manager.
 *
 * This extends and makes very small tweaks to the standard
 * Applet security manager
 */

public class ActivatorSecurityManager extends sun.applet.AppletSecurity {

    public ActivatorSecurityManager() { 
    }

    /*
     * Work around oversight in normal applet security manager.
     */

    public void checkDelete(String file) {
	// If they can write the file, we let them delete it too.
	checkWrite(file);
    }

    /**
     * Checks to see if an applet can get EventQueue access.
     * Work around oversight in normal applet security manager.
     */
    public void checkAwtEventQueueAccess() {
	// See if the java.lang.Security.Manager allows it.
	// This always fails on 1.1, but may pass on 1.2
	try {
	    super.checkAwtEventQueueAccess();
	    // the current thread is allowed access.
	    return;
	} catch (SecurityException ex) {
	    // Drop through.
	}
	// This passes on 1.1 if we are trusted.
	// It gets correctly checked on 1.2
	checkSecurityAccess("accessEventQueue");
    }

    /**
     * Checks to see if an applet can perform a given operation.
     */
    public void checkSecurityAccess(String action) {
	if (action != null && action.equals("java"))
	    return;
	else
	    super.checkSecurityAccess(action);
    }


    /**
     * Tests if a client can initiate a print job request.
     *
     * If our superclass grants permission, so do we.
     *
     * If our superclass denies permission, we pop up a Dialog
     * and ask the user if they wish to allow the print job.
     *
     */
    public void checkPrintJobAccess() {

	// See if the java.lang.Security.Manager allows the print job.
	try {
	    super.checkPrintJobAccess();
	    // the current thread is allowed to print, by default.
	    // return success.
	    return;
	} catch (SecurityException ex) {
	    // The current thread is not allowed to print by default.
	    // Drop through and pop up a dialog.
	}

	// version for JDK 1.2 and later
	// We have to push things off to a separate class (CheckPrint_1_2)
	// here to avoid verifiers errors on 1.1.
	//
	// Constructing the CheckPrint_1_2 class causes it
	// to run a privileged clostrue that pops up the Dialog.
	new CheckPrint_1_2();
    }

    private class CheckPrint_1_2 implements java.security.PrivilegedAction {
	CheckPrint_1_2() {
	    // Run a privileged closure to bring up a Dialog box
	    java.security.AccessController.doPrivileged(this);
	}
	public Object run() {
	    showPrintDialog();
	    return null;
	}
    }


    // Will throw SecurityException if it's not OK to print.
    void showPrintDialog() {
        final AppContext context = AppContext.getAppContext();
		// Check if automation is enabled
		try {
			PluginSysUtil.execute(new DeploySysAction() {
			public Object execute() throws Exception {
				showPrintDialogImpl(context);
				return null;
			}});
		} catch(SecurityException e) { 
			throw e; 
		} catch(Exception e) {
			// should never happen
			assert(false);
		}
	}

    private void showPrintDialogImpl(AppContext context) {
	String title = ResourceHandler.getMessage("print.caption");
	String message[] = ResourceHandler.getMessageArray("print.message");
	String checkBoxStr = ResourceHandler.getMessage("print.checkBox");
	String yesButtonStr = ResourceHandler.getMessage("print.buttonYes");
	String noButtonStr = ResourceHandler.getMessage("print.buttonNo");

	// Create checkBox and Buttons
        JCheckBox printCheckBox = new JCheckBox(checkBoxStr, true);
        JButton yesButton = new JButton(yesButtonStr);
        JButton noButton = new JButton(noButtonStr);
	yesButton.setMnemonic(ResourceHandler.getAcceleratorKey("print.buttonYes"));
        noButton.setMnemonic(ResourceHandler.getAcceleratorKey("print.buttonNo"));
	int result = 0;
        String printFlag = (String)context.get("sun.plugin.security.printDialog");
	
	// Check if automation is enabled
	if (Trace.isAutomationEnabled() == false && printFlag == null)
	{
	    // Pop up print dialog box
            Object[] msgs = new Object[2];
	    msgs[0] = message;
            msgs[1] = printCheckBox;

            Object[] options = {yesButton, noButton};
            result = DialogFactory.showOptionDialog(DialogFactory.QUESTION_MESSAGE, msgs, title, options, options[0]);
	}
	else
	{
	    Trace.msgSecurityPrintln("securitymgr.automation.printing");
	    result = 0;
	}

	if (result != 0) {
            throw new SecurityException("checkPrintJobAccess");
        }
        else
        {
            if (printCheckBox.isSelected())
               context.put("sun.plugin.security.printDialog","skip");
        }
    }

    /**
     * getExecutionStackContext returns all the classes that are
     * on the current execution stack.
     *
     * @return Class object array
     */	
    public Class[] getExecutionStackContext()
    {
	return super.getClassContext();
    }
}


