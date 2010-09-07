/*
 * @(#)LocalInstallHandler.java	1.74 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.util.*;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import javax.swing.SwingUtilities;
import java.awt.Component;
import com.sun.javaws.jnl.*;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.association.*;
import com.sun.deploy.association.Action;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.Environment;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.AssociationDesc;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.ui.ComponentRef;


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
 * @version 1.74 03/24/10
 */
public abstract class LocalInstallHandler {
    /** The shared instance of LocalInstallHandler. */
    private static LocalInstallHandler _installHandler;
   
    public static final int DESKTOP_INDEX = 0;
    public static final int MENU_INDEX = 1;

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
    public void install(LaunchDesc                 ld,
                        LocalApplicationProperties lap,
                        boolean                    updated,
                        boolean                    isSilent,
                        ComponentRef                  owner) {
        // Only install if this is an application that either allows offline
        // execution, or provides an href where the JNLP can be retrieved from
        // if necessary.
        if (ld.isApplicationDescriptor() &&
            ((ld.getLocation() != null) ||
             (ld.getInformation().supportsOfflineOperation()))) {

            boolean needAssociation = false;
            boolean needShortcut = false;

            // if local install is supported, then setup any associations,
            // and the optional desktop, and menu entries
            if (isLocalInstallSupported()) {
                AssociationDesc [] ad = lap.getAssociations();

                if ((ad == null) || (ad.length <= 0)) {
                    // create associations the first time the app is launched
	    	    AssociationDesc[] adesc = 
                        ld.getInformation().getAssociations();
		    needAssociation = (isAssociationSupported() && 
                        adesc.length > 0);
                } else if (updated) {
                    // the associations may have changed in the updated JNLP,
                    // so remove any existing ones, and create new ones from
                    // the new JNLP
                    removeAssociations(ld, lap);
                    createAssociations(ld, lap);
                }

                if (lap.isLocallyInstalled()) {
		    if (!lap.isLocallyInstalledSystem()) {

                        // if updated, and some shortcut still exists
                        if (updated) {
                            // remove old shortcut and add new
			    boolean [] which = whichShortcutsExist(lap);
                            removeShortcuts(ld, lap, true);

			    // If lap says it is locallyInstalled, but which[] tells 
			    // none of shortcuts exist. This is clearly not a update
			    // case. We need create shortcuts as brand new.
			    if (which[DESKTOP_INDEX] == false &&   
				which[MENU_INDEX] == false) {
				needShortcut = true;
			    } else {
				// update existing one
				createShortcuts(ld, lap, which);
			    }
                        }
		    }
                }
                else {
                    needShortcut = true;
                }
            }

            // add into registry:
            if (updated || (lap.getLaunchCount() <= 1)) {
                // remove from install panel with old title
                removeFromInstallPanel(ld, lap);
                // register with new title attribute
                registerWithInstallPanel(ld, lap);
            }

            if (needShortcut || needAssociation) {
                if (lap.getAskedForInstall() == false) {
                    try {
                        performIntegration(owner, ld, lap, isSilent, 
                                   needShortcut, needAssociation);
                    } catch (Throwable t) {
                        Trace.ignored(t);
                    }
                    lap.setAskedForInstall(true);
                }
            } 
        }
    }

    /**
     * Uninstalls a previously installed Application
     */
    public void uninstall(LaunchDesc                 ld,
                          LocalApplicationProperties lap) {

        removeShortcuts(ld, lap, true);
        removeFromInstallPanel(ld, lap);
        removeAssociations(ld, lap);
    }
    /**
     * determines if this platform supports Local Install
     */
    public abstract boolean isLocalInstallSupported();

    public abstract boolean isAssociationSupported();
    
    abstract boolean isAssociationFileExtSupported(String ext);

    public abstract String getAssociationOpenCommand(String jnlpLocation);
    public abstract String getAssociationPrintCommand(String jnlpLocation);
    public abstract void registerAssociationInternal(Association assoc)
        throws AssociationAlreadyRegisteredException, RegisterFailedException;
    public abstract void unregisterAssociationInternal(Association assoc) 
        throws AssociationNotRegisteredException, RegisterFailedException;
    public abstract boolean hasAssociation(Association assoc);

