/*
 * @(#)WinInstallHandler.java	1.67 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.net.*;
import java.util.StringTokenizer;
import javax.swing.*;

import com.sun.javaws.cache.Cache;
import com.sun.javaws.jnl.*;

import com.sun.deploy.util.WinRegistry;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.association.*;
/**
 * Instances of InstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via the class method <code>getInstallHandler</code>. This method
 * will look up the class from the property
 * <code>javaws.installer.className</code>, and create it if non-null. A
 * null return value from <code>getInstallHandler</code> indicates the
 * current platform does not support any install options.
 *
 * @version 1.8 03/02/00
 */
public class WinInstallHandler extends LocalInstallHandler {

    // Used in LocalApplicationProperties, gives the name the shortcut
    // was created as, which may differ from the real name (if a shortcut
    // already existed with that name).
    private static final String INSTALLED_DESKTOP_SHORTCUT_KEY =
        "windows.installedDesktopShortcut";
    private static final String INSTALLED_START_MENU_KEY =
        "windows.installedStartMenuShortcut";
    private static final String UNINSTALLED_START_MENU_KEY =
        "windows.uninstalledStartMenuShortcut";

    private static final String RCONTENT_START_MENU_KEY =
        "windows.RContent.shortcuts";


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

    /**
     * Extension of shortcut path files.
     */
    private static final String SHORTCUT_EXTENSION = ".lnk";

