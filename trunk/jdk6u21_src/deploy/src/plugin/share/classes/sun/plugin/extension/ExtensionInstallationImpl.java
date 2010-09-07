/*
 * @(#)ExtensionInstallationImpl.java	1.60 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.extension;

import sun.misc.ExtensionInstallationException;
import sun.misc.ExtensionInfo;
import sun.misc.ExtensionInstallationProvider;
import javax.swing.JButton;
import java.util.jar.JarFile;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.util.jar.Manifest;
import java.util.jar.JarEntry;
import java.net.URL;
import java.net.URLConnection;
import java.io.IOException;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.CodeSource;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.NoSuchProviderException;
import java.security.SignatureException;
import java.security.cert.CRLException;
import java.security.InvalidAlgorithmParameterException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.CertificateParsingException;
import java.text.MessageFormat;
import java.util.StringTokenizer;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Vector;
import java.util.Map;
import java.util.Set;
import java.util.Iterator;
import sun.security.action.GetPropertyAction;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import java.net.MalformedURLException;


/**
 * Provide facilities to install extensions in the standard extension
 * directory of the running JDK
 *
 * @author Stanley Man-Kit Ho
 */
public class ExtensionInstallationImpl implements ExtensionInstallationProvider {

    public boolean installExtension(final ExtensionInfo reqExtInfo,
				    final ExtensionInfo instExtInfo)
				    throws ExtensionInstallationException
    {
	Trace.msgExtPrintln("optpkg.install.info", new Object[] {reqExtInfo});

	try
	{
	    // Copy the jar into temp directory
	    AccessController.doPrivileged(
		new PrivilegedExceptionAction()
		{
		    public Object run() throws IOException, ExtensionInstallationException,
		    			       CertificateException, CertificateEncodingException,
					       CertificateExpiredException, CertificateNotYetValidException,
					       CertificateParsingException, KeyStoreException,
					       NoSuchAlgorithmException, IOException,
					       CRLException, InvalidAlgorithmParameterException,
					       InterruptedException
		    {
			String jarFileName = ExtensionUtils.makePlatformDependent(
						 ExtensionUtils.extractJarFileName(reqExtInfo.url));
			String platformURL = ExtensionUtils.makePlatformDependent(reqExtInfo.url);

			// Ask if the user would like to install the extension
			// If answer is anything but OK, throw exception
			if (askUserForAcknowledgment(reqExtInfo,instExtInfo) != 
                                                            UIFactory.OK) {
			    throw new ExtensionInstallationException(
					       "User denied installation of " +
					       platformURL);
			}

			// Copy extension installer jar into temp directory
			URL url = new URL(platformURL);
			URLConnection conn = url.openConnection();
			InputStream is = conn.getInputStream();
			BufferedInputStream bis = new BufferedInputStream(is);

			String localJarName = ExtensionUtils.getTempDir() + File.separator + jarFileName;

			File f = new File(localJarName);
			FileOutputStream fos = new FileOutputStream(f);
			BufferedOutputStream bos = new BufferedOutputStream(fos);

			ExtensionUtils.copy(bis, bos);

			bis.close();
			is.close();
			bos.close();
			fos.close();

			// Verify the jar file
			verifyJar(platformURL, localJarName);

			// Install optional packages through installer
			installJarFile(platformURL, localJarName);

			// Clean up temp file
			if (f.exists())
			    f.delete();

			return null;
		    }
		});
	}
	catch (Throwable e)
	{
	    Throwable ex = e;
	    String desc = null;

	    if (e instanceof PrivilegedActionException)
	    {
		ex = ((PrivilegedActionException) e).getException();

		if (ex instanceof CertificateExpiredException)
		    desc = ResourceManager.getMessage("optpkg.cert_expired");
		else if (ex instanceof CertificateNotYetValidException)
		    desc = ResourceManager.getMessage("optpkg.cert_notyieldvalid");
		else if (ex instanceof CertificateException)
		    desc = ResourceManager.getMessage("optpkg.cert_notverify");
		else {
    		    desc = ResourceManager.getMessage("optpkg.general_error");
		}
	    }
	    else
	    {
		desc = ResourceManager.getMessage("optpkg.general_error");
	    }

	    Trace.extPrintException(ex, desc, ResourceManager.getMessage("optpkg.caption"));

	    Trace.msgExtPrintln("optpkg.install.fail");
	    return true;
    	}

	Trace.msgExtPrintln("optpkg.install.ok");
	return true;
    }


