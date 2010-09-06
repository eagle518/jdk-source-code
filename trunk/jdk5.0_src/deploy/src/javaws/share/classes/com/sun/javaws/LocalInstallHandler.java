/*
 * @(#)LocalInstallHandler.java	1.36 04/04/28
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.security.*;
import java.util.*;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import javax.swing.*;
import java.awt.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.*;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.association.*;
import com.sun.deploy.association.Action;
import com.sun.javaws.cache.Cache;

/**
 * Instances of LocalInstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via the class method <code>getInstance</code>. A
 * null return value from <code>getInstance</code> indicates the
 * current platform does not support any install options.
 * <p>
 * Instances of LocalInstallHandler are thread safe, that is
 * <code>install</code> or <code>uninstall</code> can be invoked from any
 * thread. Although it is possible for <code>uninstall</code> to be
 * invoked while <code>install</code> is still running, implementations should
 * not worry about this happening.
 *
 * @version 1.36 04/28/04
 */
public abstract class LocalInstallHandler {
    /** The shared instance of LocalInstallHandler. */
    private static LocalInstallHandler _installHandler;

    /**
     * Returns the LocalInstallHandler appropriate for the current platform.
     * This may return null, indicating the platform does not support
     * an installer.
     */
    public static synchronized LocalInstallHandler getInstance() {
        if (_installHandler == null) {
            _installHandler = LocalInstallHandlerFactory.newInstance();
        }
        return _installHandler;
    }

    /**
     * Installs the Application identified by <code>lp</code> in whatever
     * manner is appropriate for the given platform.
     */
    public abstract void install(LaunchDesc ld, LocalApplicationProperties lap);

    /**
     * Uninstalls a previously installed Application
     */
    public abstract void uninstall(LaunchDesc ld,
                    LocalApplicationProperties lap, boolean desktop);
    /**
     * determines if this platform supports Local Install
     */
    public abstract boolean isLocalInstallSupported();

    public abstract boolean isAssociationSupported();
    
    public abstract void associationCompleted();

    public abstract String getAssociationOpenCommand(String jnlpLocation);
    public abstract String getAssociationPrintCommand(String jnlpLocation);
    public abstract void registerAssociationInternal(Association assoc) throws AssociationAlreadyRegisteredException, RegisterFailedException;
    public abstract void unregisterAssociationInternal(Association assoc) throws AssociationNotRegisteredException, RegisterFailedException;
    public abstract String getDefaultIconPath();

    private String getJnlpLocation(LaunchDesc ld) {
	String jnlpLocation;
	File f = null;
	try {
	    f = Cache.getCachedLaunchedFile(ld.getCanonicalHome());
	} catch (IOException ioe) {}

	if (f != null) {
	    jnlpLocation = f.getAbsolutePath();
	} else {
	    jnlpLocation = ld.getLocation().toString();
	}
	return jnlpLocation;
    }

    private boolean promptUserAssociation(final LaunchDesc ld, 
		final Association assoc_new, final boolean replace, 
		final String command, boolean isSilent, Frame owner) {
	// silent mode
	// create association without asking user
	if (isSilent) return true;

	// ask user whether to accept the association creation
	String message = "";
	String newMimeType = assoc_new.getMimeType();
	ArrayList newFileExtList = (ArrayList)assoc_new.getFileExtList();
	String fileExt = "";

	if (newFileExtList != null) {
	    Iterator i = newFileExtList.iterator();
	    while(i.hasNext()) {
		fileExt += i.next();
		if (i.hasNext()) {
		    fileExt += ", ";
		}
	    }
	}

	if (replace) {
	    // association already exist, does user want to replace it?
	    message = ResourceManager.getString("javaws.association.dialog.existAsk") + "\n\n";
	    if (fileExt != "") {
		message += ResourceManager.getString("javaws.association.dialog.ext", fileExt) + "\n";
	    }
	    if (newMimeType != null) {
		message += ResourceManager.getString("javaws.association.dialog.mime", newMimeType) + "\n";
	    }
	    if (command == null) {
		message += "\n" + ResourceManager.getString("javaws.association.dialog.exist");
	    } else {
		message += "\n" + ResourceManager.getString("javaws.association.dialog.exist.command", command);
	    }
	    message += "\n" + ResourceManager.getString("javaws.association.dialog.askReplace", ld.getInformation().getTitle());
	} else {
	    // new association creation
	    message = ResourceManager.getString("javaws.association.dialog.ask", ld.getInformation().getTitle()) + "\n";
	    if (fileExt != "") {
		message += ResourceManager.getString("javaws.association.dialog.ext", fileExt) + "\n";
	    }
	    if (newMimeType != null) {
		message += ResourceManager.getString("javaws.association.dialog.mime", newMimeType) + "\n";
	    }
	}
	int ret = 1;
	if (!isSilent) {
	    ret = DialogFactory.showConfirmDialog(owner, message, 
		ResourceManager.getString("javaws.association.dialog.title"));
	}
	if (ret == 0) return true;
	return false;
    }   

