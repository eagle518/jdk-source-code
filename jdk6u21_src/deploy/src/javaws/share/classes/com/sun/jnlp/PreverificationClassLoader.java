/*
 * @(#)PreverificationClassLoader.java	1.3 10/03/24
 * 
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.net.URL;
import java.net.URLClassLoader;
import java.io.File;
import java.util.ArrayList;
import java.security.CodeSource;
import java.security.PermissionCollection;

import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ResourcesDesc;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;

/**
 * This classloader is used only for pre-verification of cached JARs from the
 * system cache.
 */
public final class PreverificationClassLoader extends URLClassLoader  {

    /* jars that are listed in superclass URLClassLoader */
    private ArrayList _jarsInURLClassLoader = new ArrayList();

    /* jars not yet listed in the superclass URLClassLoader */
    private ArrayList _jarsNotInURLClassLoader = new ArrayList();

    /**
     * Constructs a new PreverificationClassLoader
     *
     * <p>If there is a security manager, this method first
     * calls the security manager's <code>checkCreateClassLoader</code> method
     * to ensure creation of a class loader is allowed.
     *
     * @param parent the parent class loader for delegation
     * @exception  SecurityException  if a security manager exists and its
     *             <code>checkCreateClassLoader</code> method doesn't allow
     *             creation of a class loader.
     * @see SecurityManager#checkCreateClassLoader
     */
    public PreverificationClassLoader(ClassLoader parent) {
        super(new URL[0], parent);
        // this is to make the stack depth consistent with 1.1
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkCreateClassLoader();
        }
    }

    // Need AllPermission for the pre-verification of the FX runtime JARs
    protected PermissionCollection getPermissions(CodeSource codesource) {
         PermissionCollection perms = super.getPermissions(codesource);
         perms.add(new java.security.AllPermission());
         return perms;
    }
    
    public void preverifyJARs() {
        if (Cache.isCacheEnabled() == false) {
            return;
        }
     
        long startTime = System.currentTimeMillis();
   
        for (int i = 0; i < _jarsInURLClassLoader.size(); i++) {
            JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);

            File systemCacheDir = new File(
                    Config.getInstance().getSystemCacheDirectory() +
                    File.separator + Cache.getCacheVersionString());

            // only pre-verify JARs in system cache
            CacheEntry ce = Cache.getCacheEntry(jd.getLocation(), null,
                    jd.getVersion(), systemCacheDir);

            if (ce != null && ce.getClassesVerificationStatus() ==
                    CacheEntry.PREVERIFY_NOTDONE) {
                ce.verifyJAR(this);
            }
        }
        long endTime = System.currentTimeMillis();
        Trace.println("PreverificationCL, Cached JAR preverification took (ms): " +
                (endTime - startTime), TraceLevel.CACHE);
    }

    public void initialize(LaunchDesc launchDesc) {
       
        // Add all JARDescs for application to list */
        ResourcesDesc rd = launchDesc.getResources();
        if (rd != null) {
            // Extract list of URLs from Codebase
            sortDescriptors(rd);
            for (int i = 0; i < _jarsInURLClassLoader.size(); i++) {
                JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);

                addURL(jd.getLocation());
           
            }
        }
     
    }

    private void addLoadedJarsEntry(JARDesc jd) {
        if (_jarsInURLClassLoader.contains(jd) == false) {
            _jarsInURLClassLoader.add(jd);
        }
    }

    private void sortDescriptors(ResourcesDesc rd) {

        ArrayList lazyJarDescArrayList = new ArrayList();

        JARDesc[] allJarDescs = rd.getEagerOrAllJarDescs(true);
        JARDesc main = rd.getMainJar(true);
        JARDesc progress = rd.getProgressJar();

        /* if there is a progress jar, it is first */
        if (progress != null) {
            addLoadedJarsEntry(progress);
        }

        /* then add main jar */
        if (main != null) {
            addLoadedJarsEntry(main);
        }

        for (int i = 0; i < allJarDescs.length; i++) {
            if (allJarDescs[i] != main && allJarDescs[i] != progress) {
                if ((!allJarDescs[i].isLazyDownload())) {
                    /* second add other eager jars */
                    addLoadedJarsEntry(allJarDescs[i]);
                } else if (!rd.isPackagePart(allJarDescs[i].getPartName())) {
                    lazyJarDescArrayList.add(allJarDescs[i]);
                } else {
                    /* add those with package parts to seperate list */
                    _jarsNotInURLClassLoader.add(allJarDescs[i]);
                }
            }
        }

        /* third add the lazy jars w/o package parts */
        for (int i = 0; i < lazyJarDescArrayList.size(); i++) {
            addLoadedJarsEntry((JARDesc)lazyJarDescArrayList.get(i));
        }
    }
}


