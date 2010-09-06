/*
 * @(#)DownloadServiceImpl.java	1.18 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import javax.jnlp.DownloadService;
import java.net.URL;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.javaws.cache.Cache;
import java.io.IOException;
import com.sun.javaws.exceptions.JNLPException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import com.sun.javaws.exceptions.DownloadException;
import javax.jnlp.DownloadServiceListener;
import com.sun.javaws.LaunchDownload;
import com.sun.javaws.ui.DownloadWindow;

public final class DownloadServiceImpl implements DownloadService {
    
    static private DownloadServiceImpl _sharedInstance = null;
    private DownloadServiceListener _defaultProgress = null;
    
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
    
    public DownloadServiceListener getDefaultProgressWindow() {
        if (_defaultProgress == null) {
	    _defaultProgress = (DownloadServiceListener)
		AccessController.doPrivileged(new PrivilegedAction() {
			public Object run() {
			    return new DefaultProgressImpl(new DownloadWindow(JNLPClassLoader.getInstance().getLaunchDesc(), false));
			}
		    });
        }
        return _defaultProgress;
    }
    
    private class DefaultProgressImpl implements DownloadServiceListener {
        
        private DownloadWindow _dw = null;
        
        DefaultProgressImpl(final DownloadWindow dw) {	    
	    AccessController.doPrivileged(new PrivilegedAction() {
			public Object run() {
			    _dw = dw;
			    _dw.buildIntroScreen();			    
			    _dw.showLoadingProgressScreen();
			    return null;
			}
		    });
        }
        
        public void progress(URL url, String version, long readSoFar, long total, int overallPercent) {
	    ensureVisible();
	    if (readSoFar == 0) _dw.resetDownloadTimer();
	    _dw.progress(url, version, readSoFar, total, overallPercent);
	    if (overallPercent >= 100) hideFrame();
	    // Check if cancel has been invoked. This exception will be caught by the 
	    // DownloadProtocol and converted into a failed download request
	    if (_dw.isCanceled()) {		
		hideFrame();
		throw new RuntimeException("canceled by user"); /* no need to localize */
	    }
	}
	
	
	public void validating(URL url, String version, long entry, long total, int overallPercent) {
	    ensureVisible();
	    _dw.validating(url, version, entry, total, overallPercent);
	    // for some reason sometimes we are called with overallPercent = -1
	    // when download is complete.  this is a workaround for that problem.
            //     bug #4432604
	    //  -- was if (entry >= total) hideFrame();
            //  -- this caused frame off and back on between jars of multi jar parts, 
	    //  -- so, allthough I don't know circumstances where the above comment would apply,
	    //  -- we shouldn't hide when overallPercent is in {0,...99}
	    if (entry >= total && (overallPercent < 0 || overallPercent >=99)) hideFrame();		    	    
	}
	
	public void upgradingArchive(URL url, String version, int patchPercent, int overallPercent) {
	    ensureVisible();
	    _dw.patching(url, version, patchPercent, overallPercent);
	    if (overallPercent >= 100) hideFrame();
	    
	}
	
	public void downloadFailed(URL url, String version) {	    
	    hideFrame();
	}
	
	private void ensureVisible() {
	    if (!_dw.getFrame().isVisible()) {
		_dw.getFrame().setVisible(true);
                _dw.getFrame().toFront();
	    }
	}
	
	private synchronized void hideFrame() {
	    _dw.resetCancled();
	    _dw.getFrame().hide();	    
	}    	
    }
    
    public boolean isResourceCached(final URL ref, final String version) {
	Boolean result = (Boolean)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			if (DownloadProtocol.isInCache(ref, version,
						       DownloadProtocol.JAR_DOWNLOAD) ||
			    DownloadProtocol.isInCache(ref, version,
						       DownloadProtocol.NATIVE_DOWNLOAD)) {
			    return Boolean.TRUE;
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
	Boolean result = (Boolean)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
			ResourcesDesc rd = ld.getResources();
			if (rd == null) return Boolean.FALSE;
			JARDesc [] jardescs = rd.getPartJars(parts);
			return new Boolean(isJARInCache(jardescs, true));
		    }
		});
	return result.booleanValue();
    }
    
    public boolean isExtensionPartCached(final URL ref, final String version, final String part) {
	return isExtensionPartCached(ref, version, new String [] {part});
    }
    
    public boolean isExtensionPartCached(final URL ref, final String version, final String [] parts) {
	Boolean result = (Boolean)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
			ResourcesDesc resources = ld.getResources();
			if (resources == null) return Boolean.FALSE;
			JARDesc[] jardescs = resources.getExtensionPart(ref, version, parts);
			return new Boolean(isJARInCache(jardescs, true));
		    }
		});
	return result.booleanValue();
    }
    
    public void loadResource(final URL ref, final String version, final DownloadServiceListener progress) throws IOException {
	// Don't want to do anything if part is already cached
	if (isResourceCached(ref, version)) return;
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    try {				
				JNLPClassLoader.getInstance().downloadResource(ref, version, new ProgressHelper(progress), true);
			    } catch (JNLPException e) {
				throw new IOException(e.getMessage());
			    }
			    return null;
			}
		    });
	} catch (PrivilegedActionException e) {
	    throw (IOException)e.getException();
	}
    }
    
    public void loadPart(final String part, final DownloadServiceListener progress) throws IOException {
	loadPart(new String [] {part}, progress);
    }
    
    public void loadPart(final String [] parts, final DownloadServiceListener progress) throws IOException {	
	// Don't want to do anything if part is already cached
	if (isPartCached(parts)) return;
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    try {
				JNLPClassLoader.getInstance().downloadParts(parts, new ProgressHelper(progress), true);
			    } catch (JNLPException e) {
				throw new IOException(e.getMessage());
			    }
			    return null;
			}
		    });
	} catch (PrivilegedActionException e) {
	    throw (IOException)e.getException();
	}
    }
    
    public void loadExtensionPart(final URL ref, final String version, final String part, final DownloadServiceListener progress) throws IOException {
	loadExtensionPart(ref, version, new String [] {part}, progress);
    }
    
    public void loadExtensionPart(final URL ref, final String version, final String[] parts, final DownloadServiceListener progress) throws IOException {
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    try {
				JNLPClassLoader.getInstance().downloadExtensionParts(ref, version, parts, new ProgressHelper(progress), true);
			    } catch (JNLPException e) {
				throw new IOException(e.getMessage());
			    }
			    return null;
			}
		    });
	} catch (PrivilegedActionException e) {
	    throw (IOException)e.getException();
	}
    }
    
    public void removeResource(final URL ref, final String version) throws IOException {
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
			    ResourcesDesc resources = ld.getResources();
			    if (resources == null) return null;
			    JARDesc[] jardescs = resources.getResource(ref, version);
			    removeJARFromCache(jardescs);
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
			    LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
			    ResourcesDesc rd = ld.getResources();
			    if (rd == null) return null;
			    JARDesc [] jardescs = rd.getPartJars(parts);
			    removeJARFromCache(jardescs);
			    return null;
			}
		    });
	} catch (PrivilegedActionException e) {
	    throw (IOException)e.getException();
	}
    }
    
    public void removeExtensionPart(final URL ref, final String version, final String part) throws IOException {
	removeExtensionPart(ref, version, new String [] {part});
    }
    
    public void removeExtensionPart(final URL ref, final String version, final String [] parts) throws IOException {
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
			    ResourcesDesc resources = ld.getResources();
			    if (resources == null) return null;
			    JARDesc[] jardescs = resources.getExtensionPart(ref, version, parts);
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
	DiskCacheEntry dce = null;
	for (int i = 0; i < jardescs.length; i++) {
	    int type = jardescs[i].isNativeLib() ? DownloadProtocol.NATIVE_DOWNLOAD :
		DownloadProtocol.JAR_DOWNLOAD;
	    try {
		dce = DownloadProtocol.getResource(jardescs[i].getLocation(),
						   jardescs[i].getVersion(),
						   type, true, null);
	    } catch (JNLPException e) {
		throw new IOException(e.getMessage());
	    }
	    if (dce != null) {
		Cache.removeEntry(dce);
	    }
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
		if (DownloadProtocol.isInCache(jardescs[i].getLocation(),
					       jardescs[i].getVersion(),
					       DownloadProtocol.NATIVE_DOWNLOAD)) {
		    if (!logical_and) return true;
		} else {
		    result = false;
		}
	    } else {
		/* not a native library, it's a JAR */
		if (DownloadProtocol.isInCache(jardescs[i].getLocation(),
					       jardescs[i].getVersion(),
					       DownloadProtocol.JAR_DOWNLOAD)) {
		    if (!logical_and) return true;
		} else {
		    result = false;
		}
	    }
	}
	return result;
    }
    
    private class ProgressHelper implements LaunchDownload.DownloadProgress {
	
	DownloadServiceListener _dsp = null;
	
	public ProgressHelper(DownloadServiceListener dsp) {
	    _dsp = dsp;
	    // for bug #4432604:
	    _dsp.progress(null, null, 0, 0, -1);
	}
	
	public void extensionDownload(String name, int remaining) {
	    // do nothing
	}
	
	public void jreDownload(String versionId, URL location) {
	    // do nothing
	}
	
	// The percentage might not be readSoFar/total due to the validation pass
	// The total and percentage will be -1 for unknown
	public void progress  (URL url, String version, long readSoFar, long total, int overallPercent) {
	    if (_dsp != null) {
		_dsp.progress(url, version, readSoFar, total, overallPercent);
	    }
	}
	public void validating(URL url, String version, long entry,     long total, int overallPercent) {
	    if (_dsp != null) {
		_dsp.validating(url, version, entry, total, overallPercent);
	    }
	}
	public void patching(URL url, String version, int patchPercent, int overallPercent) {
	    if (_dsp != null) {
		_dsp.upgradingArchive(url, version, patchPercent, overallPercent);
	    }
	}
	public void downloadFailed(URL url, String version) {
	    if (_dsp != null) {
		_dsp.downloadFailed(url, version);
	    }
	}
    }
}


