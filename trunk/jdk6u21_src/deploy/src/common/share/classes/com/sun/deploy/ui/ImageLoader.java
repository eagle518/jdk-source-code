/*
 * @(#)ImageLoader.java	1.25 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.ui;

import java.awt.*;
import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import javax.swing.SwingUtilities;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.cache.Cache;

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
public class ImageLoader implements Runnable {
   /**
    * Used for loading images.
    */
    private Component _component;
    // singleton object
    private static ImageLoader _instance = null;

    // synchronization lock
    private final Object _imageLoadingLock = new Object();

    // This is true while the other Thread is running.
    private boolean _running = false;;

    // List of the IconDesc's and their callbacks
    private ArrayList _toLoad = new ArrayList();

    private class LoadEntry {
	public URL url;
	public URL iconRef;
	public String iconVer;
        public ImageLoaderCallback cb;
        public boolean useCached;

        public LoadEntry(URL iconRef, String iconVer, 
                         ImageLoaderCallback cb, boolean useCached) {
            this.cb = cb;
	    this.url = null;
	    this.iconRef = iconRef;
	    this.iconVer = iconVer;
            this.useCached = useCached;
        }

        public LoadEntry(URL url, ImageLoaderCallback cb, boolean useCached) {
            this.url = url;
            this.cb = cb;
	    this.iconRef = null;
	    this.iconVer = null;
            this.useCached = useCached;
        }
	public String toString() {
	    return "LoadEntry:\n" +
		"  url: " + url + "\n" +
		"  iconRef " + iconRef + "\n" +
		"  iconVer: " + iconVer +
                "  useCached: " + useCached;
	}
    }

    // private constructor
    private ImageLoader () {
    }

    // singleton instance
    public static ImageLoader getInstance() {
        if (_instance == null) {
            _instance = new ImageLoader();
        }
        return _instance;
    }

    /**
     * Returns a component that can be used for image loading.
     */
    private Component getComponent() {
        if (_component == null) {
            synchronized(this) {
                if (_component == null) {
                    _component = new Component() {};
                }
            }
        }
        return _component;
    }

    /**
     * Loads the image at the path <code>path</code>.
     */
    public Image loadImage(String path) throws IOException {
        Image image = Toolkit.getDefaultToolkit().createImage(path);
        if (image != null) {
            Component c = getComponent();
            MediaTracker mt = new MediaTracker(c);
            mt.addImage(image, 0);
            try {
                // Give it 5 seconds to download.
                mt.waitForID(0, 5000);
            } catch (InterruptedException e) {
                throw new IOException("Failed to load");
            }
            return image;
        }
        return null;
    }

    /**
     * Loads the image at the url <code>url</code>.
     */
    public Image loadImage(URL url) throws IOException {
        Image image = Toolkit.getDefaultToolkit().createImage(url);
        if (image != null) {
            Component c = getComponent();
            MediaTracker mt = new MediaTracker(c);
            mt.addImage(image, 0);
            try {
                // Give it 5 seconds to download.
                mt.waitForID(0, 5000);
            } catch (InterruptedException e) {
                throw new IOException("Failed to load");
            }
            return image;
        }
        return null;
    }
    
    public void loadImage(URL iconRef, String iconVer, ImageLoaderCallback cb) {
        loadImage(iconRef, iconVer, cb, false);
    }

    public void loadImage(URL iconRef, String iconVer, ImageLoaderCallback cb,
                          boolean useCached) {
        boolean createThread = false;
        synchronized(_imageLoadingLock) {
            if (!_running) {
                _running = true;
                createThread = true;
            } else {
	    }
            _toLoad.add(new LoadEntry(iconRef, iconVer, cb, useCached));
        }
        if (createThread) {
            new Thread(this).start();
        }
    }

    public void loadImage(URL url, ImageLoaderCallback cb) {
        loadImage(url, cb, false);
    }

    public void loadImage(URL url, ImageLoaderCallback cb, boolean useCached) {
       boolean createThread = false;
        synchronized(_imageLoadingLock) {
            if (!_running) {
                _running = true;
                createThread = true;
            } 
            _toLoad.add(new LoadEntry(url, cb, useCached));
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
                    Image image = null;
                    File file = null;
                    URL url = entry.url;
                    if (url == null) {
                        file = DownloadEngine.getCachedFile(entry.iconRef, 
                                   entry.iconVer);
                        if (file != null) {
                            url = URLUtil.fileToURL(file);
                        }
		    }
                    if (url != null) {
                        image = loadImage(url);
                    }
                    if (image != null) {
                        if (entry.useCached) {
                            publish(entry, image, file, true);
                        } else {
                            publish(entry, image, file, false);
                            if (entry.iconRef != null) {
                                // Check for a new one on a separate thread.
                                new DelayedImageLoader(entry, image).start();
                            }
                        }
                    } else {
                        if (entry.iconRef != null) {
                            // not in cache, use DelayedImageLoader
                            new DelayedImageLoader(entry, image).start();
                        }
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
      

        public DelayedImageLoader(LoadEntry entry, Image image) {
            _entry = entry;
            _image = image;
       
        }

        public void run() {
            try {
                if (Cache.isCacheEnabled()) {
                    File file = DownloadEngine.getUpdatedFile(_entry.iconRef,
                                                              _entry.iconVer);

                    if (file != null) {
                        _image = loadImage(file.getPath());
                        ImageLoader.publish(_entry, _image, file, true);
                    }
                } else {
		    // looks broken - versioned icon with cache disabled 
                    _image = loadImage(_entry.iconRef);
                    ImageLoader.publish(_entry, _image, null, true);
                }

            } catch (MalformedURLException murle) { 
                Trace.ignoredException(murle);
            } catch (IOException ioe) { 
                Trace.ignoredException(ioe);
            }
        }
    }

    private static void publish(LoadEntry entry, final Image image, 
                                final File file, final boolean isComplete) {
	final URL url = entry.iconRef;
	final String version = entry.iconVer;
	final ImageLoaderCallback cb = entry.cb;

        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                if (isComplete) {
                    cb.finalImageAvailable(url, version, image, file);
                } else {
                    cb.imageAvailable(url, version, image, file);
                }
            }
        });
    }
}
