/*
 * @(#)ExtensionInstallationImpl.java	1.48 04/03/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import javax.swing.LookAndFeel;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.CodeSource;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.NoSuchProviderException;
import java.security.SignatureException;
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
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.Trace;
import sun.plugin.util.TraceFilter;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeployUIManager;


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
					       InterruptedException
		    {
			String jarFileName = ExtensionUtils.makePlatformDependent(
						 ExtensionUtils.extractJarFileName(reqExtInfo.url));
			String platformURL = ExtensionUtils.makePlatformDependent(reqExtInfo.url);

			// Ask if the user would like to install the extension
			//
			if (askUserForAcknowledgment(reqExtInfo, instExtInfo) == 1) {
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
		    desc = ResourceHandler.getMessage("optpkg.cert_expired");
		else if (ex instanceof CertificateNotYetValidException)
		    desc = ResourceHandler.getMessage("optpkg.cert_notyieldvalid");
		else if (ex instanceof CertificateException)
		    desc = ResourceHandler.getMessage("optpkg.cert_notverify");
		else
    		    desc = ResourceHandler.getMessage("optpkg.general_error");
	    }
	    else
	    {
		desc = ResourceHandler.getMessage("optpkg.general_error");
	    }

	    Trace.extPrintException(ex, desc, ResourceHandler.getMessage("optpkg.caption"));

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
			NoSuchAlgorithmException, IOException
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

                            if (TrustDecider.isAllPermissionGranted(cs) == false)
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
		    MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("optpkg.prompt_user.new_spec.text"));
		    Object[] args = { reqExtInfo.specVersion, extIdentifier, reqExtInfo.url };
		    dialogText = mf.format(args);
		}
		break;
	    case ExtensionInfo.REQUIRE_IMPLEMENTATION_UPGRADE:
		{
		    MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("optpkg.prompt_user.new_impl.text"));
		    Object[] args = { reqExtInfo.implementationVersion, extIdentifier, reqExtInfo.url };
		    dialogText = mf.format(args);
		}
		break;
	    case ExtensionInfo.REQUIRE_VENDOR_SWITCH:
		{
		    MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("optpkg.prompt_user.new_vendor.text"));
		    Object[] args = { reqExtInfo.vendor, reqExtInfo.vendorId, extIdentifier, reqExtInfo.url };
		    dialogText = mf.format(args);
		}
		break;
	    default:
		{
		    MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("optpkg.prompt_user.default.text"));
		    Object[] args = { reqExtInfo.name, reqExtInfo.url };
		    dialogText = mf.format(args);
		}
		break;
	    }
	}
	else
	{
	    MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("optpkg.prompt_user.default.text"));
	    Object[] args = { reqExtInfo.name, reqExtInfo.url };
	    dialogText = mf.format(args);
	}

        /* Note: Don't wrap the following portion of code under another secured thread
         * (e.g, DeploySysRun or PluginSysUtil).  The underlying download dialog is already
         * security safe as it's running under a DeploySysRun (see DialogFactory.showOptionDialog()).
         * Wrapping too many threads around download dialog causes deadlock when
         * there are multiple extension jars to install.  (see bug ID: 4961837)
         */

	int ret = 0;
	// Show dialog
	if (Trace.isAutomationEnabled() == false) {	
	    ret = DialogFactory.showDownloadDialog(DialogFactory.WARNING_MESSAGE, dialogText);
	} else {
	    // If automation is enabled
	    Trace.msgExtPrintln("optpkg.install.automation");
	        ret = 0;
	}
	
	
	if (ret==0) {
	    Trace.msgExtPrintln("optpkg.install.granted", new Object[] {reqExtInfo.url});
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
	return com.sun.deploy.resources.ResourceManager.getMessage(key);
    }
    private static int getAcceleratorKey(String key) {
	return com.sun.deploy.resources.ResourceManager.getAcceleratorKey(key);
    }

}

