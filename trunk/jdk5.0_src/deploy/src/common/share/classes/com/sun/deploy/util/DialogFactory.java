/*
 * @(#)DialogFactory.java	1.106 04/06/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.awt.Component;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Dimension;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Toolkit;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.Array;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import javax.swing.BorderFactory;

import javax.swing.JComponent;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;
import javax.swing.LookAndFeel;
import javax.swing.UIManager;
import javax.swing.border.Border;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.Dialog;
import java.awt.Window;
import java.awt.Rectangle;
import java.awt.Point;
import java.awt.event.FocusEvent;
import java.awt.event.FocusAdapter;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;


/**
* DialogFactory provides confirmation, input, exception and message dialogboxes
*
* @version 1.0 
* @Author Devananda Jayaraman
**/

public final class DialogFactory 
{
    // Dialog message type
    public static final int ERROR_MESSAGE = 1;
    public static final int INFORMATION_MESSAGE = 2;
    public static final int WARNING_MESSAGE = 3;
    public static final int QUESTION_MESSAGE = 4;
    public static final int PLAIN_MESSAGE = 5;

    // showOptionDialog return values
    public static final int YES_OPTION = 0;
    public static final int NO_OPTION = 1;

    // Dialog title    
    private static String confirmDialogTitle;
    private static String inputDialogTitle;
    private static String messageDialogTitle;
    private static String exceptionDialogTitle;
    private static String optionDialogTitle;
    private static String aboutDialogTitle;
    private static String javaHomeLink;    

    private static DialogListener dialogListener = null;

    static
    {
	confirmDialogTitle = ResourceManager.getMessage("dialogfactory.confirmDialogTitle");
	inputDialogTitle = ResourceManager.getMessage("dialogfactory.inputDialogTitle");
	messageDialogTitle = ResourceManager.getMessage("dialogfactory.messageDialogTitle");
	exceptionDialogTitle = ResourceManager.getMessage("dialogfactory.exceptionDialogTitle");
	optionDialogTitle = ResourceManager.getMessage("dialogfactory.optionDialogTitle");
	aboutDialogTitle = ResourceManager.getMessage("dialogfactory.aboutDialogTitle");
	javaHomeLink = ResourceManager.getMessage("dialogfactory.java.home.link");
    }

    public static void addDialogListener(DialogListener dl) {
	if (dialogListener == null) {
	    dialogListener = dl;
	}
    }


    private static Icon _warningIcon = null;
    public static Icon getWarningIcon() {
	if (_warningIcon == null) {
	    Object o = UIManager.get("OptionPane.warningIcon");
	    if (o instanceof Icon) {
		_warningIcon = (Icon) o;
	    } else {
		URL url = ClassLoader.getSystemResource("javax/swing/plaf/metal/icons/Warn.gif");
	        _warningIcon = loadIcon(url);
	    }
	}
	return _warningIcon;
    }

    public static Icon _infoIcon = null;
    public static Icon getInfoIcon() {
	if (_infoIcon == null) {
	    Object o = UIManager.get("OptionPane.informationIcon");
	    if (o instanceof Icon) {
		_infoIcon = (Icon) o;
	    } else {
		URL url = ClassLoader.getSystemResource("javax/swing/plaf/metal/icons/Inform.gif");
	        _infoIcon = loadIcon(url);
	    }
	}
	return _infoIcon;
    }


    // Loads the icon to be used in the dialog
    static Icon _javaIcon = null;
    public static Icon loadIcon() {
	if (_javaIcon == null) {
	    _javaIcon = loadIcon(ResourceManager.class.getResource("images/java32.png"));
	}
	return _javaIcon;
    }
    
    static Icon _javaIcon48 = null;
    public static Icon loadIcon48() {
	if (_javaIcon48 == null) {
	    _javaIcon = loadIcon(ResourceManager.class.getResource("images/java32.png"));
	}
	return _javaIcon;
    }

