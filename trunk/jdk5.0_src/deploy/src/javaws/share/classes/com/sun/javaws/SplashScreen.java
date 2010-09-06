/*
 * %W% %E%
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.net.Socket;
import java.io.IOException;
import java.io.OutputStream;
import java.net.UnknownHostException;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.util.Properties;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import com.sun.javaws.cache.CacheImageLoader;
import com.sun.javaws.cache.CacheImageLoaderCallback;
import com.sun.javaws.jnl.*;
import javax.swing.*;
import javax.swing.border.*;

import com.sun.image.codec.jpeg.*;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
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
		
		Trace.println("exception closing socket: " + ioe, TraceLevel.BASIC);
		
		// Just ignore this
	    }
	}
    }

    static public void generateCustomSplash(Frame owner, 
					    LaunchDesc ld, boolean force) {

        SplashGenerator sg = new SplashGenerator(owner, ld);
	if (force || sg.needsCustomSplash()) {
	    sg.start();
	}
    }

    static public void removeCustomSplash(LaunchDesc ld) {
        SplashGenerator sg = new SplashGenerator(null, ld);
	sg.remove();
    }

}


/*
 * 
 * SplashCacheChecker - Thread that creates new Splash file in SplashCache
 *
 */
class SplashGenerator extends Thread implements CacheImageLoaderCallback {
    private File _index;
    private File _dir;
    private final String _key;
    private final LaunchDesc _ld;
    private final Frame _owner;
    private Properties _props = new Properties();
    private boolean _useAppSplash = false;

    public SplashGenerator(Frame owner, LaunchDesc ld) {
	_owner = owner;
	_ld = ld;
	_dir = new File(Config.getSplashDir());
	_key = _ld.getCanonicalHome().toString();

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
        IconDesc id = info.getIconLocation(InformationDesc.ICON_SIZE_LARGE,
                                           IconDesc.ICON_KIND_SPLASH);
	
	_useAppSplash = (id != null);

	if (!_useAppSplash) {
            id = info.getIconLocation(InformationDesc.ICON_SIZE_LARGE,
                                      IconDesc.ICON_KIND_DEFAULT);
	}

        if (id == null) {
	    try {
		create(null, null);
	    } catch (Throwable e) {
		splashError(e);
	    }
	} else {
	    CacheImageLoader.getInstance().loadImage(id, this);
	}
    }

    // implementation of two methods of CacheImageLoaderCallback:
    public void imageAvailable(IconDesc id, Image image, File file) {
    }

    public void finalImageAvailable(IconDesc id, Image image, File file) {
	if (!Globals.isHeadless()) {
	    try {
		create(image, file);
	    } catch (Throwable e) {  // cause were looking for OUTOFMEMORY errs
		splashError(e);
	    }   
	}
    }


