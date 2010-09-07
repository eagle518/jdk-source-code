/*
 * @(#)
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.config.Config;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDescFactory;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import javax.jnlp.DownloadService2;

public class DownloadService2Impl implements DownloadService2 {

    public static interface ResourceSpecAccess {
        public void setSize(ResourceSpec spec, long size);
        public void setLastModified(ResourceSpec spec, long lm);
        public void setExpirationDate(ResourceSpec spec, long ed);
    }

    private static DownloadService2Impl instance;

    private static ResourceSpecAccess resourceSpecAccess;

    public static synchronized DownloadService2 getInstance() {
        if (instance == null) {
            instance = new DownloadService2Impl();
        }
        return instance;
    }

    public static void setResourceSpecAccess(ResourceSpecAccess rsAccess) {
        resourceSpecAccess = rsAccess;
    }

    /**
     * Allows getting an instance only via {@link #getInstance()}.
     */
    private DownloadService2Impl() {
    }

    public ResourceSpec[] getCachedResources(final ResourceSpec spec) {
        validateResourceSpec(spec);
        final ArrayList matchingResources = new ArrayList();
        AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                getCachedResourcesImpl(spec, matchingResources);
                return null;
            }
        });
        return (ResourceSpec[]) matchingResources.toArray(
                                   new ResourceSpec[matchingResources.size()]);
    }

    private void getCachedResourcesImpl(ResourceSpec spec,
                                        ArrayList matchingResources) {
        File[] idxFiles = Cache.getCacheEntries(false);
        for (int i = 0; i < idxFiles.length; i++) {
            CacheEntry ce = Cache.getCacheEntryFromFile(idxFiles[i]);
            if (matches(ce, spec)) {
                matchingResources.add(toResourceSpec(ce));
            }
        }
    }

    private boolean matches(CacheEntry ce, ResourceSpec spec) {
        boolean matches = false;

        String urlSpec = spec.getUrl();

        // TODO: Should we throw IAE when urlSpec is null or skip it?
        if (urlSpec != null && ce != null) {
            String versionSpec = spec.getVersion();
            String ceVersion = ce.getVersion();
            int typeSpec = spec.getType();
            boolean typeMatches = (typeSpec == ALL || 
                typeSpec == getResourceType(ce));
            boolean versionMatches = (
                (versionSpec == null && ceVersion == null) ||
                (versionSpec != null && ceVersion != null && 
                 stringMatch(ceVersion, versionSpec)));
               
            matches = stringMatch(ce.getURL(), urlSpec) && 
                versionMatches && typeMatches;
        }

        return matches;
    }

    private ResourceSpec toResourceSpec(CacheEntry ce) {
        ResourceSpec rs = new ResourceSpec(ce.getURL(), ce.getVersion(),
                                           getResourceType(ce));
        resourceSpecAccess.setSize(rs, ce.getSize());
        resourceSpecAccess.setLastModified(rs, ce.getLastModified());
        resourceSpecAccess.setExpirationDate(rs, ce.getExpirationDate());

        return rs;
    }

    private int getResourceType(CacheEntry ce) {
        String url = ce.getURL();
        String fixedUrl = getLowerNameExtension(url);
        int type = ALL;
        if (ce.isJNLPFile()) {
            try {
                URL ceUrl = new URL(url);
                LaunchDesc ld =
                        LaunchDescFactory.buildDescriptor(ce.getDataFile(),
                                                        URLUtil.getBase(ceUrl),
                                                        null, ceUrl);
                if (ld.isApplication()) {
                    type = APPLICATION;
                } else if (ld.isApplet()) {
                    type = APPLET;
                } else if (ld.isInstaller() || ld.isLibrary()) {
                    type = EXTENSION;
                } // else type = ALL.
            } catch (Exception ex) {
                // Let the type be ALL.
            }
        } else if (ce.isJarFile(ce.getDataFile().getAbsolutePath())) {
            type = JAR;
        } else if (fixedUrl.endsWith("png") || fixedUrl.endsWith("gif")
                || fixedUrl.endsWith("jpeg") || fixedUrl.endsWith("jpg")
                || fixedUrl.endsWith("ico")) {
            type = IMAGE;
        } else if (fixedUrl.endsWith("class")) {
            type = CLASS;
        }
        return type;
    }

    /**
     * Strips off final '/', parameters and makes the URL lowercase for
     * extension checking in #getResourceType().
     *
     * @param url the URL to fix up
     *
     * @return the fixed url
     */
    private String getLowerNameExtension(String url) {
        // Remove parameters.
        if (url.indexOf('?') != -1) {
            url = url.substring(0, url.indexOf('?'));
        }
        // Strip off trailing '/'.
        while (url.charAt(url.length() - 1) == '/') {
            url = url.substring(0, url.length() - 1);
        }
        // Make URL lowercase.
        url = url.toLowerCase();

        return url;
    }

    public ResourceSpec[] getUpdateAvailableResources(ResourceSpec spec) {
        validateResourceSpec(spec);
        ResourceSpec[] inCache = getCachedResources(spec);
        ArrayList updateAvailable = new ArrayList();
        for (int i = 0; i < inCache.length; i++) {
            ResourceSpec inCacheSpec = inCache[i];
            URL url;
            try {
                url = new URL(inCacheSpec.getUrl());
            } catch (MalformedURLException ex) {
                // This must never happen.
                InternalError err = new InternalError();
                err.initCause(ex);
                throw err;
            }
            try {
                if (inCacheSpec.getVersion() == null) {
                    // timestamped resources
                    if (DownloadEngine.isUpdateAvailable(url, null)) {
                        updateAvailable.add(inCacheSpec);
                    } 
                } else {
                    // versioned resources
                    String versionAvailable =
                        DownloadEngine.getAvailableVersion(url, 
                                       "0+", false, null);
                    if (!DownloadEngine.isResourceCached(url, 
                                        null, versionAvailable)) {
                        updateAvailable.add(inCacheSpec);
                    }
                }
            } catch (IOException ioe) {
                // ignore
            }
        }
        // Copy array 'manually', for some reason, toArray(ResourceSpec[])
        // did not return a ResourceSpec[], but an Object[].
        ResourceSpec[] array = new ResourceSpec[updateAvailable.size()];
        for (int i = 0; i < updateAvailable.size(); i++) {
            array[i] = (ResourceSpec) updateAvailable.get(i);
        }
        return array;
    }

    private void validateResourceSpec(ResourceSpec spec) {
        String url = spec.getUrl();
        int type = spec.getType();
        if (url == null) {
            throw new IllegalArgumentException("ResourceSpec has null url");
        }
        if (url == "") {
            throw new IllegalArgumentException("ResourceSpec has empty url");
        }
        if (type < ALL || type > CLASS) {
            throw new IllegalArgumentException("ResourceSpec has invalue type");
        }
    }

    private boolean stringMatch(String s1, String s2) {
        if (Config.isJavaVersionAtLeast14()) {
            return s1.matches(s2);
        } else {
            return s1.equals(s2);
        }
    }

}
