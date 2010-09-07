/*
 * @(#)RawExtensionInstaller.java	1.19 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.extension;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.URL;
import java.util.jar.JarFile;
import com.sun.deploy.util.Trace;

/**
 * Class that represents the Java standard extension installer.
 *
 * @author Stanley Man-Kit Ho
 */
public class RawExtensionInstaller implements ExtensionInstaller
{
    /**
     * Install raw extension to the destination directory.
     *
     * @param url URL of where the extension was downloaded from.
     * @param jarFileName Jar file that might contains the extensions.
     * @param destDir Destination directory to install the extension.
     * @return true if extension is installed successfully.
     */
    public boolean install(String url, String jarFileName, String destDir)
		   throws IOException
    {
	// The jar file itself is the extension that we should copy
	// into the destination directory. To minimal overhand, the
	// file is "moved" to the extension directory.
	//

	Trace.msgExtPrintln("optpkg.install.raw.launch");

	// Determine source and destination file name for the extension

	File sourceFile = new File(jarFileName);
	File destFile = new File(destDir + File.separatorChar + sourceFile.getName());

	Trace.msgExtPrintln("optpkg.install.raw.copy", new Object[] {sourceFile, destFile});

	FileInputStream fis = new FileInputStream(sourceFile);
	BufferedInputStream bis = new BufferedInputStream(fis);
	FileOutputStream fos = new FileOutputStream(destFile);
	BufferedOutputStream bos = new BufferedOutputStream(fos);

	ExtensionUtils.copy(bis, bos);

	bis.close();
	fis.close();
	bos.close();
	fos.close();

	return true;
    }
}