    public abstract String getDefaultIconPath();

    public abstract boolean isShortcutExists(LocalApplicationProperties lap);
    public abstract boolean[] whichShortcutsExist(
					     LocalApplicationProperties lap);

    protected abstract boolean createShortcuts(LaunchDesc                 ld,
                                               LocalApplicationProperties lap,
					       boolean[]                  which);

    protected abstract boolean removeShortcuts(LaunchDesc                 ld,
                                               LocalApplicationProperties lap,
                                               boolean                    desktop);

    protected abstract boolean removePathShortcut(String path);

    protected abstract void registerWithInstallPanel(LaunchDesc ld, 
                                     LocalApplicationProperties lap);
    protected abstract void removeFromInstallPanel(LaunchDesc ld, 
                                   LocalApplicationProperties lap);


    private String getJnlpLocation(LaunchDesc ld) {
        String jnlpLocation;
        File f = null;
    
        try {
            f = DownloadEngine.getCachedFile(ld.getCanonicalHome());
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        }
     

        if (f != null) {
            jnlpLocation = f.getAbsolutePath();
        } else {
            jnlpLocation = ld.getLocation().toString();
        }
        return jnlpLocation;
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
                        extensions, final String mimeType,
                        final String description, final URL iconRef) {
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
                if (isAssociationFileExtSupported(ext) == false) {
                    Trace.println("association with ext: " + ext + 
                            " is not supported", TraceLevel.BASIC);
                    return false;
                }
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
		    if (!promptReplace(ld, ext, null, command)) {
			return false;
		    }
                }
                assoc.addFileExtension(ext);
            }
        }
        if (description != null && description.length() > 0) {
            desc = description;
        }
        if (mimeType != null) {
            Trace.println("associate with mime: "+ mimeType, TraceLevel.BASIC);
            extAssoc = assocService.getMimeTypeAssociation(mimeType);
            if (extAssoc != null & !assocExist) {
                Trace.println("associate with mime: " + mimeType +
                     " already EXIST", TraceLevel.BASIC);
		String newCommand = getOpenActionCommand(extAssoc);
		if (newCommand != command) {
	            if (!promptReplace(ld, null, mimeType, newCommand)) {
		        return false;
	            }
		}
                assocExist = true;
            }
            assoc.setMimeType(mimeType);
        }
        assoc.setName(ld.getInformation().getTitle());
        assoc.setDescription(desc);

        String iconPath;

        if (iconRef != null) {
            iconPath = IcoEncoder.getIconPath(iconRef, null);
        } else {
            iconPath = IcoEncoder.getIconPath(ld);
        }

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
            Trace.println("register PRINT using: " + printAction, 
                        TraceLevel.BASIC);
            oneAction = new Action("print", printAction, "print the file");
            assoc.addAction(oneAction);
        }

        try {
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
                    removeAssociationIfCurent(ld, mimeType, extensions);
                }
                lap.setAssociations(null);
            }
        }
    }

    private void removeAssociationIfCurent(LaunchDesc ld, String mimeType, 
                 String extensions) {
        String openAction = getAssociationOpenCommand(getJnlpLocation(ld));
        AssociationService assocService = new AssociationService();
	Association extAssoc = assocService.getMimeTypeAssociation(mimeType);
        if (extAssoc != null) {
            String existingOpenAction = getOpenActionCommand(extAssoc);
	    if (openAction.equals(existingOpenAction)) {
                unregisterAssociation(ld, mimeType, extensions);
	    } else {
		Trace.println("Not removing association because existing " +
		    "command is: " + existingOpenAction + " instead of: " + 
		    openAction, TraceLevel.BASIC);
	    }
	}
    }


    public boolean createAssociations(final LaunchDesc ld,
                                      LocalApplicationProperties lap) {
        if (Config.getAssociationValue() == Config.ASSOCIATION_NEVER) {
            return false;
        }
        boolean success = true;
        if (isAssociationSupported()) {
            AssociationDesc[] adesc = ld.getInformation().getAssociations();
            String extensions;
            String mimeType;

            for (int i=0; i < adesc.length; i++) {
                extensions = adesc[i].getExtensions();
                mimeType = adesc[i].getMimeType();
                String description = adesc[i].getMimeDescription();
                URL iconRef = adesc[i].getIconUrl();
                if (registerAssociation(ld, extensions, mimeType,
                                description, iconRef)) {
                    lap.addAssociation(adesc[i]);
                    save(lap);
                } else {
                    success = false;
                }
            }
        } else {
            success = false;
        }
        return success;
    }

    /**
     * performIntegration
     * 
     * Depending on the user's preferences, this method may prompt the user
     * allowing the shortcuts and associations to be installed.
     *
     * then the required and allowed Desktop Integration will be performed
     */
    public boolean performIntegration(final ComponentRef owner,
                                      final LaunchDesc ld,
                                      final LocalApplicationProperties lap,
                                      final boolean isSilent,
                                      boolean needShortcut,
                                      boolean needAssociation) {

        boolean success = false;
        boolean allow = true;
        boolean doShortcut = false;;
        boolean doAssociation = false;;
        boolean promptShortcut = false;
        boolean promptAssociation = false;
        boolean allPermissions = ld.getSecurityModel() != ld.SANDBOX_SECURITY;

        ShortcutDesc hints = ld.getInformation().getShortcut();
        if (hints != null) {
            if (!hints.getDesktop() && (!hints.getMenu())) {
                needShortcut = false;
            }
        }

        if (needShortcut) {

            if (isSilent) {
                doShortcut = Globals.createShortcut() ||
                    (Config.getShortcutValue() == Config.SHORTCUT_ALWAYS);
                promptShortcut = false;
            } else {
                switch (Config.getShortcutValue()) {
                    case Config.SHORTCUT_NEVER:
                        doShortcut = false;
                        promptShortcut = false;
                        break;
                    case Config.SHORTCUT_ALWAYS:
                        doShortcut = true;
                        promptShortcut = false;
                        break;
                    case Config.SHORTCUT_ALWAYS_IF_HINTED:
                        doShortcut = (hints != null);
                        promptShortcut = false;
                        break;
                    case Config.SHORTCUT_ASK_IF_HINTED:
                        doShortcut = (hints != null);
                        promptShortcut = (hints != null);
                        break;
                    case Config.SHORTCUT_ASK_USER:
                    default:
                        doShortcut = true;
                        promptShortcut = true;
                        break;
                }
            }
        } 

        if (needAssociation) {
            AssociationService assocService = new AssociationService();
            boolean allNew = true;
            AssociationDesc[] adesc = ld.getInformation().getAssociations();
            for (int i=0; i<adesc.length; i++) {
                if (assocService.getMimeTypeAssociation( 
                                        adesc[i].getMimeType()) != null) {
                    allNew = false;
                    break;
                }
                String extensions = adesc[i].getExtensions();
                StringTokenizer st = new StringTokenizer(extensions);
                while (st.hasMoreTokens()) {
                    String ext = "." + st.nextToken();
                    if (assocService.getFileExtensionAssociation(ext) != null) {
                        allNew = false;
                        break;
                    }
                }
            }

            if (isSilent) {
                promptAssociation = false;
                switch (Config.getAssociationValue()) {
                    case Config.ASSOCIATION_NEVER:
                        doAssociation = false;
                        break;
                    case Config.ASSOCIATION_NEW_ONLY:
                        doAssociation = allNew;
                        break;
                    case Config.ASSOCIATION_REPLACE_ASK:
                        doAssociation = allNew || Globals.createAssoc();
                        break;
		    case Config.ASSOCIATION_ALWAYS:
			doAssociation = true;
			break;
                    case Config.ASSOCIATION_ASK_USER:
                    default:
                        doAssociation = Globals.createAssoc();
                        break;
                }
            } else {
                switch (Config.getAssociationValue()) {
                    case Config.ASSOCIATION_NEVER:
                        doAssociation = false;
                        promptAssociation = false;
                        break;
                    case Config.ASSOCIATION_NEW_ONLY:
                        doAssociation = allNew;
                        promptAssociation = false;
                        break;
                    case Config.ASSOCIATION_REPLACE_ASK:
                        doAssociation = true;
                        promptAssociation = !allNew;
                        break;
		    case Config.ASSOCIATION_ALWAYS:
			doAssociation = true;
			promptAssociation = false;
			break;
                    case Config.ASSOCIATION_ASK_USER:
                    default:
                        doAssociation = true;
                        promptAssociation = true;
                        break;
                }
            }
        }

        if (promptShortcut || promptAssociation) {
            if ((!Environment.isImportMode() && allPermissions) || 
		showDialog(((owner == null) ? null : owner.get()), ld, lap, doShortcut, doAssociation)) {

                // user accepted the dialog
                // so go ahead and do whatever he was asked to allow
                if (promptShortcut) {
                    doShortcut = true;
                }
                if (promptAssociation) {
                    doAssociation = true;
                }
            } else {
                // user rejected the dialog
                // do do not do whatever he was being asked.
                if (promptShortcut) {
                    doShortcut = false;
                } 
                if (promptAssociation) {
                    doAssociation = false;
                }
            }
        }
        if (doShortcut) {
            success = installShortcuts(ld, lap);
        }
        if (doAssociation) {
            success = createAssociations(ld, lap);
        }
        return success;
    }

    /**
     * Install only the application's shortcuts.  Do not change associations, or
     * any other aspects of desktop integration.
     */
    public boolean installShortcuts(LaunchDesc ld, LocalApplicationProperties lap)
    {
        return installShortcuts(ld, lap, null);
    }

    private boolean installShortcuts(LaunchDesc ld, LocalApplicationProperties lap,
	boolean[] which) {
        boolean success = createShortcuts(ld, lap, which);
        lap.setAskedForInstall(true);
        RContentDesc [] rc = ld.getInformation().getRelatedContent();
        if (rc != null) {
                for (int i=0; i<rc.length; i++) {
                URL href = rc[i].getHref();
                if ( !("jar".equals(href.getProtocol())) &&
                href.toString().endsWith(".jnlp")) {
                try {
                        Main.importApp(href.toString());
                    } catch (Exception e) {
                        Trace.ignoredException(e);
                    }
                }
            }
        }
        return success;
    }


    private boolean showDialog(Component owner, LaunchDesc ld, 
                               LocalApplicationProperties lap, 
                               boolean doShortcut, boolean doAssociation) {

        // setup icon info for ainfo
        InformationDesc id = ld.getInformation();
        IconDesc icon = id.getIconLocation(AppInfo.ICON_SIZE,
                                           IconDesc.ICON_KIND_DEFAULT);

        URL iconRef = (icon == null) ? null : icon.getLocation();
        String iconVer = (icon == null) ? null : icon.getVersion();

        // setup shortcut info for ainfo 
        boolean desktop = false;
        boolean menu = false;
        String submenu = null;
        if (doShortcut) {
            ShortcutDesc sd = id.getShortcut();
            if (sd != null) {
                desktop = sd.getDesktop();
                menu = sd.getMenu();
                submenu = sd.getSubmenu();
            } else {
                desktop = true;
                menu = true;
            }
        }

        // setup associations for ainfo 
        AssociationDesc [] associations = (doAssociation) ?
            id.getAssociations() : new AssociationDesc[0];

        AppInfo ainfo = new AppInfo(ld.getLaunchType(), id.getTitle(), 
                id.getVendor(), ld.getCanonicalHome(), iconRef, iconVer, 
                desktop, menu, submenu, associations);


        return (UIFactory.showIntegrationDialog(owner, ainfo) == UIFactory.OK);

    }

    private boolean promptReplace(LaunchDesc ld, String extension, String mime,
		String command) {
	
	String msg;
	if (extension != null) {
	    msg = ResourceManager.getString(
		"association.replace.ext", extension);
	} else {
	    msg = ResourceManager.getString(
		"association.replace.mime", mime);
	}
	// try to see if the command is of the form ".../javaws{.exe} <cache>
        // and if so try to convert it into the name of the Application 
        //
	String appTitle = command;
	String dir = Cache.getCacheDir().toString();
	int start = command.indexOf(dir);
	if (start >= 0) {
	    String path;
	    int end = command.indexOf("\"", start + dir.length());
	    if ( end < 0) {
	        end = command.indexOf(" ", start + dir.length());
	    }
	    if (end >= 0) {
		path = command.substring(start, end);
	    } else {
		path = command.substring(start);
	    }
	    try {
	        LaunchDesc appLd = LaunchDescFactory.buildDescriptor(path);
	        if (appLd != null) {
	            appTitle = appLd.getInformation().getTitle();
		    if (appLd.getCanonicalHome().toString().equals(
			ld.getCanonicalHome().toString())) {
			// associated with ourself - allow us to refress
			return true;
		    }
	        }
	    } catch (Exception e) {
		// in this case we are associated with something in our 
		// cache that is no longer there. 
		return true;
	    }
	}

	String info = 
	    ResourceManager.getString("association.replace.info", appTitle);
	String title = ResourceManager.getString("association.replace.title");

        return (UIFactory.showConfirmDialog(null, ld.getAppInfo(), 
					    msg, info, title) == UIFactory.OK);
    }

    /**
     * Uninstall only the application's shortcuts.  Do not change associations,
     * or any other aspects of desktop integration.
     */
    public boolean uninstallShortcuts(LaunchDesc ld,
				   LocalApplicationProperties lap) {
        return removeShortcuts(ld, lap, true);
    }

    /**
     * Returns true if the user wants to install over the existing
     * application.
     */
    public static boolean shouldInstallOverExisting(final LaunchDesc ld) {
        final int result[] = { UIFactory.ERROR };
        Runnable confirmRunnable = new Runnable() {
            public void run() {
                result[0] = UIFactory.showConfirmDialog (null, ld.getAppInfo(),
                    ResourceManager.getString( "install.alreadyInstalled",
                        ld.getInformation().getTitle()),
                    ResourceManager.getString("install.alreadyInstalledTitle"));
            }
        };

        if (!Globals.isSilentMode()) {
            invokeRunnable(confirmRunnable);
        }
        return (result[0] == UIFactory.OK);
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
            !(Environment.isSystemCacheMode()) ) {
                return true;
        }
        return false;
    }

    boolean removeShortcuts(String path) {
        return (removePathShortcut(path));
    }

    void removeAssociations(String mime, String extensions) {
        Association assoc = new Association();
        String desc = "" ;
        if (extensions != null) {
            StringTokenizer st = new StringTokenizer(extensions);

            while (st.hasMoreTokens()) {
                String ext = "." + st.nextToken();
                if (desc == "") {
                    desc = ext + " file";
                }
                assoc.addFileExtension(ext);
            }
        }
        if (mime != null) {
            assoc.setMimeType(mime);
        }
        assoc.setName(" ");
        assoc.setDescription(desc);
        try {
            unregisterAssociationInternal(assoc);
        } catch (Exception e) {
            Trace.ignored(e);
        }
    }

    void reinstallShortcuts(LaunchDesc ld, LocalApplicationProperties lap, 
	boolean doDesktop, boolean doMenu) {
	boolean [] which = new boolean[2];
	which[DESKTOP_INDEX] = doDesktop;
	which[MENU_INDEX] = doMenu;
        installShortcuts(ld, lap, which);
    }

    void reinstallAssociations(LaunchDesc ld, LocalApplicationProperties lap) {
        createAssociations(ld, lap);
    }


}