    public static Icon loadIcon(final URL resource)
    {
	Icon icon = null;
	try
	{
	    icon = (Icon) AccessController.doPrivileged(
			new PrivilegedAction() 
			{
			    public Object run() 
			    {
				return new ImageIcon(resource);
			    }
			});
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	}
	
	return icon;
    }

    public static void showAboutJavaDialog() {
	if(SwingUtilities.isEventDispatchThread()) {
		internalShowAboutJavaDialog();
	} else {
		try {
			SwingUtilities.invokeAndWait(new Runnable() {
			public void run() {
				internalShowAboutJavaDialog();
			}});
		} catch(Exception e) {
		}
	}
    }
   
	private static long	tsLastActive = 0;

    private static synchronized void internalShowAboutJavaDialog() {
       LookAndFeel lookAndFeel = null;
       try {
		  if((System.currentTimeMillis() - tsLastActive) > 500 && AboutDialog.shouldStartNewInstance()) {
			  lookAndFeel = DeployUIManager.setLookAndFeel();
			  JFrame dummyFrame = new JFrame(com.sun.deploy.resources.ResourceManager.getMessage("about.dialog.title"));
			  AboutDialog dlg = new AboutDialog(dummyFrame, true, true);
			  dlg.pack();
			  Dimension sd = Toolkit.getDefaultToolkit().getScreenSize();
			  Dimension dd = dlg.getSize();
			  Dimension fd = dummyFrame.getSize();	
			  
			  dummyFrame.setLocation(-fd.width + 1, -fd.height + 1);
			  dummyFrame.setVisible(true);
			  dlg.setLocation((sd.width - dd.width)/2, (sd.height - dd.height)/2);
			  dlg.setVisible(true);
			  dummyFrame.setVisible(false);
			  dummyFrame.dispose();
			  tsLastActive = System.currentTimeMillis();
			}
       } catch (Exception e) {
       } finally {
          // Restore theme
		  if(lookAndFeel != null)
	          DeployUIManager.restoreLookAndFeel(lookAndFeel);
       }
    }
 
    public static int showConfirmDialog(Object message)
    {
	return showConfirmDialog(null, message, confirmDialogTitle);
    }

    public static int showConfirmDialog(Component parentComponent, Object message)
    {
	return showConfirmDialog(parentComponent, message, confirmDialogTitle);
    }

    public static String showInputDialog(Object message)
    {
	return showInputDialog(null, message, inputDialogTitle);
    }

    public static String showInputDialog(Component parentComponent, Object message)
    {
	return showInputDialog(parentComponent, message, inputDialogTitle);
    }

    public static void showInformationDialog(Object message)
    {
	showInformationDialog(null, message, messageDialogTitle);
    }

    public static void showInformationDialog(Component parentComponent, Object message)
    {
	showInformationDialog(parentComponent, message, messageDialogTitle);
    }

    public static void showErrorDialog(String message)
    {
	showErrorDialog(null, message, messageDialogTitle);
    }

    public static void showErrorDialog(Component parentComponent, String message)
    {
	showErrorDialog(parentComponent, message, messageDialogTitle);
    }

    public static void showExceptionDialog(Throwable ex)
    {
	showExceptionDialog(null, ex, ex.toString(), exceptionDialogTitle);
    }

    public static void showExceptionDialog(Component parentComponent, Throwable ex)
    {
	showExceptionDialog(parentComponent, ex, ex.toString(), exceptionDialogTitle);
    }

