/*
 * @(#)CachedManifest.java	1.4 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.cache;

import com.sun.deploy.Environment;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Field;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/*
 * This is "lazy" manifest that can load subset of manifest data when
 * loaded from the cache index file that has "reduced manifest" section.
 *
 * Reduced subset includes main attributes and information about packages.
 * If anything else besides this is requested than manifest will load
 * full version.
 *
 * If created from the jar file then full version is loaded immediately.
 *
 * For signed jars size of manifest files is often significant
 * (e.g. 500k for JavaFX runtime at this moment) and vast majority of their
 * content are class signatures. We perform signature validation only once
 * (when jar is placed to cache) and therefore on subsequent runs
 * class signatures are not used. That's why we want to avoid read & parse them.
 *
 * For cases when full manifest is needed we might be doing more work
 * but as "reduced" part of manifest is very small the overhead is minimal.
 */
class CachedManifest extends Manifest {
    String resourceURL;
    boolean isReduced = false;
    boolean postponePostprocessing = false;

    static Field fEntries = null;
    static Field fAttributes = null;

    static {
        try {
            fAttributes = Manifest.class.getDeclaredField("attr");
            fAttributes.setAccessible(true);
            fEntries = Manifest.class.getDeclaredField("entries");
            fEntries.setAccessible(true);
        } catch (Exception e) {
            //should not happen on any Sun VM
            Trace.ignoredException(e);
        }
    }

    /* construct manifest from data array.
       postprocessing is done imeediately */
    CachedManifest(String resourceURL, byte[] data, boolean isReduced)
            throws IOException {
        super();
        this.resourceURL = resourceURL;
        readFromBytes(data);
        this.isReduced = isReduced;
    }

    /* If manifest is constructed from the file then read full manifest.
       Postprocessing should be requested explicitly by the callee */
    CachedManifest(JarFile file) throws IOException {
            Manifest m = file.getManifest();

            try {
              if (fAttributes != null && fEntries != null) {
                fAttributes.set(this, fAttributes.get(m));
                fEntries.set(this, fEntries.get(m));
              }
            } catch (Exception e) {
              fAttributes = null;
              fEntries = null;
            }
            isReduced = false;
    }

    public Map /* <String, Attributes> */ getEntries() {
        loadFullManifest();
        return super.getEntries();
    }

    /* Unfortunatelly Manifest.getAttributes() is implemented as
       call to getEntries() instead of direct use of entries variable.
       We do not want to load full manifest too soon and therefore
       we try to directly access private map in the super class */
    private Map /* <String, Attributes> */ getEntriesLocal() {
        if (isReduced) {
            try {
                if (fEntries != null)
                  return (Map) fEntries.get(this);
            } catch (Exception e) {
                Trace.ignoredException(e);
            }

        }
        return getEntries();
    }

    public Attributes getAttributes(String name) {
        if (!belongsToReducedManifest(name)) {
            loadFullManifest();
        }

        Map entries = getEntriesLocal();
        if (entries != null) {
            return (Attributes) entries.get(name);
        }
        return null;
    }

    public void clear() {
        super.clear();
        isReduced = false; //should not try to read full version after clear()
    }

    public int hashCode() {
        if (resourceURL != null) {
            return resourceURL.hashCode();
        }
        return 0;
    }

    public boolean equals(Object o) {
        if (o instanceof CachedManifest) {
            CachedManifest m = (CachedManifest) o;
            if (resourceURL != null && resourceURL.equals(m.resourceURL)) {
                return true;
            }
        }
        return false;
    }

    public Object clone() {
        CachedManifest m = (CachedManifest) super.clone();
        m.resourceURL = resourceURL;
        m.isReduced = isReduced;
        return m;
    }


    //Performs additional "processing" of the entry
    //  - removes class-path entry if needed
    //  - make everything immutable for security reasons
    void postprocess() {
        //should exclude class path for web start applications
        if (Environment.isJavaPlugin() == false) {
            getMainAttributes().remove(Name.CLASS_PATH);
        }

        makeManifestImmutable(this);
    }

