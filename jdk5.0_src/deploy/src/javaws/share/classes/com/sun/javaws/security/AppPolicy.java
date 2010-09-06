/*
 * @(#)AppPolicy.java	1.67 04/03/23
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.security.Policy;
import java.security.CodeSource;
import java.security.KeyStore;
import java.security.AccessControlException;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.Permission;
import java.security.AllPermission;
import java.util.PropertyPermission;
import java.util.Properties;
import java.util.Enumeration;
import java.util.Iterator;
import java.lang.RuntimePermission;
import java.awt.AWTPermission;
import java.net.URL;
import java.net.SocketPermission;
import java.io.FilePermission;
import java.io.File;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.Main;
import com.sun.javaws.Globals;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.security.BadCertificateDialog;
import com.sun.deploy.security.CeilingPolicy;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;

/** Defines the security policy for applications launched
 *   by JavaWS.
 *
 *   It implemetens to access models:
 *   - all-permissions   (or ceiling policy)
 *   - j2ee-permissions
 *   - sandbox           (applet security model)
 *
 */
public class AppPolicy {
    
    // Configuration variables
    private String     _host  = null; // Name of host that is accessible
    
    /** Location of extension directory for this particular JRE */
    private File _extensionDir = null;
    
    static private AppPolicy _instance = null;

    /** AppPolicy is a singleton object */
    static public AppPolicy getInstance() {
        return _instance;
    }
    
    /** Create the instance e based on current JNLP file */
    static public AppPolicy createInstance(String host) {
        if (_instance == null) {
            _instance = new AppPolicy(host);
        }
        return _instance;
    }

    /** Creates the JNLP "policy" object that knows how to assign the right
     *  resources to the JAR files specified in the JNLP files
     */
    private AppPolicy(String host) {
        _host = host;
        
        // Setup location of extension directory
        _extensionDir = new File(System.getProperty("java.home")
		     + File.separator + "lib" + File.separator + "ext");
    }
    
    /** Return the security policy in form of a PermissionCollection */
    public void addPermissions(PermissionCollection perms, CodeSource target) {

	Trace.println("Permission requested for: " + target.getLocation(), TraceLevel.SECURITY);
        
        
        // Find JARDesc for this entry
        JARDesc jd = JNLPClassLoader.getInstance().getJarDescFromFileURL(
						    target.getLocation());
        if (jd == null) {
	    // Not from a JARDesc attribute. will default to correct permissions
            return;
        }

	
        // Get LaunchDesc for for the JARDesc.
        LaunchDesc ld = jd.getParent().getParent();
        int access = ld.getSecurityModel();        

        if (access != LaunchDesc.SANDBOX_SECURITY) {
	    grantUnrestrictedAccess(ld, target);
	    if (access == LaunchDesc.ALLPERMISSIONS_SECURITY) {
	        CeilingPolicy.addTrustedPermissions(perms);
	    } else {
	        addJ2EEApplicationClientPermissionsObject(perms);
	    }
        }

	
	if (!perms.implies(new AllPermission())) {
	    addSandboxPermissionsObject(perms, 
		(ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE));  
        }

	// only set the system properties the first time for this ld
	if (!ld.arePropsSet()) {
	    // now - only get the props for this ld (not extensions)
	    Properties props = ld.getResources().getResourceProperties();
            Enumeration keys = props.keys();
            while(keys.hasMoreElements()) {
                String name = (String)keys.nextElement();
                String value = props.getProperty(name);
                Permission perm = new PropertyPermission(name, "write");
		// only set property if app is allowed to
                if (perms.implies(perm)) {
                    System.setProperty(name, value);
                } else {
		    Trace.ignoredException(new AccessControlException(
					       "access denied "+perm, perm));
		}
	    }
	    ld.setPropsSet(true);
	}
    }

    private void setUnrestrictedProps(LaunchDesc ld) {
        // only set the system properties the first time for this ld
        if (!ld.arePropsSet()) {
            // now - only get the props for this ld (not extensions)
            Properties props = ld.getResources().getResourceProperties();
            Enumeration keys = props.keys();
            while(keys.hasMoreElements()) {
                String name = (String)keys.nextElement();
                System.setProperty(name, props.getProperty(name));
            }
            ld.setPropsSet(true);
        }
    }    


    public void grantUnrestrictedAccess(LaunchDesc ld, CodeSource cs) {
        String codeTypeKey;
	boolean retValue = false;
        switch (ld.getLaunchType()) {
	    default:
            case LaunchDesc.APPLICATION_DESC_TYPE:
	        codeTypeKey = "trustdecider.code.type.application";
                break;

	    case LaunchDesc.APPLET_DESC_TYPE:
		codeTypeKey = "trustdecider.code.type.applet";
                break;

	    case LaunchDesc.LIBRARY_DESC_TYPE:
		codeTypeKey = "trustdecider.code.type.extension";
                break;

	    case LaunchDesc.INSTALLER_DESC_TYPE:
		codeTypeKey = "trustdecider.code.type.installer";
                break;
        }
	try {
	    if (Globals.isSecureMode() ||
		TrustDecider.isAllPermissionGranted(cs, codeTypeKey)) {
	        setUnrestrictedProps(ld);
		return;
	    } else {
                Trace.println("We were not granted permission, exiting", TraceLevel.SECURITY);
	    }
        } catch (Exception exception) {
	    BadCertificateDialog.show(cs, codeTypeKey, exception);
        }  
	// not granted - or thrown exception
        Main.systemExit(-1);
    }
    
