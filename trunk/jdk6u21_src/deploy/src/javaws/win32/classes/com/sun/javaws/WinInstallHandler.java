/*
 * @(#)WinInstallHandler.java	1.101 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.net.*;
import java.util.StringTokenizer;

import com.sun.javaws.jnl.*;

import com.sun.deploy.util.WinRegistry;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.association.*;
import com.sun.deploy.association.utility.AppUtility;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.Environment;
import com.sun.deploy.cache.LocalApplicationProperties;

/**
 * Instances of InstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via <code>LocalInstallHandler.getInstance()</code>. 
 *
 * @version 1.8 03/02/00
 */
public class WinInstallHandler extends LocalInstallHandler {

    // Used in LocalApplicationProperties, gives the name the shortcut
    // was created as, which may differ from the real name (if a shortcut
    // already existed with that name).
    private static final String INSTALLED_DESKTOP_SHORTCUT_KEY =
        "installed.desktop";
    private static final String INSTALLED_START_MENU_KEY =
        "installed.menu";
    private static final String UNINSTALLED_START_MENU_KEY =
        "installed.uninstalled";
    private static final String RCONTENT_START_MENU_KEY =
        "installed.rc";


    public static final int TYPE_DESKTOP = 1;
    public static final int TYPE_START_MENU = 2;


    /**
     * Path in registry to find shortcuts.
     */
    private static final String REG_SHORTCUT_PATH =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";    /**
     * Key under REG_SHORTCUT_PATH to find desktop shotcut paths.
     */
    private static final String REG_DESKTOP_PATH_KEY = "Desktop";
    /**
     * Key under REG_SHORTCUT_PATH to find start menu shotcut paths.
     */
    private static final String REG_START_MENU_PATH_KEY = "Programs";

    private static final int MAX_PATH = 260;

    /**
     * Extension of shortcut path files.
     */
    private static final String SHORTCUT_EXTENSION = ".lnk";

    // shortcuts must be truncated to fit MAX_PATH; this is the max length of a
    // shortcut location and name, without the extension and NULL terminator
    private static final int MAX_SHORTCUT = (MAX_PATH -
                                             (SHORTCUT_EXTENSION.length() + 1));

    /**
     * Set to true when the paths for shortcuts and start menu have been
     * looked up.
     */
    private boolean _loadedPaths = false;
    /**
     * Path where desktop shortcuts should be placed.
     */
    private String _desktopPath;
    /**
     * Path where start menu shortcuts should be placed.
     */
    private String _startMenuPath;
    private static boolean useSystem;

    static {
        String os = System.getProperty("os.name");
        if (os.indexOf("2000") != -1 || os.indexOf("XP") != -1
                || Config.getInstance().isPlatformWindowsVista()) {
            // for win2k/winxp/vista, default to register user association only
            useSystem = false;
        } else {
            // for other windows,
            useSystem = true;
        }
    }

    public boolean isShortcutExists(LocalApplicationProperties lap) {
	String desktopPath = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY);
	String menuPath = lap.get(INSTALLED_START_MENU_KEY);

	boolean desktopExists = false;
	boolean menuExists = false;

	if (desktopPath != null) {
	    desktopExists = new File(desktopPath).exists();
	}

	if (menuPath != null) {
	    menuExists = new File(menuPath).exists();
	}

