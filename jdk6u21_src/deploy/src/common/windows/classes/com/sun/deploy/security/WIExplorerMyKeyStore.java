/*
 * @(#)WIExplorerMyKeyStore.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.Key;
import java.security.PrivateKey;
import java.security.KeyStoreSpi;
import java.security.KeyStoreException;
import java.security.UnrecoverableKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.util.Date;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Iterator;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;

/**
 * This class provides the keystore implementation referred to as "WIExplorerMyKeyStore".
 *
 */
public final class WIExplorerMyKeyStore extends KeyStoreSpi 
{
    class KeyEntry 
    {
	private MSCryptoPrivateKey privateKey;
	private X509Certificate certChain[];

	KeyEntry(MSCryptoPrivateKey key, X509Certificate[] chain)
	{
	    this.privateKey = key;
	    this.certChain = chain;
	}

	/**
	 * Return alias given a WIExplorer private key.
	 */
	String getAlias()
	{
	    return privateKey.toString();
	}

	/**
	 * Return private key
	 */
	Key getPrivateKey()
	{
	    return privateKey;
	}

	/**
	 * Return certificate chain
	 */
	X509Certificate[] getCertificateChain()
	{
	    return certChain;  
	}
    };

    private Collection keyEntries = new ArrayList();

    /**
     * Returns the key associated with the given alias, using the given
     * password to recover it.
     *
     * @param alias the alias name
     * @param password the password for recovering the key
     *
     * @return the requested key, or null if the given alias does not exist
     * or does not identify a <i>key entry</i>.
     *
     * @exception NoSuchAlgorithmException if the algorithm for recovering the
     * key cannot be found
     * @exception UnrecoverableKeyException if the key cannot be recovered
     * (e.g., the given password is wrong).
     */
    public Key engineGetKey(String alias, char[] password)
	throws NoSuchAlgorithmException, UnrecoverableKeyException
    {
	if (password != null && password.length > 0)
	    throw new UnrecoverableKeyException("Password is not required for WIExplorer MY keystore.");

	if (engineIsKeyEntry(alias) == false)
	    return null;

	for (Iterator iter = keyEntries.iterator(); iter.hasNext(); )
	{
	    KeyEntry entry = (KeyEntry) iter.next();

	    if (alias.equals(entry.getAlias()))
		return entry.getPrivateKey();
	}

	return null;
    }

    /**
     * Returns the certificate chain associated with the given alias.
     *
     * @param alias the alias name
     *
     * @return the certificate chain (ordered with the user's certificate first
     * and the root certificate authority last), or null if the given alias
     * does not exist or does not contain a certificate chain (i.e., the given 
     * alias identifies either a <i>trusted certificate entry</i> or a
     * <i>key entry</i> without a certificate chain).
     */
    public Certificate[] engineGetCertificateChain(String alias) 
    {
	for (Iterator iter = keyEntries.iterator(); iter.hasNext(); )
	{
	    KeyEntry entry = (KeyEntry) iter.next();

	    if (alias.equals(entry.getAlias()))
	    {
		X509Certificate[] certChain = entry.getCertificateChain();
		X509Certificate[] tmp = new X509Certificate[certChain.length];
		System.arraycopy(certChain, 0, tmp, 0, certChain.length);
		return tmp;
	    }
	}

	return null;
    }

    /**
     * Returns the certificate associated with the given alias.
     *
     * <p>If the given alias name identifies a
     * <i>trusted certificate entry</i>, the certificate associated with that
     * entry is returned. If the given alias name identifies a
     * <i>key entry</i>, the first element of the certificate chain of that
     * entry is returned, or null if that entry does not have a certificate
     * chain.
     *
     * @param alias the alias name
     *
     * @return the certificate, or null if the given alias does not exist or
     * does not contain a certificate.
     */
    public Certificate engineGetCertificate(String alias) 
    {
	for (Iterator iter = keyEntries.iterator(); iter.hasNext(); )
	{
	    KeyEntry entry = (KeyEntry) iter.next();

	    if (alias.equals(entry.getAlias()))
	    {
		X509Certificate[] certChain = entry.getCertificateChain();
		return certChain[0];
	    }
	}

	return null;
    }	

    /**
     * Returns the creation date of the entry identified by the given alias.
     *
     * @param alias the alias name
     *
     * @return the creation date of this entry, or null if the given alias does
     * not exist
     */
    public Date engineGetCreationDate(String alias) {
	return new Date();
    }

    /**
     * Assigns the given private key to the given alias, protecting 
     * it with the given password as defined in PKCS8.
     *
     * <p>The given java.security.PrivateKey <code>key</code> must
     * be accompanied by a certificate chain certifying the
     * corresponding public key.
     *
     * <p>If the given alias already exists, the keystore information
     * associated with it is overridden by the given key and certificate
     * chain.
     *
     * @param alias the alias name
     * @param key the private key to be associated with the alias
     * @param password the password to protect the key
     * @param chain the certificate chain for the corresponding public
     * key (only required if the given key is of type
     * <code>java.security.PrivateKey</code>).
     *
     * @exception KeyStoreException if the given key is not a private key, 
     * cannot be protected, or this operation fails for some other reason
     */
    public void engineSetKeyEntry(String alias, Key key, char[] password,
				  Certificate[] chain)
	throws KeyStoreException
    {
	throw new KeyStoreException("Cannot assign the given key to the given alias.");
    }

