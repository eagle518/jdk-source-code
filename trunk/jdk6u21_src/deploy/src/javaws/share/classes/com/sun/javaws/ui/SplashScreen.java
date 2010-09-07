/*
 * %W% %E%
 * 
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import java.net.Socket;
import java.io.IOException;
import java.io.OutputStream;
import java.net.UnknownHostException;

import java.awt.*;
import java.net.URL;
import java.awt.image.BufferedImage;
import java.util.Properties;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

import com.sun.javaws.jnl.*;
import com.sun.javaws.Globals;
import com.sun.javaws.JnlpxArgs;
import com.sun.deploy.ui.ImageLoader;
import com.sun.deploy.ui.ImageLoaderCallback;
import com.sun.deploy.ui.AppInfo;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
/*
 * The SplashScreen class contains basic functionality for communicating
 * with the spash screen, e.g., to remove it.
 *
 * The port-number for the spash screen is expected in javawsx.splashport
 */
public class SplashScreen {
    static private boolean _alreadyHidden = false;
    
    /** Token to send to the spash screen in order for it to exit */
    static final private int HIDE_SPASH_SCREEN_TOKEN = 'Z';
    	 
    /**
     * Closes out any splash screen that may be showing. This determines
     * the port from the <code>LaunchProperties</code>.
     */
    static public void hide() {
        hide(JnlpxArgs.getSplashPort());
    }
    
    /**
     * Closes out any splash screen that may be showing.
     */
    static private void hide(int port) {
	// A port of -1 means no spash screen
	if (port <= 0 || _alreadyHidden) return;

	_alreadyHidden = true;
	 	
	Socket socket = null;
	try {
	    socket = new Socket("127.0.0.1", port);
	    if (socket != null) {
		OutputStream os = socket.getOutputStream();
		try {
		    os.write(HIDE_SPASH_SCREEN_TOKEN);
		    os.flush();
		} catch (IOException ioe) {
		    // Just ignore this
		}
		os.close();
	    } else {
		// Just ignore this
	    }
	} catch (IOException ioe) {
	    // Just ignore this
	    // this will happen if timeout in native code
	} catch(Exception e) {
	    // This should never happen unless socket proxy failure
	    Trace.ignoredException(e);
	}
	if (socket != null) {
	    try {
		socket.close();
	    } catch (IOException ioe) {
		Trace.println("exception closing socket: " + ioe, 
			TraceLevel.BASIC);
		// Just ignore this
	    }
	}
    }

    static public void generateCustomSplash(Component owner, 
					    LaunchDesc ld, boolean force) {
        if (Cache.isCacheEnabled() == false) {
            return;
        }
        if (ld.isApplicationDescriptor()) {
            //only for applets and applications
            SplashGenerator sg = new SplashGenerator(owner, ld);
            if (force || sg.needsCustomSplash()) {
                sg.start();
            }
        }
    }

    static public void removeCustomSplash(LaunchDesc ld) {
        if (ld.isApplicationDescriptor()) {
            //only for applets and applications
            SplashGenerator sg = new SplashGenerator(null, ld);
	    sg.remove();
        }
    }

}


/*
 * 
 * SplashCacheChecker - Thread that creates new Splash file in SplashCache
 *
 */
class SplashGenerator extends Thread implements ImageLoaderCallback {
    private File _index;
    private File _dir;
    private final String _key;
    private final LaunchDesc _ld;
    private final Component _owner;
    private Properties _props = new Properties();
    private boolean _useAppSplash = false;

    public SplashGenerator(Component owner, LaunchDesc ld) {
	_owner = owner;
	_ld = ld;
	_dir = new File(Config.getSplashDir());
	_key = _ld.getSplashCanonicalHome().toString();

	String indexFile = Config.getSplashIndex();
	_index = new File(indexFile);

	// make sure there is a entry javaws.cfg.splash.cache in the 
	// configuration file for the native code to read.
	Config.setSplashCache();
	Config.storeIfDirty();

	if (_index.exists()) {
	    try {
	        FileInputStream fis = new FileInputStream(_index);
	        if (fis != null) {
		    _props.load(fis);
		    fis.close();
	        }
            } catch (IOException ioe) {	
                Trace.ignoredException(ioe);
	    }
	}
    }

    public boolean needsCustomSplash() {
	return (!_props.containsKey(_key));
    }

    public void remove() {
	addSplashToCacheIndex(_key, null);
    }

    public void run() {
	final InformationDesc info = _ld.getInformation();
	IconDesc[] icons = info.getIcons();

	if (_dir.getParentFile().canWrite() == false ||
	       (_dir.exists() && _dir.canWrite() == false) ||
               (_index.exists() && _index.canWrite() == false)) {
	    // cannot generate splash file, use default
	    return;
	}

	// create the splash dir if it dosn't exist.
	try {
	    _dir.mkdirs();
	} catch (Throwable e) {
	    splashError(e);
	}
	
	// create the splash cache index file if it dosn't exist
	try {
	    _index.createNewFile();
	} catch (Throwable e) {
	    splashError(e);
	}

	// try to use the "SPLASH" icon;
        IconDesc id = info.getIconLocation(AppInfo.ICON_SIZE,
                                           IconDesc.ICON_KIND_SPLASH);
	
	if (id == null) {
	    // no longer generating images.  If no icon specifice with
            // kind=splash, then continue to use the default splsh screen.
	    return;
	}

        ImageLoader.getInstance().loadImage(
	    id.getLocation(), id.getVersion(), this);
    }

    // implementation of two methods of CacheImageLoaderCallback:
    public void imageAvailable(
	URL url, String version, Image image, File file) {
    }

    public void finalImageAvailable(
	URL url, String version, Image image, File file) {
	try {
	    create(image, file);
        } catch (Throwable t) {  // cause were looking for OUTOFMEMORY errs
            if (t instanceof OutOfMemoryError) {
                splashError(t);
            } else {
                Trace.ignored(t);
            }
        }
    }


    public void create(Image image, File cachedIconFile) {
	InformationDesc info = _ld.getInformation();
	int imageWidth, imageHeight;

	imageHeight = image.getHeight(null);
	imageWidth = image.getWidth(null);
	if (cachedIconFile != null) try {
	    String path = cachedIconFile.getCanonicalPath();
	    // let splashscreen libray handle all types
            addSplashToCacheIndex(_key, path);
	} catch (Throwable t) {
	    Trace.ignored(t);
	}
    }

    private void addSplashToCacheIndex(String key, String value) {
	// add new file to properties.
	if (value != null) {
	    _props.setProperty(key, value);
	} else if (_props.containsKey(key)) {
	    _props.remove(key);
	}

	// attempt to clean SplashCache of unreferenced files.
	File[] files = _dir.listFiles();

	// fix for 4744945
	// no splash yet
	if (files == null) return;

	for (int i=0; i<files.length; i++) {
	    // don't try todelete the index file
	    if (!files[i].equals(_index)) try {
		String path = files[i].getCanonicalPath();
		if (!_props.containsValue(path)) {
		    files[i].delete();
		}
	    } catch (IOException ioe) {	
		splashError(ioe);
	    }
	}

        try {
            FileOutputStream fos = new FileOutputStream(_index);
            _props.store(fos, "");
            fos.flush();
            fos.close();
        } catch (IOException ioe) {	
            splashError(ioe);
	}
    }

    private void splashError(Throwable e) {
	LaunchErrorDialog.show(_owner, e, false);
        throw new Error(e.toString());
    }
}

