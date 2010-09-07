/*
 * @(#)CachedJarFile14.java	1.15 10/04/16
 *
 * Copyright (c) 2008, 2010 Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.cache;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.io.File;
import java.io.IOException;
import java.lang.ref.Reference;
import java.lang.ref.SoftReference;
import java.net.URL;
import java.security.cert.Certificate;
import java.security.CodeSource;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;

/**
 * This class is used to read jar files from the jar cache.
 *
 * @see     java.util.zip.ZipFile
 * @see     java.util.jar.JarEntry
 */
public class CachedJarFile14 extends JarFile {

    // The certificates used to sign this jar file
    private Reference /* Certificate[] */ certificatesRef;

    // Map of entries to their signers
    private Reference /* Map */ signerMapCertRef = null;
    private boolean hasStrictSingleSigning;
    
    // Manifest for this Jar file
    private Reference /* Manifest */ manifestRef = null;

    // The code sources of this jar file
    private Reference /* CodeSource[] */ codeSourcesRef;

    // Map of signer indices to code sources
    private Reference /* <Map> */ codeSourceCertCacheRef = null;

    //key to reconstruct CacheEntry if needed
    private String resourceURL;
    private File indexFile;
   
    public String getName() {
        String name = super.getName();

        SecurityManager sm = System.getSecurityManager();

        if (sm == null) {
            // there is no security manager, return the cached JAR filepath
            return name;
        }

        try {
            sm.checkPermission(new RuntimePermission("accessDeploymentCache"));
            // return cached JAR filepath if security check allows
            return name;
        } catch (SecurityException se) {
            // safely ignored, we won't return the cached JAR filepath
        }

        // do not expose cached JAR file path
        return "";
    }


    /* clonned copy must be another instance, otherwise 
       jar file can be closed by one of the objects while still 
       needed by another (see 6340856).
       However, we only need to reopen file, it does not seem 
       we need to create new copy of manifest of signerMap */
    public Object clone() throws CloneNotSupportedException {
        try {
            return new CachedJarFile14(new File(super.getName()),
                certificatesRef, signerMapCertRef, hasStrictSingleSigning, manifestRef,
		codeSourcesRef, codeSourceCertCacheRef, resourceURL, indexFile);
        } catch (IOException ioe) {
            throw new CloneNotSupportedException();
        }
    }
    /* need this only for clone() */
    private CachedJarFile14(File file, Reference /* Certificate[] */ certificatesRef,
                          Reference /* <Map> */ signerMapCertRef,
			  boolean hasStrictSingleSigning,
                          Reference /* <Manifest> */ manifestRef,
                          Reference /* <CodeSource[]> */ codeSourcesRef,
                          Reference /* <Map> */ codeSourceCertCacheRef,
                          String resourceURL,
			  File indexFile) 
                          throws IOException {
        // Pass in false for the verify parameter to ensure that the
        // superclass does not attempt to authenticate the Jar file.
        super(file, false);

        this.certificatesRef = certificatesRef;
        this.signerMapCertRef = signerMapCertRef;
        this.hasStrictSingleSigning = hasStrictSingleSigning;
        this.manifestRef = manifestRef;
        this.codeSourcesRef = codeSourcesRef;
        this.codeSourceCertCacheRef = codeSourceCertCacheRef;
        this.resourceURL = resourceURL;
	this.indexFile = indexFile;

        MemoryCache.addResourceReference(this, resourceURL);
    }

