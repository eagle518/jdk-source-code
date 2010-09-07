/*
 * @(#)TrustDeciderDialog.java	1.71 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URL;
import java.security.Principal;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.text.MessageFormat;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.AppInfo;



class TrustDeciderDialog {

    public static final int TrustOption_GrantThisSession = 0;
    public static final int TrustOption_Deny = 1;
    public static final int TrustOption_GrantAlways = 2;
    
   
    /**
     * <P> Show TrustDeciderDialog.
     * </P>
     *
     * @return 0 if "Grant this session" button is clicked.
     * @return 1 or -1 if "Deny" is clicked.
     * @return 2 if "Grant Always" button is clicked.
     */
    public static int showDialog(                
		final java.security.cert.Certificate[] certs, 
                URL url,       
		final int start, final int end, boolean rootCANotValid, 
                int validityState, Date timeStampDate,
		AppInfo ainfo, boolean httpsDialog)  
                throws CertificateException
    {
        return showDialog(certs, url, start, end, rootCANotValid, 
                validityState, timeStampDate, ainfo, httpsDialog, null, false);
    }
    
    public static int showDialog(                
		final java.security.cert.Certificate[] certs, 
                URL url,       
		final int start, final int end, boolean rootCANotValid, 
                int validityState, Date timeStampDate,
		AppInfo ainfo, boolean httpsDialog, String hostname)  
                throws CertificateException
    {
        return showDialog(certs, url, start, end, rootCANotValid, 
                validityState, timeStampDate, ainfo, httpsDialog, hostname, false);
    }
    
    public static int showDialog(                
		final java.security.cert.Certificate[] certs, 
                URL url,       
		final int start, final int end, boolean rootCANotValid, 
                int validityState, Date timeStampDate,
		AppInfo ainfo, boolean httpsDialog, String hostname, boolean jnlpFlag)  
                throws CertificateException
    {
        int ret = -1;

        // Check if the certificate is a x.509 certificate
        //
        if (certs[start] instanceof X509Certificate 
	    && certs[end-1] instanceof X509Certificate)
        {
	    X509Certificate cert = (X509Certificate) certs[start];
	    X509Certificate cert2 = (X509Certificate) certs[end-1];

	    Principal prinSubject = cert.getSubjectDN();
	    Principal prinIssuer = cert2.getIssuerDN();

	    // Extract subject name
	    String subjectDNName = prinSubject.getName();

	    String subjectName = null;

	    int i = subjectDNName.indexOf("CN=");
	    int j = 0;

	    if (i < 0) {
	        subjectName = getMessage("security.dialog.unknown.subject");
	    } else {
	        try {
		    // Shift to the beginning of the CN text
		    i = i + 3;
    
		    // Check if it begins with a quote
		    if (subjectDNName.charAt(i) == '\"') {
		        // Skip the quote
		        i = i + 1;
    
		        // Search for another quote
		        j = subjectDNName.indexOf('\"', i);
		    } else {

		        // No quote, so search for comma
		        j = subjectDNName.indexOf(',', i);
		    }
    
		    if (j < 0) {
		        subjectName = subjectDNName.substring(i);
		    } else {
		        subjectName = subjectDNName.substring(i, j);
		    }
	        } catch (IndexOutOfBoundsException e) {
		    subjectName = 
			getMessage("security.dialog.unknown.subject");
	        }
	    }


	    // Extract issuer name
	    String issuerDNName = prinIssuer.getName();
	    String issuerName = null;
    
	    i = issuerDNName.indexOf("O=");
	    j = 0;	
    
	    if (i < 0) {
	        issuerName = getMessage("security.dialog.unknown.issuer");
	    } else {
	        try {
		    // Shift to the beginning of the O text
		    i = i + 2;
    
		    // Check if it begins with a quote
		    if (issuerDNName.charAt(i) == '\"') {
		        // Skip the quote
		        i = i + 1;
    
		        // Search for another quote
		        j = issuerDNName.indexOf('\"', i);
		    } else {
		        // No quote, so search for comma
		        j = issuerDNName.indexOf(',', i);
		    }

		    if (j < 0) {
		        issuerName = issuerDNName.substring(i);
		    } else {
		        issuerName = issuerDNName.substring(i, j);
		    }
	        } catch (IndexOutOfBoundsException e) {
		    issuerName = 
			getMessage("security.dialog.unknown.issuer");
	        }
	    }
    
	    // Construct dialog message
            
            // If something is not valid (publisher, time, etc) in certificate,
            // put a message in securityAlerts.  If something is valid in 
            // certificate, put a message in securityInfo.
            // In securityAlerts messages should be added in order of severity - 
            // most severe alerts should be added first.  The order of checks
            // should be reflected in the code below.
            ArrayList securityAlerts = new ArrayList();
            ArrayList securityInfo = new ArrayList();
            
            // These are the captions - we should always use alertCaption,
            // and use infoCaption only if alertCaption is null.
            String alertCaption = null;
            String infoCaption = null;
            
            // Text for "Continue" and "Cancel" buttons.
            String continueButtonStr = (httpsDialog) ?
                getMessage("security.dialog.https.buttonContinue") :
                getMessage("security.dialog.signed.buttonContinue");
            String cancelButtonStr = (httpsDialog) ?
                getMessage("security.dialog.https.buttonCancel") :
                getMessage("security.dialog.signed.buttonCancel");
            
            boolean majorWarning = false;
        
            // Check if this is the case when both - the rootCA and time of
            // signing is valid:
            if ( (!rootCANotValid) && (validityState == CertificateStatus.VALID)){
		// Show warning dialog box for unsigned jnlp file
		// and perform a unsecure operations
		String info = null;
		if (jnlpFlag) {
                    infoCaption = getMessage("security.dialog.unverified.signed.caption");
                    info = getMessage("security.dialog.jnlpunsigned.sub");
                    securityAlerts.add(info);
		}
		else {
                    // Use different caption text for https and signed content
                    infoCaption = (httpsDialog) ?
                        getMessage("security.dialog.verified.valid.https.caption") :
                        getMessage("security.dialog.verified.valid.signed.caption");
                
                    // Use different text for sub panel for https and signed content
                    info = (httpsDialog) ?
                        getMessage("security.dialog.verified.valid.https.sub") :
                        getMessage("security.dialog.verified.valid.signed.sub");
                    securityInfo.add(info);
		}
                
                // These messages are to be displayed in the "All trusted" 
                // case in the More Information dialog:                
                MessageFormat mf = new MessageFormat(
                        getMessage("security.dialog.verified.valid.warning"));
                Object[] args = {subjectName, subjectName};
                securityInfo.add(mf.format(args));  
                
                // Add message that publisher has been verified:
                info = (httpsDialog) ?
                    getMessage("security.dialog.verified.https.publisher") :
                    getMessage("security.dialog.verified.signed.publisher");
                securityInfo.add(info);
                
                // For Timestamp info, add a message saying that certificate
                // was valid at the time of signing. 
                // And display date of signing.
                if (timeStampDate != null) {                    
                    // Get the right date format for timestamp
                    DateFormat df = DateFormat.getDateTimeInstance(
                            DateFormat.LONG, DateFormat.LONG);
                    String tsStr = df.format(timeStampDate);
                    Object[] argsTS = {tsStr};
                    
                    mf = new MessageFormat(
                            getMessage("security.dialog.timestamp"));
                   
                    securityInfo.add(mf.format(argsTS));   
                }
            } else {
                // This is the case when either publisher or time of signing
                // is invalid - check and add corresponding messages to 
                // appropriate message arrays.                
            
                // If root CA is not valid, add a caption and a message to the 
                // securityAlerts array.
                if (rootCANotValid) { 
                    majorWarning = true;
                    
                    // Use different caption text for https and signed content
                    alertCaption = (httpsDialog) ?
                        getMessage("security.dialog.unverified.https.caption") :
                        getMessage("security.dialog.unverified.signed.caption");
                    
                    // First detailed message will be displayed in the subpanel
                    // in security dialog.  Different for http/https:
                    String info = (httpsDialog) ?
                        getMessage("security.dialog.unverified.https.sub") :
                        getMessage("security.dialog.unverified.signed.sub");
                    securityAlerts.add(info);
                                                        
                    // Next detailed message will be displayed in the
                    // More Information dialog. Different for http/https:
                    info = (httpsDialog) ?
                        getMessage("security.dialog.unverified.https.publisher") :
                        getMessage("security.dialog.unverified.signed.publisher");
                    securityAlerts.add(info);                    
                } else {   
                    // This is the case when publisher is valid, so the time
                    // of signing should be INVALID, or timestamp should be
                    // valid, else we would have been in the all-good case 
                    // where publisher is verified and time is valid.                    
                    
                    // Use different caption text for https and signed content
                    infoCaption = (httpsDialog) ?
                        getMessage(
                            "security.dialog.verified.valid.https.caption") :
                        getMessage(
                            "security.dialog.verified.valid.signed.caption");  

                    // Add details string for subpanel - different for http/https.
                    String info = (httpsDialog) ?
                        getMessage("security.dialog.verified.https.publisher"):
                        getMessage("security.dialog.verified.signed.publisher");
                    securityInfo.add(info);
                }
            
                // now check time of signing...
                if (validityState != CertificateStatus.VALID) {                    
                    // If time is EXPIRED or NOT_YET_VALID...
                    // Use this caption only if we don't have caption already -
                    if ( alertCaption == null ) {
                        // Use different caption text for https and signed content
                        alertCaption = (httpsDialog) ?
                            getMessage(
                                "security.dialog.invalid.time.https.caption") :
                            getMessage(
                                "security.dialog.invalid.time.signed.caption"); 
                    }
                    
                    // If no warnings yet, add the one that will show in the
                    // subpanel of the Security Warning dialog:
                    String alert= null;
                    String moreinfo = null;
                        
                    // Retrieve message for EXPIRED time of signing
                    if (validityState == CertificateStatus.EXPIRED) {
                        if (securityAlerts.isEmpty()) {
                            alert = (httpsDialog) ?
                                getMessage("security.dialog.expired.https.sub"):
                                getMessage("security.dialog.expired.signed.sub");
                        }
                        moreinfo = (httpsDialog) ?
                            getMessage("security.dialog.expired.https.time"):
                            getMessage("security.dialog.expired.signed.time");
                    } else {
                        // Retrieve message for NOT_YET_VALID certificate
                        if (securityAlerts.isEmpty()) {
                            alert = (httpsDialog) ?
                                getMessage("security.dialog.notyet.https.sub"):
                                getMessage("security.dialog.notyet.signed.sub");
                        }
                        moreinfo = (httpsDialog) ?
                            getMessage(
                                "security.dialog.notyetvalid.https.time"):
                            getMessage(
                                "security.dialog.notyetvalid.signed.time");
                    }
                    if (alert != null) {
                        securityAlerts.add(alert);
                    }
                    securityAlerts.add(moreinfo);
                    
                } else {                     
                    // For unverified publisher with expired time of signing
                    // and timestamp info, add a message saying that certificate
                    // was valid at the time of signing.
                    if (timeStampDate != null) {
                        // Get the right date format for timestamp
                        DateFormat df = DateFormat.getDateTimeInstance(
                                        DateFormat.LONG, DateFormat.LONG);
                        String tsStr = df.format(timeStampDate);
                        Object[] argsTS = {tsStr};                    
                        
                        MessageFormat mf = new MessageFormat(
                                getMessage("security.dialog.timestamp"));
                   
                        securityInfo.add(mf.format(argsTS));   
                    }
                    // No messages for valid time of signing.
                }    
            }
            
            // No need to check for Java version, since for < 1.6 hostname
            // will be <null> and we will not execute this code.
	    ArrayList subjectNameList = CertUtils.getServername(cert);
            if ( hostname != null && 
		 !CertUtils.checkWildcardDomainList(hostname, subjectNameList)) {
                Object[] args = {hostname, subjectName};
                
                // Set masthead string to display hostname mismatch warning,
                // only if no other warnings are found for this certificate.
                if ( alertCaption == null ) {
                    alertCaption = 
                            ResourceManager.getMessage("https.dialog.masthead");
                    
                    // If this is the first alert, we'll need to add one more
                    // message to be displayed in the lower subpanel of security
                    // warning dilog:
                    securityAlerts.add(ResourceManager.getFormattedMessage(
                            "security.dialog.hostname.mismatch.sub", args));
                }                                
                
                // Add hostname mismatch error to the securityAlerts to be 
                // displayed in the More Information dialog:                              
                securityAlerts.add(ResourceManager.getFormattedMessage(
                        "security.dialog.hostname.mismatch.moreinfo", args));
            }
            
            // This is a general message about running with all permissions.
            // For http only, make it first in .
            if ( !httpsDialog ) {
                // Make sure this general message appears first in the "More
                // Information" dialog.  In order for this to happen, this
                // message should be second in the array of alerts if it is
                // not empty, or second in the array of infos if alerts are 
                // empty.
                String generalWarning = getMessage(
                        "security.dialog.signed.moreinfo.generic");
                ArrayList addToMe = securityInfo;
                if (!securityAlerts.isEmpty()) {                   
                    addToMe = securityAlerts;
                }                                    
                addToMe.add(1, generalWarning);
            }

            
            
            if (httpsDialog) {
                // For https dialog, use real hostname for "Name:"
                if (hostname != null){
                    ainfo.setTitle(hostname);
                }else{
                    ainfo.setTitle(subjectName);
                }
            }

            // Add one more message for unsigned jnlp file and 
	    // perform a unsecure operations
	    if (jnlpFlag) {
                securityAlerts.add(getMessage("security.dialog.jnlpunsigned.more"));
	    }

            // For caption, use alertCaption if it is not null.  Else,
            // use infoCaption
            String caption = (alertCaption != null)? alertCaption : infoCaption;
            
            String [] alertStrs = null;
            String [] infoStrs = null; 
            if (!securityAlerts.isEmpty()) {
                alertStrs = new String[securityAlerts.size()];
                // Convert into array of Strings:
                for (int n = 0; n < securityAlerts.size(); n++){
                    alertStrs[n] = securityAlerts.get(n).toString();
                }
            }
            if (!securityInfo.isEmpty()) {
                infoStrs = new String[securityInfo.size()];
                for (int n = 0; n < securityInfo.size(); n++){
                    infoStrs[n] = securityInfo.get(n).toString();
                }
            }
            if (securityAlerts.isEmpty() && securityInfo.isEmpty()){
                // Throw exception
                throw new CertificateException(
                        getMessage("security.dialog.exception.message"));
            }           
            	    
            // Show dialog
            if (Trace.isAutomationEnabled() == false) {
                
		// Show extension install dialog box when certificate is valid
		// and it is not perform a unsecure operation
		if (ainfo.getType() == 3 && alertStrs == null && !jnlpFlag) {
		    caption = getMessage("security.dialog.extension.caption");
		    String installButtonStr = getMessage("security.dialog.extension.buttonInstall");
		    String [] infoExtensionStrs = null;

                    infoExtensionStrs = new String[2];
		    infoExtensionStrs[0] = getMessage("security.dialog.extension.sub");

                    MessageFormat mf = new MessageFormat(
                        getMessage("security.dialog.extension.warning"));
                    Object[] args = {subjectName, subjectName, subjectName};
                    infoExtensionStrs[1] = mf.format(args);  

                    ret = UIFactory.showSecurityDialog(ainfo,
                        getMessage("security.dialog.extension.title"),
                        caption, subjectName, url, 
                        true, false, installButtonStr, cancelButtonStr,  
                        alertStrs, infoExtensionStrs, true, certs, start, end, 
                        majorWarning);                    

		    // Trust certificate always
		    if (ret == UIFactory.OK) {
			 ret = UIFactory.ALWAYS;
		    }
		}
		else {
                    ret = UIFactory.showSecurityDialog(ainfo,
                        getMessage("security.dialog.caption"),
                        caption, subjectName, url, 
                        true, false, continueButtonStr, cancelButtonStr,  
                        alertStrs, infoStrs, true, certs, start, end, 
                        majorWarning);                    
		}
            } else {	// If automation is enabled
                Trace.msgSecurityPrintln("trustdecider.automation.trustcert");
                ret = TrustOption_GrantThisSession;
            }
        }

        return ret;	    	
    }
  
    private static String getMessage(String key)  {
	return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }
}