    private CacheEntry getCacheEntry() {
        CacheEntry ce = (CacheEntry) MemoryCache.getLoadedResource(resourceURL);
        if (ce == null) {
            //This should not happen because CacheEntry should not get collected
            // before CachedJarFile is collected.
            Trace.println("Missing CacheEntry for " + resourceURL + "\n" + ce,
                    TraceLevel.CACHE);
        }
        return ce;
    }

    private void loadFullManifest() {
        if (isReduced == false) {
            return;
        }
        Trace.print("Loading full manifest for "+resourceURL, TraceLevel.CACHE);

        CacheEntry entry = getCacheEntry();
        if (entry != null) {
            try {
                byte[] data = entry.getFullManifestBytes();
                if (data != null) {
                    try {
                        //reset all values
                        Field fattr = Manifest.class.getDeclaredField("attr");
                        fattr.setAccessible(true);
                        Field fentries = Manifest.class.getDeclaredField("entries");
                        fentries.setAccessible(true);
                        fattr.set(this, new Attributes());
                        fentries.set(this, new HashMap());

                        isReduced = false; //prevent recursion
                        readFromBytes(data);
                    } catch (Exception e) {
                    }
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }


    private int writeCompressed(Manifest m, OutputStream out) throws IOException {
        if (m == null) {
            return 0;
        }

        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        GZIPOutputStream gz = new GZIPOutputStream(bos);
        m.write(gz);
        gz.flush();
        gz.close();
        bos.close();

        byte data[] = bos.toByteArray();
        out.write(data);
        return data.length;
    }

    int writeReduced(OutputStream out) throws IOException {
        Manifest reduced = reduce();
        return writeCompressed(reduced, out);
    }

    int writeFull(OutputStream out) throws IOException {
        loadFullManifest();
        return writeCompressed(this, out);
    }

    private void readFromBytes(byte[] data) throws IOException {
        //this should be called with isReduced set to false
        //  or otherwise this will cause recursion
        //  and loading of full manifest anyway (due to Manifest.read() request
        //  for complete set of entities
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        GZIPInputStream gis = new GZIPInputStream(bis);
        read(gis);
        gis.close();
        bis.close();
    }

    public void read(InputStream in) throws IOException {
        super.read(in);
        if (!postponePostprocessing) postprocess();
    }

    private static boolean belongsToReducedManifest(String name) {
        return name.endsWith("/");
    }

    //we only use result as temp object, so copy as little as possible
    Manifest reduce() {
        if (isReduced) {
            //loaded reduced and still reduced, return ourselves
            return this;
        }

        //do not reduce small manifests
        if (getEntries().size() < 25) {
            return null;
        }

        Manifest dest = new Manifest();
        int skipped = 0;

        Attributes a = dest.getMainAttributes();
        a.putAll(getMainAttributes());

        Map entries = dest.getEntries();
        Iterator it = getEntries().keySet().iterator();
        while (it.hasNext()) {
            String key = (String) it.next();
            if (belongsToReducedManifest(key)) {
                Object o = getAttributes(key);
                entries.put(key, o);
            } else {
                skipped++;
            }
        }

        //if we can save too few - do not bother
        if (skipped < 25) {
            return null;
        }

        return dest;
    }

    /* Helper methods to make manifest immutable
     * (safe to return to the user code)
     */
    private void replaceAttributesMapWithImmutableMap(Attributes o) {
        try {
            Field f = Attributes.class.getDeclaredField("map");
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

    private void replaceEntriesMapWithImmutableMap(Manifest o) {
        try {
            Field f = Manifest.class.getDeclaredField("entries");
            f.setAccessible(true);
            Map m = (Map) f.get(o);
            if (m != null) {
		makeEntriesAttributesImmutable(m);
                m = Collections.unmodifiableMap(m);
                f.set(o, m);
            }
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
    }

    private void makeEntriesAttributesImmutable(Map m) {
        Iterator it = m.keySet().iterator();
        while (it.hasNext()) {
            Object key = it.next();
            Object o = m.get(key);
            if (o instanceof Attributes) {
		replaceAttributesMapWithImmutableMap((Attributes)o);
            }
        }
    }

    private void makeManifestImmutable(Manifest m) {
        Attributes a = m.getMainAttributes();
        replaceAttributesMapWithImmutableMap(a);
        replaceEntriesMapWithImmutableMap(m);
    }
}

