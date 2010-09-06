/*
 * @(#)ExtensionInstallerServiceImpl.java	1.46 04/03/23
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;
import java.net.URL;
import java.io.*;
import java.util.*;
import java.util.jar.JarFile;
import java.security.*;
import javax.jnlp.ExtensionInstallerService;
import com.sun.javaws.*;
import com.sun.javaws.ui.DownloadWindow;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.Globals;
import com.sun.javaws.exceptions.LaunchDescException;
import com.sun.javaws.Main;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
/**
 *
 * @version 1.20 09/14/00
 */
public final class ExtensionInstallerServiceImpl implements ExtensionInstallerService {
    private LocalApplicationProperties _lap;
    private DownloadWindow _window;
    private String _target;
    private String _installPath;
    private boolean _failedJREInstall = false;
    
    // Referenece to single instance of the service
    static ExtensionInstallerServiceImpl  _sharedInstance = null;
    
    /** Private constructor, use getInstance method to get the single instance */
    private ExtensionInstallerServiceImpl(String installPath,
                                          LocalApplicationProperties lap,
                                          DownloadWindow window) {
        _lap = lap;
        _window = window;
        _installPath = installPath;
    }
    
    /** Returns the single instance of this service. This will return null,
     *  if the extension installer object has not been initialized. It is
     *  only initialized when the JNLP Client is executing an installer.
     */
    public static synchronized ExtensionInstallerServiceImpl getInstance() {
        return _sharedInstance;
    }
    
    /** Create the ExtensionInstallerService object. Must be called before the
     *  ServiceManager will return this service
     */
    static public synchronized void initialize(String installPath,
                                               LocalApplicationProperties lap, DownloadWindow window) {
	if (_sharedInstance == null) {
	    _sharedInstance = new ExtensionInstallerServiceImpl(installPath, lap, window);
	}
    }    
    
    /** Returns prefered location for instllation */
    public String getInstallPath() { return _installPath; }
    
    /** Return extension information */
    public String getExtensionVersion()  { return _lap.getVersionId(); }
    public URL    getExtensionLocation() { return _lap.getLocation();  }
    
    
    /** Get information about installed JREs. Only product versions can be lookedup */
    public String getInstalledJRE(URL location, String version) {
        JREInfo jre = LaunchSelection.selectJRE(location, version);
        return (jre != null) ? jre.getPath() : null;
    }
    
    /** Manipulation of progress window */
    public void setHeading(String status)   { _window.setStatus(status); }
    public void setStatus(String status)    { _window.setProgressText(status); }
    public void updateProgress(int value)   { _window.setProgressBarValue(value); }
    public void hideProgressBar()           { _window.setProgressBarVisible(false); }
    public void hideStatusWindow()          { _window.getFrame().setVisible(false); }
            
    public void setJREInfo(String platformVersion, String jrePath) {
	// Check to make sure we're signed.
	int model = JNLPClassLoader.getInstance().getDefaultSecurityModel();
	if (model != LaunchDesc.ALLPERMISSIONS_SECURITY &&
	    model != LaunchDesc.J2EE_APP_CLIENT_SECURITY) {
	    throw new SecurityException("Unsigned extension installer attempting to call setJREInfo.");
	}

      
	Trace.println("setJREInfo: " + jrePath, TraceLevel.EXTENSIONS);
        
        if (jrePath != null && (new File(jrePath)).exists()) {
            // Add the new JRE
	    JREInfo.addJRE(new JREInfo(
		platformVersion, getExtensionVersion(), 
		getExtensionLocation().toString(),
		jrePath, Config.getOSName(), Config.getOSArch(), true, false));
        } else {
	    // sanity check on jre installer
	    Trace.println("jre install failed: jrePath invalid", 
		TraceLevel.EXTENSIONS);
	    _failedJREInstall = true;
	}
    }
    
    public void setNativeLibraryInfo(String path) {
        
	Trace.println("setNativeLibInfo: " + path, TraceLevel.EXTENSIONS);
        
        _lap.setNativeLibDirectory(path);
    }
    
       /** Install failed/success */
    
    public void installFailed() {
        
	Trace.println("installFailed", TraceLevel.EXTENSIONS);
        
        // Signal failed
        Main.systemExit(1);
    }
 
    
    public void installSucceeded(boolean needsReboot) {
        
	if (_failedJREInstall) {
	    return;
	}
	Trace.println("installSucceded", TraceLevel.EXTENSIONS);
        
	AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
        	Config.store(); // Might contain updated JRE info
		return null;
	    }
	});
        // Save Local installation properties
        _lap.setInstallDirectory(_installPath);
	_lap.setLastAccessed(new Date());
        if (needsReboot) {
	    _lap.setRebootNeeded(true);
	} else {
	    _lap.setLocallyInstalled(true);
	}
        try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
	        public Object run() throws IOException {
        	    _lap.store();
		    return null;
	        }
	    });
        } catch(PrivilegedActionException pae) {
	    if (pae.getException() instanceof IOException) {
            	// Show error message if fails to save
            	LaunchErrorDialog.show(_window.getFrame(), (IOException)pae.getException(), false);
	    } else {
		Trace.ignoredException(pae.getException());
	    }
        }
        // Signal success
	// this might call JnlpxArgs.removeArgumentFile so needs to be
	// privileged
	AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		Main.systemExit(0);
		return null;
	    }
	});
    }
}