    private static final int MAX_PATH = 200;

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
        NativeLibrary.getInstance().load();
	String os = System.getProperty("os.name");
	if (os.indexOf("2000") != -1 || os.indexOf("XP") != -1) {
	    // for win2k or winxp, default to register user association only
	    useSystem = false;
	} else {
	    // for other windows,
	    useSystem = true;
	}
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
	if (Globals.isSystemCache() || useSystem) {
	    // do system association registration

	    assocService.registerSystemAssociation(assoc);
	} else {
	    // do user association registration

	    assocService.registerUserAssociation(assoc);
	}
    }
    public void unregisterAssociationInternal(Association assoc) throws AssociationNotRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Globals.isSystemCache() || useSystem) {
	    // do system association unregistration

	    assocService.unregisterSystemAssociation(assoc);
	} else {
	    // do user association unregistration

	    assocService.unregisterUserAssociation(assoc);
	}
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

    public void associationCompleted() {
	// do nothing
    }

    public void uninstall(LaunchDesc desc, LocalApplicationProperties lap,
			   boolean desktop) {
        if (lap == null) {

	    Trace.println("No LAP for uninstall, bailing!", TraceLevel.TEMP);

	    return;
        }

        String path;
	String toEmpty = null;
        boolean failed = false;

        if ((path = lap.get(INSTALLED_START_MENU_KEY)) != null) {
            if (!uninstallShortcut(path)) {
                failed = true;
            } else {
                lap.put(INSTALLED_START_MENU_KEY, null);
            }
	    toEmpty = path;
        }

        if ((path = lap.get(UNINSTALLED_START_MENU_KEY)) != null) {
            if (!uninstallShortcut(path)) {
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
		    if (!uninstallShortcut(path)) {
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
                if (!uninstallShortcut(path)) {
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
    }

    private void checkEmpty(String path) {
	try {
	    File parent = (new File(path)).getParentFile();
	    if (parent != null && parent.isDirectory() &&
		((parent.list()).length == 0)) {
		parent.delete();
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

        if (iDesc == null || iDesc.getTitle().trim() == null) {

	    Trace.println("Invalid: No title!", TraceLevel.TEMP);

	    return false;
        }
        return true;
    }

    public void install(LaunchDesc desc, LocalApplicationProperties lap) {

        if (!hasValidTitle(desc)) {
            return;
        }

        if (isApplicationInstalled(desc)) {
	    if (!shouldInstallOverExisting(desc)) {
		return;
	    }
        }

	String cachedJNLP = null;

	try {
	    cachedJNLP = Cache.getCachedLaunchedFile(
		desc.getCanonicalHome()).getAbsolutePath();
	} catch (IOException ioe) {
	    Trace.ignoredException(ioe);
	}

        if (cachedJNLP == null) {
	    installFailed(desc);
	    return;
        }
	ShortcutDesc sd = desc.getInformation().getShortcut();

	boolean doDesktop = (sd == null) ? true : sd.getDesktop();
	if (doDesktop) {
            if (handleInstall(desc, lap, cachedJNLP, TYPE_DESKTOP) == false) {
                installFailed(desc);
                return;
	    }
	}

	boolean doMenu = (sd == null) ? true : sd.getMenu();
	if (doMenu) {
            if (handleInstall(desc, lap, cachedJNLP, TYPE_START_MENU) == false) {
                uninstall(desc, lap, doDesktop);
                installFailed(desc);
	        return;
            }
	}
	if (doMenu || doDesktop) {
	    lap.setLocallyInstalled(true);
	    save(lap);
	}
    }

    /**
     * Invoked when the install fails.
     */
    // err is the error code from native windows call
    private void installFailed(final LaunchDesc desc) {
        Runnable jOptionPaneRunnable = new Runnable() {
	    public void run() {
                DialogFactory.showErrorDialog(ResourceManager.getString(
                    "install.installFailed", getInstallName(desc)),
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
		DialogFactory.showErrorDialog(ResourceManager.getString(
		    "install.uninstallFailed", getInstallName(desc)),
		    ResourceManager.getString("install.uninstallFailedTitle"));
	    }
        };
        invokeRunnable(jOptionPaneRunnable);
    }


    /**
     * Invokes the appropriate native method to handle the installation.
     * Returns true if successful in installing.
     */
    private boolean handleInstall(LaunchDesc ld, LocalApplicationProperties lap,
				  String arg, int type) {

        InformationDesc iDesc        = ld.getInformation();
        ShortcutDesc    sd           = iDesc.getShortcut();
        String          path         = null;
        String          name         = null;
        String          iconPath     = IcoEncoder.getIconPath(ld);
        String          startCommand = Config.getInstance().getSystemJavawsPath();
        String          description  = iDesc.getDescription(InformationDesc.DESC_SHORT);
        boolean         result       = true;


	// if there is no icon specified
	// return path to javaws.exe in system directory
	if (iconPath == null) {
            iconPath = getDefaultIconPath();
	}

        boolean online       = !iDesc.supportsOfflineOperation() ||
                               (sd == null) ||
                               sd.getOnline();
	String  onlineString = (online) ? "" : "-offline ";
        String  launchArgs   = onlineString + "\"" + arg + "\"";
        int     ret          = 0;

        if (type == TYPE_DESKTOP) {
	    path = getDesktopPath(ld);
	    name = getDesktopName(ld);

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
                name = getStartMenuName(ld);

                // install Application shortcut in menu
                ret = installWrapper(path, name, description,
                                     startCommand, launchArgs, null, iconPath);
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
                    launchArgs = "-uninstall " + "\"" + arg + "\"";
                    path = getUninstallPath(ld);
                    name = ResourceManager.getString(
                        "install.startMenuUninstallShortcutName", name);

                    // install shortcut to uninstall application
                    ret = installWrapper(path, name, description,
                                          startCommand, launchArgs, null, iconPath);
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
                    name = rc[i].getTitle().trim();
                    if (name == null || name.length() == 0) {
                        name = getStartMenuName(ld) + " #" + i;
                    }
                    name = getName(name);
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
                        File f = Cache.getCachedFile(url);

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
     * This returns true if there is currently an application registered
     * under the name <code>name</code>.
     */
    private boolean isApplicationInstalled(LaunchDesc ld) {
        boolean desktopInstalled   = false;
        boolean startMenuInstalled = false;
        String  path               = null;

        path             = getDesktopPath(ld);
Trace.println("getDesktopPath(" + path + ").exists() = " + ((path == null) ? "N/A" : "" + new File(path).exists()), TraceLevel.TEMP);
        desktopInstalled = ((path == null) ? true : new File(path).exists());

        path               = getStartMenuPath(ld);
Trace.println("startMenuInstalled(" + path + ").exists() = " + ((path == null) ? "N/A" : "" + new File(path).exists()), TraceLevel.TEMP);
        startMenuInstalled = ((path == null) ? true : new File(path).exists());

        // return true if both paths are null, or if for each non-null path
        // the shortcut exists
        return (desktopInstalled && startMenuInstalled);
    }

    /**
     * Returns the name to install under.
     */
    private String getInstallName(LaunchDesc desc) {
	String name = desc.getInformation().getTitle().trim();
	return getName(name);
    }

    private String getName(String name) {
	if (name.length() > 32) {
	    name = name.substring(0,32);
	}
	return name;
    }

    private String getDesktopName(LaunchDesc ld) {
	return ResourceManager.getString("install.desktopShortcutName",
				       getInstallName(ld));
    }

    private String getStartMenuName(LaunchDesc ld) {
	String name = ResourceManager.getString(
		"install.startMenuShortcutName", getInstallName(ld));
	return name;
    }

    /**
     * Returns the path desktop shortcuts are to be placed in.
     */
    private String getDesktopPath(LaunchDesc ld) {
	String path = getDesktopPath();
	if (path != null) {
	    String name = getDesktopName(ld);
	    if (name != null) {
		path = path + name;
	    }
	    if (path.length() > (MAX_PATH - 8)) {
		// truncate MAX_PATH
		path = path.substring(0, (MAX_PATH - 8));
	    }
	    path = path + SHORTCUT_EXTENSION;
	}
	return path;
    }

    /**
     * Returns the path Menu shortcuts are to be placed in.
     */
    private String getStartMenuPath(LaunchDesc ld) {
	String path = getSubMenuPath(ld);
	if (path != null) {
	    String name = getStartMenuName(ld);
	    if (name != null) {
		path = path + name;
	    }
	    if (path.length() > (MAX_PATH - 8)) {
		// truncate MAX_PATH
		path = path.substring(0, (MAX_PATH - 8));
	    }
	    path = path + SHORTCUT_EXTENSION;
	}
	return path;
    }

    /**
     * Returns the path related content shortcuts are to be placed in.
     */
    private String getRCPath(LaunchDesc ld, String title) {
	String path = getSubMenuPath(ld);
	if (path != null) {
	    path += title;
	    if (path.length() > (MAX_PATH - 8)) {
		// truncate MAX_PATH
		path = path.substring(0, (MAX_PATH - 8));
	    }
	    path = path + SHORTCUT_EXTENSION;
	}
	return path;
    }

    /**
     * Returns the path shortcut to uninstall is to be placed in.
     */
    private String getUninstallPath(LaunchDesc ld) {
	String path = getSubMenuPath(ld);
	if (path != null) {
	    String name = "uninstall  " + getStartMenuName(ld);
	    path = path + name;
	    if (path.length() > (MAX_PATH - 8)) {
		// truncate MAX_PATH
		path = path.substring(0, (MAX_PATH - 8));
	    }
	    path = path + SHORTCUT_EXTENSION;
	}
	return path;
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
	String dir = getStartMenuName(ld);
	ShortcutDesc sd = ld.getInformation().getShortcut();
	if (sd != null) {
	    String submenu = sd.getSubmenu();
	    if (submenu != null) {
	        dir = submenu;
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

        if (Globals.isSystemCache()) {
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

    /**
     * Uninstalls the shortcut, returning true if either the file didn't exist,
     * or the file exists and it was deleted.
     */
    private boolean uninstallShortcut(String path) {
	File f = new File(path);

	if (f.exists()) {
	    return f.delete();
	}
	return true;
    }

    private int installWrapper(String path, String appName,
                                           String description,
                                           String appPath,
                                           String args,
                                           String directory,
                                           String iconPath) {

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
}