	if (desktopPath != null && menuPath != null) {
	    return desktopExists && menuExists;
	} else {
	    return desktopExists || menuExists;
	}
    }

    public boolean [] whichShortcutsExist(LocalApplicationProperties lap) {

	boolean [] which = new boolean[2];

	String desktopPath = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY);
	which[DESKTOP_INDEX] = ((desktopPath != null) &&
				(new File(desktopPath).exists()));
	
	String menuPath = lap.get(INSTALLED_START_MENU_KEY);
	which[MENU_INDEX] = ((menuPath != null) &&
			     (new File(menuPath).exists())); 
	
	return which;
    }


    public String getDefaultIconPath() {
	return  Config.getInstance().getSystemJavawsPath();
    }

    public String getAssociationOpenCommand(String jnlpLocation) {
	return  "\"" + Config.getJavawsCommand()+ "\"" + " \"-open\" \"%1\" " + "\"" + jnlpLocation + "\"";
    }
    public String getAssociationPrintCommand(String jnlpLocation) {
	return  "\"" + Config.getJavawsCommand()+ "\"" + " \"-print\" \"%1\" " + "\"" + jnlpLocation + "\"";
    }

    public void registerAssociationInternal(Association assoc) throws AssociationAlreadyRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Environment.isSystemCacheMode() || useSystem) {
	    // do system association registration

	    assocService.registerSystemAssociation(assoc);
	} else {
	    // do user association registration

	    assocService.registerUserAssociation(assoc);
	}
    }
    public void unregisterAssociationInternal(Association assoc) throws AssociationNotRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Environment.isSystemCacheMode() || useSystem) {
	    // do system association unregistration

	    assocService.unregisterSystemAssociation(assoc);
	} else {
	    // do user association unregistration

	    assocService.unregisterUserAssociation(assoc);
	}
    }

    public boolean hasAssociation(Association assoc) {
	AssociationService assocService = new AssociationService();
        return assocService.hasAssociation(assoc);
    }

    /**
     *  Determine if this platform supports Local Application Installation.
     *  We will want to return true for the window managers that we support
     *  in install and uninstall above.
     */
    public boolean isLocalInstallSupported() {
	return true;
    }

    public boolean isAssociationSupported() {
	return true;
    }
    
    /* 
     * Returns true if association is supported for the file extension; false
     * otherwise
     *
     */
    boolean isAssociationFileExtSupported(String extension) {
	if (extension == null) {
	    return false;
	}
        String ext = AppUtility.removeDotFromFileExtension(
            extension.toLowerCase());
        if (ext.equals("exe") || ext.equals("com") ||
            ext.equals("bat")) {
            return false;
        }
        
        return true;
    }

    public boolean removePathShortcut(String path) {
	File f = new File(path);
        if (f.exists()) {
            return f.delete();
	}
	return false;
    }

    protected boolean removeShortcuts(LaunchDesc                 ld,
                                      LocalApplicationProperties lap,
                                      boolean                    desktop) {
        if (lap == null) {

            Trace.println("No LAP for uninstall, bailing!", TraceLevel.TEMP);

            return false;
        }

        String path;
        String toEmpty = null;
        boolean failed = false;

        if ((path = lap.get(INSTALLED_START_MENU_KEY)) != null) {
            if (!deleteShortcut(path)) {
                failed = true;
            } else {
                lap.put(INSTALLED_START_MENU_KEY, null);
            }
            toEmpty = path;
        }

        if ((path = lap.get(UNINSTALLED_START_MENU_KEY)) != null) {
            if (!deleteShortcut(path)) {
                failed = true;
            } else {
                lap.put(UNINSTALLED_START_MENU_KEY, null);
            }
            toEmpty = path;
        }

        String paths = lap.get(RCONTENT_START_MENU_KEY);
        if (paths != null) {
            StringTokenizer st = new StringTokenizer(paths, File.pathSeparator);
            while (st.hasMoreElements()) {
                path = st.nextToken();
                if (path != null) {
                    if (!deleteShortcut(path)) {
                        failed = true;
                    }
                    toEmpty = path;
                }
            }
            lap.put(RCONTENT_START_MENU_KEY, null);
        }

        if (toEmpty != null) {
            checkEmpty(toEmpty);
        }

        if (desktop) {
            if ((path = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY)) != null) {
                if (!deleteShortcut(path)) {
                    failed = true;
                } else {
                    lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, null);
                }
            }
        }
        if (failed) {
            Trace.println("uninstall shortcut failed", TraceLevel.TEMP);
        }
        // Even if we failed, mark the application as not installed.
        lap.setLocallyInstalled(false);
        save(lap);

        return !failed;
    }

    private void checkEmpty(String path) {
	try {
	    File parent = (new File(path)).getParentFile();
	    while (parent != null && parent.isDirectory() &&
		((parent.list()).length == 0)) {
		parent.delete();
		parent = parent.getParentFile();
	    }
	} catch (Exception e) {
	}
    }

    /**
     * Returns true if the passed if application has a valid title.
     */
    private boolean hasValidTitle(LaunchDesc desc) {
        if (desc == null) {
	    return false;
        }
        InformationDesc iDesc = desc.getInformation();

        if (iDesc == null || nameFilter(iDesc.getTitle()) == null) {

	    Trace.println("Invalid: No title!", TraceLevel.TEMP);

	    return false;
        }
        return true;
    }

    protected boolean createShortcuts(LaunchDesc                 desc,
                                      LocalApplicationProperties lap,
				      boolean []                 which) {

        if (!hasValidTitle(desc)) {
            return false;
        }

        if (isShortcutExists(lap)) {
	    if (!shouldInstallOverExisting(desc)) {
		return false;
	    }
        }

	String cachedJNLP = null;

        File cachedFile = null;
        try {
            cachedFile = DownloadEngine.getCachedFile(desc.getCanonicalHome());
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        }
        
        if (cachedFile != null) {
            cachedJNLP = cachedFile.getPath();
        }
        
        if (cachedJNLP == null) {
	    installFailed(desc);
	    return false;
        }
	ShortcutDesc sd = desc.getInformation().getShortcut();

	boolean doDesktop = (sd == null) ? true : sd.getDesktop();
	boolean doMenu = (sd == null) ? true : sd.getMenu();

	// if which is non-null, this is to re-create shortcuts
	if (which != null) {
	    doDesktop  = doDesktop && which[DESKTOP_INDEX];
	    doMenu = doMenu && which[MENU_INDEX];
	}

	if (doDesktop) {
            if (handleInstall(desc, sd, lap, cachedJNLP, TYPE_DESKTOP) == false) {
                installFailed(desc);
                return false;
	    }
	}

	if (doMenu) {
            if (handleInstall(desc, sd, lap, cachedJNLP, TYPE_START_MENU) == false) {
                removeShortcuts(desc, lap, doDesktop);
                installFailed(desc);
	        return false;
            }
	}
	if (doMenu || doDesktop) {
	    lap.setLocallyInstalled(true);
	    save(lap);
	}
        return true;
    }

    /**
     * Registers a JavaWS application entry from the window manager's add/remove
     * programs applet.
     */
    protected void registerWithInstallPanel(LaunchDesc                 ld, 
                                            LocalApplicationProperties lap) {

        Config          config  = Config.getInstance();
        InformationDesc id      = ld.getInformation();
        String          jnlpURL = config.toExecArg(ld.getCanonicalHome()
                                      .toString());
        String          title   = nameFilter(id.getTitle());

        if ((jnlpURL != null) && (jnlpURL.length() != 0) &&
            (title != null) && (title.length() != 0)) {

            String  iconPath    = getIconPath(ld, false);
            String  vendor      = id.getVendor();
            String  description = getBestFitDescription(id);
            URL     homeURL     = id.getHome();
            String  homepage    = null;
            boolean sysCache    = Environment.isSystemCacheMode();

            if (homeURL != null) {
                homepage = id.getHome().toExternalForm();
            }

            // if the vendor isn't specified, use the homepage
            if (vendor == null) {
                vendor = homepage;
            }

            // If the vendor isstill null then there is no homepage either.
            // So, if there is no vendor, homepage, or description, make
            // sure these entries (even blank) aren't put in the registry.
            if ((vendor == null) && (description.trim().length() == 0)) {
                vendor      = null;
                description = null;
                homepage    = null;
            }

            lap.setRegisteredTitle(title);
            config.addRemoveProgramsAdd(jnlpURL, title, iconPath, vendor,
                                        description, homepage, sysCache);
        }
        // no else required; missing required parameters
    }

    /**
     * Removes a JavaWS application entry from the window manager's add/remove
     * programs applet.
     */
    protected void removeFromInstallPanel(LaunchDesc                 ld, 
                                          LocalApplicationProperties lap) {

        Config config = Config.getInstance();
        boolean sysCache    = Environment.isSystemCacheMode();
        String title = lap.getRegisteredTitle();

        if ((title != null) && (title.length() != 0)) {
            config.addRemoveProgramsRemove(title, sysCache);
        }
        // no else required; missing required parameters
    }

    /**
     * Get a description string that will fit nicely in the "Support Info"
     * dialog box Windows displays when a user selects the "Click here for
     * support information" hyperlink in the Add and Remove Programs control
     * panel.  The preferred choice is the one-line description, then the
     * default, short, and tooltip descriptions.  Once a description is found
     * it is sized to make the dialog box look more aesthetically pleasing.
     * Since the size of the dialog box is directly related to the size of the
     * description string, too small a string looks funny, and too large a
     * string makes the dialog box unusable (the dialog does not wrap this
     * text).
     */
    private String getBestFitDescription(InformationDesc id) {
        String result = null;

        // get the best choice available for a description
        result = id.getDescription(InformationDesc.DESC_ONELINE);
        if (result == null) {
            result = id.getDescription(InformationDesc.DESC_DEFAULT);
            if (result == null) {
                result = id.getDescription(InformationDesc.DESC_SHORT);
                if (result == null) {
                    result = id.getDescription(InformationDesc.DESC_TOOLTIP);
                }
            }
        }

        // if there's a result, get its length to make sure it fits nicely
        // into the dialog; otherwise return the full padding
        if (result != null) {
            int length = result.length();

            if (length < WHITESPACE_PAD.length) {
                // Pad the description with whitespace to make it look nice.
                // (This is tweaked for English, and may turn out to need more
                // tweaking for l10n.)
                result = new StringBuffer(result)
                    .append(WHITESPACE_PAD, 0, (WHITESPACE_PAD.length - length))
                    .toString();
            }
            else if (length > MAX_INSTALL_DESCRIPTION_LENGTH) {
                // Cut the description to fit in a reasonable area (again, this
                // could need tweaking for l10n).
                int eLen = ELLIPSIS.length();

                result = new StringBuffer(result)
                    .delete(MAX_INSTALL_DESCRIPTION_LENGTH - eLen, length)
                    .append(ELLIPSIS)
                    .toString();
            }
        }
        else {
            result = new String(WHITESPACE_PAD);
        }

        return (result);
    }

    /**
     * Invoked when the install fails.
     */
    // err is the error code from native windows call
    private void installFailed(final LaunchDesc desc) {
        Runnable jOptionPaneRunnable = new Runnable() {
	    public void run() {
                UIFactory.showErrorDialog(null,
		    ResourceManager.getString("install.installFailed",
		    getInstallName(desc)),
                    ResourceManager.getString("install.installFailedTitle"));
	    }

	};
        invokeRunnable(jOptionPaneRunnable);
    }

    /**
     * Invoked when the install fails.
     */
    private void uninstallFailed(final LaunchDesc desc) {
        Runnable jOptionPaneRunnable = new Runnable() {
	    public void run() {
		UIFactory.showErrorDialog(null,
		    ResourceManager.getString("install.uninstallFailed",
		    getInstallName(desc)), ResourceManager.getString(
		    "install.uninstallFailedTitle"));
	    }
        };
        invokeRunnable(jOptionPaneRunnable);
    }


    /**
     * Invokes the appropriate native method to handle the installation.
     * Returns true if successful in installing.
     */
    private boolean handleInstall(LaunchDesc                 ld,
                                  ShortcutDesc               sd,
                                  LocalApplicationProperties lap,
                                  String                     cachedJNLP,
                                  int                        type) {

        InformationDesc iDesc        = ld.getInformation();
        String          path         = null;
        String          name         = null;
        String          iconPath     = getIconPath(ld, true);
        String          menuIconPath = getIconPath(ld, false);
        String          startCommand = Config.getInstance().getSystemJavawsPath();
	String		default_desc = iDesc.getDescription(InformationDesc.DESC_DEFAULT);
	String		tooltip_desc = iDesc.getDescription(InformationDesc.DESC_TOOLTIP);
	String		description  = tooltip_desc == null ? default_desc : tooltip_desc;
        boolean         result       = true;

        boolean online       = !iDesc.supportsOfflineOperation() ||
                               (sd == null) ||
                               sd.getOnline();
	String  onlineString = (online) ? "" : "-offline ";
        String  launchArgs   =  "-localfile "+onlineString + "\"" + cachedJNLP + "\"";
        int     ret          = 0;

        if (type == TYPE_DESKTOP) {
	    path = getDesktopPath(ld);
            name = getInstallName(ld);

	    // install Application shortcut on desktop
	    ret = installWrapper(path, name, description,
			          startCommand, launchArgs, null, iconPath);
            if (ret == 0) {
	        lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, path);
	        Trace.println("Installed desktop shortcut for: " + name + ".",
                              TraceLevel.TEMP);
	    } else {
                result = false;
                Trace.println("Installed desktop shortcut for: " + name +
                              " failed (" + ret + ")!!!",
                              TraceLevel.TEMP);
	    }
        } else {
            File startMenuDir = new File(getSubMenuPath(ld));
            if (startMenuDir.exists() || startMenuDir.mkdirs()) {
                path = getStartMenuPath(ld);
                name = getInstallName(ld);

                // install Application shortcut in menu
                ret = installWrapper(path, name, description,
                      startCommand, launchArgs, null, menuIconPath);
                if (ret == 0) {
                    lap.put(INSTALLED_START_MENU_KEY, path);
                    Trace.println("Installed menu shortcut for: " + name + ".",
                                  TraceLevel.TEMP);
                } else {
                    result = false;
                    Trace.println("Installed menu shortcut for: " + name +
                                  " failed (" + ret + ")!!!",
                                  TraceLevel.TEMP);
                }

                // don't do uninstalls to start menu - and only if config says
                String submenu = getSubMenuDir(ld);
                if ((submenu == null || !(submenu.equals("Startup"))) &&
                    addUninstallShortcut()) {
                    launchArgs = "-uninstall " + "\"" + cachedJNLP + "\"";
                    path = getUninstallPath(ld);
                    name = getUninstallMenuName(ld);

                    // install shortcut to uninstall application
                    ret = installWrapper(path, name, description,
                          startCommand, launchArgs, null, menuIconPath);
                    if (ret == 0) {
                        lap.put(UNINSTALLED_START_MENU_KEY, path);
                        Trace.println("Installed menu shortcut for: " + name + ".",
                                      TraceLevel.TEMP);
                    } else {
                        result = false;
                        Trace.println("Installed menu shortcut for: " + name +
                                      " failed (" + ret + ")!!!",
                                      TraceLevel.TEMP);
                    }
                }

                RContentDesc rc[] = iDesc.getRelatedContent();
                StringBuffer rcValue = new StringBuffer(MAX_PATH * rc.length);

                if (rc != null) for (int i=0; i<rc.length; i++) {
                    // A change in RContentDesc for CR 5074087 made sure that
                    // getTitle() never returns null.  The real problem with
                    // returning null was here, since the trim() was called
                    // before testing the result.  However, I think the change
                    // in RContentDesc is better than the attempt that used to
                    // be here, which tried to create a name based on the number
                    // of RContentDesc in the array.
                    //
                    // For CR 6188893, I removed the code that limited the name
                    // to 32 characters.  The shortcut name is properly truncated
                    // if necessary by getRCPath.
                    name = nameFilter(rc[i].getTitle());
                    URL url = rc[i].getHref();
                    if (url.toString().endsWith("jnlp") == false) {
                        description = rc[i].getDescription();
                        URL iconUrl = rc[i].getIcon();
                        String rciconPath = null;
                        if (iconUrl != null) {
                            rciconPath = IcoEncoder.getIconPath(iconUrl, null);
                        }
                        if (rciconPath == null) { rciconPath = iconPath; }
                        path = getRCPath(ld, name);
                        File f = null;
                        try {
                            f = DownloadEngine.getCachedFileNative(url);
                        } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                        }
                        
                        startCommand = (new WinBrowserSupport()).getDefaultHandler(url);

                        if (f != null) {
                            // native content in cache launch with:
                            // C:\PROGRA~1\...\iexplore.exe file:...\content.html
                            launchArgs = "\"" + "file:" + f.getAbsolutePath() + "\"";

                            // shortcut to cached app or native content
                            ret = installWrapper(path, name, description,
                                    startCommand, launchArgs, null, rciconPath);
                            if (ret == 0) {
                                rcValue.append(path);
                                rcValue.append(File.pathSeparator);
                                Trace.println("Installed menu shortcut for: " + name + ".",
                                              TraceLevel.TEMP);
                            } else {
                                result = false;
                                Trace.println("Installed menu shortcut for: " + name +
                                              " failed (" + ret + ")!!!",
                                              TraceLevel.TEMP);
                            }
                        }
                        else {
                            launchArgs = url.toString();

                            // shortcut to normal related content (or jnlp not in cache)
                            ret = installWrapper(path, name, description,
                                                  startCommand, launchArgs, null, rciconPath);
                            if (ret == 0) {
                                rcValue.append(path);
                                rcValue.append(File.pathSeparator);
                                Trace.println("Installed menu shortcut for: " + name + ".",
                                              TraceLevel.TEMP);
                            } else {
                                result = false;
                                Trace.println("Installed menu shortcut for: " + name +
                                              " failed (" + ret + ")!!!",
                                              TraceLevel.TEMP);
                            }
                        }
                    }
                    // no else required; jnlp file - let it integrate itself:
                }
                if (rcValue.length() > 0) {
                    lap.put(RCONTENT_START_MENU_KEY, rcValue.toString());
                } else {
                    lap.put(RCONTENT_START_MENU_KEY, null);
                }
            }
            else {
                // start menu directory doesn't exists, so there is no point
                //trying to create the shortcuts
                result = false;
                Trace.println("Installed menu shortcut for: " + name +
                              " failed (can't create directory \"" +
                              startMenuDir.getAbsolutePath() + "\")!!!",
                              TraceLevel.TEMP);
            }
        }

        return (result);
    }

    /**
     * Returns the name to install under.
     */
    private String getInstallName(LaunchDesc ld) {
        return nameFilter(ld.getInformation().getTitle());
    }

    private String getUninstallMenuName(LaunchDesc ld) {
        String name = ResourceManager.getString(
                "install.startMenuUninstallShortcutName", getInstallName(ld));
        return name;
    }

    /**
     * Returns the path desktop shortcuts are to be placed in.
     */
    private String getDesktopPath(LaunchDesc ld) {
        return (getShortcutPath(getDesktopPath(), getInstallName(ld)));
    }

    /**
     * Returns the path Menu shortcuts are to be placed in.
     */
    private String getStartMenuPath(LaunchDesc ld) {
        return (getShortcutPath(getSubMenuPath(ld), getInstallName(ld)));
    }

    /**
     * Returns the path related content shortcuts are to be placed in.
     */
    private String getRCPath(LaunchDesc ld, String title) {
        return (getShortcutPath(getSubMenuPath(ld), title));
    }

    /**
     * Returns the path shortcut to uninstall is to be placed in.
     */
    private String getUninstallPath(LaunchDesc ld) {
        return (getShortcutPath(getSubMenuPath(ld), getUninstallMenuName(ld)));
    }

    /**
     * Returns the path to the specific submenu for this app
     */
    private String getSubMenuPath(LaunchDesc ld) {
	String path = getStartMenuPath();
	if (path != null) {
	    String dir = getSubMenuDir(ld);
	    if (dir != null) {
	        path += dir + File.separator;
	    }
	}
	return path;
    }

    private String getSubMenuDir(LaunchDesc ld) {
        String dir = getInstallName(ld);
	ShortcutDesc sd = ld.getInformation().getShortcut();
	if (sd != null) {
	    String submenu = sd.getSubmenu();
	    if (submenu != null) {
	        dir = dirFilter(submenu);
	    }
        }
        if (dir != null) {
            if (dir.equalsIgnoreCase("startup")) {
	        dir = "Startup";
            }
	}
	return dir;
    }

    /**
     * Returns the path desktop shortcuts are to be placed in.
     */
    private String getDesktopPath() {
	loadPathsIfNecessary();
	return _desktopPath;
    }

    /**
     * Returns the path start menu shortcuts are to be placed in.
     */
    private String getStartMenuPath() {
	loadPathsIfNecessary();
	return _startMenuPath;
    }

    /**
     * Loads the paths for shortcuts if the paths haven't already been
     * loaded.
     */
    private void loadPathsIfNecessary() {
        int    regKey = WinRegistry.HKEY_CURRENT_USER;
        String prefix  = "";

        if (Environment.isSystemCacheMode()) {
            regKey = WinRegistry.HKEY_LOCAL_MACHINE;
            prefix = "Common ";
        }

	if (!_loadedPaths) {
	    _desktopPath = WinRegistry.getString(regKey,
						 REG_SHORTCUT_PATH,
						 prefix + REG_DESKTOP_PATH_KEY);
	    if (_desktopPath != null && _desktopPath.length() > 0 &&
		_desktopPath.charAt(_desktopPath.length() - 1) != '\\') {
		_desktopPath += '\\';
	    }
            _startMenuPath = WinRegistry.getString(regKey,
						   REG_SHORTCUT_PATH,
                                                   prefix + REG_START_MENU_PATH_KEY);
	    if (_startMenuPath != null && _startMenuPath.length() > 0 &&
		_startMenuPath.charAt(_startMenuPath.length() - 1) != '\\') {
		_startMenuPath += '\\';
	    }
	    _loadedPaths = true;

	    Trace.println("Start path: " + _startMenuPath + " desktop " +
			  _desktopPath, TraceLevel.TEMP);

	}
    }

    private final String nameBadChars =  "\"\\/|:?*<>"; // invalid in filename
    private final String dirBadChars =  "\"|:?*<>"; // dosn't include slashes

    private String nameFilter(String name) {
        return Filter(name, nameBadChars, '-');
    }

    private String dirFilter(String name) {
        String dir = Filter(name, dirBadChars, '-');
        // also change all slashes to corect seperator for platform
        return Filter(dir, "\\/", File.separatorChar);
    }

    private String Filter(String name, String badChars, char replacement) {
	if (name == null) {
	    return null;
	} 
	// special case for invalid filename on windows: "nul"
	if (name.equalsIgnoreCase("nul")) {
	    return ("-nul-");
	}
	// also, name on windows cannot end in whitespace
	name = name.trim();
	if (name.length() == 0) {
	    return null;
	}
        StringBuffer sb = new StringBuffer();
        for (int i=0; i<name.length(); i++) {
            char c = name.charAt(i);
            if (badChars.indexOf(c) >= 0) {
                sb.append(replacement);
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }

	
    /**
     * Deletes the shortcut, returning true if either the file didn't exist,
     * or the file exists and it was deleted.
     */
    private boolean deleteShortcut(String path) {
	File f = new File(path);
        
        if (f.exists()) {
            boolean retValue = f.delete();
            try {
                // fix for 6231221: After updating title or icon in jnlp file 
                // shortcut on the desktop not update but dublicated
                // On some windows machine, when we remove a shortcut and
                // create a new one, sometimes the desktop does not get
                // refresh and the removed shortcut is still displayed.
                // Adding a 1 second delay seems to help windows to refresh
                Thread.currentThread().sleep(1000);
            } catch (InterruptedException ie) {
                Trace.ignoredException(ie);
            }
            return retValue;
	}
	return true;
    }

    /**
     * Gets the icon associated with a JNLP application.  The icon is taken from
     * the given <code>LaunchDesc</code>, or the default JavaWS icon is used.
     *
     * @param ld  the <code>LaunchDesc</code> of the JNLP application.
     *
     * @return the icon associated with a JNLP application.
     */
    private String getIconPath(LaunchDesc ld, boolean forDesktop) {
        String result = getDefaultIconPath();

        if (ld != null) {
            String appIcon = IcoEncoder.getIconPath(ld, forDesktop);
            if (appIcon != null) {
                result = appIcon;
            }
        }

        return (result);
    }

    /**
     * Gets the full pathname for a shortcut based on the location the shortcut
     * will be placed at, and the name used for the shortcut.
     *
     * Note: if the shortcut location, name, extension (.lnk), and trailing null
     *       character combine to be greater than MAX_PATH, then the path will
     *       be truncated to fit.  If the location length alone is greater than
     *       MAX_PATH, then the value returned by this method will be an invalid
     *       path.
     *
     * @param location  the path to the location where the shortcut should be
     *                  created.
     * @param name      the name of the shortcut.
     *
     * @return the full pathname for a shortcut, or <code>null</code> if a valid
     *         pathname couldn't be constructed.
     */
    private String getShortcutPath(String location, String name) {
        String result = null;

        if ((location != null) && (location.length() < MAX_SHORTCUT - 1) &&
            (name != null)) {
            StringBuffer shortcutPath = new StringBuffer(MAX_PATH + 64);

            shortcutPath.append(location).append(name);
            if (shortcutPath.length() > MAX_SHORTCUT) {
                shortcutPath.delete(MAX_SHORTCUT, shortcutPath.length());
            }
            shortcutPath.append(SHORTCUT_EXTENSION);

            result = shortcutPath.toString();
        }

        return (result);
    }

    private int installWrapper(String path, String appName,
                                           String description,
                                           String appPath,
                                           String args,
                                           String directory,
                                           String iconPath) {

        // NOTE: appName is extra data, since it is not used by the JNI function
	Trace.println("installshortcut with args:", TraceLevel.TEMP);
	Trace.println("    path: "+ path, TraceLevel.TEMP);
	Trace.println("    name: "+ appName, TraceLevel.TEMP);
	Trace.println("    desc: "+ description, TraceLevel.TEMP);
	Trace.println("    appP: "+ appPath, TraceLevel.TEMP);
	Trace.println("    args: "+ args, TraceLevel.TEMP);
	Trace.println("    dir : "+ directory, TraceLevel.TEMP);
	Trace.println("    icon: "+ iconPath, TraceLevel.TEMP);
	Trace.flush();

    	return Config.getInstance().installShortcut(path,
		appName, description, appPath, args, directory, iconPath);
    }

    private static final char [] WHITESPACE_PAD = {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    };

    private static final String ELLIPSIS = "...";

    private static final int MAX_INSTALL_DESCRIPTION_LENGTH = 80;
}
