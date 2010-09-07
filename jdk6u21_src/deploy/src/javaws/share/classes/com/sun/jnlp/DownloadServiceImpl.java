/*
 * @(#)DownloadServiceImpl.java	1.37 10/03/31
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ExtensionDesc;
import com.sun.javaws.jnl.JREDesc;
import com.sun.javaws.jnl.PropertyDesc;
import com.sun.javaws.jnl.PackageDesc;
import com.sun.javaws.jnl.ResourceVisitor;
import com.sun.javaws.jnl.LaunchDescFactory;
import com.sun.javaws.Main;
import com.sun.javaws.CacheUtil;
import com.sun.javaws.LaunchDownload;
import com.sun.javaws.ui.DownloadWindow;
import com.sun.javaws.exceptions.DownloadException;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.progress.ProgressListener;
import com.sun.javaws.progress.CustomProgress;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.util.Trace;

import javax.jnlp.DownloadService;
import javax.jnlp.DownloadServiceListener;

import java.io.IOException;
import java.io.File;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.net.URL;

public final class DownloadServiceImpl implements DownloadService {
    
    static private DownloadServiceImpl _sharedInstance = null;
    private ProgressHelper _defaultProgressHelper = null;
    
    private DownloadServiceImpl() { }
    
    /**
     * Returns singleton instance.  Might return null if service not supported
     */
    public static synchronized DownloadServiceImpl getInstance() {
        initialize();
        return _sharedInstance;
    }
    
    /**
     * Initialize the single instance of this class.  This call is ignored if
     * the instance is already initialized.
     */
    public static synchronized void initialize() {
        if (_sharedInstance == null) {
            _sharedInstance = new DownloadServiceImpl();
        }
    }

    public void setDefaultProgressHelper(DownloadServiceListener dsl) {
        _defaultProgressHelper = getProgressHelper(dsl);
    }
    
    public ProgressHelper getDefaultProgressHelper() {
        if (_defaultProgressHelper == null) {
            _defaultProgressHelper = (ProgressHelper)
                AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    DownloadWindow dw = new DownloadWindow();
                    dw.initialize(
                        JNLPClassLoaderUtil.getInstance().getLaunchDesc(),
                            false, false);
                    dw.setAllowVisible(true);
                    return new DefaultProgressHelper(dw);
                }
            });
        }
        return _defaultProgressHelper;
    }

    public DownloadServiceListener getDefaultProgressWindow() {
        return (DownloadServiceListener) getDefaultProgressHelper();
    }
    
    private class DefaultProgressHelper extends ProgressHelper {
        
        private DownloadWindow _dw = null;
        
        DefaultProgressHelper(final DownloadWindow dw) {            
            AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    _dw = dw;
                    _dw.showLoadingProgressScreen();
                    return null;
                }
            });
        }
        
        public void initialize() {
            _dw.resetCancled();
        }

        public void progress(URL url, String version, long readSoFar, 
                             long total, int overallPercent) {
            ensureVisible();
            _dw.progress(url, version, readSoFar, total, overallPercent);

            // Check if cancel has been invoked. This exception will be caught 
            // by the DownloadProtocol and converted into a failed 
            // download request
            if (_dw.isCanceled()) {                
                throw new RuntimeException("canceled by user");
            }
        }
        
        
        public void validating(URL url, String version, long entry, 
                               long total, int overallPercent) {
            ensureVisible();
            _dw.validating(url, version, entry, total, overallPercent);
        }
        
        public void upgradingArchive(URL url, String version, 
                                     int patchPercent, int overallPercent) {
            ensureVisible();
            _dw.upgradingArchive(url, version, patchPercent, overallPercent);
        }
        
        public void downloadFailed(URL url, String version) {            
            hideFrame();
        }
        
        private void ensureVisible() {
            if (!_dw.isVisible()) {
                _dw.setVisible(true);
            }
        }
        
        private synchronized void hideFrame() {
            _dw.resetCancled();
            _dw.setVisible(false);            
        }            

        public void done() {
            hideFrame();
        }
        public void extensionDownload(String name, int remaining) {
            _dw.extensionDownload(name, remaining); 
        }

        public void jreDownload(String versionId, URL location) {
            _dw.jreDownload(versionId, location);
        }

        public void setHeading(final String text, final boolean singleLine) {
            _dw.setHeading(text, singleLine);
        }

        public void setStatus(String text) {
            _dw.setStatus(text);
        }

        public java.awt.Component getOwner() {
            return _dw.getOwner();
        }

        public void setVisible(final boolean show) {
            _dw.setVisible(show);
        }

        public void setProgressBarVisible(final boolean isVisible) {
            _dw.setProgressBarVisible(isVisible);
        }

        public void setProgressBarValue(final int value) {
            _dw.setProgressBarValue(value);
        }

        public void showLaunchingApplication(final String title) {
            _dw.showLaunchingApplication(title);
        }
    }
    
    public boolean isResourceCached(final URL ref, final String version) {
        Boolean result = (Boolean)
            AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                // only check if it is valid, otherwise just false
                if (isResourceValid(ref, version)) try {
                    if (DownloadEngine.isResourceCached(ref, null, version)) {
                        return Boolean.TRUE;
                    }
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                }
                return Boolean.FALSE;
            }
        });
        return result.booleanValue();
    }
    
    public boolean isPartCached(final String part) {
        return isPartCached(new String [] {part});
    }
    
    public boolean isPartCached(final String [] parts) {
        Boolean result = (Boolean)AccessController.doPrivileged(
            new PrivilegedAction() {
            public Object run() {
                LaunchDesc ld = JNLPClassLoaderUtil.getInstance().getLaunchDesc();
                ResourcesDesc rd = ld.getResources();
                if (rd == null) return Boolean.FALSE;
                JARDesc [] jardescs = rd.getPartJars(parts);
                return new Boolean(isJARInCache(jardescs, true));
            }
        });
        return result.booleanValue();
    }
    
    public boolean isExtensionPartCached(final URL ref, 
        final String version, final String part) {
        return isExtensionPartCached(ref, version, new String [] {part});
    }
    
    public boolean isExtensionPartCached(final URL ref, final String version, 
                                         final String [] parts) {
        Boolean result = (Boolean)AccessController.doPrivileged(
            new PrivilegedAction() {
            public Object run() {
                LaunchDesc ld = 
                    JNLPClassLoaderUtil.getInstance().getLaunchDesc();
                ResourcesDesc resources = ld.getResources();
                if (resources == null) return Boolean.FALSE;
                JARDesc[] jardescs = resources.getExtensionPart(
                                                        ref, version, parts);
                return new Boolean(isJARInCache(jardescs, true));
            }
        });
        return result.booleanValue();
    }
    
    public void loadResource(final URL ref, final String version, 
        final DownloadServiceListener progress) throws IOException {

        // only load if it is valid
        if (isResourceValid(ref, version)) {
            try {
                AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws IOException {
                        ProgressHelper ph = getProgressHelper(progress);
                        try {                                
                            if (ref.toString().endsWith(".jar")) {
                                JNLPClassLoaderIf cl = 
                                    JNLPClassLoaderUtil.getInstance();
                                // always add it to the classloader
                                cl.addResource(ref, version, null);
                                // but only download if not allready cached
                                if (!isResourceCached(ref, version)) {
                                   ph = getProgressHelper(progress);
                                   cl.downloadResource(ref, version, ph, true);
                                }
                            } else {
                                // just want to put some file in the cache
                                DownloadEngine.getResource(ref, null, version,
                                                               null, true);
                                // check if this was a jnlp file:
                                CacheEntry ce = Cache.getCacheEntry(
                                                ref, null, version);
                                if (ce != null) {
                                    if (ce.isJNLPFile()) {
                                        loadResourceRecursivly(ce, progress);
                                    }
                                }
                            }
                        } catch (JNLPException e) {
                            throw new IOException(e.getMessage());
                        } finally {
                                ph.done();
                        }
                        return null;
                    }
                });
            } catch (PrivilegedActionException e) {
                throw (IOException)e.getException();
            }
        }
    }
    
    private void loadResourceRecursivly(CacheEntry ceJnlp,
        final DownloadServiceListener progress) {
        try {
            File jnlpFile = new File(ceJnlp.getResourceFilename());
            URL jnlpUrl = new URL(ceJnlp.getURL());
            LaunchDesc ld = LaunchDescFactory.buildDescriptor(
                jnlpFile, null, null, jnlpUrl);
            ResourcesDesc rsd = ld.getResources();
            if (rsd != null) {
                rsd.visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jad) { 
                        // load nested jar files (should we exceude lazy ?)
                        try {
                            loadResource(jad.getLocation(), 
                                         jad.getVersion(), progress);
                        } catch (IOException ioe) {
                            Trace.ignored(ioe);
                        }
                    }
                    public void visitExtensionDesc(ExtensionDesc ed) { 
                        // load nested extensions
                        try {
                            loadResource(ed.getLocation(), 
                                         ed.getVersion(), progress);
                        } catch (IOException ioe) {
                            Trace.ignored(ioe);
                        }
                    }
                    // ignore the rest
                    public void visitPropertyDesc(PropertyDesc prd) { }
                    public void visitPackageDesc(PackageDesc pad) { }
                    public void visitJREDesc(JREDesc jrd) { }
                });
            }
        } catch (Exception e) {
           Trace.ignored(e);
        }
    }

    public void loadPart(final String part, 
                final DownloadServiceListener progress) throws IOException {
        loadPart(new String [] {part}, progress);
    }
    
    public void loadPart(final String [] parts, 
                final DownloadServiceListener progress) throws IOException {        
        // Don't want to do anything if part is already cached
        if (isPartCached(parts)) return;
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    ProgressHelper ph = getProgressHelper(progress);
                    try {
                        JNLPClassLoaderUtil.getInstance().downloadParts(
                            parts, ph, true);
                    } catch (JNLPException e) {
                        throw new IOException(e.getMessage());
                    } finally {
                        ph.done();
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException e) {
            throw (IOException)e.getException();
        }
    }
    
    public void loadExtensionPart(final URL ref, final String version, 
        final String part, final DownloadServiceListener progress) 
            throws IOException {
        loadExtensionPart(ref, version, new String [] {part}, progress);
    }
    
    public void loadExtensionPart(final URL ref, final String version, 
        final String[] parts, final DownloadServiceListener progress) 
            throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    ProgressHelper ph = getProgressHelper(progress);
                    try {
                        JNLPClassLoaderUtil.getInstance().downloadExtensionParts(
                            ref, version, parts, ph, true);
                    } catch (JNLPException e) {
                        throw new IOException(e.getMessage());
                    } finally {
                        ph.done();
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException e) {
            throw (IOException)e.getException();
        }
    }
    
    public void removeResource(final URL ref, final String version) 
                               throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    if (isResourceValid(ref, version)) {
                        if (ref.toString().endsWith("jnlp")) {
                            // now can be used to uninstall yourself, or other
                            // apps and extensions from the same codebase.
                            CacheUtil.remove(
                                Cache.getCacheEntry(ref, null, version));
                        }
                        DownloadEngine.removeCachedResource(ref, null, version);
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException e) {
            throw (IOException)e.getException();
        }
    }
    
    public void removePart(final String part) throws IOException {
        removePart(new String [] {part});
    }
    
    public void removePart(final String[] parts) throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    LaunchDesc ld = 
                        JNLPClassLoaderUtil.getInstance().getLaunchDesc();
                    ResourcesDesc rd = ld.getResources();
                    if (rd == null) {
                        return null;
                    }
                    JARDesc [] jardescs = rd.getPartJars(parts);
                    removeJARFromCache(jardescs);
                    return null;
                }
            });
        } catch (PrivilegedActionException e) {
            throw (IOException)e.getException();
        }
    }
    
    public void removeExtensionPart(final URL ref, final String version, 
        final String part) throws IOException {
        removeExtensionPart(ref, version, new String [] {part});
    }
    
    public void removeExtensionPart(final URL ref, final String version, 
        final String [] parts) throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    LaunchDesc ld = 
                        JNLPClassLoaderUtil.getInstance().getLaunchDesc();
                    ResourcesDesc resources = ld.getResources();
                    if (resources == null) {
                        return null;
                    }
                    JARDesc[] jardescs = 
                        resources.getExtensionPart(ref, version, parts);
                    removeJARFromCache(jardescs);
                    return null;
                }
            });
        } catch (PrivilegedActionException e) {
            throw (IOException)e.getException();
        }
    }
    
    private void removeJARFromCache(JARDesc[] jardescs) throws IOException {
        if (jardescs == null) return;
        if (jardescs.length == 0) return;
        
        for (int i = 0; i < jardescs.length; i++) {
            DownloadEngine.removeCachedResource(
                    jardescs[i].getLocation(), null, jardescs[i].getVersion());
        }
    }
    
    /**
     * This will check if the given JAR represented by jardescs is in the cache.
     * If logical_and is true, isJARInCache will return true if ALL the JARs
     * are in the cache; if logical_and is false, isJARInCache will return false
     * if ANY of the JARs are in the cache.
     */
    private boolean isJARInCache(JARDesc [] jardescs, boolean logical_and) {
        if (jardescs == null) return false;
        if (jardescs.length == 0) return false;
        boolean result = true;
        for (int i = 0; i < jardescs.length; i++) {
            if (jardescs[i].isNativeLib()) {
                try {
                    if (DownloadEngine.getCachedJarFile(
                            jardescs[i].getLocation(), 
                            jardescs[i].getVersion()) != null) {
                        if (!logical_and) return true;
                    } else {
                        result = false;
                    }
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                    result = false;
                }
            } else {
                /* not a native library, it's a JAR */
                try {
                    if (DownloadEngine.getCachedJarFile(
                            jardescs[i].getLocation(), 
                            jardescs[i].getVersion()) != null) {
                        if (!logical_and) return true;
                    } else {
                        result = false;
                    }
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                    result = false;
                }
            }
        }
        return result;
    }

    private boolean isResourceValid(URL ref, String version) {
        LaunchDesc ld = JNLPClassLoaderUtil.getInstance().getLaunchDesc();
        JARDesc[] jars = ld.getResources().getEagerOrAllJarDescs(true);

        // everything valid if all-permissions
        if (ld.getSecurityModel() != ld.SANDBOX_SECURITY) {
            return true;
        }


        // is valid if listed in the jnlp file for this app
        for (int i=0; i<jars.length; i++) {
            if ((ref.toString().equals(jars[i].getLocation().toString())) &&
                (version == null || version.equals(jars[i].getVersion()))) {
                return true;
            }
        }

        URL codebase = ld.getCodebase();

        // now also valid if from the same codebase
        return (codebase != null && ref != null && 
                ref.toString().startsWith(codebase.toString()));
    }

    private ProgressHelper getProgressHelper(DownloadServiceListener dsl) {
        if (dsl instanceof ProgressHelper) {
            return ((ProgressHelper) dsl);
        } else {
            return new ProgressHelper(dsl);
        }
    }

    /* 
     * Progress Helper class
     *
     * The DownloadServiceListerner interface defined in the JNLP API is 
     * a subset of the DownloadProgressWindow interface used by elsewhere.
     *
     * this class is used to create a Helper object that implements both.
     */
    private class ProgressHelper extends CustomProgress {
 

        private DownloadServiceListener _dsp = null;

        public ProgressHelper() {
            _dsp = null;
        }

        public ProgressHelper(DownloadServiceListener dsp) {
            setAppThreadGroup(Thread.currentThread().getThreadGroup());
            setListener(dsp);
            _dsp = dsp;
            if (_dsp instanceof DefaultProgressHelper) {
                ((DefaultProgressHelper) _dsp).initialize();
            }
            // for bug #4432604:
            _dsp.progress(null, null, 0, 0, -1);
        }

        public void done() {
            if (_dsp instanceof DefaultProgressHelper) {
                ((DefaultProgressHelper) _dsp).done();
            } else {
                // make sure callbacks to DownloadServiceListener have
                // been called before returning (for TCK test)
                flush();
            }
        }
    }
}


