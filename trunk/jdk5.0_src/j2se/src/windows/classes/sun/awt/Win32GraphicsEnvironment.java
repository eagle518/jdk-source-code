/*
 * @(#)Win32GraphicsEnvironment.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import sun.java2d.SunGraphicsEnvironment;
import java.awt.GraphicsDevice;
import java.awt.Toolkit;
import java.awt.print.PrinterJob;
import java.io.File;
import java.io.IOException;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;
import sun.awt.windows.WPrinterJob;
import sun.awt.windows.WToolkit;
import sun.awt.DisplayChangedListener;
import sun.awt.SunDisplayChanger;
import sun.font.FontManager;
import sun.awt.windows.WFontConfiguration;

import java.io.File;

/**
 * This is an implementation of a GraphicsEnvironment object for the
 * default local GraphicsEnvironment used by the Java Runtime Environment
 * for Windows.
 *
 * @see GraphicsDevice
 * @see GraphicsConfiguration
 * @version 1.33 12/19/03
 */

public class Win32GraphicsEnvironment extends SunGraphicsEnvironment
 implements DisplayChangedListener {

    SunDisplayChanger displayChanger = new SunDisplayChanger();

    static {
	// Ensure awt is loaded already.  Also, this forces static init
	// of WToolkit and Toolkit, which we depend upon
	WToolkit.loadLibraries();
	// setup flags before initializing native layer
	WindowsFlags.initFlags();
	initDisplayWrapper();
	eudcFontFileName = getEUDCFontFile();
    }
    
    /**
     * Noop function that just acts as an entry point for someone to force
     * a static initialization of this class.
     */
    public static void init() {}

    /**
     * Initializes native components of the graphics environment.  This
     * includes everything from the native GraphicsDevice elements to
     * the DirectX rendering layer.
     */
    private static native void initDisplay();

    private static boolean displayInitialized;	    // = false;
    public static void initDisplayWrapper() {
	if (!displayInitialized) {
	    displayInitialized = true;
	    initDisplay();
	}
    }
    
    public Win32GraphicsEnvironment() {
    }

    protected native int getNumScreens();
    protected native int getDefaultScreen();

    public GraphicsDevice getDefaultScreenDevice() {
	return getScreenDevices()[getDefaultScreen()];
    }
  
/*
 * ----DISPLAY CHANGE SUPPORT----
 */

    /*
     * From DisplayChangeListener interface.
     * Called from WToolkit and executed on the event thread when the
     * display settings are changed.
     */
    public void displayChanged() {

        // 1. Reset screen array, saving old array
        GraphicsDevice[] oldgds = resetDisplays();

        // 2. Reset the static GC for the (possibly new) default screen
        WToolkit.resetGC();

        // 3. Tell screens in old screen array to do display update stuff
        //    This will update all top-levels windows so they can be added
        //    to the new graphics devices.

        //NOTE: for multiscreen, this could only reset the devices that
        //changed.  Be careful of situations where screens are added
        //as well as removed, yielding no net change in the number of
        //displays.  For now, we're being robust and doing them all.

        for(int s = 0; s < oldgds.length; s++) {
            if (oldgds[s] instanceof Win32GraphicsDevice) {
                ((Win32GraphicsDevice)oldgds[s]).displayChanged();
            }
        }

        // 4. Do displayChanged for SunDisplayChanger list (i.e. WVolatileImages
        //    and Win32OffscreenImages)
        displayChanger.notifyListeners();

    }

    /**
     * Part of the DisplayChangedListener interface: 
     * propagate this event to listeners
     */
    public void paletteChanged() {
        displayChanger.notifyPaletteChanged();
    }

    /*
     * Updates the array of screen devices to the current configuration.
     * Returns the previous array of screen devices.
     */
    public synchronized GraphicsDevice[] resetDisplays() {
        // REMIND : We should go through the array and update any displays
        // which may have been added or removed.  Also, we should update
        // any display state information which may now be different.
        // Note that this function used to re-create all Java
        // Win32GraphicsDevice objects.  We cannot do this for many reasons,
        // since those objects keep state that is independent of display
        // changes, and because programs may keep references to those
        // java objects.
        GraphicsDevice[] ret = screens;
        return ret;
    }

    /*
     * Add a DisplayChangeListener to be notified when the display settings
     * are changed.  
     */
    public void addDisplayChangedListener(DisplayChangedListener client) {
        displayChanger.add(client);
    }

    /*
     * Remove a DisplayChangeListener from Win32GraphicsEnvironment
     */
    public void removeDisplayChangedListener(DisplayChangedListener client) {
        displayChanger.remove(client);
    }

/*
 * ----END DISPLAY CHANGE SUPPORT----
 */
    /* Used on Windows to obtain from the windows registry the name
     * of a file containing the system EUFC font. If running in one of
     * the locales for which this applies, and one is defined, the font
     * defined by this file is appended to all composite fonts as a
     * fallback component.
     */
    private static native String getEUDCFontFile();

    /**
     * Whether registerFontFile expects absolute or relative
     * font file names.
     */
    protected boolean useAbsoluteFontFileNames() {
        return false;
    }

    /* Unlike the shared code version, this expects a base file name -
     * not a full path name.
     * The font configuration file has base file names and the FontConfiguration
     * class reports these back to the GraphicsEnvironment, so these
     * are the componentFileNames of CompositeFonts.
     */
    protected void registerFontFile(String fontFileName, String[] nativeNames,
                                    int fontRank, boolean defer) {

        // REMIND: case compare depends on platform
        if (registeredFontFiles.contains(fontFileName)) {
            return;
        }
	registeredFontFiles.add(fontFileName);

        int fontFormat;
        if (ttFilter.accept(null, fontFileName)) {
            fontFormat = FontManager.FONTFORMAT_TRUETYPE;
        } else if (t1Filter.accept(null, fontFileName)) {
            fontFormat = FontManager.FONTFORMAT_TYPE1;
        } else {
	    /* on windows we don't use/register native fonts */
	    return;
	}

	if (fontPath == null) {
	    fontPath = getPlatformFontPath(noType1Font);
	}

        StringTokenizer parser = new StringTokenizer(fontPath, 
                                                     File.pathSeparator);
        try {
            while (parser.hasMoreTokens()) {
                String newPath = parser.nextToken();
		File theFile = new File(newPath, fontFileName);
                if (theFile.canRead()) {
		    String path = theFile.getAbsolutePath();
		    if (defer) {
			FontManager.registerDeferredFont(fontFileName, path,
							 nativeNames,
							 fontFormat, true,
							 fontRank);
		    } else {
			FontManager.registerFontFile(path, nativeNames,
						     fontFormat, true,
						     fontRank);
		    }
                    break;
                }
            }
        } catch (NoSuchElementException e) {
            System.err.println(e);
        }
    }

    /* register only TrueType/OpenType fonts
     * Because these need to be registed just for use when printing,
     * we defer the actual registration and the static initialiser
     * for the printing class makes the call to registerJREFontsForPrinting()
     */
    static String fontsForPrinting = null;
    protected void registerJREFontsWithPlatform(String pathName) {
	fontsForPrinting = pathName;
    }

    public static void registerJREFontsForPrinting() {
	String pathName = null;
	synchronized (Win32GraphicsEnvironment.class) {
	    if (fontsForPrinting == null) {
		return;
	    }
	    pathName = fontsForPrinting;
	    fontsForPrinting = null;
	}
        File f1 = new File(pathName);
	String[] ls = f1.list(new TTFilter());
	if (ls == null) {
	  return;
	}
	for (int i=0; i <ls.length; i++ ) {
	  File fontFile = new File(f1, ls[i]);	  
	  registerFontWithPlatform(fontFile.getAbsolutePath());
	}
    }

    protected static native void registerFontWithPlatform(String fontName);

    protected static native void deRegisterFontWithPlatform(String fontName);

    protected GraphicsDevice makeScreenDevice(int screennum) {
        return new Win32GraphicsDevice(screennum);
    }

    /**
     * Gets a <code>PrinterJob</code> object suitable for the
     * the current platform.
     * @return    a <code>PrinterJob</code> object.
     * @see       java.awt.PrinterJob
     * @since     JDK1.2
     */
    public PrinterJob getPrinterJob() {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkPrintJobAccess();
	}

	return new WPrinterJob();
    }

    // Implements SunGraphicsEnvironment.createFontConfiguration.
    protected FontConfiguration createFontConfiguration() {
	return new WFontConfiguration(this);
    }

    public FontConfiguration createFontConfiguration(boolean preferLocaleFonts,
						     boolean preferPropFonts) {
	
	return new WFontConfiguration(this, preferLocaleFonts,preferPropFonts);
    }
}

