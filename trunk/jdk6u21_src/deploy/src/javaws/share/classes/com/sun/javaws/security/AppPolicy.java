/*
 * @(#)AppPolicy.java	1.98 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.security.Policy;
import java.security.CodeSource;
import java.security.AccessControlException;
import java.security.PermissionCollection;
import java.security.Permission;
import java.security.AllPermission;
import java.util.PropertyPermission;
import java.util.Properties;
import java.util.Enumeration;
import java.lang.RuntimePermission;
import java.awt.AWTPermission;
import java.net.SocketPermission;
import java.io.FilePermission;
import java.io.File;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ShortcutDesc;
import com.sun.javaws.Main;
import com.sun.javaws.Globals;
import com.sun.javaws.LocalInstallHandler;
import com.sun.jnlp.JNLPClassLoaderUtil;
import com.sun.jnlp.JNLPClassLoaderIf;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.security.BadCertificateDialog;
import com.sun.deploy.security.CeilingPolicy;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.PerfLogger;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.Environment;


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
    
    /** Return the security policy in form of a PermissionCollection.
        The addPermissionsFromJNLPFile argument indicates whether to
        grant trusted permissions based on the all-permissions request
        in the JNLP file. If this is true then
        LaunchDownload.checkJNLPSecurity <B>must</B> have been called
        against the LaunchDesc, since this method does no validation
        of code signing. */
    public boolean addPermissions(PermissionCollection perms, CodeSource target, boolean addPermissionsFromJNLPFile) throws ExitException {
        return addPermissions( JNLPClassLoaderUtil.getInstance(), perms, target, addPermissionsFromJNLPFile );
    }

    /** Return the security policy in form of a PermissionCollection.
        The addPermissionsFromJNLPFile argument indicates whether to
        grant trusted permissions based on the all-permissions request
        in the JNLP file. If this is true then
        LaunchDownload.checkJNLPSecurity <B>must</B> have been called
        against the LaunchDesc, since this method does no validation
        of code signing. */
    public boolean addPermissions(JNLPClassLoaderIf cl, PermissionCollection perms, CodeSource target, boolean addPermissionsFromJNLPFile) throws ExitException {

        Trace.println("JAVAWS AppPolicy Permission requested for: " + target.getLocation(), TraceLevel.SECURITY);
        
        // Find JARDesc for this entry
        JARDesc jd = cl.getJarDescFromURL(target.getLocation());
        if (jd == null) {
	    // Not from a JARDesc attribute. will default to correct permissions
            return false;
        }

	
        // Get LaunchDesc for for the JARDesc.
        LaunchDesc ld = null; 
        int access = LaunchDesc.SANDBOX_SECURITY; 
	boolean trusted = false;
        // jd will not have parent if it was not in jnlp file 
        if (jd.getParent() != null) { 
            ld = jd.getParent().getParent(); 
            access = ld.getSecurityModel();         
        } 

        if (addPermissionsFromJNLPFile) {
            if (access != LaunchDesc.SANDBOX_SECURITY) {
                grantUnrestrictedAccess(ld, target);
		trusted = true;
                if (access == LaunchDesc.ALLPERMISSIONS_SECURITY) {
                    CeilingPolicy.addTrustedPermissions(perms);
                } else {
                    addJ2EEApplicationClientPermissionsObject(perms);
                }
            }
        }

	
	if (!perms.implies(new AllPermission())) {
	    addSandboxPermissionsObject(perms, (ld != null && 
                ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE));   
        }

	// only set the system properties the first time for this ld
        if (ld != null && !ld.arePropsSet()) { 
	    // now - only get the props for this ld (not extensions)
	    Properties props = ld.getResources().getResourceProperties();
            Enumeration keys = props.keys();
            while(keys.hasMoreElements()) {
                String name = (String)keys.nextElement();
                String value = props.getProperty(name);
                Permission perm = new PropertyPermission(name, "write");
		PermissionCollection policyPerms = 
		    Policy.getPolicy().getPermissions(target);
		// only set property if app is allowed to
                if (perms.implies(perm) || policyPerms.implies(perm)) {
                    System.setProperty(name, value);
                } else {
		    Trace.ignoredException(new AccessControlException(
					       "access denied "+perm, perm));
		}
	    }
	    ld.setPropsSet(true);
	}
	return trusted;
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


    /* If no exception is thrown then access was granted.
     * If return value is > 0 then access was granted on permanent basis 
     *   until given timestampt.
     * If 0 is returned then access either granted for session or 
     *   no actual validation were performed (fasttrack case)
     */
    public long grantUnrestrictedAccess(LaunchDesc ld, CodeSource cs) 
            throws ExitException {
        /* Skip actual check if descriptor is known to be trusted, i.e.
         * if it was successfully validated during previous run previous
         * and nothing had changed since then.
         * NB: to benefit from caching of security validation results
         * we should avoid creation of multiple instances of
         * LaunchDesc objects for the same input jnlp xml file */
        boolean fasttrack = ld.isTrusted() || (Globals.isSecureMode() && ld.isInstaller());
        AppInfo ainfo = null;

        if (!fasttrack) {
            ainfo = ld.getAppInfo();
            boolean dti = false;

            /* Only applets or applications can try to do desktop integration.
               If ld is extension descriptor then there will be no LAP file anyway. */
            if (ld.getAppletDescriptor() != null ||
                    ld.getApplicationDescriptor() != null) {
                LocalInstallHandler lih = LocalInstallHandler.getInstance();
                if (lih.isLocalInstallSupported()) {
                    LocalApplicationProperties lap =
                            Cache.getLocalApplicationProperties(ld.getCanonicalHome());
                    if (lap != null) {
                        if (lap.getAskedForInstall() == false) {
                            dti = true;
                        }
                    }
                }

                if (dti) {
                    ShortcutDesc sd = ld.getInformation().getShortcut();
                    switch (Config.getShortcutValue()) {
                        default:
                        case Config.SHORTCUT_NEVER:
                            break;
                        case Config.SHORTCUT_ASK_IF_HINTED:
                            if (sd == null) {
                                break;
                            }
                        // fall thru
                        case Config.SHORTCUT_ALWAYS:
                        case Config.SHORTCUT_ASK_USER:
                        case Config.SHORTCUT_ALWAYS_IF_HINTED:
                            if (sd != null) {
                                ainfo.setDesktopHint(sd.getDesktop());
                                ainfo.setMenuHint(sd.getMenu());
                                ainfo.setSubmenu(sd.getSubmenu());
                            } else {
                                ainfo.setDesktopHint(true);
                                ainfo.setMenuHint(true);
                            }
                            break;
                    }
                    if (lih.isAssociationSupported()) {
                        if (Config.getAssociationValue() != Config.ASSOCIATION_NEVER) {
                            ainfo.setAssociations(ld.getInformation().getAssociations());
                        }
                    }
                }
            }
        }

        try { 
	        // -secure only have effect when we are an signed installer
	        // we use -secure during uninstall to prevent security
	        // dialog pops up
	        // Display different message in security warning dialog box
            // for unsigned jnlp files that perform a trusted operations
            boolean jnlpUnsignUnsecu = !ld.isSigned() && !ld.isSecureJVMArgs();

	    if (fasttrack) {
                setUnrestrictedProps(ld);
                return 0;
            }

            long r = TrustDecider.isAllPermissionGranted(cs, ainfo, jnlpUnsignUnsecu);
            if (r != TrustDecider.PERMISSION_DENIED) {
                PerfLogger.setTime("End isAllPermissionGranted " + cs.getLocation());
                setUnrestrictedProps(ld);
                if (r != TrustDecider.PERMISSION_GRANTED_FOR_SESSION) {
                    return r;
                }
                return 0;
            } else {
                Trace.println("We were not granted permission, exiting", TraceLevel.SECURITY);
            }
        } catch (Exception exception) {
            BadCertificateDialog.showDialog(cs, ainfo, exception);
        }  
        // not granted - or thrown exception
        Main.systemExit(-1);
        return 0;
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
        if (JNLPClassLoaderUtil.getInstance() instanceof JNLPClassLoader) {
            perms.add(new RuntimePermission("exitVM"));
            perms.add(new RuntimePermission("stopThread"));
        }
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
	
	// allow read/write to jnlp.*, javaws.*, and javapi.*
        perms.add(new PropertyPermission("jnlp.*", "read,write"));
        perms.add(new PropertyPermission("javaws.*", "read,write"));
        perms.add(new PropertyPermission("javapi.*", "read,write"));

	// Add properties considered "Secure"
	String [] secureProps = Config.getSecureProperties();
	for (int i=0; i<secureProps.length; i++) {
	    perms.add(new PropertyPermission(secureProps[i], "read,write"));
	}
	String [] args = Globals.getApplicationArgs();
	if (args != null && (args.length == 2)) {
	    if (args[0].equals("-open")) {
 	        // allow application to access the given file
	        perms.add(new FilePermission(args[1], "read, write"));
	    } else if (args[0].equals("-print")) {
 	        // allow application to access and print the given file
	        perms.add(new FilePermission(args[1], "read, write"));
		perms.add(new RuntimePermission("queuePrintJob"));
	    }
	}
    }
}


