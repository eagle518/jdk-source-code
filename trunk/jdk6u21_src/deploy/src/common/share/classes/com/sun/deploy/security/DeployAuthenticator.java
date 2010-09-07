/*
 * @(#)DeployAuthenticator.java	1.4 03/04/18
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.net.Authenticator;
import java.net.PasswordAuthentication;
import java.net.URL;
import java.text.MessageFormat;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.ComponentRef;


/**
 * Assist proxy authentication and web server authentication.
 * A unique instance of this class is registered at startup.
 *
 * @see java.net.Authenticator
 * @see java.net.PasswordAuthentication
 */
public class DeployAuthenticator extends Authenticator implements AuthKey {
    // NTLM scheme
    private static final String SCHEME_NTLM = "NTLM";
    private static final String SCHEME_DIGEST = "DIGEST";
    private static final String SCHEME_BASIC = "BASIC";
    
    // The duration of the cancel state is 3 seconds
    private final long CANCEL_DURATION = 3000;
    private final int ACTIVE = 1;
    private final int CANCEL = 2;
        
    // Parent frame
    protected ComponentRef parentComponent = null;
    
    // Lazier instantiation of CredentialManager
    private CredentialManager cm;
    private boolean cmInitialized;
    private CredentialManager getCredentialManager() {
        if (!cmInitialized) {
            cmInitialized = true;
            cm = ServiceManager.getService().getCredentialManager();
        }
        return cm;
    }

    
    private StateMonitor stateMonitor = new StateMonitor();
    
    /**
     * Return browser authenticator.
     */
    private BrowserAuthenticator getBrowserAuthenticator() {
        Service service =
                com.sun.deploy.services.ServiceManager.getService();
        return service.getBrowserAuthenticator();
    }
    
    /**
     * <p>
     * Called when password authorization is needed by the superclass. The
     * instance is registered as being the default http proxy authentication
     * facility and will be called by the HTTP framework classes.
     * </p>
     * @return The PasswordAuthentication collected from the
     *		user, or null if none is provided.
     */
    protected synchronized PasswordAuthentication getPasswordAuthentication() {
        PasswordAuthentication pa = null;
        
        // Check to see if we are in a cancel state, if we are return null
	// Only when you are running JRE 5.0 and later
	if ( Config.isJavaVersionAtLeast15() ) {
           if (stateMonitor.getState(getCredentialManager().buildConnectionKey(this)) == CANCEL) {
              return null;
           } 
	}

        try {   
            java.net.InetAddress site = getRequestingSite();
            String siteName;
            if (site != null) {
                siteName = site.toString();
            } else {
                siteName = getHost();
                if (siteName == null || siteName.length() == 0) {
                    siteName = getMessage("net.authenticate.unknownSite");
                }
            }
            
            // Print out tracing
            StringBuffer buffer = new StringBuffer();
            buffer.append("Firewall authentication: site=");
            buffer.append(getRequestingSite());
            buffer.append(":" + getRequestingPort());
            buffer.append(", protocol=");
            buffer.append(getRequestingProtocol());
            buffer.append(", prompt=");
            buffer.append(getRequestingPrompt());
            buffer.append(", scheme=");
            buffer.append(getRequestingScheme());
            
            Trace.netPrintln(buffer.toString());
            
            CredentialInfo cred = new CredentialInfo();
	    CredentialInfo info = null;
                        
            // Try to get the credential from the CredentialManager
	    // Only when you are running JRE 5.0 and later
	    if ( Config.isJavaVersionAtLeast15() ) {
               if (getCredentialManager() != null) {
                  cred = getCredentialManager().getCredential(this);
               }
            
               // If the CredentialManager had no credential or 
	       // it has credential but not valid, try the web browser
               if (cred.isCredentialEmpty() || !getCredentialManager().isCredentialValid(cred)) {
                  cred = CredentialInfo.passAuthToCredentialInfo( getBrowserCredential() );
                  // add a valid session ID to make credential valid
                  if(getCredentialManager() != null) {
                    cred.setSessionId(getCredentialManager().getLoginSessionId());
                  }
               }
            
               // If the credential is not complete get user input
               if (getCredentialManager() == null || !getCredentialManager().isCredentialValid(cred)) {
                  info = openDialog(siteName, getRequestingPrompt(),
                      getRequestingScheme(), cred );
                
                  // If the user didn't hit cancel save their input
                  if (info != null) {
                     getCredentialManager().saveCredential( this, info );
                     pa = info.getPasswordAuthentication();
                  } else {
                     // User hit cancel so update the state
                     stateMonitor.setCancel(getCredentialManager().buildConnectionKey(this));
                  }
               } else {
                 pa = cred.getPasswordAuthentication();
               }
 	    }	
	    else { // For old JRE early than 5.0
	       info = openDialog(siteName, getRequestingPrompt(), getRequestingScheme(), cred );
	       pa = info.getPasswordAuthentication();
	    }
        } catch (Exception e) {
            // We should catch all exception so the connection may continue
            Trace.netPrintException(e);
        }
       
        return pa;
    }
    
    /**
     * gets a saved credential from the web browser
     *
     * @return the saved credential or null
     */
    private PasswordAuthentication getBrowserCredential() {
        
        PasswordAuthentication pa = null;

        BrowserAuthenticator browserAuthenticator = getBrowserAuthenticator();
        
        if (browserAuthenticator != null) {            
            pa = browserAuthenticator.getAuthentication(
                    getRequestingProtocol(), getHost(),
                    getRequestingPort(), getRequestingScheme(),
                    getRequestingPrompt(), getURL(), isProxy());
        }
        return pa;
    }
    
