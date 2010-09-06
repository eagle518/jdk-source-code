/*
 * @(#)CertificateesInfo.java	1.50 03/03/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

 /**
  * Model class for Certificate Panel information pertinent to the Java
  * Control panel
  *
  * End users should never be editing the properties file directly.  They
  * should be using the (carefully internatiuonalzied) ControlPanel.
  *
  * @version 1.0, 04/19/03
  */

import java.util.*;
import java.io.*;
import java.security.Key;
import java.security.cert.*;
import java.text.MessageFormat;
import javax.swing.JDialog;
import com.sun.deploy.security.CertStore;
import com.sun.deploy.security.RootCertStore;
import com.sun.deploy.security.SSLRootCertStore;
import com.sun.deploy.security.DeploySigningCertStore;
import com.sun.deploy.security.DeploySSLCertStore;
import com.sun.deploy.security.DeployClientAuthCertStore;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.*;

class CertificatesInfo {

    /**
     * Contruct a new configuration information
     * 
     */
    public CertificatesInfo() {

        reset();
    }

    /**
     * Return a collection which represents the list of alias 
     * of all the active certificate in the certificate store.
     *
     * @return Collection of active certificate alias
     */
    public Collection getTrustedCertificates(int level) {

	if (level == 0)
	   return activeTrustedCertsMap.keySet();
	else
	   return activeSysTrustedCertsMap.keySet();
    }

    /* For Https Certificates */
    public Collection getHttpsCertificates(int level) {

	if (level == 0)
           return activeHttpsCertsMap.keySet();
	else
           return activeSysHttpsCertsMap.keySet();
    }

    /* For Root CA */
    public Collection getRootCACertificates(int level) {

	if (level == 0)
           return activeRootCACertsMap.keySet();
	else
           return activeSysRootCACertsMap.keySet();
    }

    /* For Https Root CA */
    public Collection getHttpsRootCACertificates(int level) {

	if (level == 0)
           return activeHttpsRootCACertsMap.keySet();
	else
           return activeSysHttpsRootCACertsMap.keySet();
    }

    /* For Client Authentication Certificate */
    public Collection getClientAuthCertificates(int level) {

	if (level == 0)
	   return activeClientAuthCertsMap.keySet();
	else
	   return activeSysClientAuthCertsMap.keySet();
    }