    /**
     * Assigns the given key (that has already been protected) to the given
     * alias.
     * 
     * <p>If the protected key is of type
     * <code>java.security.PrivateKey</code>, it must be accompanied by a
     * certificate chain certifying the corresponding public key. If the
     * underlying keystore implementation is of type <code>jks</code>,
     * <code>key</code> must be encoded as an
     * <code>EncryptedPrivateKeyInfo</code> as defined in the PKCS #8 standard.
     *
     * <p>If the given alias already exists, the keystore information
     * associated with it is overridden by the given key (and possibly
     * certificate chain).
     *
     * @param alias the alias name
     * @param key the key (in protected format) to be associated with the alias
     * @param chain the certificate chain for the corresponding public
     * key (only useful if the protected key is of type
     * <code>java.security.PrivateKey</code>).
     *
     * @exception KeyStoreException if this operation fails.
     */
    public void engineSetKeyEntry(String alias, byte[] key,
				  Certificate[] chain)
	throws KeyStoreException
    {
	throw new KeyStoreException("Cannot assign the given key to the given alias.");
    }

    /**
     * Assigns the given certificate to the given alias.
     *
     * <p>If the given alias already exists in this keystore and identifies a
     * <i>trusted certificate entry</i>, the certificate associated with it is
     * overridden by the given certificate.
     *
     * @param alias the alias name
     * @param cert the certificate
     *
     * @exception KeyStoreException if the given alias already exists and does
     * not identify a <i>trusted certificate entry</i>, or this operation
     * fails for some other reason.
     */
    public void engineSetCertificateEntry(String alias, Certificate cert)
	throws KeyStoreException
    {
	throw new KeyStoreException("Cannot assign the given certificate to the given alias.");
    }

    /**
     * Deletes the entry identified by the given alias from this keystore.
     *
     * @param alias the alias name
     *
     * @exception KeyStoreException if the entry cannot be removed.
     */
    public void engineDeleteEntry(String alias)
	throws KeyStoreException
    {
	throw new KeyStoreException("WIExplorer does not support alias removal.");
    }

    /**
     * Lists all the alias names of this keystore.
     *
     * @return enumeration of the alias names
     */
    public Enumeration engineAliases() {

	final Iterator iter = keyEntries.iterator();

	return new Enumeration() 
	{
	    public boolean hasMoreElements()
	    {
		return iter.hasNext();
	    }

	    public Object nextElement()
	    {
		KeyEntry entry = (KeyEntry) iter.next();
		return entry.getAlias();
	    }    
	};
    }

    /**
     * Checks if the given alias exists in this keystore.
     *
     * @param alias the alias name
     *
     * @return true if the alias exists, false otherwise
     */
    public boolean engineContainsAlias(String alias) {
	for (Enumeration enumerator = engineAliases(); enumerator.hasMoreElements();)
	{
	    String a = (String) enumerator.nextElement();

	    if (a.equals(alias))
		return true;
	}
	return false;
    }

    /**
     * Retrieves the number of entries in this keystore.
     *
     * @return the number of entries in this keystore
     */
    public int engineSize() {
	return keyEntries.size();
    }

    /**
     * Returns true if the entry identified by the given alias is a
     * <i>key entry</i>, and false otherwise.
     *
     * @return true if the entry identified by the given alias is a
     * <i>key entry</i>, false otherwise.
     */
    public boolean engineIsKeyEntry(String alias) {
	return alias.startsWith("MSCryptoRSAPrivateKey") || alias.startsWith("MSCryptoDSAPrivateKey");
    }

    /**
     * Returns true if the entry identified by the given alias is a
     * <i>trusted certificate entry</i>, and false otherwise.
     *
     * @return true if the entry identified by the given alias is a
     * <i>trusted certificate entry</i>, false otherwise.
     */
    public boolean engineIsCertificateEntry(String alias) 
    {
	return false;
    }

    /**
     * Returns the (alias) name of the first keystore entry whose certificate
     * matches the given certificate.
     *
     * <p>This method attempts to match the given certificate with each
     * keystore entry. If the entry being considered
     * is a <i>trusted certificate entry</i>, the given certificate is
     * compared to that entry's certificate. If the entry being considered is
     * a <i>key entry</i>, the given certificate is compared to the first
     * element of that entry's certificate chain (if a chain exists).
     *
     * @param cert the certificate to match with.
     *
     * @return the (alias) name of the first entry with matching certificate,
     * or null if no such entry exists in this keystore.
     */
    public String engineGetCertificateAlias(Certificate cert) 
    {
	for (Iterator iter = keyEntries.iterator(); iter.hasNext(); )
	{
	    KeyEntry entry = (KeyEntry) iter.next();

	    if (entry.certChain != null && entry.certChain[0].equals(cert))
		return entry.getAlias();
	}
	
	return null;
    }