    /*
     * <p>
     * Open the dialog to request the user/password information
     * from the user.
     * </p>
     *
     * @param site the HTTP site we are trying to connect
     * @param prompt the HTTP prompt from the server
     * @param scheme the HTTP scheme
     * @param cred the Info to suggest to the user
     * @return the CredentialInfo object encapsulating the
     * username/password entered by the user
     */
    private CredentialInfo openDialog( String site,
            String prompt, String scheme, CredentialInfo cred ) {
        if (site == null) {
            site = "";
        }
        if (prompt == null || prompt.trim().equals("")) {
            prompt = "<default>";
        }
        
        boolean isDomainNeeded = false;
        
	// Get authentication scheme text
	String authDisplayString = null;

        if (scheme != null) {
	    if (scheme.equalsIgnoreCase(SCHEME_BASIC)) {
	    	authDisplayString = 
                        getMessage("net.authenticate.basic.display.string");
	    }
	    else if (scheme.equalsIgnoreCase(SCHEME_DIGEST)) {
	    	authDisplayString = 
                        getMessage("net.authenticate.digest.display.string");
	    }
	    else if (scheme.equalsIgnoreCase(SCHEME_NTLM)) {
	    	authDisplayString = 
                        getMessage("net.authenticate.ntlm.display.string");
	    	isDomainNeeded = true;
	    } 
	    else {
	    	authDisplayString = 
                        getMessage("net.authenticate.unknown.display.string");
	    }
	} else {
            authDisplayString  =  getMessage("net.authenticate.unknown.display.string");
        }

        MessageFormat mf =
                new MessageFormat(getMessage("net.authenticate.text"));
        
        Object [] args = { prompt, site };
        String details = mf.format(args);
        
        CredentialInfo info = null;
        boolean isEncryptionEnabled = false;
        
        // If there is a Credential Manager see if it supports Encryption
        if (getCredentialManager() != null) {
            isEncryptionEnabled = getCredentialManager().isPasswordEncryptionSupported(); 
        }
        
        try {
            // Must get some input from the user first
            info = UIFactory.showPasswordDialog(parentComponent == null ? null : parentComponent.get(),
                    getMessage("password.dialog.title"),
                    details, true, isDomainNeeded, cred,
                    isEncryptionEnabled, authDisplayString );
        } catch( Exception e ) {
            Trace.securityPrintException( e );
        }
        
        // info will be <null> if user pressed "Cancel" in password dialog
        return info;
    }
    
    /*
     * <p>
     * Helper method to load resources for i18n
     * </p>
     */
    private String getMessage(String key) {
        return ResourceManager.getMessage(key);
    }
    
    public void setParentComponent(ComponentRef c) {
        parentComponent = c;
    }
    
    public String getProtocolScheme() {
        return getRequestingProtocol();
    }
    
    public String getHost() {
        try { 
            return getRequestingHost();
        } catch (NoSuchMethodError nsme) {
            // no such method before 1.4 - fall thru
        }
        return "";
    }
    
    public int getPort() {
        return getRequestingPort();
    }

    public String getPath() {
	URL myURL = getURL();

	if (myURL != null) {
           return myURL.getPath();
	}
	else { // This is for old JRE before 1.5
	   return null;
	}
    }
    
    public boolean isProxy() {
        try {
            return (getRequestorType() == RequestorType.PROXY);
        } catch (NoSuchMethodError nsme) {
            // no such method before 1.5 - fall thru
        }
        return false;
    }
    
    public URL getURL() {
        try {
            return getRequestingURL();
        } catch (NoSuchMethodError nsme) {
            // no such method before 1.5 - fall thru
        }
        return null;
    }
    
    /*
     * The StateMonitor class is an inner class of the DeployAuthenticator
     * that is responsible for keeping track of the Authenticator's state.
     * There are two possible states, ACTIVE and CANCEL.  In a CANCEL state
     * the Authenticator should take no action to determine credentials for
     * a resource.  The plug-in will continue to try to download resources 
     * even after a user has indicated they don't know the credentials.  
     * By keeping state information the
     * Authenticator can determine when to ask the user for input and when to
     * do nothing.
     */
    private class StateMonitor {

        private int currentState = ACTIVE;
        private long timeOfLastCancel = 0;
        private String siteKey = "";
        
        /*
         * Changes the State to the CANCEL state for a given server path key
         * 
         * @param key The resource that was being loaded when cancel was pressed
         */
        private void setCancel(String key) {
            currentState = CANCEL;
            timeOfLastCancel = System.currentTimeMillis();
            
            // Save the location of the file by trimming off the file name
            siteKey = trimPath(key);
        }
        
        /* 
         * gets the curent state of the Authenticator
         * 
         * @return returns ACTIVE durning normal operation.  If the user hit
         * cancel or dismissed the Authentication Dialog the state returned will
         * be CANCEL for a duration of <code>CANCEL_DURATION</code>
         */
        private int getState(String key) {
             
            int state = ACTIVE;
            
            // If we are in a cancel state 
            if (currentState == CANCEL) {
                // If enough time has expired return the state to ACTIVE
                if((System.currentTimeMillis() - timeOfLastCancel) > CANCEL_DURATION) {
                    currentState = ACTIVE;
                } else {
                    // If the key is in the same directory or a subdirectory
                    // of the original resource for wich the credential was not
                    // known return CANCEL for the state
                    if( key.startsWith(siteKey)) {
                        state = CANCEL;
                    } 
                }
            }
            return state;
        }
      
        /**
         * trims a full resource location path back to just the path
         *   I.E. .../myApplet/sounds/chirp.au will get trimmed to 
         *        .../myApplet/sounds/
         *
         * @return the path to the resource
         */
        public String trimPath(String key) {
            int i = key.lastIndexOf('/');
            String str = key.substring(0,i+1);
            return str;
        }
    }
}