    private String getOpenActionCommand(Association assoc) {
	Action action = assoc.getActionByVerb("open");
	String command = null;
	if (action != null) {
	    command = action.getCommand();	
	}
	return command;
    }

    private boolean registerAssociation(final LaunchDesc ld, final String 
	extensions, final String mimeType, boolean isSilent, Frame owner) {
	AssociationService assocService = new AssociationService();
	Association assoc = new Association();
	boolean assocExist = false;
        Association extAssoc = null;
	String desc = "";
	String command = null;

	if (extensions != null) {
	    StringTokenizer st = new StringTokenizer(extensions);
	    String ext;

	    while (st.hasMoreTokens()) {
	        ext = "." + st.nextToken();
	        Trace.println("associate with ext: " + ext, TraceLevel.BASIC);
	        if (desc == "") {
		    desc = ext + " file";
	        }
	        extAssoc = assocService.getFileExtensionAssociation(ext);
	        if (extAssoc != null) {
		    Trace.println("associate with ext: " + ext + 
			" already EXIST", TraceLevel.BASIC);
		    if (command == null) {
		        command = getOpenActionCommand(extAssoc);	
		    }
		    assocExist = true;
	        }
	        assoc.addFileExtension(ext);
	    }
	}
	if (mimeType != null) {
	    Trace.println("associate with mime: "+ mimeType, TraceLevel.BASIC);
	    extAssoc = assocService.getMimeTypeAssociation(mimeType);
	    if (extAssoc != null) {
	        Trace.println("associate with mime: " + mimeType +
		     " already EXIST", TraceLevel.BASIC);
	        if (command == null) {
		    command = getOpenActionCommand(extAssoc);
	        }
	        assocExist = true;
	    }
	    assoc.setMimeType(mimeType);
	}
	assoc.setName(ld.getInformation().getTitle());
	assoc.setDescription(desc);
	String iconPath = IcoEncoder.getIconPath(ld);

	if (iconPath == null) {
	    iconPath = getDefaultIconPath();
	}

	assoc.setIconFileName(iconPath);
	
	Action oneAction;
	String jnlpLocation = getJnlpLocation(ld);

	String openAction = getAssociationOpenCommand(jnlpLocation);

	String printAction = getAssociationPrintCommand(jnlpLocation);

	Trace.println("register OPEN using: " + openAction, TraceLevel.BASIC);
	oneAction = new Action("open", openAction, "open the file");
	assoc.addAction(oneAction);

	if (printAction != null) {
	    Trace.println("register PRINT using: " + printAction, TraceLevel.BASIC);
	    oneAction = new Action("print", printAction, "print the file");
	    assoc.addAction(oneAction);
	}

	try {
	    // Register an association in user level.
	    // only do system level association if in system mode

	    // check if we should prompt user first
	    if (!Globals.createAssoc()) {
		switch (Config.getAssociationValue()) {
		case Config.ASSOCIATION_NEVER:
		    return false;  // should not get here
		    
		case Config.ASSOCIATION_NEW_ONLY:
		    // do not create association if it exist already
		    if (assocExist) return false;
		    break;
		    
		case Config.ASSOCIATION_ASK_USER:
		    // ask user
		    if (!promptUserAssociation(ld, assoc, assocExist, 
						command, isSilent, owner)) {
			return false;
		    }
		    break;
		    
		case Config.ASSOCIATION_REPLACE_ASK:
		    if (assocExist) {
			if (!promptUserAssociation(ld, assoc, assocExist, 
						command, isSilent, owner)) {
			    return false;
			}
		    }
		    break;
	
		default:       
		    // should not get here
		    // ask user
		    if (!promptUserAssociation(ld, assoc, assocExist, 
						command, isSilent, owner))  {
			return false;
		    }
		}
	    }
	    registerAssociationInternal(assoc);
	} catch (AssociationAlreadyRegisteredException e) {

	    // unregister it and register again
	    try {
		unregisterAssociationInternal(assoc);
		registerAssociationInternal(assoc);
	    } catch (AssociationNotRegisteredException anre) {
		Trace.ignoredException(anre);	
		return false;
	    } catch (AssociationAlreadyRegisteredException aare) {
		Trace.ignoredException(aare);
		return false;
	    } catch (RegisterFailedException rfe) {
		Trace.ignoredException(rfe); 
		return false;
	    }
	} catch (RegisterFailedException e) {
	    Trace.ignoredException(e); 
	    return false;
	}	
	return true;
    }
    
