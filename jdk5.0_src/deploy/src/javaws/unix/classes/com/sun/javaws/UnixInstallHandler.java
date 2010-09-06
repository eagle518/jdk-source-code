/*
 * @(#)UnixInstallHandler.java	1.18 03/12/19
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
import com.sun.deploy.association.*;
import com.sun.deploy.association.utility.GnomeVfsWrapper;
import com.sun.deploy.association.Action;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.javaws.cache.*;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.DesktopIntegration;


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
public class UnixInstallHandler extends LocalInstallHandler {

    // Used in LocalApplicationProperties, gives the name the shortcut
    // was created as, which may differ from the real name (if a shortcut
    // already existed with that name).
    private static final String INSTALLED_DESKTOP_SHORTCUT_KEY =
        "unix.installedDesktopShortcut";
    private static final String INSTALLED_DIRECTORY_KEY =
        "unix.installedDirectoryFile";
    private static final String INSTALLED_GNOME_START_MENU_KEY =
	"unix.gnome.installedStartMenuShortcut";
    private static final String INSTALLED_UNINSTALL_KEY =
	"unix.gnome.installedUninstallShortcut";
    private static final String INSTALLED_RC_KEY =
	"unix.gnome.installedRCShortcut";

    private boolean replace = false;

    public String getAssociationPrintCommand(String jnlpLocation) {
	// print assoication not supported on gnome
	return null;
    }

    public String getAssociationOpenCommand(String jnlpLocation) {

	return Config.getJavawsCommand() + " " + jnlpLocation + " -open";
    }
    public void registerAssociationInternal(Association assoc) throws AssociationAlreadyRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Globals.isSystemCache()) {
	    // do system association registration

	    assocService.registerSystemAssociation(assoc);
	} else {
	    // do user association registration

	    assocService.registerUserAssociation(assoc);
	}
    }
    public void unregisterAssociationInternal(Association assoc) throws AssociationNotRegisteredException, RegisterFailedException {
	AssociationService assocService = new AssociationService();
	if (Globals.isSystemCache()) {
	    // do system association unregistration

	    assocService.unregisterSystemAssociation(assoc);
	} else {
	    // do user association unregistration

	    assocService.unregisterUserAssociation(assoc);
	}
    }
    /**
     * Handles the install of the passed in application.
     */
    public void install(LaunchDesc desc, LocalApplicationProperties lap) {
        Trace.println("install called in UnixInstallHandler", TraceLevel.BASIC);

        File         jnlp           = null;
        ShortcutDesc sd             = desc.getInformation().getShortcut();
        boolean      doDesktop      = (sd == null) ? true : sd.getDesktop();
        boolean      doMenu         = (sd == null) ? true : sd.getMenu();
        boolean      allUsers       = false;
        boolean      installSuccess = true;

        if (Globals.isSystemCache()) {
            // GNOME doesn't have a way to create shortcuts for all users
            doDesktop = false;
	    doMenu = false;
            allUsers  = true;
        }
	try {

	    if (doMenu || doDesktop) {
		try {
		    jnlp = Cache.getCachedLaunchedFile(desc.getCanonicalHome());
		} catch (IOException ioe) {
		    Trace.ignoredException(ioe);
		}
		
		if (jnlp == null) {
		    installSuccess = false;
		}
		
		if (doDesktop && installSuccess) {
		    String desktopFilePath = createDesktopShortcut(desc, jnlp);
		    
		    if (desktopFilePath != null) {
			lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, desktopFilePath);
		    }
		    else {
			installSuccess = false;
		    }
		}
		
		if (doMenu && installSuccess) {
		    String[] startmenuFilePath = createStartMenuShortcut(desc, jnlp, allUsers);
		    
		    if (startmenuFilePath[0] != null) {
			lap.put(INSTALLED_GNOME_START_MENU_KEY, startmenuFilePath[0]);
			
			if (startmenuFilePath[1] != null) {
                        lap.put(INSTALLED_DIRECTORY_KEY, startmenuFilePath[1]);
			}
			
			if (startmenuFilePath[2] != null) {
			    lap.put(INSTALLED_UNINSTALL_KEY, startmenuFilePath[2]);
			}
			
			if (startmenuFilePath[3] != null) {
			    lap.put(INSTALLED_RC_KEY, startmenuFilePath[3]);
			}
		    }
		    else {
			installSuccess = false;
			uninstall(desc, lap, doDesktop);
		    }
		}
		
		if (installSuccess) {
		    lap.setLocallyInstalled(true);
		    save(lap);
		}
		else {
		    installFailed(desc);
		}
	    }
	} finally {
	    replace = false;
	}
    }

    public String getDefaultIconPath() {
	return Config.getJavaHome() + File.separator + "lib" +
		File.separator + "javaws" + File.separator + "Java1.5.ico";
    }

    private String getIcon(LaunchDesc ld) {
	// create the ico file
	String iconPath = IcoEncoder.getIconPath(ld);

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
	    rciconPath = getIcon(ld);
	}
	return rciconPath;
    }

    private String[] createStartMenuShortcut(LaunchDesc ld,
                                             File       jnlp,
                                             boolean    allUsers) {
        InformationDesc    iDesc         = ld.getInformation();
        ShortcutDesc       sd            = iDesc.getShortcut();
        String          [] menuPath      = new String[5];
        String             name          = iDesc.getTitle();
        String             iconPath      = getIcon(ld);
        String             folderName    = null;
        String             onlineArg     = "";
        String             deskEntryPath = getDesktopEntryPath(allUsers);
        String             dirEntryPath  = getDirectoryEntryPath(allUsers);

// all user support is currently buggy
if (allUsers == false) {

        if ((iDesc.supportsOfflineOperation() == true) &&
            (sd != null) &&
            (sd.getOnline() == false)) {
            // allow offline launch
            onlineArg  = "-offline";
        }

        Trace.println("iconPath: " + iconPath, TraceLevel.BASIC);

        // create the .desktop file
        // for gnome2
        menuPath[0] = createDesktopFile(ld,
            name,
            iconPath,
            deskEntryPath,
            jnlp.getAbsolutePath(),
            onlineArg);

        // directory file
        menuPath[1] = createDirectoryFile(ld, iconPath, dirEntryPath);

        // uninstall menu shortcut
        if (addUninstallShortcut()) {
            menuPath[2] = createDesktopFile(ld,
                ResourceManager.getString("install.startMenuUninstallShortcutName",
                    name),
                iconPath,
                deskEntryPath,
                jnlp.getAbsolutePath(),
                "-uninstall");
        } else {
            menuPath[2] = null;
        }

	folderName = getFolderName(ld);

        Trace.println("folderName: " + folderName, TraceLevel.BASIC);
        Trace.println("directoryFileName: " + menuPath[1], TraceLevel.BASIC);
        Trace.println("desktopFileName: " + menuPath[0], TraceLevel.BASIC);

        VFolderEditor.updateVFolderInfo(folderName, menuPath[1], menuPath[0], allUsers, false);

        // related contents
        RContentDesc [] rc         = iDesc.getRelatedContent();
        String          categories = getFolderName(ld);

        if (rc != null) {
            StringBuffer rcValue = new StringBuffer(512 * rc.length);

            for (int i=0; i<rc.length; i++) {
                URL url = rc[i].getHref();	
		if (url != null && url.toString().endsWith(".jnlp")) {
		    continue;
		}
         
		String rcpath = createRCDesktopFile(rc[i],
                               categories,
                               getRCIcon(rc[i], ld),
						    deskEntryPath);
		if (rcpath != null) {
		    rcValue.append(rcpath);
		    VFolderEditor.updateVFolderInfo(folderName, menuPath[1], rcpath, allUsers, false);
		    rcValue.append(File.pathSeparator);
		}
            }

            menuPath[3] = rcValue.toString();
        } else {
            menuPath[3] = null;
        }

        // for uninstall menu shortcut
        if (menuPath[2] != null) {
            Trace.println("UNINSTALL desktopFileName: " + menuPath[2], TraceLevel.BASIC);
            VFolderEditor.updateVFolderInfo(folderName, menuPath[1], menuPath[2], allUsers, false);
        }
}
// no support for all users
        return menuPath;
    }

    private String getFolderName(LaunchDesc ld) {
	String folderName = null;
	if (ld.getInformation().getShortcut() != null) {
	    folderName = ld.getInformation().getShortcut().getSubmenu();
	}
	if (folderName == null) {
	    folderName = ld.getInformation().getTitle();
	}
	return folderName;
    }

    private String createDesktopShortcut(LaunchDesc ld, File jnlp) {
        InformationDesc    iDesc      = ld.getInformation();
        ShortcutDesc       sd         = iDesc.getShortcut();
        String             name       = iDesc.getTitle();
        String             iconPath   = getIcon(ld);
        String             onlineArg  = "";

        Trace.println("iconPath: " + iconPath, TraceLevel.TEMP);

        if ((iDesc.supportsOfflineOperation() == true) &&
            (sd != null) &&
            (sd.getOnline() == false)) {
            // allow offline launch
            onlineArg  = "-offline";
        }

        // create the .desktop file
        return createDesktopFile(ld,
                                 name,
                                 iconPath,
                                 getGnomeDesktopPath(),
                                 jnlp.getAbsolutePath(),
                                 onlineArg);
    }

    private String getGnomeDesktopPath() {
	return System.getProperty("user.home") + File.separator + ".gnome-desktop";
    }

    private String getDesktopEntryPath(boolean allUsers) {
        if (allUsers) {
            return File.separator + "usr" + File.separator + "share" + File.separator + "applications";
        }
        else {
            return System.getProperty("user.home") + File.separator + ".gnome2" + File.separator + "vfolders" + File.separator + "applications";
        }
    }

    private String getDirectoryEntryPath(boolean allUsers) {
        if (allUsers) {
            return File.separator + "usr" + File.separator + "share" + File.separator + "gnome" + File.separator + "vfolders";
        }
        else {
            return System.getProperty("user.home") + File.separator + ".gnome2" + File.separator + "vfolders" + File.separator + "applications";
        }
    }

    private String getRCCommand(URL url) {

	File f = Cache.getCachedFile(url);
	String filePath;
	String browserPath = "";
	if (url.toString().endsWith(".jnlp")) {	  
	    return Config.getJavawsCommand() + " " + url.toString();	    
	} else if (f != null) {
	    filePath = f.getAbsolutePath();
	    // native content, try to find out default handler for this file ext
	    String ext = filePath.substring(filePath.lastIndexOf("."), filePath.length());

	    // try to find out default handler for this file ext
	    if (isAssociationSupported()) {
		AssociationService assocService = new AssociationService();
		Association assoc = assocService.getFileExtensionAssociation(ext);
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
		associationCompleted();
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

    private String createRCDesktopFile(RContentDesc rc, String categories, String iconPath, String desktopPath) {
	URL url = rc.getHref();

	String appTitle = rc.getTitle();

	String fileContents = "[Desktop Entry]\n" +
	    "Encoding=UTF-8\n" +
	    "Version=1.0\n" +
	    "Type=Application\n" +
	    "Exec=" + getRCCommand(url) + "\n" +
	    "Icon=" + iconPath + "\n" +
	    "Terminal=false\n" +
	    "Name=" + appTitle + "\n" +
	    "Comment=" + rc.getDescription() + "\n" +
	    "Categories=Application;" + categories + "\n";

	// create directory if not exist
	new File(desktopPath).mkdirs();

	String desktopFilePath = desktopPath + File.separator + appTitle + ".desktop";


	File desktopFile = new File(desktopFilePath);


	try {

	    PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(desktopFile)));
	    out.print(fileContents);
	    out.close();
	} catch(IOException ioe) {
	    Trace.ignoredException(ioe);
	    // pop up error dialog?
	    return null;
	}


	return desktopFilePath;
    }

    /*
     * see http://www.freedesktop.org/standards/desktop-entry-spec/desktop-entry-spec.html
     * for the .desktop file specification
     */
    private String createDesktopFile(LaunchDesc ld,
                                     String     appTitle,
                                     String     iconPath,
                                     String     desktopPath,
                                     String     jnlpLocation,
                                     String     args) {
        InformationDesc id         = ld.getInformation();
        String          categories = getFolderName(ld);

        // normalize the args
        if (args == null) {
            args = "";
        }
        else if ((args.length() > 0) &&
                 (args.endsWith(" ") == false)) {
            args = args + " ";
        }

        // create path for destop file
        StringBuffer desktopFilePath = new StringBuffer(512);

        desktopFilePath.append(desktopPath)
                       .append(File.separator)
                       .append(appTitle)
                       .append(".desktop");

        Trace.println("desktopFilePath: " + desktopFilePath, TraceLevel.BASIC);

        File desktopFile = new File(desktopFilePath.toString());
        if (desktopFile.exists()) {
	    if (!replace) {
		replace = shouldInstallOverExisting(ld);
		if (replace) {
		    return desktopFile.getAbsolutePath();
		}
	    }
        } else {
            // create directory for this file
            desktopFile.getParentFile().mkdirs();
        }

        StringBuffer fileContents = new StringBuffer(4096);

        fileContents.append("[Desktop Entry]\n");
        fileContents.append("Encoding=UTF-8\n");
        fileContents.append("Version=1.0\n");
        fileContents.append("Type=Application\n");
        fileContents.append("Exec=").append(Config.getJavawsCommand())
            .append(" ")
            .append(args)
            .append(jnlpLocation)
            .append("\n");
        fileContents.append("Icon=").append(iconPath).append("\n");
        fileContents.append("Terminal=false\n");
        fileContents.append("Name=").append(appTitle).append("\n");
        fileContents.append("Comment=")
            .append(id.getDescription(InformationDesc.DESC_SHORT))
            .append("\n");
        fileContents.append("Categories=Application;").append(categories).append("\n");

        try {
            Trace.println("fileContents: " + fileContents, TraceLevel.BASIC);

            PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(desktopFile)));
            out.print(fileContents.toString());
            out.close();
        } catch(IOException ioe) {
            Trace.ignoredException(ioe);
            // pop up error dialog?
            return null;
        }

        return (desktopFile.getAbsolutePath());
    }

    private String createDirectoryFile(LaunchDesc ld, String iconPath, String desktopPath) {
	InformationDesc id = ld.getInformation();
	String menuName = getFolderName(ld);
	String directoryFilePath = desktopPath + File.separator + menuName + ".directory";
	Trace.println("directoryFilePath: " + directoryFilePath, TraceLevel.BASIC);
	File directoryFile = new File(directoryFilePath);

	String fileContents = "[Desktop Entry]\n" +
	    "Name=" + menuName + "\n" +
	    "Comment=" + id.getDescription(InformationDesc.DESC_SHORT) + "\n" +
	    "Icon=" + iconPath + "\n" +
	    "Type=Directory;\n";

	Trace.println("fileContents: " + fileContents, TraceLevel.BASIC);
	try {
	    // create directory for this file
	    new File(desktopPath).mkdirs();
	    PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(directoryFile)));
	    out.print(fileContents);
	    out.close();
	} catch(IOException ioe) {
	    Trace.ignoredException(ioe);
	    // pop up error dialog?
	    return null;
	}


	return directoryFilePath;
    }

    /**
     * Invoked when the install fails.
     */
    // err is the error code from native windows call
    private void installFailed(final LaunchDesc desc) {
        Runnable jOptionPaneRunnable = new Runnable() {
            public void run() {
                DialogFactory.showErrorDialog(ResourceManager.getString(
                    "install.installFailed", desc.getInformation().getTitle().trim()),
                    ResourceManager.getString("install.installFailedTitle"));
            }

        };
        invokeRunnable(jOptionPaneRunnable);
    }

    /**
     * Uninstalls a previously installed application
     */
    public void uninstall(LaunchDesc                desc,
                          LocalApplicationProperties lap,
                          boolean                    desktop) {
        boolean allUsers = false;

        if (Globals.isSystemCache()) {
            // GNOME doesn't have a way to create desktop shortcuts for all users
            desktop  = false;
            allUsers = true;
        }

        Trace.println("uninstall called in UnixInstallHandler", TraceLevel.BASIC);

        if (desktop) {
            String desktopFilePath = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY);
            if (desktopFilePath != null) {
                new File(desktopFilePath).delete();
                Trace.println("file removed: " +
                        desktopFilePath, TraceLevel.BASIC);
                lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, null);
            }
        }

        String startmenuFilePath = lap.get(INSTALLED_GNOME_START_MENU_KEY);
        if (startmenuFilePath != null) {
            // determine if this is
            new File(startmenuFilePath).delete();
            Trace.println("file removed: " + startmenuFilePath, TraceLevel.BASIC);
            lap.put(INSTALLED_GNOME_START_MENU_KEY, null);
        }

        startmenuFilePath = lap.get(INSTALLED_UNINSTALL_KEY);
        if (startmenuFilePath != null) {
            new File(startmenuFilePath).delete();
            Trace.println("file removed: " + startmenuFilePath, TraceLevel.BASIC);
            lap.put(INSTALLED_UNINSTALL_KEY, null);
        }

	String directoryFilePath = lap.get(INSTALLED_DIRECTORY_KEY);
        startmenuFilePath = lap.get(INSTALLED_RC_KEY);
        if (startmenuFilePath != null) {
            StringTokenizer st = new StringTokenizer(startmenuFilePath, File.pathSeparator);
            String filename;
            while (st.hasMoreElements()) {
                filename = st.nextToken();
                new File(filename).delete();
                Trace.println("file removed: " + filename, TraceLevel.BASIC);
		if (directoryFilePath != null) {
		    // rc_desktop is the fullpath to the rc desktopfile, minus
		    // the .desktop at the end	
		    String rc_desktop = filename.substring(0, filename.length() - 8);
		    VFolderEditor.updateVFolderInfo(getFolderName(desc), directoryFilePath, rc_desktop, allUsers, true);
		}
            }
            lap.put(INSTALLED_RC_KEY, null);
        }

        // need to update applications.vfolder info
        
        if (directoryFilePath != null) {
            VFolderEditor.updateVFolderInfo(getFolderName(desc), directoryFilePath, desc.getInformation().getTitle(), allUsers, true);

            // set lap to null if directoryFilePath is removed
            if (new File(directoryFilePath).exists() == false) {
                lap.put(INSTALLED_DIRECTORY_KEY, null);
            }
        }

        lap.setLocallyInstalled(false);
        save(lap);
    }

    /**
     *  Determine if this platform supports Local Application Installation.
     *  We will want to return true for the window managers that we support
     *  in install and uninstall above.
     */
    public boolean isLocalInstallSupported() {
	return Config.getInstance().isLocalInstallSupported();
    }

    // only support association if libgnomevfs-2.so can be loaded and
    // all the required functions exist
    public boolean isAssociationSupported() {
	return (GnomeVfsWrapper.openGNOMELibrary() && GnomeVfsWrapper.initGNOMELibrary());
    }

    public void associationCompleted() {
	// only call dlclose on solaris
	// calling dlclose on linux will crash the vm
	if (Config.getOSName() == "SunOS") {
	    GnomeVfsWrapper.closeGNOMELibrary();
	}
    }
}

