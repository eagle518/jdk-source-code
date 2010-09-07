/*
 * @(#)ExtensionInstaller.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
