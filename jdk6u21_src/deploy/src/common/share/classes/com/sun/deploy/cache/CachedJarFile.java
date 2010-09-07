/*
 * @(#)CachedJarFile.java	1.41 10/04/16
 *
 * Copyright (c) 2008, 2010 Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.cache;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.security.CodeSigner;
import java.security.CodeSource;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import com.sun.deploy.config.Config;
import java.lang.ref.SoftReference;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.lang.ref.Reference;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.AccessController;
import java.lang.reflect.Field;
import java.util.Collections;
import java.util.Iterator;

/**
 * This class is used to read jar files from the jar cache.
 *
 * @see     java.util.zip.ZipFile
 * @see     java.util.jar.JarEntry
 */
public class CachedJarFile extends JarFile {

    // The signers used to sign this jar file
    private Reference /* CodeSigner[] */ signersRef;

    // Map of entries to their signers
    private Reference /* <Map> */ signerMapRef = null;
    private boolean hasStrictSingleSigning;

    // Manifest for this Jar file
    private Reference /* <Manifest> */ manRef = null;

    // The code sources of this jar file
    private Reference /* CodeSource[] */ codeSourcesRef;

    // Map of signer indices to code sources
    private Reference /* <Map> */ codeSourceCacheRef = null;

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
            return new CachedJarFile(new File(super.getName()),
                signersRef, signerMapRef, hasStrictSingleSigning, manRef, codeSourcesRef,
		codeSourceCacheRef, resourceURL, indexFile);
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
            throw new CloneNotSupportedException();
        }
    }

    /* need this only for clone() */
    private CachedJarFile(File file, Reference /* CodeSigner[] */ signersRef,
                          Reference /* <Map> */ signerMapRef,
			  boolean hasStrictSingleSigning,
                          Reference /* <Manifest> */ manRef,
                          Reference /* <CodeSource[]> */ codeSourcesRef,
                          Reference /* <Map> */ codeSourceCacheRef,
                          String resourceURL,
			  File indexFile)
                          throws IOException {
        // Pass in false for the verify parameter to ensure that the
        // superclass does not attempt to authenticate the Jar file.
        super(file, false);

        this.signersRef = signersRef;
        this.signerMapRef = signerMapRef;
	this.hasStrictSingleSigning = hasStrictSingleSigning;
        this.manRef = manRef;
        this.codeSourcesRef = codeSourcesRef;
        this.codeSourceCacheRef = codeSourceCacheRef;
        this.resourceURL = resourceURL;
	this.indexFile = indexFile;

        ensureAncestorKnowsAboutManifest(this);
        MemoryCache.addResourceReference(this, resourceURL);
    }

    static void ensureAncestorKnowsAboutManifest(final JarFile jar) throws IOException {
        //if manLoaded is not set in the super class, whenever
        //getInputStream() is called, getManfiest() is called which
        //is expensive. 
        //This overhead is avoided by making sure manLoaded flag is set
        
        // manLoaded flag no longer exists in 6.0 or above
        if (Config.isJavaVersionAtLeast16() == false) {
            try {
              AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    try {
                        Field f = Manifest.class.getDeclaredField("manLoaded");
                        if (f != null){
                            f.setAccessible(true);
                            f.setBoolean(jar, true);
                            return null;
                        }
                    } catch(Exception e){
                        //If something went wrong then jre likely has no manLoaded 
                        //and we should not do anything special
                        //(only some old updates of jre 5 actually had manLoaded
                        // so for latest JRE 5 we will end up here and we should not waste 
                        // heap and CPU to read manifest greedy) 
                    }
                    return null;
                }}); 
            } catch (PrivilegedActionException pe) {
                throw (IOException) pe.getException();
            }
         }

    }

    /**
     * Constructor.  This is protected, since it only makes sense for the
     * caching code to instantiate this class.
     */
    protected CachedJarFile(CacheEntry entry) throws IOException {
        // Pass in false for the verify parameter to ensure that the
        // superclass does not attempt to authenticate the Jar file.
        super(new File(entry.getResourceFilename()), false);

        this.resourceURL = entry.getURL(); 

        //Do not be greedy loading signers and manifest
        //Create empty references, this will cause attempt to read data
        //from cache entry on first request

        this.signersRef  = new SoftReference(null);
        this.signerMapRef = new SoftReference(null);
	hasStrictSingleSigning = false;
        this.manRef = new SoftReference(null);
	this.codeSourcesRef = new SoftReference(null);
	this.codeSourceCacheRef = new SoftReference(null);
	this.indexFile = entry.getIndexFile();

        ensureAncestorKnowsAboutManifest(this);
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
        final Enumeration entryList = super.entries();

        return new Enumeration() {

            public boolean hasMoreElements() {
                return entryList.hasMoreElements();
            }

            public Object nextElement() {
                try {
                    ZipEntry ze = (ZipEntry) entryList.nextElement();
                    return new JarFileEntry(ze);
                } catch (InternalError ie) {
                    // should not expose cache file path in error
                    throw new InternalError("Error in CachedJarFile entries");
                }
            }
        };
    }

    /**
     * Returns the JAR file manifest, or <code>null</code> if none.
     *
     * @return the JAR file manifest, or <code>null</code> if none
     */
    public Manifest getManifest() throws IOException {
        if (manRef == null)
            return null;
        
        Manifest manifest = (Manifest) manRef.get();
        if (manifest == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                manifest = ce.getManifest();
            } else {
                //We could reread from jar file (from saved handle)
                // but this should not EVER happen
                // (resource is still loaded!)
                Trace.print("Warning: NULL cache entry for loaded resource!");
            }
            if (manifest != null) {
                manRef = new SoftReference(manifest);
            } else {
                //Two possible reasons:
                // 1) this is first attempt to load manifest and there
                //     is no manifest in the cache entry => clear reference
                //     to avoid further attempts to retry
                // 2) There was some runtime error => ignore it
                manRef = null;
            }
        }
	return manifest;
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

    private Map getSignerMap() {
        if (signerMapRef == null)
            return null;

        Map signerMap = (Map) signerMapRef.get();

        if (signerMap == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
              signerMap = ce.getSignerMap();
              if (signerMap != null) {
                signerMapRef = new SoftReference(signerMap);
		if (!signerMap.isEmpty()) {
		    hasStrictSingleSigning = ce.hasStrictSingleSigning();
		}
              } else {
                //Two possible reasons:
                // 1) this is first attempt to load signer map and there
                //     is no signer map in the cache entry => clear reference
                //     to avoid further attempts to retry
                // 2) There was some runtime error => ignore it
                signerMapRef = null;
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
    
    private CodeSigner[] getSigners() {
        if (signersRef == null)
            return null;

        CodeSigner[] signers = (CodeSigner[]) signersRef.get();
        
        if (signers == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
              signers = ce.getCodeSigners();
              if (signers != null) {
                signersRef = new SoftReference(signers);
              } else {
                //Two possible reasons:
                // 1) this is first attempt to load signers and there
                //     are no signers in the cache entry => clear reference
                //     to avoid further attempts to retry
                // 2) There was some runtime error => ignore it
                signersRef = null;
              }
            } else {
                //This should not happen because CacheEntry should not get collected
                // before CachedJarFile is collected.
                Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                        TraceLevel.CACHE);
            }
        }
        return signers;
    }

   private static void replaceMapFieldWithImmutableMap(Class c, 
                                              Object o, String mapFieldName) {
        try {
           Field f = c.getDeclaredField(mapFieldName);
           f.setAccessible(true);
           Map m = (Map) f.get(o);
           if (m != null) {
               m = Collections.unmodifiableMap(m);
               f.set(o, m);
           }
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
    }
    
   private static Map makeAttributesImmutable(Attributes m) {
        Iterator it = m.keySet().iterator();
        while (it.hasNext()) {
            Object key = it.next();
            Object o = m.get(key);
            if (o instanceof Attributes) {
               makeAttributesImmutable((Attributes) o);
            } 
        }
        replaceMapFieldWithImmutableMap(Attributes.class, m, "map");
        return m;
    }
    
   static void makeManifestImmutable(Manifest m) {
        Attributes a = m.getMainAttributes();
        makeAttributesImmutable(a);       
        replaceMapFieldWithImmutableMap(Manifest.class, m, "entries");
    }
    
    // Private class to represent an entry in a cached JAR file
    private class JarFileEntry extends JarEntry {
        JarFileEntry(ZipEntry ze) {
            super(ze);
        }

        public Attributes getAttributes() throws IOException {
            Manifest manifest = CachedJarFile.this.getManifest();

            // Get the entry's attributes from the JAR manifest
            if (manifest != null) {
                Attributes attr = manifest.getAttributes(getName());
                return attr; //its immutable, no need to clone
            }
            return null;
        }

        public Certificate[] getCertificates() {
            Certificate[] certs = null;
            int[] signerIndices = getSignerIndices();
            CodeSigner[] signers = CachedJarFile.this.getSigners();

            if (signers != null && signerIndices != null) {
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
            CodeSigner[] signers = CachedJarFile.this.getSigners();
            
            if (signers != null && signerIndices != null) {
                // Create an array of code signers
                codeSigners = new CodeSigner[signerIndices.length];
                for (int i = 0; i < signerIndices.length; i++) {
                    if (signers != null) {
                        codeSigners[i] = signers[signerIndices[i]];
                    } 
                }
            }
            return codeSigners;
        }

        private int[] getSignerIndices() {
            Map signerMap = CachedJarFile.this.getSignerMap();
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
	Map cache = getCodeSourceCache();
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
	if (cs.getCodeSigners() == null) {
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

	Map map = getSignerMap();
	if (map != null && !map.isEmpty() && hasStrictSingleSigning && !req.isEmpty()) {
	    /*
	     * If JAR has only a single code source and only signed entries
	     * then, as an optimization,  signerMap only contains a single
	     * null entry rather than a full set of entry names. If we need
	     * to enumerate all the entries then fallback on the "unsigned"
	     * zip directory enumeration code.
	     */
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

    Map getCodeSourceCache() {
        if (codeSourceCacheRef == null)
            return null;

        Map codeSourceCache = (Map) codeSourceCacheRef.get();
        
        if (codeSourceCache == null) {
            CacheEntry ce = getCacheEntry();
            if (ce != null) {
                codeSourceCache = ce.getCodeSourceCache();
                if (codeSourceCache != null) {
                    codeSourceCacheRef = new SoftReference(codeSourceCache);
                } else {
		    codeSourceCacheRef = null;
		}          
            } else {
                //We could reread from jar file (from saved handle)
                // but this should not EVER happen
                // (resource is still loaded!)
                Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                              TraceLevel.CACHE);
            }
        }
        return codeSourceCache;
    }

    /*
     * Return a CodeSource for the named JAR cache entry. The name is
     * trusted to come from a successfull ZIP entry lookup.
     */
    CodeSource getCodeSource(URL url, String name) {
        Map signerMap = getSignerMap();
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
	    Map cache = getCodeSourceCache();
	    if (cache != null) {
	        // XXX check URL?
	        return (CodeSource) cache.get(signerIndices);
	    }
	}
	return CacheEntry.getUnsignedCS(url);
    }
}