    public void create(Image image, File cachedIconFile) {
	InformationDesc info = _ld.getInformation();
	String title = info.getTitle();
	String vendor = info.getVendor();
	BufferedImage bi;
	int imageWidth, imageHeight;
	int w, h;
	final int imageSize = 64;
	int offset = 5;
	Component comp = new JPanel();
	Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();

	Border b = new CompoundBorder(
		   BorderFactory.createLineBorder(Color.black),
                   BorderFactory.createBevelBorder(BevelBorder.RAISED));
	Insets insets = b.getBorderInsets(comp);

        w = 320;
        h = imageSize + 2 * offset + insets.top + insets.bottom;

	if (image == null) {
	    imageWidth = imageHeight = 0;
	} else {
	    if (_useAppSplash) {
		imageHeight = image.getHeight(_owner);
		imageWidth = image.getWidth(_owner);
		if (cachedIconFile != null) try {
		    String path = cachedIconFile.getCanonicalPath();
		    // if jpeg, we can just use it ...
		    if (path.endsWith(".jpg")) {
	                addSplashToCacheIndex(_key, path);
		 	return;
		    }
		} catch (IOException ioe) { /* ignore */ }
		offset = 0;
		w = Math.min(imageWidth, screenSize.width);
		h = Math.min(imageHeight, screenSize.height);
	    } else {
	    	imageWidth = imageHeight = imageSize;
	    }
	}


	bi = new BufferedImage(w, h, BufferedImage.TYPE_3BYTE_BGR); 
	Graphics2D g2d = bi.createGraphics();

	Rectangle imageRect;
	if (_useAppSplash) {
	    imageRect = new Rectangle(0, 0, w, h);
	} else {
	    g2d.setColor(new Color(238, 238, 238));
	    g2d.fillRect(0, 0, w, h);
	    g2d.setColor(Color.black);
	    b.paintBorder(comp, g2d, 0, 0, w, h);

	    Rectangle rect = new Rectangle(insets.left, insets.top,
					   w - insets.left - insets.right,
					   h - insets.top - insets.bottom);

	    Border imageBorder = BorderFactory.createLineBorder(Color.black);
	    Insets imageInsets = imageBorder.getBorderInsets(comp); 
	    imageRect = new Rectangle (insets.left + offset,
			insets.top + offset, imageWidth, imageHeight);

	    if (image != null) {
	        imageBorder.paintBorder(comp, g2d, 
		    imageRect.x - imageInsets.left,
		    imageRect.y - imageInsets.top,
		    imageRect.width + imageInsets.left + imageInsets.right,
		    imageRect.height + imageInsets.top + imageInsets.bottom);
		rect.x += imageWidth + 2 * offset;
		rect.width -= imageWidth + 2 * offset;
	    }
            Font titleFont   = new Font("SansSerif", Font.BOLD, 20);
            Font vendorFont  = new Font("SansSerif", Font.BOLD, 16);
	    g2d.setColor(Color.black);
       	    g2d.setFont(titleFont);
	    Rectangle textRect = new Rectangle(rect.x, rect.y+6, 
					       rect.width, rect.height-12);
	    textRect.height /= 2;
	    drawStringInRect(g2d, title, textRect, Label.CENTER);
       	    g2d.setFont(vendorFont);
	    textRect.y += textRect.height;
	    drawStringInRect(g2d, vendor, textRect, Label.CENTER);
	}

	if (image != null) {
	    int counter = 0;
	    while (g2d.drawImage(image, imageRect.x, imageRect.y,
		imageRect.width, imageRect.height, _owner) == false) {
		// Image not ready for primetime yet ...
	        try { 
		    Thread.sleep(2000);
	        } catch (Exception e) {};
		if (++counter > 5) {
		    // don't wait for ever ...
		    
		    Trace.println("couldnt draw splash image : "+image, TraceLevel.BASIC);
		    
		    break;	// ok skip image
		}
	    }
	}

	try {
	    File jpgFile = File.createTempFile("splash", ".jpg", _dir);
	    writeImage(jpgFile, bi);
	    addSplashToCacheIndex(_key, jpgFile.getCanonicalPath());
        } catch (IOException ioe) {	
            splashError(ioe);
	}
    }

    private void drawStringInRect(Graphics2D g, String str, 
				  Rectangle r, int alignment) {
	FontMetrics fm = g.getFontMetrics();
	Rectangle2D extent = fm.getStringBounds(str, g);
	int ascent = fm.getMaxAscent();
	int x,y;
	int swidth = (int) extent.getWidth();
	int sheight = (int) extent.getHeight();

	if (swidth > r.width) {
	    x = r.x;
	    String s1 = str.substring(0, str.length() - 3);
	    int len = s1.length();
	    while ((len > 3) && (fm.stringWidth(s1 + "...") > r.width)) {
		len--;
		s1 = s1.substring(0, len);
	    }
	    str = s1 + "...";
	} else switch (alignment) {
	    default:
	    case Label.LEFT:
		x = r.x;
		break;
	    case Label.CENTER:
		x = r.x + (r.width - swidth) / 2;
		break;
	    case Label.RIGHT:
		x = r.x + (r.width - swidth - 1);
		break;
	}
	if (x < r.x) { x = r.x; }

	y = r.y + ascent + (r.height - sheight)/2;
	g.drawString(str, x, y);
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

    // note - using code in : com.sun.image.codec.jpeg
    //
    private void writeImage(File file, BufferedImage image) {
	try {
	    FileOutputStream fos = new FileOutputStream(file);
	    JPEGImageEncoder jie = JPEGCodec.createJPEGEncoder(fos);
	    jie.encode(image);
        } catch (Throwable ioe) {	
            splashError(ioe);
	}

    }
    private void splashError(Throwable e) {
	LaunchErrorDialog.show(_owner, e, false);
        throw new Error(e.toString());
    }
}

