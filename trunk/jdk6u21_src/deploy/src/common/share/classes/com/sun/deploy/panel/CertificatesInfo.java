/*
 * @(#)CertificateesInfo.java	1.50 03/03/25
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
	CertStore userCertStore = DeploySigningCertStore.getUserCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            userCertStore.load(true);
            if (userCertStore.remove(cert)) {
	       userCertStore.save();

	       // Remove from active HashMap
	       Object rmCert = activeTrustedCertsMap.remove(cert);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    /* Remove Secure Site certificate */
    public void removeHttpsCertificate(Certificate certHttps)
    {
	CertStore userSSLCertStore = DeploySSLCertStore.getUserCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            userSSLCertStore.load(true);
            if (userSSLCertStore.remove(certHttps)) {
	       userSSLCertStore.save();

	       // Remove from active HashMap
	       Object rmCert = activeHttpsCertsMap.remove(certHttps);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void removeRootCACertificate(Certificate certCA)
    {
	CertStore userRootCertStore = RootCertStore.getUserCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            userRootCertStore.load(true);
            if (userRootCertStore.remove(certCA)) {
	       userRootCertStore.save();

	       // Remove from active HashMap
	       Object rmCert = activeRootCACertsMap.remove(certCA);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void removeHttpsRootCACertificate(Certificate certHttpsCA)
    {

	CertStore userSSLRootCertStore = SSLRootCertStore.getUserCertStore();

	try {
	    // Load keyStore, remove certificate and save keyStore
            userSSLRootCertStore.load(true);
            if (userSSLRootCertStore.remove(certHttpsCA)) {
	       userSSLRootCertStore.save();

	       // Remove from active HashMap
	       Object rmCert = activeHttpsRootCACertsMap.remove(certHttpsCA);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void removeClientAuthCertificate(JDialog inParent, Certificate[][] certClientAuth)
    {
	DeployClientAuthCertStore clientAuthCertStore = 
			DeployClientAuthCertStore.getUserCertStore(inParent);
        Certificate[] certAlias = new Certificate[certClientAuth.length];
        
	try {
	    // Load keyStore, remove certificate(s) and save keyStore 
            clientAuthCertStore.load(true);
            
            // The CertStore uses the 1st element of the certchain array as an alias 
            // to the cert.  Build a list of the certs to be removed from the store.
            for( int j=0; j<certClientAuth.length; j++) {
                certAlias[j] = certClientAuth[j][0];
            }
            
            // Remove the certificate(s) from the CertStore
            boolean result = clientAuthCertStore.remove(certAlias);

	    // Remove the certs from the active HashMap
	    if (result) {
	       clientAuthCertStore.save();

               for( int i=0; i<certClientAuth.length; i++) {
	           activeClientAuthCertsMap.remove(certClientAuth[i]);
               }
            }
	}
	catch (Exception e)
        {
	    Trace.printException(inParent, e);
	}
    }

    /* Add Trusted certificate */
    public void addTrustedCertificate(Certificate cert)
    {
	CertStore userCertStore = DeploySigningCertStore.getUserCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            userCertStore.load(true);
            if (userCertStore.add(cert)) {
	       userCertStore.save();

	       // Add to active HashMap
               activeTrustedCertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }

    public void addHttpsCertificate(Certificate cert)
    {
	CertStore userSSLCertStore = DeploySSLCertStore.getUserCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            userSSLCertStore.load(true);
            if (userSSLCertStore.add(cert)) {
	       userSSLCertStore.save();

	       // Add to active HashMap
               activeHttpsCertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void addCACertificate(Certificate cert)
    {
	CertStore userRootCertStore = RootCertStore.getUserCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            userRootCertStore.load(true);
            if (userRootCertStore.add(cert)) {
	       userRootCertStore.save();

	       // Add to active HashMap
               activeRootCACertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void addHttpsCACertificate(Certificate cert)
    {
	CertStore userSSLRootCertStore = SSLRootCertStore.getUserCertStore();

	try {
	    // Load keyStore, add certificate and save keyStore
            userSSLRootCertStore.load(true);
            if (userSSLRootCertStore.add(cert)) {
	       userSSLRootCertStore.save();

	       // Add to active HashMap
               activeHttpsRootCACertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
        {
	    Trace.printException(e);
	}
    }
    
    public void addClientAuthCertChain(JDialog inParent, Certificate[] certChain, Key newKey)
    {
	DeployClientAuthCertStore clientAuthCertStore = 
				DeployClientAuthCertStore.getUserCertStore(inParent);

	try {
	    // Load keyStore, add certificate and save keyStore
            clientAuthCertStore.load(true);

            boolean result = clientAuthCertStore.addCertKey(certChain, newKey);

	    if (result)
	    {
	       clientAuthCertStore.save();

	       // Add to active HashMap
               activeClientAuthCertsMap.put(certChain, certChain);
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

	// Load user signing cert store
	CertStore userSigningCertStore = DeploySigningCertStore.getUserCertStore();
	try
	{
	    // Open and load certificate store
	    userSigningCertStore.load();

	    // Obtain certificate iterator
	    Iterator iter = userSigningCertStore.getCertificates().iterator();
	    while (iter.hasNext()) {
		X509Certificate cert = (X509Certificate) iter.next();
		activeTrustedCertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load system signing cert store
	CertStore systemSigningCertStore = DeploySigningCertStore.getSystemCertStore();
	try
	{
	    // Open and load certificate store
	    systemSigningCertStore.load();

	    Iterator iterSys = systemSigningCertStore.getCertificates().iterator();
	    while (iterSys.hasNext()) {
		X509Certificate cert = (X509Certificate) iterSys.next();
		activeSysTrustedCertsMap.put(cert, cert);
	    }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load user SSL cert store
	CertStore userSSLCertStore = DeploySSLCertStore.getUserCertStore();
	try
	{
	    userSSLCertStore.load();

	    Iterator iterHttps = userSSLCertStore.getCertificates().iterator();
	    while (iterHttps.hasNext()) {
                X509Certificate certHttps = (X509Certificate) iterHttps.next();
                activeHttpsCertsMap.put(certHttps, certHttps);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load system SSL cert store
	CertStore systemSSLCertStore = DeploySSLCertStore.getSystemCertStore();
	try
	{
	    systemSSLCertStore.load();

	    Iterator iterHttpsSys = systemSSLCertStore.getCertificates().iterator();
	    while (iterHttpsSys.hasNext()) {
                X509Certificate certHttps = (X509Certificate) iterHttpsSys.next();
                activeSysHttpsCertsMap.put(certHttps, certHttps);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load user root cert store
        CertStore userRootCertStore = RootCertStore.getUserCertStore();
	try
	{
            userRootCertStore.load();

            Iterator iterCA = userRootCertStore.getCertificates().iterator();
            while (iterCA.hasNext()) {
                X509Certificate rootCAcert = (X509Certificate) iterCA.next();
                activeRootCACertsMap.put(rootCAcert, rootCAcert);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load system root cert store
        CertStore systemRootCertStore = RootCertStore.getSystemCertStore();
	try
	{
            systemRootCertStore.load();

            Iterator iterCASys = systemRootCertStore.getCertificates().iterator();
            while (iterCASys.hasNext()) {
                X509Certificate rootCAcert = (X509Certificate) iterCASys.next();
                activeSysRootCACertsMap.put(rootCAcert, rootCAcert);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load user SSL root cert store
        CertStore userSSLRootCertStore = SSLRootCertStore.getUserCertStore();
	try
	{
            userSSLRootCertStore.load();

            Iterator iterHttpsCA = userSSLRootCertStore.getCertificates().iterator();
            while (iterHttpsCA.hasNext()) {
                X509Certificate HttpsRootCAcert = (X509Certificate) iterHttpsCA.next();
                activeHttpsRootCACertsMap.put(HttpsRootCAcert, HttpsRootCAcert);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load system SSL root cert store
        CertStore systemSSLRootCertStore = SSLRootCertStore.getSystemCertStore();
	try
	{
            systemSSLRootCertStore.load();

            Iterator iterHttpsCASys = systemSSLRootCertStore.getCertificates().iterator();
            while (iterHttpsCASys.hasNext()) {
                X509Certificate HttpsRootCAcert = (X509Certificate) iterHttpsCASys.next();
                activeSysHttpsRootCACertsMap.put(HttpsRootCAcert, HttpsRootCAcert);
            }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load user client authentication cert store
	CertStore userClientAuthCertStore = 
				DeployClientAuthCertStore.getUserCertStore(null);
	try
	{
	    userClientAuthCertStore.load();

	    // Obtain certificate iterator
            Iterator iterClientAuth = userClientAuthCertStore.getCertificates().iterator();
	    while (iterClientAuth.hasNext()) {
	 	Certificate[] ClientAuthcert = (Certificate[]) iterClientAuth.next();
		activeClientAuthCertsMap.put(ClientAuthcert, ClientAuthcert);
	    }
	}
	catch (Exception e)
	{
	    Trace.printException(e);
	}

	// Load system client authentication cert store
	CertStore systemClientAuthCertStore = 
				DeployClientAuthCertStore.getSystemCertStore(null);
	try
	{
	    systemClientAuthCertStore.load();

            Iterator iterClientAuthSys = systemClientAuthCertStore.getCertificates().iterator();
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