    public static int showConfirmDialog(Object message, String title)
    {
	return showConfirmDialog(null, message, title);
    }

    
    public static int showConfirmDialog(final Component parentComponent, final Object message, final String title) {
	try {
	    return ((Integer)DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    return new Integer(showConfirmDialogImpl(parentComponent, message, title));
		}})).intValue();
	}
	catch(Exception e) {
	    // should never happen
	    Trace.ignoredException(e);
	    return 1;
	}
    }

    private static int showConfirmDialogImpl(Component parentComponent, Object message, String title)
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    JButton yes = new JButton(ResourceManager.getMessage("dialogfactory.confirm.yes")); 
	    JButton no = new JButton(ResourceManager.getMessage("dialogfactory.confirm.no")); 
	    yes.setMnemonic(ResourceManager.getAcceleratorKey("dialogfactory.confirm.yes")); 
	    no.setMnemonic(ResourceManager.getAcceleratorKey("dialogfactory.confirm.no")); 

	    // order corrisponds to YES_OPTION=0, NO_OPTION=1
	    Object[] options = { yes, no };

	    if(title == null)
		title = confirmDialogTitle;

	    // Popup YES/NO dialog, 
	    return showOptionDialogImpl(parentComponent, QUESTION_MESSAGE, message, title, options, options[0], false);
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    public static int showOptionDialog(Object message, String title, Object[] options, Object initValue)
    {
	return showOptionDialog(null, message, title, options, initValue);
    }


    public static int showOptionDialog(Component parentComponent, Object message, String title, Object[] options, Object initValue)
    {
	// Popup YES/NO dialog, 
	return showOptionDialog(parentComponent, PLAIN_MESSAGE, message, title, options, initValue);
    }

    // Fix for 4994797: Standard Extensions - Accelerator keys not working for download window
    // All UI Components should be created under the correct
    // threadgroup, wrapped with a single DeploySysRun
    public static int showDownloadDialog(final int messageType, final String dialogText)
    {
	// Popup YES/NO dialog, 
	try {
	    return ((Integer)DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    return new Integer(showDownloadDialogImpl(messageType, dialogText));
	    }})).intValue();
	}
	catch(Exception e) {
	    // should never happen
	    Trace.ignoredException(e);
	    return 1;
	}
    }

    private static int showDownloadDialogImpl(int messageType, String dialogText) {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();	    
	    
	    String captionString = ResourceManager.getMessage("security.dialog.caption");
	    
	    JButton yesButton = new JButton(ResourceManager.getMessage("security.dialog.buttonYes"));
	    JButton noButton = new JButton(ResourceManager.getMessage("security.dialog.buttonNo"));
	    yesButton.setMnemonic(ResourceManager.getAcceleratorKey("security.dialog.buttonYes"));
	    noButton.setMnemonic(ResourceManager.getAcceleratorKey("security.dialog.buttonNo"));
	    Object[] options = { yesButton, noButton };
	    	 
	    // Show dialog
	    return showOptionDialogImpl(null, messageType,
					dialogText,
					captionString,
					options,
					options[0],
					false);
	    	    
	} finally {
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    static int showUpdateCheckDialog() {
	String yesButtonStr = ResourceManager.getMessage("autoupdatecheck.buttonYes");
        String noButtonStr = ResourceManager.getMessage("autoupdatecheck.buttonNo");
        String askLaterButtonStr = ResourceManager.getMessage("autoupdatecheck.buttonAskLater");
        String title = ResourceManager.getMessage("autoupdatecheck.caption");
        String message = ResourceManager.getMessage("autoupdatecheck.message");

        JButton yesButton = new JButton(yesButtonStr);
        JButton noButton = new JButton(noButtonStr);
        JButton askLaterButton = new JButton(askLaterButtonStr);
        yesButton.setMnemonic(ResourceManager.getAcceleratorKey("autoupdatecheck.buttonYes"));
  	noButton.setMnemonic(ResourceManager.getAcceleratorKey("autoupdatecheck.buttonNo"));
    	askLaterButton.setMnemonic(ResourceManager.getAcceleratorKey("autoupdatecheck.buttonAskLater"));
        Object[] options = {yesButton, noButton, askLaterButton};
        return showOptionDialogImpl(null, DialogFactory.WARNING_MESSAGE, 
				    message, title, 
				    options, options[0], false);
    }


    public static int showOptionDialog(int messageType, Object message, String title, Object[] options, Object initValue)
    {
	return showOptionDialog(null, messageType, message, title, options, initValue);
    }

    public static int showOptionDialog(final Component parentComponent, final int messageType, 
	final Object message, final String title, final Object[] options, final Object initValue) {
	try {
	    return ((Integer)DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    return new Integer(showOptionDialogImpl(parentComponent, messageType, message, 
		    	title, options, initValue, false));
	    }})).intValue();
	}
	catch(Exception e) {
	    // should never happen
	    Trace.ignoredException(e);
	    return 1;
	}
    }
    // Fix for BugId: 4803342 we load the dialog without displaying it so as to prevent a
    // dead lock in the class loader.
    public static void preLoadDialog(int messageType, Object message, String title, Object[] options, Object initValue) {
	showOptionDialogImpl(null, messageType, message, title, options, options[0], true);
    }
    
    private static int showOptionDialogImpl(Component parentComponent, int messageType, Object message, String title, Object[] options, Object initValue, boolean preload)
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    int selection = -1;

    	    JOptionPane pane = new JOptionPane();

	    switch (messageType)
	    {
		case DialogFactory.ERROR_MESSAGE: 
		    pane.setMessageType(JOptionPane.ERROR_MESSAGE);
		    break;
		case DialogFactory.INFORMATION_MESSAGE: 
		    pane.setMessageType(JOptionPane.INFORMATION_MESSAGE);
		    break;
		case DialogFactory.WARNING_MESSAGE: 
		    pane.setMessageType(JOptionPane.WARNING_MESSAGE);
		    break;
		case DialogFactory.QUESTION_MESSAGE: 
		    pane.setMessageType(JOptionPane.WARNING_MESSAGE);
		    break;
		default:
    //		pane.setIcon(loadIcon());
		    break;
	    }
	    pane.setOptions(options);
	    pane.setInitialValue(initValue);
	    pane.setWantsInput(false);

	    Object msg = extractMessage(pane, message);
	    
	    if(title == null)
		title = optionDialogTitle;

	    if(showDialog(pane, parentComponent, title, msg, true, preload) == true)
	    {
		Object selectedValue = pane.getValue();
		if(selectedValue != null)
		{
		    for (int i=0; i < options.length; i++)
		    {
			if (options[i].equals(selectedValue))
			{
			    selection = i;
			    break;
			}
		    }
		
		    Trace.msgPrintln("dialogfactory.user.selected", new Object[] {new Integer(selection)}, TraceLevel.BASIC);
		}
	    }

	    return selection;
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    public static String showInputDialog(Object message, String title)
    {
	return showInputDialog(null, message, title);
    }

    public static String showInputDialog(final Component parentComponent, final Object message, final String title) {

	try {
	    return (String)DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    return showInputDialogImpl(parentComponent, message, title);
		}});
	}
	catch(Exception e) {
	    // should never happen
	    Trace.ignoredException(e);
	    return null;
	}
    }

    private static String showInputDialogImpl(Component parentComponent, Object message, String title)
    {
	LookAndFeel lookAndFeel = null;
	
	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    String selection = null;
    	    JOptionPane pane = new JOptionPane();
	    pane.setMessageType(JOptionPane.QUESTION_MESSAGE);
	    pane.setOptionType(JOptionPane.OK_CANCEL_OPTION);
	    pane.setWantsInput(true);

	    Object msg = extractMessage(pane, message);

	    if (title == null)
		title = inputDialogTitle;

	    if(showDialog(pane, parentComponent, title, msg) == true)
	    {
		Object selectedValue = pane.getInputValue();
		if(selectedValue != null)
		{
    		    if(selectedValue instanceof String)
			selection = selectedValue.toString();
		
	    	    Trace.msgPrintln("dialogfactory.user.typed", new Object[] {selection});
		}
	    }

	    return selection;
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    public static void showMessageDialog(int messageType, Object message, String title, boolean fModal)    
    {
	showMessageDialog(null, messageType, message, title, fModal);    
    }

    public static void showMessageDialog(final Component parentComponent, final int messageType, 

	final Object message, final String title, final boolean fModal) {
	try {
	    DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    showMessageDialogImpl(parentComponent, messageType, message, title, fModal);
		    return null;
		}});
	}
	catch(Exception e) {
	    // should never happen
	    Trace.ignoredException(e);
	}
    }


    private static void showMessageDialogImpl(Component parentComponent, int messageType, Object message, String title, boolean fModal)    
    {
	LookAndFeel lookAndFeel = null;
	
	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

    	    JOptionPane pane = new JOptionPane();

	    switch (messageType)
	    {
		case DialogFactory.ERROR_MESSAGE: 
		    pane.setMessageType(JOptionPane.ERROR_MESSAGE);
		    break;
		case DialogFactory.INFORMATION_MESSAGE: 
		    pane.setMessageType(JOptionPane.INFORMATION_MESSAGE);
		    break;
		case DialogFactory.WARNING_MESSAGE: 
		    pane.setMessageType(JOptionPane.WARNING_MESSAGE);
		    break;
		case DialogFactory.QUESTION_MESSAGE: 
		    pane.setMessageType(JOptionPane.QUESTION_MESSAGE);
		    break;
		default:
    //		pane.setIcon(loadIcon());
		    break;
	    }

	    pane.setOptionType(JOptionPane.DEFAULT_OPTION);
	    pane.setWantsInput(false);

	    Object msg = extractMessage(pane, message);

	    if (title == null)
		title = messageDialogTitle;

	    showDialog(pane, parentComponent, title, msg, fModal);
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    public static void showInformationDialog(Object message, String title)    
    {
	showInformationDialog(null, message, title);
    }

    public static void showInformationDialog(Component parentComponent, Object message, String title)    
    {
	showMessageDialog(parentComponent, INFORMATION_MESSAGE, message, title, true);
    }

    public static void showErrorDialog(String message, String title)    
    {
	showErrorDialog(null, message, title);
    }

    public static void showErrorDialog(Component parentComponent, String message, String title)    
    {
	showMessageDialog(parentComponent, ERROR_MESSAGE, message, title, true);
    }

    public static void showExceptionDialog(Throwable ex, String message)    
    {
	showExceptionDialog(ex, message, exceptionDialogTitle);
    }

    public static void showExceptionDialog(Component parentComponent, Throwable ex, String message)    
    {
	showExceptionDialog(parentComponent, ex, message, exceptionDialogTitle);
    }

    public static void showExceptionDialog(Throwable e, String message, String title)    
    {
	showExceptionDialog(null, e, message, title);
    }


    public static void showExceptionDialog(final Component parentComponent, final Throwable e, final String message, final String title) {
	try {
	    DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    showExceptionDialogImpl(parentComponent, e, message, title);
		    return null;
		}});
	}
	catch(Exception err) {
	    // should never happen
	    Trace.ignoredException(err);
	}
    }

    public static void showProxyDialog() {
	Trace.println("ProxyDialogNotImplemented !!!! XXX", TraceLevel.NETWORK);
    }

    private static void showExceptionDialogImpl(Component parentComponent, Throwable e, String message, String title)    
    {
	LookAndFeel lookAndFeel = null;
	
	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    JOptionPane pane = new JOptionPane();
	    pane.setMessageType(JOptionPane.ERROR_MESSAGE);
	    pane.setOptionType(JOptionPane.DEFAULT_OPTION);
	    pane.setWantsInput(false);

	    final Component ex = (Component) extractMessage(pane, e);

	    // Create morePanel which only contains a "More" button
	    final JPanel morePanel = new JPanel();
	    JButton moreButton = new JButton(ResourceManager.getMessage("dialogfactory.moreInfo")); 
	    moreButton.setMnemonic(ResourceManager.getAcceleratorKey("dialogfactory.moreInfo")); 
	    morePanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
	    morePanel.add(moreButton);

	    // Create lessPanel which contains a "Less" button and the exception message
	    final JPanel lessPanel = new JPanel();
	    JButton lessButton = new JButton(ResourceManager.getMessage("dialogfactory.lessInfo")); 
	    lessButton.setMnemonic(ResourceManager.getAcceleratorKey("dialogfactory.lessInfo")); 
	    JPanel buttonPanel = new JPanel();
	    buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
	    buttonPanel.add(lessButton);
	    lessPanel.setLayout(new BorderLayout());
	    lessPanel.add(buttonPanel, BorderLayout.CENTER);
	    lessPanel.add(ex, BorderLayout.SOUTH);

	    // containerPanel either contains morePanel or lessPanel
	    final JPanel containerPanel = new JPanel();
	    containerPanel.setLayout(new BorderLayout());
	    containerPanel.add(morePanel, BorderLayout.CENTER);


	    // If "More Details" is pressed, dd exception info
	    moreButton.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
		{
		    Component source = (Component) e.getSource();
		    JDialog dialog = null;

		    // Iterate through container to find JDialog
		    while (source.getParent() != null)
		    {
			if (source instanceof JDialog)
			    dialog = (JDialog) source;

			source = source.getParent();
		    }

		    if (dialog != null)
		    {
			dialog.setVisible(false);
			dialog.setResizable(true);

			// Replace button
			containerPanel.remove(morePanel);
			containerPanel.add(lessPanel, BorderLayout.CENTER);

			// Force dialog to layout new component
			dialog.doLayout();
			dialog.pack();

			dialog.setResizable(false);
			dialog.setVisible(true);
		    }
		}
	    });

	    // If "Less Details" is pressed, remove exception info
	    lessButton.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
		{
		    Component source = (Component) e.getSource();
		    JDialog dialog = null;

		    // Iterate through container to find JDialog
		    while (source.getParent() != null)
		    {
			if (source instanceof JDialog)
			    dialog = (JDialog) source;

			source = source.getParent();
		    }

		    if (dialog != null)
		    {
			dialog.setVisible(false);
			dialog.setResizable(true);

			// Replace button
			containerPanel.remove(lessPanel);
			containerPanel.add(morePanel, BorderLayout.CENTER);

			// Force dialog to layout new component
			dialog.doLayout();
			dialog.pack();

			dialog.setResizable(false);
			dialog.setVisible(true);
		    }
		}
	    });


	    Object[] msgs = new Object[2];
	    msgs[0] = extractMessage(pane, message);
	    msgs[1] = containerPanel;

	    if (title == null)
		title = exceptionDialogTitle;

	    showDialog(pane, parentComponent, title, msgs);
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }


    private static boolean showDialog(JOptionPane pane, Component parentComponent, String title, Object msg)
    {
	return showDialog(pane, parentComponent, title, msg, true, false);
    }
    
    private static boolean showDialog(JOptionPane pane, Component parentComponent, String title, Object msg, boolean fModal)
    {
	return showDialog(pane, parentComponent, title, msg, fModal, false);
    }
    
    private static boolean showDialog(final JOptionPane pane, final Component parentComponent, 
				      final String title, final Object msg, final boolean fModal, final boolean preload)
    {
      boolean dialogNoError = true;
	try {
	    pane.setValue(null);

	    Runnable work = new Runnable()
	    {
		public void run()
		{
			// The app may have a different decorated state for JDialog, 
			// so it is important to set it and restore it if necessary.
			//

			// Store decorated state
			// boolean isDecorated = JDialog.isDefaultLookAndFeelDecorated();

			// Set decorated state to true
			// if (isDecorated != true)
			//    JDialog.setDefaultLookAndFeelDecorated(true);

			// We need visible owner for modal dialogs on Windows
			// because bug #4255200 can cause dialog to get lost
			JFrame fown = getFrameOwner(parentComponent, 
						    title, fModal);

			// Create dialog
		        final JDialog dialog = pane.createDialog(
			    (fown != null) ? fown : parentComponent, title); 

                        // add DefaultActionListener for option buttons
                        Object[] options = pane.getOptions();

                        if (null != options) {
                           DefaultActionListener lsr = new DefaultActionListener(dialog, pane);
                           JButton btn;

                           for (int index=0; index < options.length; index++) {
                               if (options[index] instanceof JButton) {
                                  btn = (JButton)options[index];
				  btn.addActionListener(lsr);
                               }
                           }
                         }

			pane.setMessage(msg);
			dialog.pack();

			// Call method to fix swing bug.
			fixSwingLayoutBug(msg);	
			dialog.pack();

			dialog.setResizable(false); 
			dialog.setModal(fModal);

			positionDialog(dialog);

			// Show dialog
		    	if (preload == false) {
			    if (dialogListener != null) {
				dialogListener.beforeShow();
			    }
			    if (fown != null) { 
				fown.setVisible(true); 
			    }
			    dialog.setVisible(true);
			}
		        if (fown != null) { 
			    fown.setVisible(false);
			    fown.dispose(); 
			}

			// Restore decorated state to true
			//if (isDecorated != true)
			//   JDialog.setDefaultLookAndFeelDecorated(isDecorated);
		    }
		};

	  	String pluginVer = (String)AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run(){
                                return System.getProperty("javaplugin.version");
                        }
                });

		if (pluginVer == null || (preload == true) || SwingUtilities.isEventDispatchThread())
		{
			// If we are already in event dispatch thread or called from a classloader, just call it.
			work.run();	
		}
		else
		{
		    if (fModal)
		    {
			SwingUtilities.invokeAndWait(work);
		    }
		    else
		    {
			SwingUtilities.invokeLater(work);
		    }
		}
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	    dialogNoError = false;
	}

	return dialogNoError;
    }

    private static JFrame getFrameOwner(Component owner, 
					String title, boolean modal) {
        if (owner != null || !modal || !Config.getOSName().equals("Windows")) {
	    return null;
	}
	JFrame f = new JFrame(title);
	f.setLocation(-200, -200);
	return f;
    }

    /** 
     * placeWindow 
     *
     *      code supplied by UI team for best placement of an owned window
     */
    public static void placeWindow(Window window) {
        Window owner = window.getOwner();
        Rectangle screenBounds = new Rectangle(new Point(0,0),
                                 Toolkit.getDefaultToolkit().getScreenSize());
        Rectangle winBounds = window.getBounds();
        Rectangle ownerBounds = (owner == null || !owner.isVisible()) ?
                                        screenBounds : owner.getBounds();
        double goldenOffset = ownerBounds.height - (ownerBounds.height / 1.618);        winBounds.x = ownerBounds.x + (ownerBounds.width - winBounds.width)/2;
        int computedOffset = (int) (goldenOffset - winBounds.height/2);

        // if the owner is smaller (less height) than the window this
        // goldenMean offset computation results in computed offset < 0.
        // this causes dialog to obscure parent - make minimum of parents inset
        int minOffset = (owner == null) ? 0 : owner.getInsets().top;

        winBounds.y = ownerBounds.y + Math.max(computedOffset, minOffset);

        if ((winBounds.x + winBounds.width) > screenBounds.width) {
            winBounds.x = Math.max(screenBounds.width - winBounds.width, 0);
        }
        if ((winBounds.y + winBounds.height) > screenBounds.height) {
            winBounds.y = Math.max(screenBounds.height - winBounds.height, 0);
        }
        window.setBounds(winBounds);
    }

    public static void positionDialog(Dialog dialog) {
	Window w = dialog.getOwner();
	if (w != null && w.isVisible()) {
	    placeWindow(dialog);
	} else {
	    // no visible owner, position relative to screen
	    Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
	    dialog.setLocation((screen.width-dialog.getWidth())/2,
			       (screen.height-dialog.getHeight())/2);
	}
    }

    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, Throwable message)
    {
	return formatExceptionMessage(pane, message);
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, String message)
    {
	return formatStringMessage(pane, message);
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, Object[] messages)
    {
	Object[] result = null;

	if (messages != null)
	{
	    result = new Object[messages.length];

	    for (int i=0; i < messages.length; i++)
		result[i] = extractMessage(pane, messages[i]);
	}

	return result;
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, Object message)
    {
	if (message instanceof Object[])
	    return extractMessage(pane, (Object[]) message);
	else if (message instanceof String)
	    return extractMessage(pane, (String) message);
	else if (message instanceof Exception)
	    return extractMessage(pane, (Exception) message);
	else
    	    return message;
    }

    private static Object formatStringMessage(JOptionPane pane, String in)
    {
	if (in == null)
	    in = "null";

	int i = in.indexOf("</html>");

	if (i == -1)
	{
	    // It doesn't contain HTML text

	    JTextArea text = new JTextArea() {
		public void paintComponent(Graphics g) {
		    setBackground(new Color(
			getParent().getBackground().getRGB()));
		    super.paintComponent(g);
		}
	    };
	    text.setFont(ResourceManager.getUIFont());
	    text.setColumns(40);
	    text.setAutoscrolls(true);
	    text.setEditable(false);
	    text.setLineWrap(true);
	    text.setWrapStyleWord(true);
	    text.setText(in);
	    text.addFocusListener(new FocusAdapter() {
		public void focusGained(FocusEvent e) {
		    ((JTextArea)(e.getSource())).transferFocus();
     		}
	    } );
	    return text;
	}
	else
	{
	    // Contains <html> and </html>
	    //
	    if ((i + 7) == in.length())
	    {
		// Contains <html> and </html> only
		return new JLabel(in);
	    }
	    else
	    {
		// Some strings follows </html>
    		Object[] msgs = new Object[2];

		msgs[0] = new JLabel(in.substring(0, i + 7));
		msgs[1] = formatStringMessage(pane, in.substring(i + 7));

		return msgs;
	    }
	}
    }

    private static Object formatExceptionMessage(JOptionPane pane, Throwable e)
    {
	StringWriter byt = new StringWriter();
	PrintWriter print = new PrintWriter(byt);
	e.printStackTrace(print);

        JTextArea text = new JTextArea() {
	    public void paintComponent(Graphics g) {
                setBackground(new Color(
		    getParent().getBackground().getRGB()));
                super.paintComponent(g);
	    }
        };

	text.setText("" + byt.toString());
 	text.setFont(ResourceManager.getUIFont());
	text.setColumns(40);
	text.setRows(10);
	text.setEditable(false);
        text.addFocusListener(new FocusAdapter() { 
            public void focusGained(FocusEvent e) { 
                ((JTextArea)(e.getSource())).transferFocus(); 
            }
        } ); 
	JScrollPane spane = new JScrollPane( text ); 
	text.setBackground(pane.getBackground());

	return spane;
    }


    /**
     * Fix Swing Layout bug.
     */ 
    private static void fixSwingLayoutBug(Object msg)
    {
        if (msg == null)
	{
           return;
	}
        else if (msg instanceof JTextArea)
        {
           JTextArea text = (JTextArea) msg;
           text.getUI().getPreferredSize(text);
           return;
        }
        else if (msg.getClass().isArray())
        {
           int len = Array.getLength(msg);

           for (int i=0; i<len; i++)
	   {
               fixSwingLayoutBug(Array.get(msg, i));
	   }
        }
    }

}

class DefaultActionListener implements ActionListener{
 
     private Dialog dlg;
     private JOptionPane pane;
     
     /** Creates new DefaultActionListener */
     protected DefaultActionListener(Dialog dlg, JOptionPane pane) {
         this.dlg = dlg;
         this.pane = pane;
     }
     
     /**
      * Invoked when an action occurs.
      */
     public void actionPerformed(ActionEvent e) {
	pane.setValue(e.getSource());
	dlg.setVisible(false);
     }

}
