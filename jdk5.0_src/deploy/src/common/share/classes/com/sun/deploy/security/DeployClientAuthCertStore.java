/*
 * @(#)DeployClientAuthCertStore.java	1.18 04/01/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import javax.swing.LookAndFeel;
import java.util.Collection;
import java.util.Enumeration;
import java.util.LinkedHashSet;
import java.util.Iterator;
import java.util.Random;
import java.util.TreeSet;
import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPasswordField;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DialogFactory;


/**
 * DeployClientAuthCertStore is a class that represents the permanent certificate 
 * store which contains all the certificates that Java Plug-in recognize. It 
 * is used in the certification verification process when client authentication
 * encountered.
 */
public final class DeployClientAuthCertStore implements CertStore
{
    private static JDialog myParent = null;
    private static String _filenameUser = null;
    private static String _filenameSys = null;
    private char[] savePassword = null;

    static
    {
	// Get deployment certificate file deployment.certs
	_filenameUser = Config.getUserClientAuthCertFile();
        _filenameSys = Config.getSystemClientAuthCertFile();
    }

    // Collection of deployment certificates
    private KeyStore _deploymentUserClientCerts = null;
    private KeyStore _deploymentSystemClientCerts = null;

    public DeployClientAuthCertStore()
    {
    }

    public DeployClientAuthCertStore(JDialog inParent)
    {
	myParent = inParent;
    }

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
        _deploymentUserClientCerts = load(_filenameUser);
        _deploymentSystemClientCerts = load(_filenameSys);
    }    

    private KeyStore load(final String filename) throws IOException,
        CertificateException, KeyStoreException, NoSuchAlgorithmException
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

			// Initialize the keystore with no password
			keyStore.load(bis, null);

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
	Trace.msgSecurityPrintln("clientauthcertstore.cert.saving", new Object[]{_filenameUser});

        final char[] keyPass = getPasswordDialog("clientauth.password.dialog.text");
        // User click OK button
        if (keyPass != null)
	{
	  try
	  { 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws IOException, CertificateException,
				   KeyStoreException, NoSuchAlgorithmException
		{
		    File file = new File(_filenameUser);
                    file.getParentFile().mkdirs();
		    FileOutputStream fos = new FileOutputStream(file);
		    BufferedOutputStream bos = new BufferedOutputStream(fos);

             	    _deploymentUserClientCerts.store(bos, keyPass);

		    bos.close();
		    fos.close();

		    setPassword(null);
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
				  new Object[] {_filenameUser});
    }


    /**
     * Add a certificate into the certificate store.
     * 
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException 
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
    }

    public boolean addCertKey(Certificate[] certChain, Key inKey) throws 
	KeyStoreException, IOException, CertificateException, NoSuchAlgorithmException 
    {
	if (contains(certChain[0]) == false)
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

	  // Pop up password dialog box to get keystore password
          char[] keyPass = getPasswordDialog("clientauth.password.dialog.text");

          // User click OK button
          if (keyPass != null)
	  {
	     // Make sure the keyStore file exist
             File ksFile = new File(_filenameUser);
             if (ksFile.exists())
	     {
	        // Before use this password, make sure this is the right keyStore password
	        FileInputStream fis = new FileInputStream(_filenameUser);
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
	     else // It is a new keyStore file	
           	_deploymentUserClientCerts.setKeyEntry(alias, inKey, keyPass, certChain);
 
	     // Remember password
	     setPassword(keyPass);
	     return true;
	  }
	  else // user click cancel
	     return false;
	}
	else // alias exist
	  return false;
    }

    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException 
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.removing");

	String alias = null;

        final char[] keyPass = getPasswordDialog("clientauth.password.dialog.text");
        // User click OK button
        if (keyPass != null)
	{
	   try
	   {
	      File infile = new File(_filenameUser);
              KeyStore mykeyStore = KeyStore.getInstance("JKS");
              mykeyStore.load(new BufferedInputStream(new FileInputStream(infile)), keyPass);

	      // Delete alias from keyStore
	      alias = _deploymentUserClientCerts.getCertificateAlias(cert);
	      if (alias != null)
	         _deploymentUserClientCerts.deleteEntry(alias);

	      // Remember password
	      setPassword(keyPass);

	      // Save keyStore
	      save();
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
     * @return Iterator for iterating certificates
     */
    public Iterator iterator(int storeType) throws KeyStoreException
    {
        return null;
    }

    public Iterator iteratorChain(int storeType) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.iterator");

	KeyStore ks = getKeyStore(storeType);
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

        LinkedHashSet certCollection = new LinkedHashSet();
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
        return certCollection.iterator();
    }

    /**
    * Obtain KeyStore Object
    */
    private KeyStore getKeyStore(int storeType)
    {
	Trace.msgSecurityPrintln("clientauthcertstore.cert.getkeystore");
	return (storeType == CertStore.USER) ?
                _deploymentUserClientCerts : _deploymentSystemClientCerts ;
    }

    public char[] getPasswordDialog(String inLabel)
    {
	if (getPassword() != null)
	    return getPassword();	
	else
	{
	    LookAndFeel lookAndFeel = null;

	    try
	    {
		// Change look and feel
		lookAndFeel = DeployUIManager.setLookAndFeel();
			
		// Pop up password dialog box
		Object dialogMsg = getMessage(inLabel);
		JPasswordField passwordField = new JPasswordField();

		Object[] msgs = new Object[2];
		msgs[0] = dialogMsg.toString();
		msgs[1] = passwordField;

		JButton okButton = new JButton(getMessage("cert.dialog.password.okButton"));
		JButton cancelButton = new JButton(getMessage("cert.dialog.password.cancelButton"));

		String title = getMessage("cert.dialog.password.caption");
		Object[] options = {okButton, cancelButton};
		int selectValue = DialogFactory.showOptionDialog(myParent, DialogFactory.QUESTION_MESSAGE,
		    msgs, title, options, options[0]);

		// for security purpose, DO NOT put password into String.
		// Reset password as soon as possible.
		final char[] mypassword = passwordField.getPassword();

		// User click OK button
		if (selectValue == 0)
		    return mypassword;
		else
		    return null;
	    }
	    finally
	    {
		// Restore look and feel
		DeployUIManager.restoreLookAndFeel(lookAndFeel);
	    }	
	}
    }

    private void setPassword(char[] inPassword)
    {
	savePassword = inPassword;
    }

    private char[] getPassword()
    {
	return savePassword;
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
        return ResourceManager.getMessage(key);
    }
}