    /**
     * <P> Verify a signed jar file.
     * </P>
     *
     * @param url URL of the signed jar file.
     * @param jarName Name of the jar file.
     */
    private void verifyJar(String url, String jarName)
		 throws ExtensionInstallationException,
			CertificateException, CertificateEncodingException,
			CertificateExpiredException, CertificateNotYetValidException,
			CertificateParsingException, KeyStoreException,
			NoSuchAlgorithmException, IOException,
			CRLException, InvalidAlgorithmParameterException
    {

        // Load the jar file
	JarFile jf = null;

	try
	{
            HashMap certMap = new HashMap();
            byte[] buffer = new byte[8192];
	    Certificate[] certChain = null;

            // Enumerate all entries in Jar file,
            // and check the signature/digest of each entries
	    jf = new JarFile(jarName, true);
            Enumeration entries = jf.entries();
            while (entries.hasMoreElements())
            {
                JarEntry je = (JarEntry)entries.nextElement();
                String name = je.getName();
                InputStream is = jf.getInputStream(je);
                int n;

		while ((n = is.read(buffer, 0, buffer.length)) != -1)
                {
                    // we just read. this will throw a SecurityException
		    // if a signature/digest check fails.
                }
                is.close();

                // Now we check whether this entry is signed in the jar file.
                // Entries in META-INF, directories, and zero-length files are
                // not signed, so skip those
                if (!name.startsWith("META-INF/") && !name.endsWith("/") && je.getSize() != 0)
                {
                    Certificate[] certs = je.getCertificates();
                    boolean isSigned = ((certs != null) && (certs.length > 0));

                    if (isSigned)
                    {
			if (certChain == null)
			{
			   // Check if certs as multiple certificates for one entry, not allowed
			   if (hasMultipleSigners(certs))
                              throw new ExtensionInstallationException("Error: one entry has multiple certificates");
			   certChain = certs;
		    	}
		   	else if (!equalChains(certChain, certs))
		   	{
			   // All entries must be signed by same signer.
                           throw new ExtensionInstallationException("Error: Entries signed by different signer");
    		    	}

                        // Determine if the certificate has been encountered
                        // before
                        CodeSource cs = (CodeSource) certMap.get(certs);

		        if (cs == null)
                        {
                            // No, we haven't seen this cert before, so
                            // popup security dialog
                            //
                            cs = new CodeSource(new URL(url), certs);
                            certMap.put(certs, cs);

                            if (TrustDecider.isAllPermissionGranted(cs) == TrustDecider.PERMISSION_DENIED)
                            {
                                throw new ExtensionInstallationException("User deny optional package installer"
                                                                        + " to be launched.");
                            }
                        }
                    }
                    else // The entry didn't get signed, throw exception
                        throw new ExtensionInstallationException("Optional package installer is unsigned."
                                            + " (signatures missing or not parsable)");
                }
	    }

	    // Get the manifest entry. If result is OK,
	    // still need to check that all files that got
            // signed are actually in the JAR file
            Manifest man = jf.getManifest();
            if (man != null)
            {
                Set mfEntries = man.getEntries().entrySet();
                Iterator itr = mfEntries.iterator();
                while(itr.hasNext()) {
                    Map.Entry me = (Map.Entry)itr.next();
                    String name = (String)me.getKey();

                    // Make sure name is in the JAR file
                    if (jf.getEntry(name) == null)
                        throw new ExtensionInstallationException("Manifest entry not in the JAR file");
                }
            }
            else
                throw new ExtensionInstallationException("No manifest in the optional package installer.");
	}
	catch (IOException e)
	{
	    throw new ExtensionInstallationException("IO Error. Unable to verify optional package installer.");
	}
	finally
	{
	    // Close the JarFile handle
	    if (jf != null)
               jf.close();
	}
    }

    private boolean hasMultipleSigners(Certificate[] chain)
    {
        Certificate target = chain[0];

        // If a self-signed certificate
        for(int i = 1; i < chain.length; i++) {
            Certificate signer = chain[i];
            if (!isSigner(target, signer))
		return true;

            // Switch signer to target for next iteration
            target = signer;
        }
        return false;
    }

    private boolean isSigner(Certificate check, Certificate signer) {
        try
	{
            check.verify(signer.getPublicKey());
            return true;
        } catch(InvalidKeyException ike) {
            return false;
        } catch(NoSuchProviderException nsp) {
            return false;
        } catch(NoSuchAlgorithmException nsae) {
            return false;
        } catch(SignatureException se) {
            return false;
        } catch(CertificateException ce) {
            return false;
        }
    }

    private boolean equalChains(Certificate[] chain1, Certificate[] chain2)
    {
        if (chain1.length != chain2.length)
	   return false;

        for(int i = 0; i < chain1.length; i++) {
            if (!chain1[i].equals(chain2[i]))
		return false;
        }
        return true;
    }