    /**
     * Constructor.  This is protected, since it only makes sense for the
     * caching code to instantiate this class.
     */
    protected CachedJarFile14(CacheEntry entry) 
                            throws IOException {
        // Pass in false for the verify parameter to ensure that the
        // superclass does not attempt to authenticate the Jar file.
        super(new File(entry.getResourceFilename()), false);

        this.resourceURL = entry.getURL(); 

        this.certificatesRef = new SoftReference(null);
        this.signerMapCertRef = new SoftReference(null);
	hasStrictSingleSigning = false;
        this.manifestRef = new SoftReference(null); 
        this.codeSourcesRef = new SoftReference(null);
        this.codeSourceCertCacheRef = new SoftReference(null);
	this.indexFile = entry.getIndexFile();

        MemoryCache.addResourceReference(this, resourceURL);
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
        final Enumeration enum14 = super.entries();
        return new Enumeration() {
            public boolean hasMoreElements() {
                return enum14.hasMoreElements();
            }
            public Object nextElement() {
                try {
                    ZipEntry ze = (ZipEntry) enum14.nextElement();
                    return new JarFileEntry(ze);
                } catch (InternalError ie) {
                    // should not expose cache file path in error
                    throw new InternalError("Error in CachedJarFile entries");
                }
            }
        };
    }

