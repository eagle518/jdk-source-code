/*
 * @(#)BasicServiceImpl.java	1.24 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import javax.jnlp.BasicService;

import java.net.*;
import java.io.*;
import java.security.*;

import com.sun.javaws.BrowserSupport;
import com.sun.deploy.net.offline.DeployOfflineManager;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.PerfLogger;


public final class BasicServiceImpl implements BasicService {
    
    // Launch property file
    private URL        _codebase = null;
    private String     _codebaseProtocol = null;
    private boolean    _isWebBrowserSupported;
 
    // Only one instance of this will ever exist
    static private BasicServiceImpl _sharedInstance = null;
    
    /** Creates a BasicServiceImpl with information about a particular application and JNLP file. 
        If no previous DesktopBrowse was registered, core JAVAWS internal one will be used.
      */
    private BasicServiceImpl(URL codebase, boolean isWebBrowserSupported,
            String codebaseProtocol) {
        _codebaseProtocol = codebaseProtocol;
        _codebase = codebase;
        _isWebBrowserSupported = isWebBrowserSupported;
        if (Config.isJavaVersionAtLeast16())
        {
            if ( sun.awt.DesktopBrowse.getInstance()==null ) {
                try {
                    sun.awt.DesktopBrowse.setInstance(new BasicServiceBrowser());
                } catch (Throwable t) {
                    Trace.ignored(t);
                }
            }
        }
    }
    
    /** Returns singelton instance. This might return null, if this service is not supported */
    public static BasicServiceImpl getInstance() { return _sharedInstance; }
    
    /**
     *  Initialize the single instance of this class. This call is ignored if the instance is already
     *  initialized.
     */
    public static void initialize(URL codebase, boolean isWebBrowserSupported,
            String codebaseProtocol) {
        if (_sharedInstance == null) {
            _sharedInstance = new BasicServiceImpl(codebase,
                    isWebBrowserSupported, codebaseProtocol);
        }
    }
    
    /** Returns the codebase for an application. See description in service interface */
    public URL getCodeBase() { 
        return _codebase; 
    }
    
    /** Determines if the system is offline. See description in service interface */
    public boolean isOffline() { 
        return DeployOfflineManager.isGlobalOffline(); 
    }

    private boolean isFileProtocolCodebase() {
        if (_codebaseProtocol != null &&
                _codebaseProtocol.equalsIgnoreCase("file")) {
            return true;
        }
        return false;
    }
    
    /**
     * Replaces the Web page currently being viewed with the given URL, or
     * launches a browser to show the given URL.
     *
     * @param   url   an absolute URL giving the location of the document.
     * @return  <code>true</code> if the request succeded, <code>false</code> otherwise.
     */
    public boolean showDocument(URL url) {
     
        if (url == null) {
            return false;
        }

        // Allow file URL if we have permission to read or codeboase protocol
        // is file
        if (url.getProtocol().equalsIgnoreCase("file")) {
            Permission p = null;
            // get permission object to read the url
            try {
                URLConnection urlc = url.openConnection();
                p = urlc.getPermission();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                return false;
            }
            try {
                SecurityManager sm = System.getSecurityManager();
                if (sm != null) {
                    sm.checkPermission(p);
                }
            } catch (SecurityException se) {
                // allow file url if codebase protocol is file
                if (!isFileProtocolCodebase()) {
                    Trace.ignoredException(se);
                    return false;
                }
            }

        }

        if(Config.isJavaVersionAtLeast16()) {
            sun.awt.DesktopBrowse browser = sun.awt.DesktopBrowse.getInstance();
            if ( browser!=null && !(browser instanceof BasicServiceBrowser) ) {
                // follow a generic way ..
                if (!isWebBrowserSupported()) {
                    return false;
                }
                
                // Convert to absolute URL
                try { 
                    url = new URL(_codebase, url.toString());
                } catch(MalformedURLException mue) { /* just ignore */ }
                browser.browse(url);

                // let's assume life is good, 
                // since sun.awt.DesktopBrowse.browse() doesn't return any value
                return true; 
            }
        }
        return showDocumentHelper(url);
    }

    private boolean showDocumentHelper(final URL url) {
        Boolean retValue = (Boolean)java.security.AccessController.doPrivileged
            (new java.security.PrivilegedAction() {
            public Object run() {
                if (DownloadEngine.isJnlpURL(url)) {
                    try {
                        String cmd [] = new String[3];
                        cmd[0] = Config.getJavawsCommand();
                        cmd[1] = "-Xnosplash";
                        cmd[2] = url.toString();
                        Runtime.getRuntime().exec(cmd);
                    } catch (Exception e) {
                        Trace.ignored(e);
                    }
                    return new Boolean(true);
                }
                return new Boolean(false);
            }
        });
        if (retValue != null && (retValue.booleanValue() == true)) {
            return true;
        }
        if (!isWebBrowserSupported()) {
            return false;
        }
        
        // Do priviledged is used as we may not be able to exec if
        // running in a sandbox.
        retValue = (Boolean)java.security.AccessController.doPrivileged
            (new java.security.PrivilegedAction() {
                    public Object run() {
                        URL _url = url;
                        // Convert to absolute URL
                        try { 
                            URL absURL = new URL(_codebase, _url.toString());
                            _url = absURL;
                        } catch(MalformedURLException mue) { /* just ignore */ }
                        return new Boolean(BrowserSupport.showDocument(_url));
                    }
                });
        return (retValue == null) ? false : retValue.booleanValue();
    }
    
    /**
     * Checks if a Web browser is supported on the current platform. If
     * this is <i>not</i> the case, then <code>{@link #showDocument}</code>
     * will always return <code>false</code>.
     *
     * @return <code>true</code> if a Web browser is supported on the
     *          current platform and by the current JNLP Client,
     *          otherwise <code>false</code>
     */
    public boolean isWebBrowserSupported() { 
        PerfLogger.setEndTime("BasicService.isWebBrowserSupported called");
        PerfLogger.outputLog();
        return _isWebBrowserSupported; 
    }

    private class BasicServiceBrowser extends sun.awt.DesktopBrowse {
        public void browse(URL url) {
            showDocument(url);
        }
    }
}

