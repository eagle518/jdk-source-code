/*
 * @(#)CachedJarFile.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.cache;

import java.io.File;
import java.io.IOException;
import java.security.CodeSigner;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import sun.plugin.util.Trace;

/**
 * This class is used to read jar files from the jar cache.
 *
 * @see     java.util.zip.ZipFile
 * @see     java.util.jar.JarEntry
 */
public class CachedJarFile extends JarFile {

    // The signers used to sign this jar file
    private CodeSigner[] signers;

    // Map of entries to their signers
    private HashMap signerMap = null;
    
    // Manifest for this Jar file
    private Manifest manifest = null;

    /**
     * Constructor.  This is protected, since it only makes sense for the
     * caching code to instantiate this class.
     */
    protected CachedJarFile(File file, CodeSigner[] signers,
                            HashMap signerMap, Manifest manifest) 
                            throws IOException {
        // Pass in false for the verify parameter to ensure that the
        // superclass does not attempt to authenticate the Jar file.
        super(file, false);
        this.signers = signers;
        this.signerMap = signerMap;
        this.manifest = manifest;

        //if manLoaded is not set in the super class, whenever
        //getInputStream() is called, getManfiest() is called which
        //is expensive. This overhead is avoided by making a call to 
        //the super.getManifest() which sets the manLoaded flag
        super.getManifest();
    }

    /**
     * Returns the <code>ZipEntry</code> for the given entry name or
     * <code>null</code> if not found.
     *
     * @param  name the JAR file entry name
     * @return the <code>ZipEntry</code> for the given entry name or
     *         <code>null</code> if not found
     * @see    java.util.zip.ZipEntry
     */
    public ZipEntry getEntry(String name) {
        ZipEntry ze = super.getEntry(name);
        if (ze != null) {
            return new JarFileEntry(ze);
        }
        return null;
    }

    /**
     * Returns an enumeration of the entries in this JAR file.
     *
     * @return an <code>Enumeration</code> of the entries in this JAR file
     * @see    java.util.Enumeration
     */
    public Enumeration entries() {
        final Enumeration entryList = super.entries();
        return new Enumeration() {
            public boolean hasMoreElements() {
                return entryList.hasMoreElements();
            }
            public Object nextElement() {
                ZipEntry ze = (ZipEntry)entryList.nextElement();
                return new JarFileEntry(ze);
            }
        };
    }


    /**
     * Returns the JAR file manifest, or <code>null</code> if none.
     *
     * @return the JAR file manifest, or <code>null</code> if none
     */
    public Manifest getManifest() throws IOException {
	Manifest copy = null;

	if(manifest != null) {
	    // Return a copy of the manifest for security reasons
	    copy = new Manifest();
	    Attributes copyAttr = copy.getMainAttributes();
	    copyAttr.putAll((Map)manifest.getMainAttributes().clone());
	    Map entries = manifest.getEntries();
	    if (entries != null) {
		Map copyEntries = copy.getEntries();
		Iterator it = entries.keySet().iterator();
		while (it.hasNext()) {
		    Object key = it.next();
		    Attributes at = (Attributes)entries.get(key);
		    copyEntries.put(key, at.clone());
		}
	    }
	}

	return copy;
    }


    // Private class to represent an entry in a cached JAR file
    private class JarFileEntry extends JarEntry {
        JarFileEntry(ZipEntry ze) {
            super(ze);
        }

        public Attributes getAttributes() throws IOException {

            // Get the entry's attributes from the JAR manifest
            if (manifest != null) {
                return (Attributes)manifest.getAttributes(getName()).clone();
            } else {
                return null;
            }
        }

        public Certificate[] getCertificates() {
            Certificate[] certs = null;
	    int[] signerIndices = getSignerIndices();

	    if (signerIndices != null) {
		ArrayList certChains = new ArrayList();
		for (int i = 0; i < signerIndices.length; i++) {
		    certChains.addAll(
			signers[signerIndices[i]]
			    .getSignerCertPath().getCertificates());
		}

		// Convert into a Certificate[]
		return (Certificate[]) certChains.toArray(
			new Certificate[certChains.size()]);
	    }
            return certs;
	}

	public CodeSigner[] getCodeSigners() {
	    CodeSigner[] codeSigners = null;
	    int[] signerIndices = getSignerIndices();

	    if (signerIndices != null) {
		// Create an array of code signers
		codeSigners = new CodeSigner[signerIndices.length];

		for (int i = 0; i < signerIndices.length; i++) {
		    codeSigners[i] = signers[signerIndices[i]];
		}
	    }
	    return codeSigners;
	}

	private int[] getSignerIndices() {
	    if (signerMap != null && !signerMap.isEmpty()) {
		// Find this entry in the signer map
		return (int[]) signerMap.get(getName());
	    }
	    return null;
	}
    }
}
