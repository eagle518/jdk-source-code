/*
 * @(#)CacheImageLoader.java	1.14 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.cache;

import com.sun.javaws.jnl.IconDesc;
import com.sun.javaws.exceptions.JNLPException;
import java.awt.Image;
import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import javax.swing.SwingUtilities;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/**
 * Handles loading of the image. Loading is initiated by invoking
 * loadImage, which will spawn an image loading thread if necessary.
 * <p>
 * This will spawn at most two threads. One for checking if the cache has
 * the image, and the other for checking if there is a new image available.
 * <p>
 * The loading of an image is a two step process. First the cache
 * is checked and if an image is available it is used (passed back to the callback.
 * Then a DelayedImageLoader is used (a separate thread) to check for a new image.
 * This two step process gives the effect of using all cached images
 * first, then checking for new ones.
 * <p>
 * When the image is found, the callback is invoked on the event dispatch thread.
 * Invokers should be interested in only one of the two callbacks.
 * imageAvailable() will be called once with the image from the cache, and then 
 * again (if necessary) if an update is found.
 * finalImageAvailable() - will be called after checking for an update
 */
public class CacheImageLoader implements Runnable {

    // singleton object
    private static CacheImageLoader _instance = null;

    // synchronization lock
    private final Object _imageLoadingLock = new Object();

    // This is true while the other Thread is running.
    private boolean _running = false;;

    // List of the IconDesc's and their callbacks
    private ArrayList _toLoad = new ArrayList();

    private class LoadEntry {
        public IconDesc _id;
	public URL _url;
        public CacheImageLoaderCallback _cb;
        public LoadEntry(IconDesc id, CacheImageLoaderCallback cb) {
            _id = id;
            _cb = cb;
	    _url = null;
        }
        public LoadEntry(URL url, CacheImageLoaderCallback cb) {
            _url = url;
            _cb = cb;
	    _id = null;
        }
    }

    // private constructor
    private CacheImageLoader () {
    }

    // singleton instance
    public static CacheImageLoader getInstance() {
        if (_instance == null) {
            _instance = new CacheImageLoader();
        }
        return _instance;
    }

    public void loadImage(IconDesc id, CacheImageLoaderCallback cb) {
        boolean createThread = false;
        synchronized(_imageLoadingLock) {
            if (!_running) {
                _running = true;
                createThread = true;
            } else {
	    }
            _toLoad.add(new LoadEntry(id, cb));
        }
        if (createThread) {
            new Thread(this).start();
        }
    }


    public void loadImage(URL url, CacheImageLoaderCallback cb) {
       boolean createThread = false;
        synchronized(_imageLoadingLock) {
            if (!_running) {
                _running = true;
                createThread = true;
            } else {
            } 
            _toLoad.add(new LoadEntry(url, cb));
        }
        if (createThread) {
            new Thread(this).start();
        }
    }

    public void run() {
        boolean done = false;
        while (!done) {
            LoadEntry entry = null;
            synchronized(_imageLoadingLock) {
                if (_toLoad.size() > 0) {
                     entry = (LoadEntry)_toLoad.remove(0);
                }
                else {
                    done = true;
                    _running = false;
                }
            }
            if (!done) {
                try {
                    DiskCacheEntry dce = null;
                    Image image = null;
		    File file = null;
		    URL url = entry._url;
		    if (url == null) {
                        dce = DownloadProtocol.getCachedVersion( 
                                         entry._id.getLocation(), 
                                         entry._id.getVersion(), 
                                         DownloadProtocol.IMAGE_DOWNLOAD);
                        if (dce != null) {
			    try {
				file = dce.getFile();
				url = file.toURL();
			    } catch (Exception e) {
			    }
                        }   
		    }
                    if (url != null) {
                        image = 
			    CacheUtilities.getSharedInstance().loadImage(url);
                    } 
                    if (image != null) {
                        publish(entry, image, file, false);
                    }
		    if (entry._id != null) {
                        // Check for a new one on a separate thread.
                        new DelayedImageLoader(entry, image, dce).start();
		    }
                } catch (MalformedURLException murle) {
                    Trace.ignoredException(murle);
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                }
            }
          
        }
    }
    
    private class DelayedImageLoader extends Thread {

        private LoadEntry _entry;
        private Image _image;
        private DiskCacheEntry _dce;

        public DelayedImageLoader(LoadEntry entry, Image image, DiskCacheEntry dce) {
            _entry = entry;
            _image = image;
            _dce = dce;
        }

        public void run() {
            try {
                File file = null;
                if (DownloadProtocol.isUpdateAvailable(_entry._id.getLocation(),
                        _entry._id.getVersion(), DownloadProtocol.IMAGE_DOWNLOAD)) {
                    _dce = DownloadProtocol.getResource(_entry._id.getLocation(), 
                        _entry._id.getVersion(), DownloadProtocol.IMAGE_DOWNLOAD, 
                        false, null);
                    if (_dce != null) {

                        file = _dce.getFile();
                    }
                    if (file != null) {
                        _image = CacheUtilities.getSharedInstance().
                            loadImage(file.getPath());
                    }
                    CacheImageLoader.publish(_entry, _image, file, false);
                } else if (_dce != null) {
                    file = _dce.getFile();
                }
                CacheImageLoader.publish(_entry, _image, file, true);

            } catch (MalformedURLException murle) { Trace.ignoredException(murle);
            } catch (IOException ioe) { Trace.ignoredException(ioe);
            } catch (JNLPException je) { Trace.ignoredException(je); }
        }
    }

    private static void publish(final LoadEntry entry, final Image image, 
                                final File file, final boolean isComplete) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                if (isComplete) {
                    entry._cb.finalImageAvailable(entry._id, image, file);
                } else {
                    entry._cb.imageAvailable(entry._id, image, file);
                }
            }
        });
    }
}
