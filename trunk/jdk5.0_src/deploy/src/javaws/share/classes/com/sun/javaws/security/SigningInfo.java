/*
 * @(#)SigningInfo.java	1.29 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;
import java.io.InputStream;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.File;
import java.net.URL;
import java.util.*;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.jar.Attributes;
import java.security.CodeSource;
import com.sun.javaws.Globals;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.exceptions.JARSigningException;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.exceptions.FailedDownloadingResourceException;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

public class SigningInfo {

    /** Scan a jar file for certificates, and returns it if found. Throws signing
     *  exception if not found
     *
     *  This checks that all signed entries JAR file can be verified, and that
     *  all entries (except the ones in META-INF) are signed.
     */
    static public java.security.cert.Certificate[] checkSigning(URL location,
								String versionId,
								JarFile jf,
								DownloadProtocol.DownloadDelegate delegate,
								File extractDir)
        throws JARSigningException {

        
        // Check signing of all entries
        java.security.cert.Certificate[] certChain = null;
        boolean hasSignedEntry = false;
        boolean hasUnsignedEntry = false;
        String msg = null;
        int total = jf.size();
        int count = 0;
        
        // Notify loading delegate
        if (delegate != null) delegate.validating(location, 0, total);
        BufferedOutputStream bos = null;
        InputStream is = null;
        try {
	    byte[] buffer = new byte[32 * 1024];
	    Enumeration entries = jf.entries();
	    while(entries.hasMoreElements()) {
		JarEntry je = (JarEntry)entries.nextElement();
		String name = je.getName();
		// Entries in META-INF, directories, and zero-length files are not signed,
		// so skip those
		if (!name.startsWith("META-INF/") && !name.endsWith("/") && je.getSize() != 0) {
		    is = jf.getInputStream(je);
		    int n;
		    
		    if (extractDir != null && name.indexOf("/") == -1) {
			File extractFile = new File(extractDir, je.getName());
			bos = new BufferedOutputStream(new FileOutputStream(extractFile));
		    }
		    
		    while((n = is.read(buffer, 0, buffer.length)) != -1) {
			// Just read. This will throw a security exception if a
			// signature fails. Native libs are extract at the same time
			if (bos!= null) bos.write(buffer, 0, n);
		    }
		    if (bos != null) { bos.close(); bos = null; }
		    is.close(); is = null;
		    
		    java.security.cert.Certificate[] chain = je.getCertificates();
		    if (chain != null && chain.length == 0) chain = null;
		    
		    boolean isSignedEntry = false;
		    if (chain != null) {
			isSignedEntry = true;
			
			if (certChain == null) {
			    certChain = chain;    			    
			} else if (!equalChains(certChain, chain)) {
			    // All entries must be signed by same signer according to spec.
			    throw new JARSigningException(location, versionId, JARSigningException.MULTIPLE_SIGNERS);
			}
		    }		    
		    
		    // The signing property is all or nothing.
		    hasSignedEntry = hasSignedEntry || isSignedEntry;
		    hasUnsignedEntry = hasUnsignedEntry || (!isSignedEntry);
		}
		
		// Notify loading delegate (must notify for each entry since we
		// use this for total)
		if (delegate != null) delegate.validating(location, ++count, total);
	    }
	} catch(SecurityException e) {
	    throw new JARSigningException(location, versionId, JARSigningException.BAD_SIGNING, e);
	} catch(IOException e) {
	    throw new JARSigningException(location, versionId, JARSigningException.BAD_SIGNING, e);
	} finally {
	    try {
		if (bos != null) bos.close();
		if (is != null) is.close();
	    } catch(IOException ioe) { Trace.ignoredException(ioe); }
	}
	
	if (hasSignedEntry) {
	    // Check that all entries were signed
	    if (hasUnsignedEntry) {
		throw new JARSigningException(location, versionId, JARSigningException.UNSIGNED_FILE);
	    }
	}
	
	// If result is OK, still need to check that all files that got
	// signed are actually in the JAR file
	if (certChain != null) {
	    try {
		Manifest mf = jf.getManifest();
		Set entries = mf.getEntries().entrySet();
		Iterator itr = entries.iterator();
		while(itr.hasNext()) {
		    Map.Entry me = (Map.Entry)itr.next();
		    String name = (String)me.getKey();

		    // Make sure name is in the JAR file
		    if (isSignedManifestEntry(mf, name) &&
			jf.getEntry(name) == null) {	
			throw new JARSigningException(location, versionId, JARSigningException.MISSING_ENTRY, name);
		    }
		}
	    } catch(IOException ioe) {
		throw new JARSigningException(location, versionId, JARSigningException.BAD_SIGNING, ioe);
	    }
	}
	
	// Returns signing certificate, or null if JAR file is not signed.
	return certChain;
    }

    /**
     * returns the CodeSource of the first real entry in the jar file
     */
    public static CodeSource getCodeSource(URL url, JarFile jf) {
        Enumeration entries = jf.entries(); 
	byte[] buffer = new byte[32 * 1024];
        while(entries.hasMoreElements()) {
            JarEntry je = (JarEntry)entries.nextElement();
            String name = je.getName();
	    Trace.println(" ... name=" + name, TraceLevel.SECURITY);
            // Entries in META-INF, directories, and zero-length files 
	    // are not signed, so skip those
            if (!name.startsWith("META-INF/") && 
		!name.endsWith("/") && je.getSize() != 0) {
	        try {
                    InputStream is = jf.getInputStream(je);
                    int n;
                    while((n = is.read(buffer, 0, buffer.length)) != -1) { }
                    is.close();
		} catch (IOException ioe) { 
		    Trace.ignoredException(ioe);
		}
		if (Globals.isJavaVersionAtLeast15()) {
		    // new way - with 1.5+ construct CodeSource with CodeSigner
		    return new CodeSource(url, je.getCodeSigners());
		} else {
		    // pre 1.5 jre - construct with just Certificate Chain
		    return new CodeSource(url, je.getCertificates());
		}
            }    
        } 
        return null;
    }

    public static boolean equalChains(java.security.cert.Certificate[] chain1, 
				java.security.cert.Certificate[] chain2) {
        if (chain1.length != chain2.length) {
	    return false;
        }
        for(int i = 0; i < chain1.length; i++) {
            if (!chain1[i].equals(chain2[i])) return false;
        }
        return true;
    }
    

    private static boolean isSignedManifestEntry(Manifest mf, String name) {
        Attributes attr = mf.getAttributes(name);
	if (attr != null) {
            Iterator it = attr.keySet().iterator();
            while (it.hasNext()) {
		// it.next() will return instance of 
		// java.util.jar.Attributes.Name, which cannot be hard cast
		// to String to get the attributes name.  We should use
		// it's toString() to get the attribute name
		String key = it.next().toString();
                key = key.toUpperCase(Locale.ENGLISH);
                if (key.endsWith("-DIGEST") || 
                    (key.indexOf("-DIGEST-") != -1)) {
	            return true;
	        }
	    }
	}
	return false;
    }
    
}