    /* Remove Trusted certificate */
    public void removeTrustedCertificate(Certificate cert)
    {
	CertStore certStore = new DeploySigningCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            certStore.load();
            certStore.remove(cert);
	    certStore.save();

	    // Remove from active HashMap
	    Object rmCert = activeTrustedCertsMap.remove(cert);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    /* Remove Secure Site certificate */
    public void removeHttpsCertificate(Certificate certHttps)
    {
	CertStore sslCertStore = new DeploySSLCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            sslCertStore.load();
            sslCertStore.remove(certHttps);
	    sslCertStore.save();

	    // Remove from active HashMap
	    Object rmCert = activeHttpsCertsMap.remove(certHttps);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void removeRootCACertificate(Certificate certCA)
    {
	CertStore rootCertStore = RootCertStore.getInstance();

	try {
	    // Load keyStore, remove certificate and save keyStore
            rootCertStore.load();
            rootCertStore.remove(certCA);
	    rootCertStore.save();

	    // Remove from active HashMap
	    Object rmCert = activeRootCACertsMap.remove(certCA);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void removeHttpsRootCACertificate(Certificate certHttpsCA)
    {

	CertStore sslRootCertStore = new SSLRootCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            sslRootCertStore.load();
            sslRootCertStore.remove(certHttpsCA);
	    sslRootCertStore.save();

	    // Remove from active HashMap
	    Object rmCert = activeHttpsRootCACertsMap.remove(certHttpsCA);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void removeClientAuthCertificate(JDialog inParent, Certificate[] certClientAuth)
    {
	CertStore clientAuthCertStore = new DeployClientAuthCertStore(inParent);

	try {
	    // Load keyStore, remove certificate and save keyStore
            clientAuthCertStore.load();
            boolean result = clientAuthCertStore.remove(certClientAuth[0]);

	    // Remove from active HashMap
	    if (result)
	       activeClientAuthCertsMap.remove(certClientAuth);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    /* Add Trusted certificate */
    public void addTrustedCertificate(Certificate cert)
    {
	CertStore certStore = new DeploySigningCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            certStore.load();
            certStore.add(cert);
	    certStore.save();

	    // Add to active HashMap
            activeTrustedCertsMap.put(cert, cert);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void addHttpsCertificate(Certificate cert)
    {
	CertStore certStore = new DeploySSLCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            certStore.load();
            certStore.add(cert);
	    certStore.save();

	    // Add to active HashMap
            activeHttpsCertsMap.put(cert, cert);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void addCACertificate(Certificate cert)
    {
	CertStore rootCertStore = RootCertStore.getInstance();

	try {
	    // Load keyStore, add certificate and save keyStore
            rootCertStore.load();
            rootCertStore.add(cert);
	    rootCertStore.save();

	    // Add to active HashMap
            activeRootCACertsMap.put(cert, cert);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void addHttpsCACertificate(Certificate cert)
    {
	CertStore sslRootCertStore = new SSLRootCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            sslRootCertStore.load();
            sslRootCertStore.add(cert);
	    sslRootCertStore.save();

	    // Add to active HashMap
            activeHttpsRootCACertsMap.put(cert, cert);
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void addClientAuthCertChain(JDialog inParent, Certificate[] certChain, Key newKey)
    {
	DeployClientAuthCertStore clientAuthCertStore = new DeployClientAuthCertStore(inParent);

	try {
	    // Load keyStore, add certificate and save keyStore
            clientAuthCertStore.load();

	    if (clientAuthCertStore.contains(certChain[0]) == false)
	    {
               boolean result = clientAuthCertStore.addCertKey(certChain, newKey);

	       if (result)
	       {
	          clientAuthCertStore.save();

	          // Add to active HashMap
                  activeClientAuthCertsMap.put(certChain, certChain);
	       }
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    /**
     * Read all the information from the configuration storage
     */
    public void reset() {

	//////////////////////////////////////////////////////////////////////////
    	// Load the certificates from the certificate store

	// Clear the certificate maps
	activeTrustedCertsMap.clear();
        activeHttpsCertsMap.clear();
        activeRootCACertsMap.clear();
        activeHttpsRootCACertsMap.clear();
        activeClientAuthCertsMap.clear();

	activeSysTrustedCertsMap.clear();
        activeSysHttpsCertsMap.clear();
        activeSysRootCACertsMap.clear();
        activeSysHttpsRootCACertsMap.clear();
        activeSysClientAuthCertsMap.clear();

	// Load signing cert store
	try
	{
	    // Open and load certificate store
	    CertStore signingCertStore = new DeploySigningCertStore();
	    signingCertStore.load();

	    // Obtain certificate iterator
	    Iterator iter = signingCertStore.iterator(0);
	    Iterator iterSys = signingCertStore.iterator(1);

	    while (iter.hasNext()) {
		X509Certificate cert = (X509Certificate) iter.next();
		activeTrustedCertsMap.put(cert, cert);
	    }

	    while (iterSys.hasNext()) {
		X509Certificate cert = (X509Certificate) iterSys.next();
		activeSysTrustedCertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load SSL cert store
	try
	{
	    CertStore sslCertStore = new DeploySSLCertStore();
	    sslCertStore.load();

	    Iterator iterHttps = sslCertStore.iterator(0);
	    Iterator iterHttpsSys = sslCertStore.iterator(1);

	    while (iterHttps.hasNext()) {
                X509Certificate certHttps = (X509Certificate) iterHttps.next();
                activeHttpsCertsMap.put(certHttps, certHttps);
            }

	    while (iterHttpsSys.hasNext()) {
                X509Certificate certHttps = (X509Certificate) iterHttpsSys.next();
                activeSysHttpsCertsMap.put(certHttps, certHttps);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load root cert store
	try
	{
            CertStore rootCertStore = RootCertStore.getInstance();
            rootCertStore.load();

            Iterator iterCA = rootCertStore.iterator(0);
            Iterator iterCASys = rootCertStore.iterator(1);

            while (iterCA.hasNext()) {
                X509Certificate rootCAcert = (X509Certificate) iterCA.next();
                activeRootCACertsMap.put(rootCAcert, rootCAcert);
            }

            while (iterCASys.hasNext()) {
                X509Certificate rootCAcert = (X509Certificate) iterCASys.next();
                activeSysRootCACertsMap.put(rootCAcert, rootCAcert);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load SSL root cert store
	try
	{
            CertStore sslRootCertStore = new SSLRootCertStore();
            sslRootCertStore.load();

            Iterator iterHttpsCA = sslRootCertStore.iterator(0);
            Iterator iterHttpsCASys = sslRootCertStore.iterator(1);

            while (iterHttpsCA.hasNext()) {
                X509Certificate HttpsRootCAcert = (X509Certificate) iterHttpsCA.next();
                activeHttpsRootCACertsMap.put(HttpsRootCAcert, HttpsRootCAcert);
            }

            while (iterHttpsCASys.hasNext()) {
                X509Certificate HttpsRootCAcert = (X509Certificate) iterHttpsCASys.next();
                activeSysHttpsRootCACertsMap.put(HttpsRootCAcert, HttpsRootCAcert);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load client authentication cert store
	try
	{
	    DeployClientAuthCertStore clientAuthCertStore = new DeployClientAuthCertStore();
	    clientAuthCertStore.load();

	    // Obtain certificate iterator
            Iterator iterClientAuth = clientAuthCertStore.iteratorChain(0);
            Iterator iterClientAuthSys = clientAuthCertStore.iteratorChain(1);

	    while (iterClientAuth.hasNext()) {
	 	Certificate[] ClientAuthcert = (Certificate[]) iterClientAuth.next();
		activeClientAuthCertsMap.put(ClientAuthcert, ClientAuthcert);
	    }

	    while (iterClientAuthSys.hasNext()) {
	 	Certificate[] ClientAuthcert = (Certificate[]) iterClientAuthSys.next();
		activeSysClientAuthCertsMap.put(ClientAuthcert, ClientAuthcert);
	    }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}
    }

   /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
        return com.sun.deploy.resources.ResourceManager.getMessage(key);
    }

    // For RSA signing support in Win32 and UNIX
    private LinkedHashMap   activeTrustedCertsMap = new LinkedHashMap();
    private LinkedHashMap   activeHttpsCertsMap = new LinkedHashMap();
    private LinkedHashMap   activeRootCACertsMap = new LinkedHashMap();
    private LinkedHashMap   activeHttpsRootCACertsMap = new LinkedHashMap();
    private LinkedHashMap   activeClientAuthCertsMap = new LinkedHashMap();

    private LinkedHashMap   activeSysTrustedCertsMap = new LinkedHashMap();
    private LinkedHashMap   activeSysHttpsCertsMap = new LinkedHashMap();
    private LinkedHashMap   activeSysRootCACertsMap = new LinkedHashMap();
    private LinkedHashMap   activeSysHttpsRootCACertsMap = new LinkedHashMap();
    private LinkedHashMap   activeSysClientAuthCertsMap = new LinkedHashMap();
}