    private void unregisterAssociation(final LaunchDesc ld, 
		final String mimeType, final String extensions) {
	AssociationService assocService = new AssociationService();
	Association assoc = null;
	if (extensions != null) {
	    StringTokenizer st = new StringTokenizer(extensions);
	    String ext;
	    while (st.hasMoreTokens()) {
	        ext = "." + st.nextToken();

	        assoc = assocService.getFileExtensionAssociation(ext);
	
	        if (assoc != null) {
		    assoc.setName(ld.getInformation().getTitle());
	
		    Trace.println("remove association with ext: " +
			 ext, TraceLevel.BASIC);
		    try {
		        unregisterAssociationInternal(assoc);
		    } catch (AssociationNotRegisteredException anre) {
		        Trace.ignoredException(anre);
		    } catch (RegisterFailedException rfe) {
		        Trace.ignoredException(rfe);
		    }	
	        }
    	    }
	} 

	if (mimeType != null) {
	    assoc = assocService.getMimeTypeAssociation(mimeType);

	    if (assoc != null) {
	        assoc.setName(ld.getInformation().getTitle());

	        Trace.println("remove association with mime: " + 
			mimeType, TraceLevel.BASIC);
	        try {
		    unregisterAssociationInternal(assoc);
	        } catch (AssociationNotRegisteredException anre) {
		    Trace.ignoredException(anre);
	        } catch (RegisterFailedException rfe) {
		    Trace.ignoredException(rfe);
	        }	
	    }
	}

    }

    public void removeAssociations(final LaunchDesc ld, 
				   LocalApplicationProperties lap) {
	if (isAssociationSupported()) {
	    AssociationDesc[] adesc = lap.getAssociations();
	    String extensions;
	    String mimeType;
	    if (adesc != null) {
	        for (int i=0; i < adesc.length; i++) {
		    extensions = adesc[i].getExtensions();
		    mimeType = adesc[i].getMimeType();
	    	
		    unregisterAssociation(ld, mimeType, extensions);
	        }
		lap.setAssociations(null);
	        associationCompleted();
	    }
	}
    }


    public void createAssociations(final LaunchDesc ld,
				   LocalApplicationProperties lap, 
				   boolean isSilent, Frame owner) {
	if (Config.getAssociationValue() == Config.ASSOCIATION_NEVER) return;
	if (isAssociationSupported()) {
	    AssociationDesc[] adesc = ld.getInformation().getAssociations();
	    String extensions;
	    String mimeType;
	    
	    for (int i=0; i < adesc.length; i++) {
		extensions = adesc[i].getExtensions();
		mimeType = adesc[i].getMimeType();	    
		if (registerAssociation(ld, extensions, mimeType, 
					isSilent, owner)) {
		    lap.addAssociation(adesc[i]);
		    save(lap);
		} else {
		}
	    }
	    associationCompleted();
	}
    }