    /**
     * Stores this keystore to the given output stream, and protects its
     * integrity with the given password.
     *
     * @param stream the output stream to which this keystore is written.
     * @param password the password to generate the keystore integrity check
     *
     * @exception IOException if there was an I/O problem with data
     * @exception NoSuchAlgorithmException if the appropriate data integrity
     * algorithm could not be found
     * @exception CertificateException if any of the certificates included in
     * the keystore data could not be stored
     */
    public void engineStore(OutputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	throw new IOException("WIExplorer cert store cannot be stored into stream.");
    }

    /**
     * Loads the keystore from the given input stream.
     *
     * <p>If a password is given, it is used to check the integrity of the
     * keystore data. Otherwise, the integrity of the keystore is not checked.
     *
     * @param stream the input stream from which the keystore is loaded
     * @param password the (optional) password used to check the integrity of
     * the keystore.
     *
     * @exception IOException if there is an I/O or format problem with the
     * keystore data
     * @exception NoSuchAlgorithmException if the algorithm used to check
     * the integrity of the keystore cannot be found
     * @exception CertificateException if any of the certificates in the
     * keystore could not be loaded
     */
    public void engineLoad(InputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	// security check
	SecurityManager sm = System.getSecurityManager();
	if (sm != null) 
	    sm.checkPermission(new java.security.SecurityPermission("authProvider.SunDeploy-MSCrypto"));
	    
	if (stream != null)
	    throw new IOException("WIExplorer cert store cannot be loaded from stream.");

	// Clear all key entries
	keyEntries.clear();

	// Load client authentication certificate chains from browsers
	loadKeysAndCertificateChains(getName(), keyEntries);
    }

    /**
     * Load keys and certificates from IE cert store into Collection.
     *
     * @param name Name of cert store.
     * @param extendedKeyUsageOIDFilter EKU OID filters
     * @param privateKeyRequired Certificate must have an associated private key
     * @param keyEntries Collection of key/certificate.
     */
    private native void loadKeysAndCertificateChains(String name, Collection keyEntries);

    /**
     * Generates RSA key and certificate chain from the private key handle, collection of 
     * certificates and stores the result into key entries.
     */
    private void generateRSAKeyAndCertificateChain(int hCryptProv, int hCryptKey, int keyLength, Collection certCollection, Collection keyEntries)
    {
	try
	{
	    X509Certificate[] certChain = new X509Certificate[certCollection.size()];

	    int i = 0;
	    for (Iterator iter=certCollection.iterator(); iter.hasNext(); i++)
	    {
		certChain[i] = (X509Certificate) iter.next();
	    }

	    KeyEntry entry = new KeyEntry(new MSCryptoRSAPrivateKey(hCryptProv, hCryptKey, keyLength), certChain);

	    // Add cert chain
	    keyEntries.add(entry);
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	}
    }

    /**
     * Generates DSA key and certificate chain from the private key handle, collection of 
     * certificates and stores the result into key entries.
     */
    private void generateDSAKeyAndCertificateChain(int hCryptProv, int hCryptKey, int keyLength, Collection certCollection, Collection keyEntries)
    {
	try
	{
	    X509Certificate[] certChain = new X509Certificate[certCollection.size()];

	    int i = 0;
	    for (Iterator iter=certCollection.iterator(); iter.hasNext(); i++)
	    {
		certChain[i] = (X509Certificate) iter.next();
	    }

	    KeyEntry entry = new KeyEntry(new MSCryptoDSAPrivateKey(hCryptProv, hCryptKey, keyLength), certChain);

	    // Add cert chain
	    keyEntries.add(entry);
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	}
    }

    /**
     * Generates certificates from byte data and stores into cert collection.
     *
     * @param data Byte data.
     * @param certCollection Collection of certificates.
     */
    private void generateCertificate(byte[] data, Collection certCollection) 
    {
	try
	{
	    ByteArrayInputStream bis = new ByteArrayInputStream(data);

	    // Obtain certificate factory
	    CertificateFactory cf = CertificateFactory.getInstance("X.509");

	    // Generate certificate
	    Collection c = cf.generateCertificates(bis);
	    Iterator i = c.iterator();
	    while (i.hasNext()) 
	    {
		X509Certificate cert = (X509Certificate)i.next();

		certCollection.add(cert);
	    }
	}
	catch (CertificateException e)
	{
	    e.printStackTrace();
	}
	catch (Throwable te)
	{
	    te.printStackTrace();
	}
    }

    /**
     * Return name of the Internet Explorer cert store.
     */
    protected String getName()
    {
	return "MY";
    }
}
