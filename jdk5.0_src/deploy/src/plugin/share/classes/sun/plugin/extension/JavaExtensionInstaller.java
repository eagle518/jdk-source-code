/*
 * @(#)JavaExtensionInstaller.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.extension;

import java.net.URL;
import java.io.File;
import java.io.IOException;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.Trace;
import com.sun.deploy.util.DialogFactory;



/**
 * Class that represents the Java standard extension installer.
 *
 * @author Stanley Man-Kit Ho
 */
public class JavaExtensionInstaller implements ExtensionInstaller
{
    /**
     * Install extension to the destination directory by
     * launching the Java Extension Installer.
     *
     * @param url URL of where the extension was downloaded from.
     * @param jarFileName Jar file that might contains the extensions.
     * @param destDir Destination directory to install the extension.
     * @return true if extension is installed successfully.
     */
    public boolean install(String url, String jarFileName, String destDir)
		   throws IOException
    {
	// The jar file itself contains the actual native extension 
	// installer. We launch the Java installer through the
	// "java -jar" command.
	//
	// There is no easy way to determine if the installation has
	// completed, so we popup a modal dialog and assume the user 
	// will dismiss the dialog when the installation has 
	// completed.
	//

	Trace.msgExtPrintln("optpkg.install.java.launch");
	
	String javaHome = System.getProperty("java.home");
	String command = javaHome + File.separator + "bin" + File.separator 
			 + "java -jar " + jarFileName;
	
	String[] cmndarray = new String[3];
	cmndarray[0] = javaHome + File.separator + "bin" + File.separator + "java";
	cmndarray[1] = "-jar";
	cmndarray[2] = jarFileName;
	
	Trace.msgExtPrintln("optpkg.install.java.launch.command", new Object[] {command});

	// Create a new process to launch the Java installer
	Process p = Runtime.getRuntime().exec(cmndarray);	

	// Popup a modal dialog and hope the user will dismiss 
	// the dialog when the installation has completed.
	//
	DialogFactory.showInformationDialog(ResourceHandler.getMessage("optpkg.installer.launch.wait"),
					    ResourceHandler.getMessage("optpkg.installer.launch.caption"));

	return true;
    }
}
