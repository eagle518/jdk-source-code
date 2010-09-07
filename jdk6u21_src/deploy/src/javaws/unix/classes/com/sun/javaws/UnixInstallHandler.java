/*
 * @(#)UnixInstallHandler.java	1.70 10/03/24
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
import com.sun.deploy.association.*;
import com.sun.deploy.association.utility.GnomeVfsWrapper;
import com.sun.deploy.association.Action;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.jnl.*;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.Environment;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.association.utility.DesktopEntryFile;
import com.sun.deploy.association.utility.DesktopEntry;
import com.sun.deploy.config.UnixConfig;

/**
 * Instances of InstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via <code>LocalInstallHandler.getInstance()</code>. 
 *
 * @version 1.8 03/02/00
 */
public class UnixInstallHandler extends LocalInstallHandler {
    
    // Used in LocalApplicationProperties, gives the name the shortcut
    // was created as, which may differ from the real name (if a shortcut
    // already existed with that name).
    private static final String INSTALLED_DESKTOP_SHORTCUT_KEY =
        "installed.desktop";
    private static final String INSTALLED_DESKTOP_SHORTCUT_GNOME26_KEY =
        "installed.desktop.gnome26";
    private static final String INSTALLED_DIRECTORY_KEY =
        "installed.directory";
    private static final String INSTALLED_GNOME_START_MENU_KEY =
	"installed.menu";
    private static final String INSTALLED_UNINSTALL_KEY =
	"installed.uninstalled";
    private static final String INSTALLED_RC_KEY =
	"installed.rc";


	static boolean GNOMELoaded = GnomeVfsWrapper.openGNOMELibrary();
	static boolean GNOMEInitializeded = GnomeVfsWrapper.initGNOMELibrary();

    
    public boolean isShortcutExists(LocalApplicationProperties lap) {
         
	String desktopPath = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY);
	String menuPath = lap.get(INSTALLED_GNOME_START_MENU_KEY);

	boolean desktopExists = false;
	boolean menuExists = false;
	
        if (desktopPath == null) {
	    desktopPath = lap.get(INSTALLED_DESKTOP_SHORTCUT_GNOME26_KEY);
        }

	if (desktopPath != null) {
	    desktopExists = new DesktopEntryFile(desktopPath).exists();
	}

	if (menuPath != null) {
	    menuExists = new DesktopEntryFile(menuPath).exists();
	}