    /**
     * Invoked when an application is first launched. This should determine
     * if the user should be prompted for an install, and if necessary do
     * the install.
     */  
    public void installFromLaunch(final LaunchDesc ld, 
				final LocalApplicationProperties lap,
				boolean isSilent, Frame owner) {

	ShortcutDesc hints = ld.getInformation().getShortcut();
	if (hints != null) {
	    if (!hints.getDesktop() && (!hints.getMenu())) {
		return;
	    }
	}
	if (isSilent) {
	    if (Globals.createShortcut()) {
		doInstall(ld, lap);
		return;
	    }
	}
        switch (Config.getShortcutValue()) {
            case Config.SHORTCUT_NEVER:
                return;

            case Config.SHORTCUT_ALWAYS:
                doInstall(ld, lap);
                return;

	    case Config.SHORTCUT_ALWAYS_IF_HINTED:
		if (hints != null) {
                    doInstall(ld, lap);
	 	}
                return;

	    case Config.SHORTCUT_ASK_IF_HINTED:
		if (hints == null) {
		     return;
		}
		// intentionally fall thru to ask ...

            case Config.SHORTCUT_ASK_USER:
            default:
                if (lap.getAskedForInstall()) {
                    return; // already asked
                }
                break;          // OK - go ahead and ask
        }
	if (isSilent) {
            // silent and not Globals.createShortcut()
	    return;
	} else {
            showDialog(ld, lap, owner);
	}
    }

    private void showDialog(LaunchDesc ld, LocalApplicationProperties lap, 
			    Frame owner) {

        int ret = DesktopIntegration.showDTIDialog(owner, ld);
       
        switch (ret) {
            case Config.SHORTCUT_YES:
                doInstall(ld, lap);
                break;
            case Config.SHORTCUT_NO:
                lap.setAskedForInstall(true);
                break;
            default:
                lap.setAskedForInstall(false);
                break;
        }
    }

    public void doInstall(LaunchDesc ld, LocalApplicationProperties lap) {
	install(ld, lap);
        lap.setAskedForInstall(true);
	RContentDesc [] rc = ld.getInformation().getRelatedContent();
	if (rc != null) {
	    for (int i=0; i<rc.length; i++) {
		URL href = rc[i].getHref();
		if ( !("jar".equals(href.getProtocol())) &&
		    href.toString().endsWith(".jnlp"))  {
		    try {
			Main.importApp(href.toString());
		    } catch (Exception e) {
		        Trace.ignoredException(e);
		    }
		}
	    }
	}
    }
 

    /**
     * Returns true if the user wants to install over the existing
     * application.
     */
    public static boolean shouldInstallOverExisting(final LaunchDesc ld) {
        final int result[] = { DialogFactory.NO_OPTION };
        Runnable confirmRunnable = new Runnable() {
	    public void run() {
		result[0] = DialogFactory.showConfirmDialog (
		    ResourceManager.getString( "install.alreadyInstalled",
		        ld.getInformation().getTitle()), 
		    ResourceManager.getString("install.alreadyInstalledTitle"));
	    }
        };
	
	if (!Globals.isSilentMode()) {
            invokeRunnable(confirmRunnable);
	}
        return (result[0] == DialogFactory.YES_OPTION);
    }

    public static void invokeRunnable(Runnable runner) {
        if (SwingUtilities.isEventDispatchThread()) {
	    runner.run();
        }
        else {
	    try {
		SwingUtilities.invokeAndWait(runner);
	    } catch (InterruptedException ie) {
	    } catch (java.lang.reflect.InvocationTargetException ite) {}
        }
    }
    public static void save(LocalApplicationProperties lap) {
        // Save the LocalApplicationProperties state.
        try {
	    lap.store();
        } catch (IOException ioe) {
	    Trace.ignoredException(ioe);
        }
    }
    public boolean addUninstallShortcut() {
	if (Config.getBooleanProperty(Config.SHORTCUT_UNINSTALL_KEY) &&
	    !(Globals.isSystemCache()) ) {
		return true;
	}
	return false;
    }
}
