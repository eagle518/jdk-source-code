/*
 * @(#)ShellFolderManager.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.shell;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;

/**
 * @author Michael Martak
 * @since 1.4
 */

class ShellFolderManager {

    /**
     * Create a shell folder from a file.
     * Override to return machine-dependent behavior.
     */
    public ShellFolder createShellFolder(File file) throws FileNotFoundException {
        return new DefaultShellFolder(null, file);
    }
    
    /**
     * @param key a <code>String</code>
     *  "fileChooserDefaultFolder":
     *    Returns a <code>File</code> - the default shellfolder for a new filechooser
     *	"roots":
     *    Returns a <code>File[]</code> - containing the root(s) of the displayable hieararchy
     *  "fileChooserComboBoxFolders":
     *    Returns a <code>File[]</code> - an array of shellfolders representing the list to
     *    show by default in the file chooser's combobox
     *   "fileChooserShortcutPanelFolders":
     *    Returns a <code>File[]</code> - an array of shellfolders representing well-known
     *    folders, such as Desktop, Documents, History, Network, Home, etc.
     *    This is used in the shortcut panel of the filechooser on Windows 2000
     *    and Windows Me.
     *	"fileChooserIcon nn":
     *    Returns an <code>Image</code> - icon nn from resource 124 in comctl32.dll (Windows only).
     *
     * @return An Object matching the key string.
     */
    public Object get(String key) {
	if (key.equals("fileChooserDefaultFolder")) {
	    // Return the default shellfolder for a new filechooser
	    File homeDir = new File(System.getProperty("user.home"));
	    try {
		return createShellFolder(homeDir);
	    } catch (FileNotFoundException e) {
		return homeDir;
	    }
	} else if (key.equals("roots")) {
	    // The root(s) of the displayable hieararchy
	    return File.listRoots();
	} else if (key.equals("fileChooserComboBoxFolders")) {
	    // Return an array of ShellFolders representing the list to
	    // show by default in the file chooser's combobox
	    return get("roots");
	} else if (key.equals("fileChooserShortcutPanelFolders")) {
	    // Return an array of ShellFolders representing well-known
	    // folders, such as Desktop, Documents, History, Network, Home, etc.
	    // This is used in the shortcut panel of the filechooser on Windows 2000
	    // and Windows Me
	    return new File[] { (File)get("fileChooserDefaultFolder") };
	}
	return null;
    }

    /**
     * Does <code>dir</code> represent a "computer" such as a node on the network, or
     * "My Computer" on the desktop.
     */
    public boolean isComputerNode(File dir) {
	return false;
    }

    public boolean isFileSystemRoot(File dir) {
	if (dir instanceof ShellFolder && !((ShellFolder)dir).isFileSystem()) {
	    return false;
	}
	return (dir.getParentFile() == null);
    }

    public void sortFiles(List files) {
	Collections.sort(files, fileComparator);
    }

    private Comparator fileComparator = new Comparator() {
	public int compare(Object a, Object b) {
	    return compare((File)a, (File)b);
	}

	public int compare(File f1, File f2) {
	    ShellFolder sf1 = null;
	    ShellFolder sf2 = null;

	    if (f1 instanceof ShellFolder) {
		sf1 = (ShellFolder)f1;
		if (sf1.isFileSystem()) {
		    sf1 = null;
		}
	    }
	    if (f2 instanceof ShellFolder) {
		sf2 = (ShellFolder)f2;
		if (sf2.isFileSystem()) {
		    sf2 = null;
		}
	    }

	    if (sf1 != null && sf2 != null) {
		return sf1.compareTo(sf2);
	    } else if (sf1 != null) {
		return -1;	// Non-file shellfolders sort before files
	    } else if (sf2 != null) {
		return 1;
	    } else {
		String name1 = f1.getName();
		String name2 = f2.getName();

		// First ignore case when comparing
		int diff = name1.toLowerCase().compareTo(name2.toLowerCase());
		if (diff != 0) {
		    return diff;
		} else {
		    // May differ in case (e.g. "mail" vs. "Mail")
		    // We need this test for consistent sorting
		    return name1.compareTo(name2);
		}
	    }
	}
    };
}