	if (desktopPath != null && menuPath != null) {
	    return desktopExists && menuExists;
	} else {
	    return desktopExists || menuExists;
	}
    }

    public boolean [] whichShortcutsExist(LocalApplicationProperties lap) {

	String desktopPath = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY);
	String menuPath = lap.get(INSTALLED_GNOME_START_MENU_KEY);
        if (desktopPath == null) {
	    desktopPath = lap.get(INSTALLED_DESKTOP_SHORTCUT_GNOME26_KEY);
        }

	boolean [] which = new boolean[2];

	which[DESKTOP_INDEX] = ((desktopPath != null) &&
				(new DesktopEntryFile(desktopPath).exists()));
  
	which[MENU_INDEX] = ((menuPath != null) &&
			     (new DesktopEntryFile(menuPath).exists()));

	return which;
    }


    public String getAssociationPrintCommand(String jnlpLocation) {
	// print assoication not supported on gnome
	return null;
    }

    public String getAssociationOpenCommand(String jnlpLocation) {

	return Config.getJavawsCommand() + " " + jnlpLocation + " -open";
    }
    
    public void registerAssociationInternal(Association assoc) throws 
	    AssociationAlreadyRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Environment.isSystemCacheMode()) {
	    // do system association registration

	    assocService.registerSystemAssociation(assoc);
	} else {
	    // do user association registration

	    assocService.registerUserAssociation(assoc);
	}
    }
    
    public void unregisterAssociationInternal(Association assoc) throws 
	    AssociationNotRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Environment.isSystemCacheMode()) {
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
     * Installs shortcuts for the application.
     */
    protected boolean createShortcuts(LaunchDesc                 desc,
                                      LocalApplicationProperties lap,
				      boolean []                 which) {
        Trace.println("createShortcuts called in UnixInstallHandler", 
            TraceLevel.BASIC);

        File         jnlp           = null;
        ShortcutDesc sd             = desc.getInformation().getShortcut();
        boolean      doDesktop      = (sd == null) ? true : sd.getDesktop();
        boolean      doMenu         = (sd == null) ? true : sd.getMenu();
        boolean      allUsers       = false;
        boolean      installSuccess = true;

        if (Environment.isSystemCacheMode()) {
            // GNOME doesn't have way to create desktop shortcuts for all users
            doDesktop = false;
            allUsers  = true;
        }

	// if removing and recreating shortcuts, which will be non-null, and
        // tell us which actually existed so we can restore only that one.
	if (which != null) {
	    doDesktop = doDesktop && which[DESKTOP_INDEX];
	    doMenu = doMenu && which[MENU_INDEX];
	}

        //Older gnome vfs doesn't support uri(applications-all-users:///)
        //and older gnome vfs doesn't support to create start menu entry
        if(new DesktopEntryFile("applications-all-users:///").exists()
           == false){
            Trace.println("Found old gnome vfs api, don't create start menu", 
                TraceLevel.TEMP);
            doMenu = false;
        }

	if (isShortcutExists(lap)) {
	    if (!shouldInstallOverExisting(desc)) {
		return false;
	    }
	}

	if (doMenu || doDesktop) {
	    try {
		jnlp = DownloadEngine.getCachedFile(desc.getCanonicalHome());
	
	    } catch (IOException ioe) {
		Trace.ignoredException(ioe);
	    }

	    if (jnlp == null) {
		installSuccess = false;
	    }

	    if (doDesktop && installSuccess) {
                String desktopPath = getGnomeDesktopPath();
		String desktopFilePath = null;
		String desktopFilePathGnome26 = null;
                if( new File(desktopPath).exists()){
		    desktopFilePath = createDesktopShortcut(
		        desc, jnlp, "file://" + desktopPath);
		    if(desktopFilePath != null) {
		        lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, desktopFilePath);
		    }
                }

                String desktop26Path = getGnome26DesktopPath();
                if( new File(desktop26Path).exists()){
		    desktopFilePathGnome26 = createDesktopShortcut(
		        desc, jnlp, "file://" + desktop26Path);
		    if(desktopFilePathGnome26 != null) {
		        lap.put(INSTALLED_DESKTOP_SHORTCUT_GNOME26_KEY, 
			    desktopFilePathGnome26);
		    }
                }

	        if(desktopFilePath == null && desktopFilePathGnome26 == null) { 
	          installSuccess = false;
	        }
	    }

	    if (doMenu && installSuccess) {
		String[] startmenuFilePath = createStartMenuShortcut(
		    desc, jnlp, allUsers);

		if (startmenuFilePath[0] != null) {
		    lap.put(INSTALLED_GNOME_START_MENU_KEY, 
			startmenuFilePath[0]);
		    if (startmenuFilePath[1] != null) {
                        lap.put(INSTALLED_DIRECTORY_KEY, startmenuFilePath[1]);
		    }

		    if (startmenuFilePath[2] != null) {
			lap.put(INSTALLED_UNINSTALL_KEY, startmenuFilePath[2]);
		    }

		    if (startmenuFilePath[3] != null) {
			lap.put(INSTALLED_RC_KEY, startmenuFilePath[3]);
		    }
		} else {
		    installSuccess = false;
                    removeShortcuts(desc, lap, doDesktop);
		}
	    }

	    if (installSuccess) {
		lap.setLocallyInstalled(true);
		save(lap);
	    } else {
		installFailed(desc);
	    }
	}
        return installSuccess;
    }

    /**
     * This is just a stub on Unix, until GNOME provides this functionality.
     */
    protected void registerWithInstallPanel(LaunchDesc                 ld, 
                                            LocalApplicationProperties lap) { }

    /**
     * This is just a stub on Unix, until GNOME provides this functionality.
     */
    protected void removeFromInstallPanel(LaunchDesc                 ld, 
                                          LocalApplicationProperties lap) { }

    public String getDefaultIconPath() {
	return Config.getJavaHome() + File.separator + "lib" +
		File.separator + "deploy" + File.separator + "java-icon.ico";
    }

    private String getIcon(LaunchDesc ld, boolean forDesktop) {
	// create the ico file
	String iconPath = IcoEncoder.getIconPath(ld, forDesktop);

	// if there is no icon specified
	// return path to Java icon
	if (iconPath == null) {
	    iconPath = getDefaultIconPath();
	}
	return iconPath;
    }

    private String getRCIcon(RContentDesc rc, LaunchDesc ld) {
	URL iconUrl = rc.getIcon();
	String rciconPath = null;
	if (iconUrl != null) {
	    rciconPath = IcoEncoder.getIconPath(iconUrl, null);
	}
	if (rciconPath == null) {
	    rciconPath = getIcon(ld, false);
	}
	return rciconPath;
    }

    private String[] createStartMenuShortcut(LaunchDesc ld,
                                             File       jnlp,
                                             boolean    allUsers) {
        InformationDesc    iDesc         = ld.getInformation();
        ShortcutDesc       sd            = iDesc.getShortcut();
        String          [] menuPath      = new String[5];
        String             name          = nameFilter(iDesc.getTitle());
        String             iconPath      = getIcon(ld, false);
        String             onlineArg     = "";
        
        String menuEntryDirPath = getMenuEntryDirPath(ld, allUsers);
        
        if ((iDesc.supportsOfflineOperation() == true) &&
            (sd != null) &&
            (sd.getOnline() == false)) {
            // allow offline launch
            onlineArg  = "-offline";
        }

       // create the shortcut for gnome
        menuPath[0] = createDesktopFile(ld,
            name,
            iconPath,
            menuEntryDirPath,
            jnlp.getAbsolutePath(),
            onlineArg);

        menuPath[1] = menuEntryDirPath;
        
        // create uninstall menu shortcut
        if (addUninstallShortcut()) {
            menuPath[2] = createDesktopFile(ld,
                ResourceManager.getString(
		    "install.startMenuUninstallShortcutName", name),
                iconPath,
                menuEntryDirPath,
                jnlp.getAbsolutePath(),
                "-uninstall");
        } 

        Trace.println("directoryFileName: " + menuPath[1], TraceLevel.BASIC);
        Trace.println("desktopFileName: " + menuPath[0], TraceLevel.BASIC);

        // related contents
        RContentDesc [] rc = iDesc.getRelatedContent();

        if (rc != null) {
            StringBuffer rcFilePaths = new StringBuffer(512 * rc.length);
            String rcFile;
            for (int i=0; i<rc.length; i++) {
                URL url = rc[i].getHref();
		if (url != null && url.toString().endsWith(".jnlp")) {
		    continue;
		}

                        
                rcFile = createRCDesktopFile(rc[i], 
                               getRCIcon(rc[i], ld),
			       menuEntryDirPath) ;
                if(rcFile != null){
		    rcFilePaths.append(rcFile);
                    rcFilePaths.append(";");
                }
            }
            menuPath[3] = rcFilePaths.toString();
        }
        
        return menuPath;
    }

    private String getFolderName(LaunchDesc ld) {
	String folderName = null;
	if (ld.getInformation().getShortcut() != null) {
	    folderName = ld.getInformation().getShortcut().getSubmenu();
	}
	if (folderName == null) {
	    folderName = nameFilter(ld.getInformation().getTitle());
	}
	// since this name is put in xml file, angle brackets will mess it up
	folderName = folderName.replace('<', '-');
	folderName = folderName.replace('>', '-');
	return folderName;
    }

    private String createDesktopShortcut(LaunchDesc ld, File jnlp,
					 String gnomeDesktopPath) {
        InformationDesc    iDesc      = ld.getInformation();
        ShortcutDesc       sd         = iDesc.getShortcut();
        String             name       = nameFilter(iDesc.getTitle());
        String             iconPath   = getIcon(ld, true);
        String             onlineArg  = "";

        Trace.println("iconPath: " + iconPath, TraceLevel.TEMP);

        if ((iDesc.supportsOfflineOperation() == true) &&
            (sd != null) &&
            (sd.getOnline() == false)) {
            // allow offline launch
            onlineArg  = "-offline";
        }

	String launchArgs = "-localfile "+onlineArg;

        // create the .desktop file
        return createDesktopFile(ld,
                                 name,
                                 iconPath,
                                 gnomeDesktopPath,
                                 jnlp.getAbsolutePath(),
                                 launchArgs);
    }

    private String getGnomeDesktopPath() {
	return System.getProperty("user.home") + 
				  File.separator + ".gnome-desktop";
    }

    private String getGnome26DesktopPath() {
	return System.getProperty("user.home") + File.separator + "Desktop";
    }

    private String getMenuEntryDirPath(LaunchDesc ld, boolean allUsers) {
        InformationDesc iDesc = ld.getInformation();
        ShortcutDesc sd = iDesc.getShortcut();
	String submenu = null;
	if (sd != null) {
	  submenu = dirFilter(sd.getSubmenu());
        }
	//submenu has implied attribute, might be null.
	if( submenu == null ){
	  submenu = dirFilter(iDesc.getTitle());
	}

        if( allUsers){
            if( submenu.startsWith("applications://")){
				String tmpStr = "applications://";
                submenu = "applications-all-users://" +
							submenu.substring(
								submenu.indexOf(tmpStr), tmpStr.length());

            }else{
                submenu = "applications-all-users://" + File.separator + submenu;
            }
        }else{
            if( submenu.startsWith("applications://")){
                // No operations
            }else{
                submenu = "applications://" + File.separator + submenu;
            }
        }
        return submenu;
    }
    

    private String getRCCommand(URL url) {
        File f = null;
	try {

	    f = DownloadEngine.getCachedFileNative(url);
	} catch (IOException ioe) {
	    Trace.ignoredException(ioe);
	}
	
	
	String filePath;
	String browserPath = "";
	if (url.toString().endsWith(".jnlp")) {
	    return Config.getJavawsCommand() + " " + url.toString();
	} else if (f != null) {
	    filePath = f.getAbsolutePath();
	    // native content, try to find out default handler for this file ext
	    String ext = filePath.substring(
		filePath.lastIndexOf("."), filePath.length());

	    // try to find out default handler for this file ext
	    // except for html file, use browser (would otherwise use gedit)
	    if (isAssociationSupported() && !ext.equals(".html")) {
		AssociationService assocService = new AssociationService();
		Association assoc = 
		    assocService.getFileExtensionAssociation(ext);
		if (assoc != null) {
		    Action act = assoc.getActionByVerb("open");
		    if (act != null) {
			String command = act.getCommand();
			StringTokenizer st = new StringTokenizer(command);
			if (st.hasMoreTokens()) {
			    command = st.nextToken();
			}


			browserPath = command;
		    }
		}
	    }
	    // default to browser path if nothing found
	    if (browserPath == "") {
		browserPath = Config.getProperty(Config.BROWSER_PATH_KEY);
	    }

	} else {
	    // web content
	    filePath = url.toString();
	    browserPath = Config.getProperty(Config.BROWSER_PATH_KEY);
	}


	return browserPath + " " + filePath;
    }

    private String createRCDesktopFile(RContentDesc rc, 
				       String iconPath, String menuEntryDirPath) {
	URL url = rc.getHref();

	String appTitle = nameFilter(rc.getTitle());
        
        DesktopEntry desktopEntry = new DesktopEntry();
        desktopEntry.setType("Application");
        desktopEntry.setExec(getRCCommand(url));
        desktopEntry.setIcon(iconPath);
        desktopEntry.setTerminal(false);
        desktopEntry.setName(appTitle);
        desktopEntry.setComment(rc.getDescription());
        desktopEntry.setCategories("Applications;" + appTitle);

	String desktopFilePath = menuEntryDirPath + 
	    File.separator + uniqDesktopFileName(appTitle);

	try {
            new DesktopEntryFile(desktopFilePath).writeEntry(desktopEntry);
	} catch(IOException ioe) {
	    Trace.ignoredException(ioe);
	    // pop up error dialog?
	    return null;
	}
	return desktopFilePath;
    }

    /*
     * see http://www.freedesktop.org/
     *     standards/desktop-entry-spec/desktop-entry-spec.html
     * for the .desktop file specification
     */
    private String createDesktopFile(LaunchDesc ld,
                                     String     appTitle,
                                     String     iconPath,
                                     String     desktopPath,
                                     String     jnlpLocation,
                                     String     args) {
        InformationDesc id  = ld.getInformation();
        String categories   = getFolderName(ld);
        String default_desc = id.getDescription(InformationDesc.DESC_DEFAULT);
        String tooltip_desc = id.getDescription(InformationDesc.DESC_TOOLTIP);
        String description  = tooltip_desc == null ? 
			      default_desc : tooltip_desc; 

        // normalize the args
        if (args == null) {
            args = "";
        } else if ((args.length() > 0) &&
                 (args.endsWith(" ") == false)) {
            args = args + " ";
        }

        // create path for destop file
        StringBuffer desktopFilePath = new StringBuffer(512);

        desktopFilePath.append(desktopPath)
                       .append(File.separator)
                       .append(uniqDesktopFileName(appTitle));

        Trace.println("desktopFilePath: " + desktopFilePath, TraceLevel.BASIC);

        DesktopEntry desktopEntry = new DesktopEntry();
        desktopEntry.setEncoding("UTF-8");
        desktopEntry.setType("Application");
        desktopEntry.setExec(Config.getJavawsCommand() +
                             " " + args + jnlpLocation);
        desktopEntry.setIcon(iconPath);
        desktopEntry.setTerminal(false);
        desktopEntry.setName(appTitle);
        desktopEntry.setComment(description);
        desktopEntry.setCategories("Applications;" + categories);

        try {
            Trace.println("fileContents: " + desktopEntry, TraceLevel.TEMP);
            
            new DesktopEntryFile(desktopFilePath.toString()).writeEntry(desktopEntry);
        } catch(IOException ioe) {
            Trace.ignoredException(ioe);
            // pop up error dialog?
            return null;
        }

        return desktopFilePath.toString();
    }
    
    /**
     * Generate a .desktop file with an uniq name.
     * File name can't contain blank characters.
     */
    private String uniqDesktopFileName(String name){
        //Name might contain any characters, so don't use it in a file name.
        return ("jws_app_shortcut_" + System.currentTimeMillis()  
                + ".desktop");
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
		    nameFilter(desc.getInformation().getTitle())),
                    ResourceManager.getString("install.installFailedTitle"));
            }

        };
        invokeRunnable(jOptionPaneRunnable);
    }

    public boolean removePathShortcut(String path) {
        DesktopEntryFile def =  new DesktopEntryFile(path);
        if (def.exists()) {
            return def.delete();
        }
        return false;
    }

    /**
     * Uninstalls previously installed application shortcuts.
     */
    protected boolean removeShortcuts(LaunchDesc                 desc,
                                      LocalApplicationProperties lap,
                                      boolean                    desktop) {
        boolean allUsers = false;

        if (Environment.isSystemCacheMode()) {
            // GNOME doesn't have way to create desktop shortcuts for all users
            desktop  = false;
            allUsers = true;
        }

        Trace.println("uninstall called in UnixInstallHandler", 
	    TraceLevel.BASIC);

        if (desktop) {
            String desktopFilePath = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY);
            String desktopFilePathGnome26 = 
		lap.get(INSTALLED_DESKTOP_SHORTCUT_GNOME26_KEY);
            if (desktopFilePath != null) {
                new DesktopEntryFile(desktopFilePath).delete();
                Trace.println("file removed: " +
                    desktopFilePath, TraceLevel.BASIC);
                lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, null);
            }
            if (desktopFilePathGnome26 != null) {
                new DesktopEntryFile(desktopFilePathGnome26).delete();
                Trace.println("file removed: " +
                    desktopFilePathGnome26, TraceLevel.BASIC);
                lap.put(INSTALLED_DESKTOP_SHORTCUT_GNOME26_KEY, null);
            }
        }

        String startmenuFilePath = lap.get(INSTALLED_GNOME_START_MENU_KEY);
        if (startmenuFilePath != null) {
            // determine if this is
            new DesktopEntryFile(startmenuFilePath).delete();
            Trace.println("file removed: " + startmenuFilePath, 
		TraceLevel.BASIC);
            lap.put(INSTALLED_GNOME_START_MENU_KEY, null);
        }

        startmenuFilePath = lap.get(INSTALLED_UNINSTALL_KEY);
        if (startmenuFilePath != null) {
            new DesktopEntryFile(startmenuFilePath).delete();
            Trace.println("file removed: " + startmenuFilePath, 
		TraceLevel.BASIC);
            lap.put(INSTALLED_UNINSTALL_KEY, null);
        }

        startmenuFilePath = lap.get(INSTALLED_RC_KEY);
        if (startmenuFilePath != null) {
            StringTokenizer st = new StringTokenizer(
		startmenuFilePath, ";");
            String filePath;
            while (st.hasMoreElements()) {
                filePath = st.nextToken();
                if( filePath == null || filePath.trim().length() == 0 ){
                    continue;
                }
                new DesktopEntryFile(filePath).delete();
                Trace.println("file removed: " + filePath, TraceLevel.BASIC);
            }
            lap.put(INSTALLED_RC_KEY, null);
        }
        
        String directoryFilePath = lap.get(INSTALLED_DIRECTORY_KEY);
        if(directoryFilePath != null){
            new DesktopEntryFile(directoryFilePath).deleteToNonEmptyParent();
            Trace.println("directory removed: " + directoryFilePath, TraceLevel.BASIC);
            lap.put(INSTALLED_DIRECTORY_KEY, null);
        }
       

        lap.setLocallyInstalled(false);
        save(lap);
        return true;
    }

    /**
     *  Determine if this platform supports Local Application Installation.
     *  We will want to return true for the window managers that we support
     *  in install and uninstall above.
     */
    public boolean isLocalInstallSupported() {
        return GNOMELoaded && GNOMEInitializeded &&
           Config.getInstance().isLocalInstallSupported();
    }
    
    boolean isAssociationFileExtSupported(String extension) {
        // support all file extension association on UNIX
        return true;
    }

    // only support association if libgnomevfs-2.so can be loaded and
    // all the required functions exist
    public boolean isAssociationSupported() {
	String version = GnomeVfsWrapper.getVersion();
        // Don't support association after Gnome 2.8 util Gnome provide
        // a new API to create it.
	boolean isValidVersion = version != null 
                && compareVersion(version, "2.8") < 0;
	boolean result = GNOMELoaded && GNOMEInitializeded
                && isValidVersion; 
    	return result;
    }

    /**
     * Compare two version strings(only support dot and number).
     * The result is zero if they were equal, positive integer if the first 
     * one's higher and negative integer if the second one's higher.
     */
    private int compareVersion(String s1, String s2){
	StringTokenizer st1 = new StringTokenizer(s1, ".");
	StringTokenizer st2 = new StringTokenizer(s2, ".");
	int result = 0;
	while(st1.hasMoreTokens() && result == 0){
            if(st2.hasMoreTokens()){
		result = Integer.parseInt(st1.nextToken())
                        - Integer.parseInt(st2.nextToken());
	    }else {
		result = Integer.parseInt(st1.nextToken());
	    }
	}
	while(result == 0 && st2.hasMoreTokens()){
	    result = 0 - Integer.parseInt(st2.nextToken());
	}
	return result;
    }

  
    private final String nameBadChars =  "\"\\/|:?*<>#"; // invalid in filename
    private final String dirBadChars =  "\"|:?*<>#"; // don't include slashes

    private String nameFilter(String name) {
        return Filter(name, nameBadChars, '-');
    }

    private String dirFilter(String name) {
        String dir = Filter(name, dirBadChars, '-');
        // also change all slashes to the correct separator for the platform
        return Filter(dir, "/\\", File.separatorChar);
    }

    private String Filter(String name, String badChars, char replacement) {
	if (name == null) {
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

}