    private synchronized CacheEntry getCacheEntry() {
        /* if it was not created by Cache do not search for entry */
        if (resourceURL == null)
            return null;
        
        CacheEntry ce = (CacheEntry) MemoryCache.getLoadedResource(resourceURL);
        if (ce == null || !indexFile.equals(ce.getIndexFile())) {
            // This should not happen because CacheEntry should not get collected 
            // before CachedJarFile is collected....
            Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce, TraceLevel.CACHE);

 	    // Unfortunately, JNLP launching
	    // code always forces download checks by blowing things out of the
	    // MemoryCache even if they have active references.
	    // So, get a new CacheEntry but make sure it binds to the same index
	    // file to ensure it matches our open JAR file. If not, then fail.
	    // The JNLP loading and MemoryCache story needs to be
	    // revisited now that we are living in a multiple JNLP app-per-jvm
	    // world.
	    if (ce != null) {
                Trace.println("CachedJarFile getCacheEntry " + indexFile + " != " +
	            ce.getIndexFile() + " for " + resourceURL/*, TraceLevel.CACHE*/);
	        //ce = null;
	    }
            try {
                ce = Cache.getCacheEntryFromFile(indexFile);
                if (ce == null) {
                    Trace.println("getCacheEntry failed for " + resourceURL + "\n" + ce, TraceLevel.CACHE);
                }
            } catch (Exception e) {
                Trace.ignoredException(e);
            }
        }
        return ce;
    }

    /**
     * Returns the JAR file manifest, or <code>null</code> if none.
     *
     * @return the JAR file manifest, or <code>null</code> if none
     */
    public Manifest getManifest() throws IOException {
        if (manifestRef == null)
            return null;
        
        Manifest manifest = (Manifest) manifestRef.get();
        if (manifest == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                manifest = ce.getManifest();
            } else {
                //We could reread from jar file (from saved handle)
                // but this should not EVER happen
                // (resource is still loaded!)
                Trace.println("Warning: NULL cache entry for loaded resource!", TraceLevel.CACHE);
            }
            if (manifest != null) {
                manifestRef = new SoftReference(manifest);
            } else {
                //Two possible reasons:
                // 1) this is first attempt to load manifest and there
                //     is no manifest in the cache entry => clear reference
                //     to avoid further attempts to retry
                // 2) There was some runtime error => ignore it
                manifestRef = null;
            }
        }
	return manifest;
    }

    private Map getCertificateMap() {
        if (signerMapCertRef == null)
            return null;

        Map signerMap = (Map) signerMapCertRef.get();

        if (signerMap == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                signerMap = ce.getCertificateMap();
                if (signerMap != null) {
                    signerMapCertRef = new SoftReference(signerMap);
		    if (!signerMap.isEmpty()) {
                        hasStrictSingleSigning = ce.hasStrictSingleSigning();
		    }
                } else {
                    //We could reread from jar file (from saved handle)
                    // but this should not EVER happen
                    // (resource is still loaded!)
		    signerMapCertRef = null;
                }
	    } else {
                //This should not happen because CacheEntry should not get collected
                // before CachedJarFile is collected.
                Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                        TraceLevel.CACHE);
	    }
	}
        return signerMap;       
    }
    
    private Certificate[] getCertificates() {
        if (certificatesRef == null)
            return null;

        Certificate[] certs = (Certificate[]) certificatesRef.get();
        
        if (certs == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                certs = ce.getCertificates();
                if (certs != null) {
                    certificatesRef = new SoftReference(certs);
                } else {
                    //Two possible reasons:
                    // 1) this is first attempt to load signers and there
                    //     are no signers in the cache entry => clear reference
                    //     to avoid further attempts to retry
                    // 2) There was some runtime error => ignore it
                    certificatesRef = null;
		}           
            } else {
                //This should not happen because CacheEntry should not get collected
                // before CachedJarFile is collected.
                Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                        TraceLevel.CACHE);
	    }
        }
        return certs;
    }

    // Private class to represent an entry in a cached JAR file
    private class JarFileEntry extends JarEntry {
        JarFileEntry(ZipEntry ze) {
            super(ze);
        }
        public Attributes getAttributes() throws IOException {
            Manifest manifest = CachedJarFile14.this.getManifest();

            // Get the entry's attributes from the JAR manifest
            if (manifest != null) {
                return (Attributes)manifest.getAttributes(getName());
            } else {
                return null;
            }
        }
        public java.security.cert.Certificate[] getCertificates() {
            Certificate[] certs = null;
            int[] certIndices = getCertIndices();
            Certificate[] certificates = CachedJarFile14.this.getCertificates();
            
            if (certificates != null && certIndices != null) {
                // Create an array listing the certificates of each signer
                certs = new Certificate[certificates.length];
                for (int i = 0; i < certificates.length; i++) {
                    certs[i] = certificates[certIndices[i]];
                }
            }
            return certs;
        }

        private int[] getCertIndices() {
            Map signerMap = CachedJarFile14.this.getCertificateMap();
            String name = getName();

            if (signerMap == null || signerMap.isEmpty()) {
                return null;
            }
            if (hasStrictSingleSigning) {
                if (!CacheEntry.isSigningRelated(name) && !name.endsWith("/")) {
                    return (int[]) signerMap.get(null);
                } else {
                    return null;
                }
            }
            // Find this entry in the signer map
            return (int[]) signerMap.get(name);
        }

    }

    //
    // DeployCacheJarAccessImpl methods.
    //

    private int[] emptySignerIndices = new int[0];

    /*
     * Match CodeSource to a signer indices in the signer cache.
     */
    private int[] findMatchingSignerIndices(CodeSource cs) {
	Map cache = getCodeSourceCertCache();
	if (cache == null) {
	    return emptySignerIndices;
	}
        Iterator itor = cache.entrySet().iterator();
	Map.Entry e;
	while (itor.hasNext()) {
	    e = (Map.Entry)itor.next();
	    if (e.getValue().equals(cs)) {
		return (int[]) e.getKey();
	    }
	}
	if (cs.getCertificates() == null) {
	    return emptySignerIndices;
	}
	return null;
    }

    Enumeration entryNames(CodeSource cs[]) {
	boolean matchUnsigned = false;

        /*
	 * Create a list of unique signer indices to match with.
         */
        List req = new ArrayList(cs.length);
        for (int i=0; i < cs.length; i++) {
	    int[] match = findMatchingSignerIndices(cs[i]);
	    if (match != null) {
		if (match.length > 0) {
                    req.add(match);
		} else {
		    matchUnsigned = true;
		}
	    }
        }

	Map map = getCertificateMap();
	if (map != null && !map.isEmpty() && hasStrictSingleSigning && !req.isEmpty()) {
	    map = null;
	    matchUnsigned = true;
	    req.clear();
	}

	final List signersReq = req;
	final Map signerMap = map;
	final Iterator signerKeys = (signerMap != null) ? signerMap.keySet().iterator() : emptyIterator;
	final Enumeration enum2 = (matchUnsigned) ? unsignedEntryNames(signerMap) : emptyEnumeration;

	return new Enumeration() {
	    String name;
	
	    public boolean hasMoreElements() {
		if (name != null) {
		    return true;
		}

		while (signerKeys.hasNext()) {
		    String s = (String)signerKeys.next();
		    if (signersReq.contains((int[])signerMap.get(s))) {
			name = s;
			return true;
		    } else {
			Trace.println("entryNames checking signer failed for " + s, TraceLevel.CACHE);
		    }
		}
		while (enum2.hasMoreElements()) {
		    name = (String)enum2.nextElement();
		    return true;
		}
		return false;
	    }
	    public Object nextElement() {
		if (hasMoreElements()) {
		    String value = name;
		    name = null;
		    return value;
		}
		throw new NoSuchElementException();
	    }
	};
    }

    private Enumeration unsignedEntryNames(final Map signerMap) {
	final Enumeration entries = entries();
	return new Enumeration() {
	    String name;

	    /*
	     * Grab entries from ZIP directory but screen out
	     * metadata.
	     */
	    public boolean hasMoreElements() {
		if (name != null) {
		    return true;
		}
		while (entries.hasMoreElements()) {
		    String value;
		    ZipEntry e = (ZipEntry) entries.nextElement();
		    value = e.getName();
		    if (e.isDirectory() || CacheEntry.isSigningRelated(value)) {
			continue;
		    }
	    	    if (signerMap == null || signerMap.get(value) == null) {
			name = value;
			return true;
		    }
		}
		return false;
	    }
	    public Object nextElement() {
                if (hasMoreElements()) {
                     String value = name;
                     name = null;
                     return value;
                 }
                 throw new NoSuchElementException();
	    }
	};
    }

    private static Enumeration emptyEnumeration = new Enumeration() {
        public boolean hasMoreElements() {
	    return false;
	}
	public Object nextElement() {
	    throw new NoSuchElementException();
	}
    };

    private static Iterator emptyIterator = Collections.EMPTY_MAP.keySet().iterator();

    CodeSource[] getCodeSources(URL url) {
        if (codeSourcesRef == null)
            return null;

        CodeSource[] codeSources = (CodeSource[]) codeSourcesRef.get();
        
        if (codeSources == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                codeSources = ce.getCodeSources(url);
                if (codeSources != null) {
                    codeSourcesRef = new SoftReference(codeSources);
                } else {
                    codeSourcesRef = null;
		}
            } else {
                //We could reread from jar file (from saved handle)
                // but this should not EVER happen
                // (resource is still loaded!)
                Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                        TraceLevel.CACHE);
            }
        }
        return codeSources;
    }

    Map getCodeSourceCertCache() {
        if (codeSourceCertCacheRef == null)
            return null;

        Map codeSourceCertCache = (Map) codeSourceCertCacheRef.get();
        
        if (codeSourceCertCache == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                codeSourceCertCache = ce.getCodeSourceCertCache();
                if (codeSourceCertCache != null) {
                    codeSourceCertCacheRef = new SoftReference(codeSourceCertCache);
                } else {
		    codeSourceCertCacheRef = null;
		}          
            } else {
                //We could reread from jar file (from saved handle)
                // but this should not EVER happen
                // (resource is still loaded!)
                Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                        TraceLevel.CACHE);
            }
        }
        return codeSourceCertCache;
    }

    /*
     * Return a CodeSource for the named JAR cache entry. The name is
     * trusted to come from a successfull ZIP entry lookup.
     */
    CodeSource getCodeSource(URL url, String name) {
        Map signerMap = getCertificateMap();
	int[] signerIndices = null;

	if (signerMap == null || signerMap.isEmpty()) {
	    signerIndices = null;
	} else if (hasStrictSingleSigning) {
            if (!CacheEntry.isSigningRelated(name) && !name.endsWith("/")) {
                signerIndices = (int[]) signerMap.get(null);
	    } else {
	        signerIndices = null;
	    }
	} else {
            // Find this entry in the signer map
            signerIndices = (int[]) signerMap.get(name);
	}

	if (signerIndices != null) {
	    Map cache = getCodeSourceCertCache();
	    if (cache != null) {
	        // XXX check URL?
	        return (CodeSource) cache.get(signerIndices);
	    }
	}
	return CacheEntry.getUnsignedCS(url);
    }
}