    private void addJ2EEApplicationClientPermissionsObject(PermissionCollection perms) {

	Trace.println("Creating J2EE-application-client-permisisons object", TraceLevel.SECURITY);
	
	
	// AWT permissions
	perms .add(new AWTPermission("accessClipboard"));
	perms .add(new AWTPermission("accessEventQueue"));
	perms .add(new AWTPermission("showWindowWithoutWarningBanner"));
	
	// Runtime permissions
	perms.add(new RuntimePermission("exitVM"));
	perms.add(new RuntimePermission("loadLibrary"));
	perms.add(new RuntimePermission("queuePrintJob"));
	
	// Socket permissions
	perms.add(new SocketPermission("*", "connect"));
	perms.add(new SocketPermission("localhost:1024-", "accept,listen"));
	
	// File permissions
	perms.add(new FilePermission("*", "read,write"));
	
	// Property permissions
        perms.add(new PropertyPermission("*", "read"));
    }
    
    /** Creates a JavaWS Sandbox Security policy. This will deny access to the file-system, limit
     *  access to properties, and only provide access to the ports that are listed in
     *  the classpath
     */
    private void addSandboxPermissionsObject(PermissionCollection perms, 
					     boolean isApplet) {
	
	Trace.println("Add sandbox permissions", TraceLevel.SECURITY);
	
	
	// Grant read-only access to select number of properties
	perms.add(new PropertyPermission("java.version"       , "read"));
	perms.add(new PropertyPermission("java.vendor"        , "read"));
	perms.add(new PropertyPermission("java.vendor.url"    , "read"));
	perms.add(new PropertyPermission("java.class.version" , "read"));
	perms.add(new PropertyPermission("os.name"            , "read"));
	perms.add(new PropertyPermission("os.arch"            , "read"));
	perms.add(new PropertyPermission("os.version"         , "read"));
	perms.add(new PropertyPermission("file.separator"     , "read"));
	perms.add(new PropertyPermission("path.separator"     , "read"));
	perms.add(new PropertyPermission("line.separator"     , "read"));
	
	perms.add(new PropertyPermission("java.specification.version", "read"));
	perms.add(new PropertyPermission("java.specification.vendor",  "read"));
	perms.add(new PropertyPermission("java.specification.name",    "read"));
	
	perms.add(new PropertyPermission("java.vm.specification.version", "read"));
	perms.add(new PropertyPermission("java.vm.specification.vendor",  "read"));
	perms.add(new PropertyPermission("java.vm.specification.name",    "read"));
	perms.add(new PropertyPermission("java.vm.version",               "read"));
	perms.add(new PropertyPermission("java.vm.vendor",                "read"));
	perms.add(new PropertyPermission("java.vm.name",                  "read"));
	
	// Java Web Start specific permission
	perms.add(new PropertyPermission("javawebstart.version",          "read"));
	
	// Runtime perms
	perms.add(new RuntimePermission("exitVM"));
	perms.add(new RuntimePermission("stopThread"));
	// Accordiing to rev 13. these are not granted. (stricly applet sandbox complient)
	//perms.add(new RuntimePermission("modifyThread"));	
	//perms.add(new RuntimePermission("modifyThreadGroup"));
	
	// According to rev. 9 these are not granted.
	//perms.add(new RuntimePermission("setIO"));
	//perms.add(new RuntimePermission("accessDeclaredMembers"));
	
	// Applets might access stuff in the sun.applet part.
	// NOPE. This is wrong - fails applet security test if so
	//perms.add(new RuntimePermission("accessClassInPackage.sun.applet"));
	
	// AWT perms
	// The following two permissions will be removed in the final revision
	// of the 1.0 JNLP specificaion
	//perms.add(new AWTPermission("accessEventQueue"));
	//perms.add(new AWTPermission("listenToAllAWTEvents"));

	// Note: this string is not translated on purpose
	String warningString =  "Java " + 
				(isApplet ? "Applet" : "Application") + 
				" Window";
	if (Config.getBooleanProperty(Config.SEC_AWT_WARN_WINDOW_KEY)) {
	    System.setProperty("awt.appletWarning", warningString);
	} else {
	    perms.add(new AWTPermission("showWindowWithoutWarningBanner"));
	}
	
	// allows anyone to listen on un-privileged ports.
	perms.add(new SocketPermission("localhost:1024-", "listen"));
	// Access to exactly one host
	perms.add(new SocketPermission(_host, "connect, accept"));
	
	// Add user-defined jnlp.* properties to access list
        perms.add(new PropertyPermission("jnlp.*", "read,write"));
        perms.add(new PropertyPermission("javaws.*", "read,write"));

	// Add properties considered "Secure"
	String [] secureProps = Config.getSecureProperties();
	for (int i=0; i<secureProps.length; i++) {
	    perms.add(new PropertyPermission(secureProps[i], "read,write"));
	}
    }
}


