/*
 * @(#)UIFactory.java	1.45 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

import java.net.URL;
import java.net.PasswordAuthentication;
import java.util.ArrayList;
import java.awt.Dialog;
import java.awt.Window;
import java.awt.Toolkit;
import java.awt.Image;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Point;
import java.awt.MouseInfo;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.security.cert.Certificate;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import javax.swing.JFrame;
import javax.swing.JList;
import java.util.HashMap;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.AssociationDesc;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DialogListener;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.security.CredentialInfo;

public class UIFactory{

    public final static int ERROR        = -1;
    public final static int OK           =  0;
    public final static int CANCEL       =  1;
    public final static int ALWAYS       =  2;
    public final static int ASK_ME_LATER =  3;
    
    private static long tsLastActive = 0;
    private static DialogListener dialogListener = null;
    private static DialogHook dialogHook;

    /**
     * @param   ainfo      - AppInfo object for this context
     * @param   title      - dialog title string from resource bundle
     * @param   topText    - short description of the dialog to be displayed in
     *                       top sub panel
     * @param   publisher  - publisher name. 
     * @param   appFrom    - location
     * @param   showAlways - should "Always" checkbox be shown
     * @param   checkAlways- should "Always" checkbox be checked
     * @param   okBtnStr   - strings to be used for OK button
     * @param   cancelBtnStr - strings to be used for Cancel button
     * @param   alerts     - array of string alerts
     * @param   info       - array of string info
     * @param   showMoreInfo - should "More Information" link be displayed
     * @param   certs      - array of parsed certifivates
     * @param   start      - start index in certs array
     * @param   end        - end index in certs array
     */
    public static int showSecurityDialog( final AppInfo ainfo,
                                          final String title,
                                          final String topText,
                                          final String publisher,
                                          final URL appFrom,
                                          final boolean showAlways,
                                          final boolean checkAlways,
                                          final String okBtnStr,
                                          final String cancelBtnStr,
                                          final String [] securityAlerts,
                                          final String [] securityInfo,
                                          final boolean showMoreInfo,
                                          final Certificate[] certs,
                                          final int start,
                                          final int end,
                                          final boolean majorWarning )
    {
        final Component parent = beforeDialog(null);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
                new DeploySysAction() {
                public Object execute() throws Exception {                                        
                
                    // add the desktop integration items to securityInfo array.
                    String [] info = new String[0];
                    if (securityInfo != null) {
                        info = securityInfo;
                    }

                    int securityInfoCount = info.length;
                    info = addDetail(info, ainfo, true, true);
		    ainfo.setVendor(publisher);
		    ainfo.setFrom(appFrom);

                    DialogTemplate template = 
			new DialogTemplate(ainfo, parent, title, topText);

		    // Show extension install dialog box if certificate is valid
   		    if (ainfo.getType() == 3 && securityAlerts == null) {
                      template.setSecurityContent(false,
                        checkAlways, okBtnStr, cancelBtnStr,
                        securityAlerts, info, securityInfoCount,
                        showMoreInfo, certs, start, end, majorWarning );
		    }
		    else {
                      template.setSecurityContent(showAlways,
                        checkAlways, okBtnStr, cancelBtnStr,
                        securityAlerts, info, securityInfoCount,
                        showMoreInfo, certs, start, end, majorWarning );
		    }

                    placeWindow(template.getDialog());

                    template.setVisible(true);
                
                    // we won't return from setVisible(true) until the dialog
                    // is not visible anymore == user clicked on one 
                    // of the buttons.                

                    int userAnswer = template.getUserAnswer();
                    template.disposeDialog(); 
                    return new Integer(userAnswer);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }

    /**
     * showIntegrationDialog()
     *
     * @param  owner   - Component to parent the dialog to
     * @param  ainfo   - AppInfo object for this context
     * 
     */
    public static int showIntegrationDialog(Component owner,
                                            final AppInfo ainfo ) {
        final Component fOwner = beforeDialog(owner);
        try {
            return ((Integer)DeploySysRun.execute(new DeploySysAction() {
                public Object execute() throws Exception {
                    String title = 
                        ResourceManager.getString("integration.title");
                    boolean doShortcut = 
                        ainfo.getDesktopHint() || ainfo.getMenuHint();
                    boolean doAssociation = ainfo.getAssociations() != null && 
                        ainfo.getAssociations().length > 0;

                    String key = "integration.text.shortcut";
                    if (doAssociation) {
                        if (doShortcut) {
                            key = "integration.text.both";
                        } else {
                            key = "integration.text.association";
                        }
                    }
                    String topText = ResourceManager.getString(key);

                    String [] security = new String[0];
                
                    // put association detail in the alerts array
                    String [] alerts = new String[0];
                    alerts = addDetail(alerts, ainfo, false, true);

                    String [] info = new String[0];
                    // put shortcut detail in the info array
                    info = addDetail(info, ainfo, true, false);

                    boolean showMoreInfo = alerts.length + info.length > 1;

                    String okBtnStr = 
                        ResourceManager.getString("common.ok_btn");
                    String cancelBtnStr = 
                        ResourceManager.getString("integration.skip.button");

                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, topText);

                    template.setSecurityContent(false,
                        false, okBtnStr, cancelBtnStr,
                        alerts, info, 0, showMoreInfo, null, 0, 0, false );

                    placeWindow(template.getDialog());

                    template.setVisible(true);
                
                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }

    /**
     * showErrorDialog (original form) - with label
     */
    public static void showErrorDialog(Component owner, 
                                      final AppInfo ainfo,
                                      final String title,
                                      final String message1,
                                      final String message2,
                                      final String okBtnStr,
                                      final String detailBtnStr,
                                      final Throwable throwable,
                                      final JPanel detailPanel,
				      final Certificate[] certs) {
        showErrorDialog(owner, ainfo, title, message1, message2,
			okBtnStr, detailBtnStr, throwable, detailPanel, certs, false);
    }

    /**
     * showErrorDialog()
     *
     * @param  owner           - Component to parent the dialog to
     * @param  ainfo           - AppInfo object for this context
     * @param  title           - dialog title string from resource bundle
     * @param  messaage1       - this is the text to diaplay in the masthead
     * @param  messaage2       - this is the text to diaplay in the body
     * @param  okBtnStr        - strings to be used for OK button
     * @param  detailBtnStr    - strings to be used for Detail button
     * @param  throwable       - this is the exception to show on detail
     * @param  detailPanel     - this is the content to show on detail
     * @param  certs           - Certificate [] for BadCertificateDialog
     * @param  hideLabel       - hide label in center panel
     *
     */
    public static int showErrorDialog(Component owner,
                                      final AppInfo ainfo,
                                      final String title,
                                      final String message1,
                                      final String message2,
                                      final String okBtnStr,
                                      final String detailBtnStr,
                                      final Throwable throwable,
                                      final JPanel detailPanel,
				      final Certificate[] certs,
				      final boolean hideLabel) {
        final Component fOwner = beforeDialog(owner);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
                public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, message1);
                        

                    template.setErrorContent(message2, okBtnStr, detailBtnStr, 
                                             throwable, detailPanel, certs, hideLabel);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }

    /*
     * Show Error dialog with 2 or 3 buttons.  
     * Return the number of the button selected by user.  
     * Left to right:
     * first button = UIFactory.OK              =  0;
     * second button = UIFactory.CANCEL         =  1;
     * third button = UIFactory.ASK_ME_LATER    =  3;
     *
     * This dialog has masthead with error icon, 
     * middle panel with message and 2 or 3 action buttons.
     */
    public static int showErrorDialog(Component owner,
                                      final AppInfo ainfo,
                                      final String masthead,
                                      final String message,
                                      final String btnOneKey,
                                      final String btnTwoKey,
                                      final String btnThreeKey) {
        final Component fOwner = beforeDialog(owner);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
                public Object execute() throws Exception {
                    String title = 
                            ResourceManager.getString("error.default.title");
                    
                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, masthead);
                        

                    template.setMultiButtonErrorContent(
                            message, btnOneKey, btnTwoKey, btnThreeKey);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }

            
    /*
     * Call showPasswordDialog with scheme NULL.
     * @param parent - parent for the password dialog
     * @param title - title for the password dialog, should be translated 
     *                already.
     * @param notes - text explaning what the password is for.  Should
     *                be translated already.
     * @param showUsername - should user name label/text field pair be shown
     * @param showDomain - should domain label/text field pair be shown
     * @param info - Information to suggest to the user
     * @param saveEnabled - should save check box be shown
     *
     * @return CredentialInfo object.
     */
    public static CredentialInfo showPasswordDialog( 
                                                final Component parent, 
                                                final String title,
                                                final String notes,
                                                final boolean showUsername,
                                                final boolean showDomain,
                                                final CredentialInfo info,
                                                final boolean saveEnabled ){
        return showPasswordDialog( parent, title, notes, showUsername, 
                                   showDomain, info, saveEnabled, null);
    }
        
    /*
     * Show Password Dialog in deploy threadgroup.
     * @param parent - parent for the password dialog
     * @param title - title for the password dialog, should be translated 
     *                already.
     * @param notes - text explaning what the password is for.  Should
     *                be translated already.
     * @param showUsername - should user name label/text field pair be shown
     * @param showDomain - should domain label/text field pair be shown
     * @param info - Information to suggest to the user
     * @param saveEnabled - should save check box be shown
     * @param scheme - authentication scheme
     *
     * @return CredentialInfo object.
     */
    public static CredentialInfo showPasswordDialog( 
                                                Component parent, 
                                                final String title,
                                                final String notes,
                                                final boolean showUsername,
                                                final boolean showDomain,
                                                final CredentialInfo info,
                                                final boolean saveEnabled,
                                                final String scheme ){
        final Component fParent = beforeDialog(parent);
        try {
            return (CredentialInfo) DeploySysRun.executePrivileged(
                new DeploySysAction() {
                    public Object execute() { 
                        CredentialInfo result = null;
                        CredentialInfo input = info;
                
                        DialogTemplate template = 
                            new DialogTemplate(new AppInfo(), fParent, title, "");
                
                        if (input == null) {
                            input = new CredentialInfo();
                        }
                
                        template.setPasswordContent( notes, showUsername, showDomain, 
                                                     input.getUserName(), input.getDomain(), saveEnabled, 
                                                     input.getPassword() , scheme );
                                    
                        placeWindow(template.getDialog());
                        template.setVisible(true);
                
                        // Need to differentiate between "Cancel" and "OK" with
                        // empty field(s).  If answer from the dialog is anything
                        // but OK - return <null>.
                        int answer = template.getUserAnswer();
                
                        if ( answer == OK || answer == ALWAYS) {
                            result = new CredentialInfo();
                            result.setUserName( template.getUserName() );
                            result.setDomain( template.getDomain() );                
                            result.setPassword( template.getPassword() );  
                            result.setPasswordSaveApproval( template.isPasswordSaved() );
                        }
                    
                        // Once we return from the password dialog, retrieve
                        // the password and then dispose of the dialog.
                        template.disposeDialog();
                
                        return result;
                
                    }
                }, null);
        } finally {
            afterDialog();
        }
    }
          

    /**
     * showErrorDialog (simplest form) - only masthead, no body
     */
    public static void showErrorDialog(Component owner, 
                                       String masthead, 
                                       String title) {   
        showErrorDialog(owner, masthead, null, title);
    }
    
    /*
     * showErrorDialog - simple form with masthead and body, explaining 
     *                   the possible cause of error.
     */
    public static void showErrorDialog(Component owner, 
                                       String masthead,
                                       String msg, 
                                       String title) {
        String btnString = ResourceManager.getString("common.ok_btn");
        if (title == null) {
            title = ResourceManager.getString("error.default.title");
        }
        showErrorDialog(owner, new AppInfo(), title, masthead, msg, 
                        btnString, null, null, null, null);
    }
    
    
    /**
     * showExceptionDialog - simple form with no body.  Wrapper for the 
     * same dialog with body.
     */
    public static void showExceptionDialog(Component owner, 
                                           Throwable throwable, 
                                           String masthead, 
                                           String title) {
        showExceptionDialog(owner, throwable, masthead, null, title);        
    }
    
    /*
     * showExceptionOCSPDialog with body message and without label, 
     * explaining possible cause of the exception due to OCSP validatoin.
     */
    public static void showExceptionOCSPDialog(Component owner, 
                                           Throwable throwable,
                                           String masthead,
                                           String msg, 
                                           String title) {
        String closeBtnStr = ResourceManager.getString("common.close_btn");
        String detailBtnStr = ResourceManager.getString("common.detail.button");
        if (msg == null) {
            msg = throwable.toString();
        }
        showErrorDialog(owner, new AppInfo(), title, masthead, msg, 
                        closeBtnStr, detailBtnStr, throwable, null, null, true);
    }

    /*
     * showExceptionDialog with body message, explaining possible cause
     * of the exception.
     */
    public static void showExceptionDialog(Component owner, 
                                           Throwable throwable,
                                           String masthead,
                                           String msg, 
                                           String title) {
        String okBtnStr = ResourceManager.getString("common.ok_btn");
        String detailBtnStr = ResourceManager.getString("common.detail.button");
        if (msg == null) {
            msg = throwable.toString();
        }
        if (title == null) {
            title = ResourceManager.getString("error.default.title");
        }
        showErrorDialog(owner, new AppInfo(), title, masthead, msg, 
                        okBtnStr, detailBtnStr, throwable, null, null);
    }

    /**
     * showCertificateExceptionDialog
     */
    public static void showCertificateExceptionDialog(Component owner, 
						      AppInfo ainfo, 
                                                      Throwable throwable, 
                                                      String msg, 
                                                      String title,
					              Certificate [] certs) {
        String okBtnStr = ResourceManager.getString("common.ok_btn");
        String detailBtnStr = ResourceManager.getString("common.detail.button");
        if (msg == null) {
            msg = throwable.toString();
        }
        if (title == null) {
            title = ResourceManager.getString("error.default.title");
        }
        showErrorDialog(owner, ainfo, title, msg, null, 
                        okBtnStr, detailBtnStr, throwable, null, certs);
    }



    /**
     * showContentDialog()
     *
     * @param  owner           - Component to parent the dialog to
     * @param  ainfo           - AppInfo object for this context
     * @param  title           - dialog title string from resource bundle
     * @param  content         - content to display
     * @param  scroll          - true if we want to include scroll content
     * @param  okBtnStr        - strings to be used for OK button
     * @param  cancelBtnStr    - strings to be used for Cancel button
     *
     */
    public static int showContentDialog(Component owner,
                                        final AppInfo ainfo,
                                        final String title,
                                        final String content,
                                        final boolean scroll,
                                        final String okBtnStr,
                                        final String cancelBtnStr) {
        final Component fOwner = beforeDialog(owner);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
                public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, null);

                    template.setSimpleContent(content, scroll, null, okBtnStr,
                                              cancelBtnStr, false, false);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }
    /**
     * showConfirmDialog
     *
     * @param  owner           - Component to parent the dialog to
     * @param  message         - question to display
     * @param  title           - dialog title string from resource bundle
     *
     */
    public static int showConfirmDialog(final Component owner, 
                AppInfo appInfo, final String message, final String title) {
	return showConfirmDialog(owner, appInfo, message, null, title);
    }

    /**
     * showConfirmDialog
     *
     * @param  owner           - Component to parent the dialog to
     * @param  message         - question to display
     * @param  info            - further info to show as a bullet
     * @param  title           - dialog title string from resource bundle
     *
     */
    public static int showConfirmDialog(final Component owner, AppInfo appInfo, 
	final String message, final String info, final String title)
    {
        String okBtnStr = ResourceManager.getString("common.ok_btn");
        String cancelBtnStr = ResourceManager.getString("common.cancel_btn");

	return showConfirmDialog(owner, appInfo, message, info, title, 
		okBtnStr, cancelBtnStr, false);
    }

    /**
     * showConfirmDialog
     *
     * @param  owner           - Component to parent the dialog to
     * @param  AppInfo         - application information
     * @param  message         - question to display
     * @param  info            - further info to show as a bullet
     * @param  title           - dialog title string from resource bundle
     * @param  okBtnStr        - String to use for ok button
     * @param  cancelBtnStr    - String to use for cancel button
     * @param  useWarning      - boolean to use security warning icons
     *
     */
    public static int showConfirmDialog(Component owner, AppInfo appInfo, 
	final String message, final String info, final String title,
	final String okBtnStr, final String cancelBtnStr, 
        final boolean useWarning)
    {
	final AppInfo ainfo = (appInfo == null) ? (new AppInfo()) : appInfo;
        final Component fOwner = beforeDialog(owner);
    	try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
		public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, message);

                    template.setSimpleContent(null, false, info, okBtnStr,
                                              cancelBtnStr, true, useWarning);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
		}
            }, new Integer(ERROR))).intValue();
	} catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
	} finally {
            afterDialog();
        }
    }
    
    public static int showMixedCodeDialog(Component owner, AppInfo appInfo, 
        final String title, final String message, final String info, final String bottom,
        final String okBtnStr, final String cancelBtnStr, 
        final boolean useWarning)
    {
        final AppInfo ainfo = (appInfo == null) ? (new AppInfo()) : appInfo;
        final Component fOwner = beforeDialog(owner);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
                new DeploySysAction() {
                public Object execute() throws Exception {

                    DialogTemplate template =  
                        new DialogTemplate(ainfo, fOwner, title, message);

                    template.setMixedCodeContent(null, false, info, bottom, okBtnStr,
                                              cancelBtnStr, true, useWarning);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }
        
    /**
     * showInformationDialog - wrapper
     */
    public static void showInformationDialog(final Component parent,
                                             final String text,
                                             final String title ){
        showInformationDialog(parent, text, null, title);
    }


    /*
     * showInformationDialog
     *
     * Info message string displayed in the masthead
     * Info icon 48x48 displayed in the masthead
     * "OK" button at the bottom.
     *
     * text and title strings are already translated strings.
     */
    public static void showInformationDialog(final Component parent,
                                             final String masthead,
                                             final String text,
                                             final String title ){

        final String okBtnStr = ResourceManager.getString("common.ok_btn");
        final AppInfo ainfo = new AppInfo();
        
        try {
            DeploySysRun.executePrivileged(new DeploySysAction() {		
		public Object execute(){
                    DialogTemplate template =  
			new DialogTemplate(ainfo, parent, title, masthead);
                    
                    template.setInfoContent(text, okBtnStr);
                    
                    placeWindow(template.getDialog());

                    template.setVisible(true);
                    return null;
                }
            }, null);
	} catch (Throwable e){
            Trace.ignored(e);
	}        
    }



    /**
     * showApiDialog
     *
     * @param  owner           - Component to parent the dialog to
     * @param  ainfo           - AppInfo
     * @param  title           - dialog title string from resource bundle
     * @param  message         - question to display
     * @param  label           - the lable above the content
     * @param  files           - the content of the list of files to show
     * @param  always          - the String for the checkbox
     * @param  checked         - the state of the checkbox
     *
     */
    public static int showApiDialog(Component owner, AppInfo appInfo, 
	final String title, final String message, final String label,
	final String files, final String always, final boolean checked)
    {
        final String okBtnStr = ResourceManager.getString("common.ok_btn");
        final String cancelBtnStr = 
	    ResourceManager.getString("common.cancel_btn");
	final AppInfo ainfo = (appInfo == null) ? (new AppInfo()) : appInfo;
        final Component fOwner = beforeDialog(owner);
    	try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
		public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, message);

                    template.setApiContent(files, label, always, checked,
                                              okBtnStr, cancelBtnStr);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
		}
            }, new Integer(ERROR))).intValue();
	} catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
	} finally {
            afterDialog();
        }
    }

    /**
     * createProgressDialog()
     *
     * @param  ainfo      - AppInfo object for this context
     * @param  title      - dialog title string from resource bundle
     * @param  okBtn      - true if we want to include an OK button
     *
     */
    public static ProgressDialog createProgressDialog(final AppInfo ainfo, 
                                                      final Component owner, 
                                                      final String title,
                                                      final String contentStr,
                                                      final boolean okBtn) 
    {
        try {
            return ((ProgressDialog)DeploySysRun.execute(new DeploySysAction() {                
                public Object execute() throws Exception {
                    return new ProgressDialog(ainfo, owner, title, contentStr, 
                            okBtn);
                }
            }));
        } catch (Throwable e) {
            Trace.ignored(e);
            return null;
        }
    }

    /**
     * showProgressDialog()
     *
     * @param progressDialog - the progressDialog to show
     */
    public static int showProgressDialog(final ProgressDialog progressDialog) {
        beforeDialog(null);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
                    new DeploySysAction() {
                        public Object execute() throws Exception {

                    placeWindow(progressDialog.getDialog());
                    progressDialog.setVisible(true);
                    return new Integer(progressDialog.getUserAnswer());
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        } finally {
            afterDialog();
        }
    }
    
    /**
     * hideProgressDialog()
     *
     * @param progressDialog - the progressDialog to show
     */
    public static int hideProgressDialog(final ProgressDialog progressDialog) {
        try {
            return ((Integer)DeploySysRun.executePrivileged(
                    new DeploySysAction() {
                        public Object execute() throws Exception {

                    progressDialog.setVisible(false);
                    return new Integer(OK);
                }
            }, new Integer(ERROR))).intValue();
        } catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
        }
    }
    
    /*
     * Show About Dialog.
     */
    public static void showAboutJavaDialog() {
	if(SwingUtilities.isEventDispatchThread()) {
	    internalShowAboutJavaDialog();
	} else {
	    try {
	        SwingUtilities.invokeAndWait(new Runnable() {
	            public void run() {
		        internalShowAboutJavaDialog();
	            }
	        });
	    } catch (Exception e) {
		Trace.ignored(e);
	    }
	}
    }
            


    private static synchronized void internalShowAboutJavaDialog() {
        if ((System.currentTimeMillis() - tsLastActive) > 500 && 
                AboutDialog.shouldStartNewInstance()) {                    
            AboutDialog dlg = new AboutDialog((JFrame)null, true, true);
            dlg.pack();
            Dimension sd = Toolkit.getDefaultToolkit().getScreenSize();
            Dimension dd = dlg.getSize();
            dlg.setLocation((sd.width - dd.width)/2, (sd.height - dd.height)/2);
	    if (Config.isJavaVersionAtLeast15()) {
		dlg.setAlwaysOnTop(true);
	    }
            dlg.setVisible(true);
            tsLastActive = System.currentTimeMillis();
        }
    }
    
        
    /**
     * showWarningDialog - displays warning icon instead of "Java" logo icon
     *                     in the upper right corner of masthead.  Has masthead
     *                     and message that is displayed in the middle part
     *                     of the dialog.  No bullet is displayed.
     *                     
     *
     * @param  owner           - Component to parent the dialog to
     * @param  appInfo         - AppInfo object
     * @param  masthead        - masthead in the top part of the dialog
     * @param  message         - question to display in the middle part 
     * @param  title           - dialog title string from resource bundle 
     *
     */
    public static int showWarningDialog(final Component owner, 
                AppInfo appInfo, final String masthead, final String message, 
                final String title)
    {
        final String okBtnStr = ResourceManager.getString("common.ok_btn");
        final String cancelBtnStr = 
	    ResourceManager.getString("common.cancel_btn");
        return showWarningDialog(owner, appInfo, masthead, message, title, 
                okBtnStr, cancelBtnStr);
    }
    
    /*
     * wrapper to allow customised names for approve and cancel buttons.
     */
    public static int showWarningDialog(Component owner, 
                AppInfo appInfo, final String masthead, final String message,
                final String title, final String okBtnString, 
                final String cancelBtnString)
    {        
	final AppInfo ainfo = (appInfo == null) ? (new AppInfo()) : appInfo;
        final Component fOwner = beforeDialog(owner);
    	try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
		public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(ainfo, fOwner, title, masthead);

                    template.setSimpleContent(message, false, null, okBtnString, 
                                              cancelBtnString, true, true);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
		}
            }, new Integer(ERROR))).intValue();
	} catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
	} finally {
            afterDialog();
        }
    }
    
    /*
     * showUpdateCheckDialog - has warning icon, question about enabling java
     *                         update, and three buttons:  
     *                         "Yes", "No" and "Ask me later".
     *
     */
    public static int showUpdateCheckDialog(){
        final String yesBtnKey = "autoupdatecheck.buttonYes";
        final String noBtnKey = "autoupdatecheck.buttonNo";
        final String askBtnKey = "autoupdatecheck.buttonAskLater";
        
        final String title = ResourceManager.getMessage(
	            "autoupdatecheck.caption");
                
        final String infoStr = ResourceManager.getMessage(
	            "autoupdatecheck.message");
        
        final String masthead = ResourceManager.getMessage(
                    "autoupdatecheck.masthead");
        
        final AppInfo ainfo = new AppInfo();
        
        final Component owner = beforeDialog(null);
        try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
		public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(ainfo, owner, title, masthead);

                    // Set warning icon before creating content.
                    template.setUpdateCheckContent( infoStr, yesBtnKey,
                            noBtnKey, askBtnKey);                    
                    
                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
		}
            }, new Integer(ASK_ME_LATER))).intValue();
	} catch (Throwable e){
            Trace.ignored(e);
            return ASK_ME_LATER;
	} finally {
            afterDialog();
        }
    }
    
                
    /**
     * showListDialog - has masthead with warning icon in the upper right
     *                  corner; 
     *                - scroll pane with list of items to choose from;
     *                - a label above scroll pane;
     *                - "Certificate Details..." clickable label (link) below
     *                  scroll pane, right-aligned;
     *                - "OK" and "Cancel" buttons.
     *
     * @param owner   - Component to parent the dialog to
     * @param title   - dialgo title string from resource bundle
     * @param message - text to display in masthead  
     * @param label   - label to display above scroll pane
     * @param details - boolean to indicate if "Certificate Details..." 
     *                  clickable label should be shown
     * @param scrollList - list to display in scroll pane
     * @param clientAuthCertsMap - hashmap of X509Certificate(s)  This argument
     *                             is needed to display details of selected
     *                             certificate.
     * 
     */
    public static int showListDialog(Component owner, final String title, 
            final String message, final String label, final boolean details,
            final JList scrollList, final HashMap clientAuthCertsMap)
    {
        final String okBtnStr = ResourceManager.getString("common.ok_btn");
        final String cancelBtnStr = 
                ResourceManager.getString("common.cancel_btn");
        final Component fOwner = beforeDialog(owner);
    	try {
            return ((Integer)DeploySysRun.executePrivileged(
		new DeploySysAction() {
		public Object execute() throws Exception {

                    DialogTemplate template =  
			new DialogTemplate(new AppInfo(), fOwner, 
                            title, message);

                    template.setListContent(label, scrollList, details,
                            okBtnStr, cancelBtnStr, clientAuthCertsMap);

                    placeWindow(template.getDialog());

                    template.setVisible(true);

                    int userAnswer = template.getUserAnswer();

                    template.disposeDialog();
                    return new Integer(userAnswer);
		}
            }, new Integer(ERROR))).intValue();
	} catch (Throwable e){
            Trace.ignored(e);
            return ERROR;
	} finally {
            afterDialog();
        }
    }
    


    private static String[] addDetail ( String[] list, 
                                        AppInfo ainfo, 
                                        boolean doShortcut, 
                                        boolean doAssociation) {
        String title = ainfo.getTitle();
        if (title == null) { 
            title = ""; 
        }

        ArrayList al = new ArrayList();
        for (int i=0; i<list.length; i++) {
            al.add(list[i]);
        }
        if (doAssociation) {
            AssociationDesc [] ad = ainfo.getAssociations();
            if (ad != null) {
                for (int i=0; i<ad.length; i++) {
                    String extensions = ad[i].getExtensions();
                    String mimeType = ad[i].getMimeType();
                    String description = ad[i].getMimeDescription();
                    String message = ResourceManager.getString(
                        "association.dialog.ask", mimeType, extensions);
                    al.add(message);
                }
            }
        }
        if (doShortcut) {
            String message = null;
            if (ainfo.getDesktopHint() && ainfo.getMenuHint()) {
                if (Config.getOSName().equalsIgnoreCase("Windows")) {
                    message = ResourceManager.getString(
                        "install.windows.both.message");
                } else {
                    message = ResourceManager.getString(
                        "install.gnome.both.message");
                }
            } else if (ainfo.getDesktopHint()) {
                message = ResourceManager.getString("install.desktop.message");
            } else if (ainfo.getMenuHint()) {
                if (Config.getOSName().equalsIgnoreCase("Windows")) {
                    message = ResourceManager.getString(
                        "install.windows.menu.message");
                } else {
                    message = ResourceManager.getString(
                        "install.gnome.menu.message");
                }
            }
            if (message != null) {
                al.add(message);
            }
        }
        return (String []) al.toArray(list);
    }


    /**
     * placeWindow
     *
     *      code supplied by UI team for best placement of an owned window
     *      modified by awt team to avoid spaning mutiple screens
     */
    public static void placeWindow(Window window) {
        Window owner = window.getOwner();
        if (ignoreOwnerVisibility()) {
            owner = null;
        }
	boolean ownerVis = (owner != null && owner.isVisible());

        Rectangle screenBounds = getMouseScreenBounds();
        Rectangle winBounds = window.getBounds();
        Rectangle ownerBounds = (owner == null || !owner.isVisible()) ?
                                        screenBounds : owner.getBounds();
	if (ownerBounds.x + ownerBounds.width < 0) {
	    ownerVis = false;
	    ownerBounds = screenBounds;
	}
        double goldenOffset = ownerBounds.height -
                (ownerBounds.height / 1.618);

        winBounds.x = ownerBounds.x +
                (ownerBounds.width - winBounds.width)/2;
        int computedOffset = (int) (goldenOffset - winBounds.height/2);

        // if the owner is smaller (less height) than the window this
        // goldenMean offset computation results in computed offset < 0.
        // this causes dialog to obscure parent -
        // so make minimum of parents inset

        int minOffset = (ownerVis) ? owner.getInsets().top : 
	    (screenBounds.height / 2 - (2 * winBounds.height) / 3);

        winBounds.y = ownerBounds.y + Math.max(computedOffset, minOffset);

        if ((winBounds.x + winBounds.width) > 
            (screenBounds.x + screenBounds.width)) {
            winBounds.x = screenBounds.x +
                          Math.max(screenBounds.width - winBounds.width, 0);
        }
        if ((winBounds.y + winBounds.height) > 
            (screenBounds.y + screenBounds.height)) {
            winBounds.y = screenBounds.y +
                          Math.max(screenBounds.height - winBounds.height, 0);
        }
	// on Solaris, when already running inside dispatch Thread, setBounds
        // before showing prevents the pack from working right;

	// ensuring the x and y coordinates are within the screen boundary
	// because for lower resolution settings such as 1024x600, part of the top of 
	// the Java Control Panel window won't be visible without this check
	if (winBounds.y < 0 || winBounds.x < 0) {
	    //center it in middle of screen
	    Dimension sd = Toolkit.getDefaultToolkit().getScreenSize();
	    Dimension wd = window.getSize();
	    window.setLocation(Math.abs(sd.width - wd.width)/2, Math.abs(sd.height - wd.height)/2);
	} else {
	    window.setLocation(winBounds.x, winBounds.y);
	}
    }

    public static Rectangle getMouseScreenBounds() {
        if (Config.isJavaVersionAtLeast13()) {
            Point mousePoint = new Point(1,1);
            if (Config.isJavaVersionAtLeast15()) {
                mousePoint = MouseInfo.getPointerInfo().getLocation();
            }
            GraphicsDevice[] devices = GraphicsEnvironment.
                getLocalGraphicsEnvironment().getScreenDevices();
            for (int i=0; i<devices.length; i++) {
                Rectangle bounds = 
                    devices[i].getDefaultConfiguration().getBounds();
                //check to see if the mouse cursor is within these bounds
                if (mousePoint.x >= bounds.x && mousePoint.y >= bounds.y
                    && mousePoint.x <= (bounds.x + bounds.width)
                    && mousePoint.y <= (bounds.y + bounds.height)) {
                    //this is it
                    return bounds;
                }
            }
        }
        return new Rectangle(new Point(0,0),
            Toolkit.getDefaultToolkit().getScreenSize());
    }

    public static void setDialogListener(DialogListener dl) {
        dialogListener = dl;
    }

    public static DialogListener getDialogListener() {
        return dialogListener;
    }

    public static void setDialogHook(DialogHook hook) {
        dialogHook = hook;
    }

    public static DialogHook getDialogHook() {
        return dialogHook;
    }

    private static Component beforeDialog(Component component) {
        if (dialogHook != null) {
            return dialogHook.beforeDialog(component);
        } else {
            return component;
        }
    }

    private static boolean ignoreOwnerVisibility() {
        if (dialogHook != null) {
            return dialogHook.ignoreOwnerVisibility();
        } else {
            return false;
        }
    }

    private static void afterDialog() {
        if (dialogHook != null) {
            dialogHook.afterDialog();
        }
    }
}
