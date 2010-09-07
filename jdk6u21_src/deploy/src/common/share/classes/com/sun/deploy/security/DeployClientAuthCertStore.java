/*
 * @(#)DeployClientAuthCertStore.java	1.33 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.AccessController;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collection;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;
import java.util.TreeSet;
import java.util.HashSet;
import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPasswordField;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.UIFactory;
import java.net.PasswordAuthentication;


/**
 * DeployClientAuthCertStore is a class that represents the permanent certificate 
 * store which contains all the certificates that Java Plug-in recognize. It 
 * is used in the certification verification process when client authentication
 * encountered.
 */
public final class DeployClientAuthCertStore implements CertStore
{
    private static JDialog myParent = null;
    private static String _userFilename = null;
    private static String _systemFilename = null;

    private long _userLastModified = 0;
    private long _sysLastModified = 0;

    // Collection of deployment certificates
    private KeyStore _deploymentUserClientCerts = CertUtils.createEmptyKeyStore();
    private KeyStore _deploymentSystemClientCerts = CertUtils.createEmptyKeyStore();

    // Password for keystore
    private char[] keyPass = null;
    private boolean cancelFlag = false;
    private int certStoreType = 0;

    static
    {
	// Get deployment certificate file deployment.certs
	_userFilename = Config.getUserClientAuthCertFile();
        _systemFilename = Config.getSystemClientAuthCertFile();
    }

    private DeployClientAuthCertStore(JDialog inParent, int storeType)
    {
	myParent = inParent;
	certStoreType = storeType;
    }

    public static CertStore getCertStore(JDialog inParent) {
	return new ImmutableCertStore(new DeployClientAuthCertStore(inParent, CertStore.ALL));
    }

    public static DeployClientAuthCertStore getUserCertStore(JDialog inParent) {
	return new DeployClientAuthCertStore(inParent, CertStore.USER);
    }

