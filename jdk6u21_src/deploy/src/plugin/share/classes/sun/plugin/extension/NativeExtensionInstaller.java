/*
 * @(#)NativeExtensionInstaller.java	1.33 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.extension;

import java.util.jar.JarFile;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.util.jar.Manifest;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.IOException;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;

/**
 * Class that represents the native standard extension installer.
 *
 * @author Stanley Man-Kit Ho
 */
public class NativeExtensionInstaller implements ExtensionInstaller
{
    /**
     * Install extension to the destination directory by launching
     * the native installer.
     *
     * @param url URL of where the extension was downloaded from.
     * @param jarFileName Jar file that might contains the extensions.
     * @param destDir Destination directory to install the extension.
     * @return true if extension is installed successfully.
     */
    public boolean install(String url, String jarFileName, String destDir)
		   throws IOException, InterruptedException
    {
	// The jar file itself contains the actual native extension 
	// installer. We need to obtain the name of the installer
	// through the manifest, copy the installer to a temp directory
	// and launch it.
	//
	// There is no easy way to determine if the installation has
	// completed, so we popup a modal dialog and assume the user 
	// will dismiss the dialog when the installation has 
	// completed.
	//

	Trace.msgExtPrintln("optpkg.install.native.launch");

	String tempInstall = null;

	try
	{
	    JarFile jarFile = new JarFile(jarFileName);	    
	    // Obtain name of the native extension installer	
	    Manifest man = jarFile.getManifest();
	    Attributes attr = man.getMainAttributes();
	    String installerName = attr.getValue(Name.EXTENSION_INSTALLATION);
	    if (installerName != null)
		installerName = installerName.trim();

	    InputStream is = jarFile.getInputStream(jarFile.getEntry(installerName));
	    BufferedInputStream bis = new BufferedInputStream(is);

	    tempInstall = ExtensionUtils.getTempDir() + File.separator + installerName;

	    FileOutputStream os = new FileOutputStream(tempInstall);
	    BufferedOutputStream bos = new BufferedOutputStream(os);

	    // Copy input stream into output stream
	    ExtensionUtils.copy(bis, bos);

	    bis.close();
	    is.close();
	    bos.close();
	    os.close();

	    String osName = System.getProperty("os.name");
	    String chDir=System.getProperty("java.home");

	    if (osName.indexOf("Windows") == -1)
	    {

		    // Create a new process to change the permission on the file on Unix
		    Process p0 = Runtime.getRuntime().exec("chmod 755 " + tempInstall);

		    // Wait for the process to exit properly
		    
		    p0.waitFor();
		try
		{

		    File f=new File(chDir);
		    // Create a new process to launch the native installer
		    // Due to bug #4396676 the process's directory should be 
		    // jre home in unix
		    Process p = Runtime.getRuntime().exec(tempInstall, null,f);
		    int i=p.waitFor();
		    if ( i != 0 )
		    {
			Trace.msgExtPrintln("optpkg.install.native.launch.fail.0", new Object[] {tempInstall});
			return false;
		    }	  
		}
		catch(SecurityException exp)
		{
		    Trace.msgExtPrintln("optpkg.install.native.launch.fail.1", new Object[] {chDir});
		    Trace.securityPrintException(exp);
		}
		finally
		{
		    // Remove temp file
		    if (tempInstall != null)
			{
			    File nativeInstall = new File(tempInstall);
			    if (nativeInstall.exists())
				nativeInstall.delete();
			}
		}
		return false;
	    }
	    else
	    {
		// Create a new process to launch the native installer
		Process p = Runtime.getRuntime().exec(tempInstall);
	     }
	    // Popup a modal dialog and hope the user will dismiss 
	    // the dialog when the installation has completed.
	    //
	    UIFactory.showInformationDialog(null, 
		ResourceManager.getMessage("optpkg.installer.launch.wait"),
	        ResourceManager.getMessage("optpkg.installer.launch.caption"));

	    return true;
	}
	finally
	{
	    // Remove temp file
	    if (tempInstall != null)
	    {
		File nativeInstall = new File(tempInstall);
		if (nativeInstall.exists())
    		    nativeInstall.delete();
	    }
	}
    }
}