    private int askUserForAcknowledgment(ExtensionInfo reqExtInfo,
					 ExtensionInfo instExtInfo)
    {
	String dialogText = null;
        // Create AppInfo object to show info about package to be downloaded.
        AppInfo pkgInfo = new AppInfo();

	if (instExtInfo != null) {
	    // this extension is already installed, let's check why we need to
	    // reinstall it.

	    // check to see which extension identifier to use, title
	    // (optional manifest attribute) or name (required) in messages
	    String extIdentifier = reqExtInfo.name;

	    if (reqExtInfo.title != null) {
	    	extIdentifier = reqExtInfo.title;
	    }

	    int reason = instExtInfo.isCompatibleWith(reqExtInfo);
	    switch(reason) {
	    case ExtensionInfo.REQUIRE_SPECIFICATION_UPGRADE:
		{
		    dialogText = ResourceManager.getMessage(
                            "optpkg.prompt_user.text");
                    
                    MessageFormat mf = new MessageFormat(
                            ResourceManager.getMessage(
                            "optpkg.prompt_user.specification"));
		    Object[] args = { reqExtInfo.specVersion };
                    
                    // Set "Name:" of the package to be of form
                    // Name: <Extension name> (<version> specification)
                    pkgInfo.setTitle(extIdentifier + mf.format(args));		    
		}
		break;
	    case ExtensionInfo.REQUIRE_IMPLEMENTATION_UPGRADE:
		{
                    dialogText = ResourceManager.getMessage(
                            "optpkg.prompt_user.text");
                    
		    MessageFormat mf = new MessageFormat(
                            ResourceManager.getMessage(
                            "optpkg.prompt_user.implementation"));
		    Object[] args = { reqExtInfo.implementationVersion };
                    
                    // Set "Name:" of the package to be of form
                    // Name: <Extension name> (<version> implementation)
                    pkgInfo.setTitle(extIdentifier + mf.format(args));		    
		}
		break;
	    case ExtensionInfo.REQUIRE_VENDOR_SWITCH:
		{
                    dialogText = ResourceManager.getMessage(
                            "optpkg.prompt_user.text");
                    		    
                    // Set "Name:" of the package to be of form
                    // Name: <Extension name> (<vendor>)
                    pkgInfo.setTitle(
                            extIdentifier + " (" + reqExtInfo.vendor + ")");
		}
		break;
	    default:
		{
                    pkgInfo.setTitle(reqExtInfo.name);
		    dialogText = ResourceManager.getMessage(
                            "optpkg.prompt_user.default.text");
		}
		break;
	    }
	}
	else
	{
            pkgInfo.setTitle(reqExtInfo.name);
	    dialogText = ResourceManager.getMessage(
                    "optpkg.prompt_user.default.text");
	}

        /* Note: Don't wrap the following portion of code under another secured 
         * thread (e.g, DeploySysRun or PluginSysUtil).  The underlying 
         * message dialog is already security safe as it's running under a 
         * DeploySysRun (see UIFactory.showOptionDialog()).
         * Wrapping too many threads around download dialog causes deadlock when
         * there are multiple extension jars to install.  (see bug ID: 4961837)
         */

        // Set return value to ERROR
	int ret = UIFactory.ERROR;
	// Show dialog
	if (Trace.isAutomationEnabled() == false) {                        
            try{
                pkgInfo.setFrom(new URL(reqExtInfo.url));
            }catch(MalformedURLException mue){
                // if we could not get URL, no problem - we will not show it.
            }            
	    ret = UIFactory.showWarningDialog(null, pkgInfo, dialogText, null, 
                    ResourceManager.getMessage("optpkg.prompt_user.caption"));
	} else {
	    // If automation is enabled
	    Trace.msgExtPrintln("optpkg.install.automation");
	        ret = UIFactory.OK;
	}
	
	
	if (ret==UIFactory.OK) {
	    Trace.msgExtPrintln("optpkg.install.granted", 
                                new Object[] {reqExtInfo.url});
	} else {
	    Trace.msgExtPrintln("optpkg.install.deny");
	}
	
	return ret;

    }

    /**
     * <P> Installs optional packages.
     * </P>
     *
     * @param url URL of the downloaded optional packages.
     * @param jarFileName Jar file that represents the downloaded optional packages.
     */
    private void installJarFile(String url, String jarFileName)
	throws ExtensionInstallationException, IOException, InterruptedException
    {
	Trace.msgExtPrintln("optpkg.install.begin", new Object[] {jarFileName});

	JarFile jarFile = new JarFile(jarFileName);
	Manifest man = jarFile.getManifest();

	if (man != null) {

	    // Get the extension directory, we'll need it anyway
	    String extDirs = (String) AccessController.doPrivileged(new GetPropertyAction("java.ext.dirs"));

	    StringTokenizer st = new StringTokenizer(extDirs, File.pathSeparator);
	    String destDir = st.nextToken();

	    Attributes attr = man.getMainAttributes();
	    ExtensionInstaller installer = new RawExtensionInstaller();

	    if (attr != null) {
		if (attr.getValue(Name.MAIN_CLASS) != null)
		{
		    // Main attribute exists - Java installer
		    installer = new JavaExtensionInstaller();
		}
		else if (attr.getValue(Name.EXTENSION_INSTALLATION) != null)
		{
		    // Extension-installation attribute exists, native installer
		    installer = new NativeExtensionInstaller();
		}
	    }

	    // Make sure jarFile is set to null for GC
	    jarFile = null;

	    // Launch extenson installer
	    installer.install(url, jarFileName, destDir);
	}
    }


    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
	return ResourceManager.getMessage(key);
    }
    private static int getAcceleratorKey(String key) {
	return ResourceManager.getAcceleratorKey(key);
    }

}

