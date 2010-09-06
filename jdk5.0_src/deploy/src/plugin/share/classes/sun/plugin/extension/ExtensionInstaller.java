/*
 * @(#)ExtensionInstaller.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.extension;

import java.io.IOException;
import java.util.jar.JarFile;
import sun.misc.ExtensionInstallationException;


/**
 * Interface that represents the Java standard extension installer.
 */
public interface ExtensionInstaller
{
    /**
     * Install extension to the destination directory.
     *
     * @param url URL of where the extension was downloaded from.
     * @param jarFileName Jar file that might contains the extensions.
     * @param destDir Destination directory to install the extension.
     * @param true if extension is installed successfully.
     */
    public boolean install(String url, String jarFileName, String destDir)
		    throws ExtensionInstallationException, 
			   IOException, InterruptedException;
}