    public static CertStore getSystemCertStore(JDialog inParent) {
	return new ImmutableCertStore(new DeployClientAuthCertStore(inParent, CertStore.SYSTEM));
    }

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
	load(false);
    }    

    public void load(boolean integrityCheck) throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
	long lastModified;

	if ((certStoreType & CertStore.USER) == CertStore.USER) {
           if (_userFilename != null) {
              // lastModified will return 0 if file not exist, so it
              // won't be loaded
              lastModified = CertUtils.getFileLastModified(_userFilename);
              if (lastModified != _userLastModified) {
                 _deploymentUserClientCerts = loadCertStore(_userFilename, integrityCheck);
                 _userLastModified = lastModified;
              }
           }
        }

	if ((certStoreType & CertStore.SYSTEM) == CertStore.SYSTEM) {
           if (_systemFilename != null) {
              lastModified = CertUtils.getFileLastModified(_systemFilename);
              if (lastModified != _sysLastModified) {
                 _deploymentSystemClientCerts = loadCertStore(_systemFilename, integrityCheck);
                 _sysLastModified = lastModified;
              }
           }
        }
    }

    private KeyStore loadCertStore(final String filename, final boolean integrityCheck) 
			throws IOException, CertificateException, 
			KeyStoreException, NoSuchAlgorithmException
    {
        Trace.msgSecurityPrintln("clientauthcertstore.cert.loading",
                                  new Object[] {filename});

        final KeyStore keyStore = KeyStore.getInstance("JKS");
        keyStore.load(null, null);

        try
        {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {

                public Object run() throws IOException, CertificateException,
                                   KeyStoreException, NoSuchAlgorithmException
                {
                    File file = new File(filename);

                    // Only load the cert store if it exists
                    if (file.exists())
                    {
                        FileInputStream fis = new FileInputStream(file);
                        BufferedInputStream bis = new BufferedInputStream(fis);

			// Initialize the keystore with or without password
			if (integrityCheck) {
			   keyPass = getPasswordDialog();
			   if (keyPass != null) {			
			      cancelFlag = false;
			      keyStore.load(bis, keyPass);
			   }
			   else {
			      cancelFlag = true;
			   }
			}
			else {
			   keyStore.load(bis, null);
			}

                        bis.close();
                        fis.close();
                    }

                    return null;
                }
           });
        }
        catch (PrivilegedActionException e)
        {
            Exception ex = e.getException();

            if (ex instanceof IOException)
                throw (IOException)ex;
            else if (ex instanceof CertificateException)
                throw (CertificateException)ex;
            else if (ex instanceof KeyStoreException)
                throw (KeyStoreException)ex;
            else if (ex instanceof NoSuchAlgorithmException)
                throw (NoSuchAlgorithmException)ex;
            else
                Trace.securityPrintException(e);
        }

        Trace.msgSecurityPrintln("clientauthcertstore.cert.loaded",
                                  new Object[]{filename});
        return keyStore;
    }


    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.saving", new Object[]{_userFilename});

        // User click OK button
        if (keyPass != null)
	{
	  try
	  { 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws IOException, CertificateException,
				   KeyStoreException, NoSuchAlgorithmException
		{
		    File file = new File(_userFilename);
                    file.getParentFile().mkdirs();
		    FileOutputStream fos = new FileOutputStream(file);
		    BufferedOutputStream bos = new BufferedOutputStream(fos);

             	    _deploymentUserClientCerts.store(bos, keyPass);

		    bos.close();
		    fos.close();

		    return null;
		}
    	     });
	  }
	  catch (PrivilegedActionException e)
	  {
	    Exception ex = e.getException();
	    
	    if (ex instanceof IOException)
		throw (IOException)ex;
	    else if (ex instanceof CertificateException)
		throw (CertificateException)ex;
	    else if (ex instanceof KeyStoreException)
		throw (KeyStoreException)ex;
	    else if (ex instanceof NoSuchAlgorithmException)
		throw (NoSuchAlgorithmException)ex;
	    else
		Trace.securityPrintException(e);
	  }
	}

	Trace.msgSecurityPrintln("clientauthcertstore.cert.saved", 
				  new Object[] {_userFilename});
    }


    /**
     * Add a certificate into the certificate store.
     * 
     * @param cert Certificate object.
     */
    public boolean add(Certificate cert) throws KeyStoreException 
    {
	return add(cert, false);
    }

    /**
     * Add a certificate into the certificate store.
     * 
     * @param cert Certificate object.
     * @param tsFlag true if certificate is valid.
     */
    public boolean add(Certificate cert, boolean tsFlag) throws KeyStoreException 
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.adding");

	// Add one only if it doesn't exist in User keyStore
        String newAlias = _deploymentUserClientCerts.getCertificateAlias(cert);
        if (newAlias == null)
	{
    	    // Generate a unique alias for the certificate
	    Random rand = new Random();
	    boolean found = false;
	    String alias = null;

	    // Loop until we found a unique alias that is not in the store
	    do 
	    {
		alias = "clientauthcert" + rand.nextLong();
    		Certificate c = _deploymentUserClientCerts.getCertificate(alias);
		if (c == null)
		    found = true;
	    }
	    while (found == false);

	    _deploymentUserClientCerts.setCertificateEntry(alias, cert);

	    Trace.msgSecurityPrintln("clientauthcertstore.cert.added", 
				      new Object[]{ alias});
	}

	return true;
    }

    public boolean addCertKey(Certificate[] certChain, Key inKey) throws 
	KeyStoreException, IOException, CertificateException, NoSuchAlgorithmException 
    {
	// Add one only if it doesn't exist in User keyStore
        String newAlias = _deploymentUserClientCerts.getCertificateAlias(certChain[0]);
        if (newAlias == null)
        {
          // Generate a unique alias for the certificate
          Random rand = new Random();
          boolean found = false;

          String alias = null;

          // Loop until we found a unique alias that is not in the store
          do {
             alias = "clientauthcert" + rand.nextLong();
             Certificate c = _deploymentUserClientCerts.getCertificate(alias);
             if (c == null)
                found = true;
          }
          while (found == false);

	  // Pop up password dialog box to get keystore password
	  if (keyPass == null && !cancelFlag)
             keyPass = getPasswordDialog();

          // User click OK button
          if (keyPass != null)
	  {
	     // Make sure the keyStore file exist
             File ksFile = new File(_userFilename);
             if (ksFile.exists())
	     {
	        // Before use this password, make sure this is the right keyStore password
	        FileInputStream fis = new FileInputStream(_userFilename);
                BufferedInputStream bis = new BufferedInputStream(fis);

	        try 
	        {
          	  _deploymentUserClientCerts.load(bis, keyPass);
                  _deploymentUserClientCerts.setKeyEntry(alias, inKey, keyPass, certChain);
	     	}
	        catch (Exception e)
	        {
		  Trace.securityPrintException(e);
		  return false;
	     	}
	     	finally
	     	{
             	  bis.close();
             	  fis.close();
	     	}
	     }
	     else {// It is a new keyStore file	
           	_deploymentUserClientCerts.setKeyEntry(alias, inKey, keyPass, certChain);
	     }
 
	     return true;
	  }
	  else // user click cancel
	     return false;
	}
	else {// alias exist
	  return false;
	}
    }

    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException 
    {
	Certificate[] certArray = new Certificate[1];
        certArray[0] = cert;
        return remove( certArray );
    }

    /**
     * Remove multiple certificates from the certificate store.
     * 
     * @param cert An array of Certificate objects.
     */
    public boolean remove(Certificate[] cert) throws IOException, KeyStoreException 
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.removing");

	String alias = null;

        // User clicked OK button
        if (keyPass != null)
	{
	   try
	   {
	      File infile = new File(_userFilename);
              KeyStore mykeyStore = KeyStore.getInstance("JKS");
              mykeyStore.load(new BufferedInputStream(new FileInputStream(infile)), keyPass);
              
              // Delete each Certificate from the Keystore
              for( int j=0; j<cert.length; j++) {
	          // Look up alias
	          alias = _deploymentUserClientCerts.getCertificateAlias(cert[j]);

                  // Remove From keystore
	          if (alias != null)
	             _deploymentUserClientCerts.deleteEntry(alias);
              }
	   }
	   catch (Exception e)
	   {
	    	if (e instanceof IOException)
	 	   throw (IOException)e;
	    	else if (e instanceof KeyStoreException)
		   throw (KeyStoreException)e;
	    	else
		   Trace.securityPrintException(e);

		return false;
	   }
	}
	else // user click Cancel
	   return false;

	Trace.msgSecurityPrintln("clientauthcertstore.cert.removed", 
				  new Object[] {alias});
	return true;
    }
    
    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException
    {
	return contains(cert, false);
    }

    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @param tsFlag true if only valid certificate is checked.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert, boolean tsFlag) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.instore");

	// Certificate alias returned only if there is a match
        String alias = null;

        alias = _deploymentSystemClientCerts.getCertificateAlias(cert);

        if (alias != null) // in system cert store
           return true;

        alias = _deploymentUserClientCerts.getCertificateAlias(cert);
        return (alias != null);
    }

    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */

    public boolean verify(Certificate cert)
    {
        Trace.msgSecurityPrintln("clientauthcertstore.cert.canverify");

	// Client Authentication Certificate store is 
	// not intended to be used for verification.

	return false;
    }

    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return Collection for certificates chain 
     */
    public Collection getCertificates() throws KeyStoreException
    {
        HashSet deployClientAuthCerts = new HashSet();

        if ((certStoreType & CertStore.USER)  == CertStore.USER) {
           deployClientAuthCerts.addAll(getCertificates(CertStore.USER));
        }

        if ((certStoreType & CertStore.SYSTEM) == CertStore.SYSTEM) {
           deployClientAuthCerts.addAll(getCertificates(CertStore.SYSTEM));
        }

        return deployClientAuthCerts;
    }

    private Collection getCertificates(int myCertStoreType) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.getcertificates");
	KeyStore ks = null;

        if (myCertStoreType == CertStore.USER) {
           ks = _deploymentUserClientCerts;
        }
        else {
           ks = _deploymentSystemClientCerts;
        }
        Enumeration keyAliases = ks.aliases();

	// Construct a TreeSet object to sort the certificate list
        TreeSet tsCerts = new TreeSet();

        while (keyAliases.hasMoreElements())
        {
            // Get certificate alias from iterator
            String alias = (String) keyAliases.nextElement();
            
            // Only add to the list if it is a key entry
            if (ks.isKeyEntry(alias))
		tsCerts.add(alias);
        }

        Collection certCollection = new ArrayList();
	Iterator itrCerts = tsCerts.iterator();
        while (itrCerts.hasNext())
        {
            // Get certificate chain from store
            String sortAlias = (String) itrCerts.next();
            Certificate[] certChain = ks.getCertificateChain(sortAlias);

            // Add certificate chain into collection
            if (certChain != null)
		certCollection.add(certChain);
        }              

        return certCollection;
    }

    private char[] getPasswordDialog()
    {
	// Pass already translated strings to the dialog.
	CredentialInfo passwordInfo = 
                UIFactory.showPasswordDialog(myParent,
                getMessage("password.dialog.title"),
                getMessage("clientauth.user.password.dialog.text"), 
                false, false, null, false);
             
        // If user pressed "Cancel" in password dialog, return value
        // from dialog will be null.  Check here if user pressed "Cancel".
        if ( passwordInfo == null ) {
            return null;
        } else {
            // get password
            return passwordInfo.getPassword();
        }
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
        return ResourceManager.getMessage(key);
    }
}
