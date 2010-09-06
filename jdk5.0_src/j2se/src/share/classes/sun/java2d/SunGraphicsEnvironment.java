/*
 * @(#)SunGraphicsEnvironment.java	1.124 04/03/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.font.TextAttribute;
import java.awt.image.BufferedImage;
import java.awt.print.PrinterJob;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.InputStreamReader;
import java.io.IOException;
import java.text.AttributedCharacterIterator;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.TreeMap;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import sun.awt.AppContext;
import sun.awt.FontConfiguration;
import sun.font.CompositeFontDescriptor;
import sun.font.Font2D;
import sun.font.FontManager;
import sun.font.NativeFont;

/**
 * This is an implementation of a GraphicsEnvironment object for the
 * default local GraphicsEnvironment.
 *
 * @see GraphicsDevice
 * @see GraphicsConfiguration
 * @version 1.124 03/09/04
 */

public abstract class SunGraphicsEnvironment extends GraphicsEnvironment
    implements FontSupport {

    public static boolean isLinux;
    public static boolean isSolaris;
    protected static boolean noType1Font;
    private static Font defaultFont;
    private static String lucidaSansFileName;
    public static final String lucidaFontName = "Lucida Sans Regular";
    public static boolean debugFonts = false;
    protected static Logger logger = null;
    private static ArrayList badFonts;
    public static String jreLibDirName;
    public static String jreFontDirName;

    private FontConfiguration fontConfig;

    /* fontPath is the location of all fonts on the system, excluding the
     * JRE's own font directory but including any path specified using the
     * sun.java2d.fontpath property. Together with that property,  it is
     * initialised by the getPlatformFontPath() method
     * This call must be followed by a call to registerFontDirs(fontPath)
     * once any extra debugging path has been appended. 
     */
    protected String fontPath;

    /* loadedAllFonts is set to true when all fonts on the font path are
     * actually opened, validated and registered.
     */
    private boolean loadedAllFonts = false;

    protected HashSet registeredFontFiles = new HashSet();
    public static String eudcFontFileName; /* Initialised only on windows */

   
    public SunGraphicsEnvironment() {
	java.security.AccessController.doPrivileged(
				    new java.security.PrivilegedAction() {
	    public Object run() {
	        String osName = System.getProperty("os.name");
		if ("Linux".equals(osName)) {
		    isLinux = true;
		} else if ("SunOS".equals(osName)) {
		    isSolaris = true;
		}
		String debugLevel =
		    System.getProperty("sun.java2d.debugfonts");

		if (debugLevel != null && !debugLevel.equals("false")) {
		    debugFonts = true;
		    logger = Logger.getLogger("sun.java2d");
		    if (debugLevel.equals("warning")) {
			logger.setLevel(Level.WARNING);
		    } else if (debugLevel.equals("severe")) {
			logger.setLevel(Level.SEVERE);
		    }
		}

		noType1Font = "true".
		    equals(System.getProperty("sun.java2d.noType1Font"));
                    
	        jreLibDirName = System.getProperty("java.home","") +
		    File.separator + "lib";

		jreFontDirName = jreLibDirName + File.separator + "fonts";

                if (useAbsoluteFontFileNames()) {
                    lucidaSansFileName = jreFontDirName +
                        File.separator + "LucidaSansRegular.ttf";
                } else {
                    lucidaSansFileName = "LucidaSansRegular.ttf";
                }

		File badFontFile =
		    new File(jreFontDirName + File.separator + "badfonts.txt");
		if (badFontFile.exists()) {
		    FileInputStream fis = null;
		    try {
			badFonts = new ArrayList();
		        fis = new FileInputStream(badFontFile);
			InputStreamReader isr = new InputStreamReader(fis);
			BufferedReader br = new BufferedReader(isr);
			while (true) {
			    String name = br.readLine();
			    if (name == null) {
				break;
			    } else {
				if (debugFonts) {
				    logger.warning("read bad font: " + name);
				}
				badFonts.add(name);
			    }
			}
		    } catch (IOException e) {
			try {
			    if (fis != null) {
				fis.close();
			    }
			} catch (IOException ioe) {
			}
		    }
		}

		/* Here we get the fonts in jre/lib/fonts and register them
		 * so they are always available and preferred over other fonts.
		 * This needs to be registered before the composite fonts as
		 * otherwise some native font that corresponds may be found
		 * as we don't have a way to handle two fonts of the same
		 * name, so the JRE one must be the first one registered.
		 * Pass "true" to registerFonts method as on-screen these
		 * JRE fonts always go through the T2K rasteriser.
		 */
		if (isLinux) {
		    /* Linux font configuration uses these fonts */
		    registerFontDir(jreFontDirName);
		}
		registerFontsInDir(jreFontDirName, true, Font2D.JRE_RANK,
				   true, false);

		/* Register the JRE fonts so that the native platform can
		 * access them. This is used only on Windows so that when
		 * printing the printer driver can access the fonts.
		 */
		registerJREFontsWithPlatform(jreFontDirName);

		/* Create the font configuration and get any font path
		 * that might be specified.
		 */
		fontConfig = createFontConfiguration();
		getPlatformFontPathFromFontConfig();

                String extraFontPath = fontConfig.getExtraFontPath();

		/* In prior releases the debugging font path replaced
		 * all normally located font directories except for the
		 * JRE fonts dir. This directory is still always located and
                 * placed at the head of the path but as an augmentation
                 * to the previous behaviour the
		 * changes below allow you to additionally append to
		 * the font path by starting with append: or prepend by
		 * starting with a prepend: sign. Eg: to append 
		 * -Dsun.java2d.fontpath=append:/usr/local/myfonts 
		 * and to prepend
		 * -Dsun.java2d.fontpath=prepend:/usr/local/myfonts Disp 
                 *
                 * If there is an appendedfontpath it in the font configuration
                 * it is used instead of searching the system for dirs.
		 * The behaviour of append and prepend is then similar
                 * to the normal case. ie it goes after what
                 * you prepend and * before what you append. If the
                 * sun.java2d.fontpath property is used, but it 
		 * neither the append or prepend syntaxes is used then as
		 * except for the JRE dir the path is replaced and it is
		 * up to you to make sure that all the right directories
		 * are located. This is platform and locale-specific so
		 * its almost impossible to get right, so it should be
		 * used with caution.
		 */
		boolean prependToPath = false;
		boolean appendToPath = false;
		String dbgFontPath = System.getProperty("sun.java2d.fontpath");

		if (dbgFontPath != null) {
		    if (dbgFontPath.startsWith("prepend:")) {
			prependToPath = true;
			dbgFontPath =
			    dbgFontPath.substring("prepend:".length());
		    } else if (dbgFontPath.startsWith("append:")) {
			appendToPath = true;
			dbgFontPath =
			    dbgFontPath.substring("append:".length());
		    }
		}

		if (debugFonts) {
		    logger.info("JRE font directory: " + jreFontDirName);
		    logger.info("Extra font path: " + extraFontPath);
		    logger.info("Debug font path: " + dbgFontPath);
		}

                if (dbgFontPath != null) {
		    /* In debugging mode we register all the paths
		     * Caution: this is a very expensive call on Solaris:-
		     */
		    fontPath = getPlatformFontPath(noType1Font);

		    if (extraFontPath != null) {
			fontPath =
			    extraFontPath + File.pathSeparator + fontPath;
		    }    
		    if (appendToPath) {
			fontPath = fontPath + File.pathSeparator + dbgFontPath;
		    } else if (prependToPath) {
			fontPath = dbgFontPath + File.pathSeparator + fontPath;
		    } else {
			fontPath = dbgFontPath;
		    }
		    registerFontDirs(fontPath);
		} else if (extraFontPath != null) {
		    /* If the font configuration contains an "appendedfontpath"
		     * entry, it is interpreted as a set of locations that
		     * should always be registered.
		     * It may be additional to locations normally found for
		     * that place, or it may be locations that need to have
		     * all their paths registered to locate all the needed
		     * platform names.
		     * This is typically when the same .TTF file is referenced
		     * from multiple font.dir files and all of these must be
		     * read to find all the native (XLFD) names for the font,
		     * so that X11 font APIs can be used for as many code
		     * points as possible.
		     */
		    registerFontDirs(extraFontPath);
		}

		/* On Solaris, we need to register the Japanese TrueType
		 * directory so that we can find the corresponding bitmap
		 * fonts. This could be done by listing the directory in
		 * the font configuration file, but we don't want to
		 * confuse users with this quirk. There are no bitmap fonts
		 * for other writing systems that correspond to TrueType
		 * fonts and have matching XLFDs. We need to register the
		 * bitmap fonts only in environments where they're on the
		 * X font path, i.e., in the Japanese locale.
		 * Note that if the X Toolkit is in use the font path isn't
		 * set up by JDK, but users of a JA locale should have it
		 * set up already by their login environment.
		 */
		if (isSolaris && Locale.JAPAN.equals(Locale.getDefault())) {
		    registerFontDir("/usr/openwin/lib/locale/ja/X11/fonts/TT");
		}

		initCompositeFonts(fontConfig, null);

		/* Establish the default font to be used by SG2D etc */
		defaultFont = new Font("Dialog", Font.PLAIN, 12);

		return null;
	    }
	});
    }

    protected GraphicsDevice[] screens;

    /**
     * Returns an array of all of the screen devices.
     */
    public synchronized GraphicsDevice[] getScreenDevices() {
	GraphicsDevice[] ret = screens;
	if (ret == null) {
	    int num = getNumScreens();
	    ret = new GraphicsDevice[num];
	    for (int i = 0; i < num; i++) {
		ret[i] = makeScreenDevice(i);
	    }
	    screens = ret;
	}
	return ret;
    }

    protected abstract int getNumScreens();
    protected abstract GraphicsDevice makeScreenDevice(int screennum);

    /**
     * Returns the default screen graphics device.
     */
    public GraphicsDevice getDefaultScreenDevice() {
	return getScreenDevices()[0];
    }

    /**
     * Returns a Graphics2D object for rendering into the
     * given BufferedImage.
     * @throws NullPointerException if BufferedImage argument is null
     */
    public Graphics2D createGraphics(BufferedImage img) {
	if (img == null) {
	    throw new NullPointerException("BufferedImage cannot be null");
	}
	SurfaceData sd = SurfaceData.getDestSurfaceData(img);
	return new SunGraphics2D(sd, Color.white, Color.black, defaultFont);
    }

    /* A call to this method should be followed by a call to
     * registerFontDirs(..)
     */
    protected String getPlatformFontPath(boolean noType1Font) {
	return FontManager.getFontPath(noType1Font);
    }
    
    /**
     * Whether registerFontFile expects absolute or relative
     * font file names.
     */
    protected boolean useAbsoluteFontFileNames() {
        return true;
    }

    /**
     * Returns file name for default font, either absolute
     * or relative as needed by registerFontFile.
     */
    public String getDefaultFontFile() {
	return lucidaSansFileName;
    }

    /**
     * Returns face name for default font, or null if
     * no face names are used for CompositeFontDescriptors
     * for this platform.
     */
    public String getDefaultFontFaceName() {
        return lucidaFontName;
    }

    public void loadFonts() {
        if (loadedAllFonts) {
            return;
        }
        /* Use lock specific to the font system */
        synchronized (lucidaFontName) {
	    if (debugFonts) {
	        Thread.dumpStack();
	        logger.info("SunGraphicsEnvironment.loadFonts() called");
	    }
	    FontManager.initialiseDeferredFonts();

	    java.security.AccessController.doPrivileged(
				    new java.security.PrivilegedAction() {
	        public Object run() {
		    if (fontPath == null) {
		        fontPath = getPlatformFontPath(noType1Font);
		        registerFontDirs(fontPath);
		    }
		    if (fontPath != null) {
		        // this will find all fonts including those already
		        // registered. But we have checks in place to prevent
		        // double registration.
		        registerFontsOnPath(fontPath, false,
                                            Font2D.UNKNOWN_RANK, false, true);
		    }
                    loadedAllFonts = true;
		    return null;
	        }
	    });
        }
    }

    private Font[] allFonts;
   
    /**
     * Returns all fonts available in this environment.
     */
    public Font[] getAllFonts() {
        if (allFonts == null) {
	    loadFonts();
	    TreeMap fontMapNames = new TreeMap();
	    /* warning: the number of composite fonts could change dynamically
	     * if applications are allowed to create them. "allfonts" could
	     * then be stale.
	     */
	    Font2D[] allfonts = FontManager.getRegisteredFonts();
	    for (int i=0; i < allfonts.length; i++) {
		if (!(allfonts[i] instanceof NativeFont)) {
		    fontMapNames.put(allfonts[i].getFontName(null),
				     allfonts[i]);
		}
	    }
	    String[] fontNames = null;
	    if (fontMapNames.size() > 0) {
		fontNames = new String[fontMapNames.size()];
		Object [] keyNames = fontMapNames.keySet().toArray();
		for (int i=0; i < keyNames.length; i++) {
		    fontNames[i] = (String)keyNames[i];
		}
	    }
	    Font[] fonts = new Font[fontNames.length];
	    for (int i=0; i < fontNames.length; i++) {
		fonts[i] = new Font(fontNames[i], Font.PLAIN, 1);
		Font2D f2d = (Font2D)fontMapNames.get(fontNames[i]);
		FontManager.setFont2D(fonts[i], f2d.handle);
	    }
	    allFonts = fonts;
	}

	Font []copyFonts = new Font[allFonts.length];
	System.arraycopy(allFonts, 0, copyFonts, 0, allFonts.length);
	return copyFonts;
    }

    private String[] allFamilies; // cache for default locale only
    private Locale lastDefaultLocale;

    public String[] getAvailableFontFamilyNames(Locale requestedLocale) {
	if (requestedLocale == null) {
	    requestedLocale = Locale.getDefault();
	}
	if (allFamilies != null && lastDefaultLocale != null &&
	    requestedLocale.equals(lastDefaultLocale)) {
		String[] copyFamilies = new String[allFamilies.length];
		System.arraycopy(allFamilies, 0, copyFamilies,
                                 0, allFamilies.length);
		return copyFamilies;
	}
        loadFonts();

	TreeMap familyNames = new TreeMap();
	//  these names are always there and aren't localised
	String str;
	str = "Serif";	      familyNames.put(str.toLowerCase(), str);
	str = "SansSerif";    familyNames.put(str.toLowerCase(), str);
	str = "Monospaced";   familyNames.put(str.toLowerCase(), str);
	str = "Dialog";	      familyNames.put(str.toLowerCase(), str);
	str = "DialogInput";  familyNames.put(str.toLowerCase(), str);

	Font2D[] physicalfonts = FontManager.getPhysicalFonts();
	for (int i=0; i < physicalfonts.length; i++) {
	    if (!(physicalfonts[i] instanceof NativeFont)) {
		String name = physicalfonts[i].getFamilyName(requestedLocale);
		familyNames.put(name.toLowerCase(requestedLocale), name);
	    }
	}
        String[] retval =  new String[familyNames.size()];
	Object [] keyNames = familyNames.keySet().toArray();
	for (int i=0; i < keyNames.length; i++) {
            retval[i] = (String)familyNames.get(keyNames[i]);
	}
        if (requestedLocale.equals(Locale.getDefault())) {
	    lastDefaultLocale = requestedLocale;
            allFamilies = new String[retval.length];
            System.arraycopy(retval, 0, allFamilies, 0, allFamilies.length);
	}
        return retval;
    }

    public String[] getAvailableFontFamilyNames() {
        return getAvailableFontFamilyNames(Locale.getDefault());
    }

    /**
     * Returns a file name for the physical font represented by this platform
     * font name. The default implementation tries to obtain the file name
     * from the font configuration.
     * Subclasses may override to provide information from other sources.
     */
    protected String getFileNameFromPlatformName(String platformFontName) {
        return fontConfig.getFileNameFromPlatformName(platformFontName);
    }

    /**
     * Gets a <code>PrintJob2D</code> object suitable for the
     * the current platform.
     * @return    a <code>PrintJob2D</code> object.
     * @see       java.awt.PrintJob2D
     * @since     JDK1.2
     */
    public PrinterJob getPrinterJob() {
        new Exception().printStackTrace();
	return null;
    }

    public static class TTFilter implements FilenameFilter {
        public boolean accept(File dir,String name) {
	    /* all conveniently have the same suffix length */
	    int offset = name.length()-4;
	    if (offset <= 0) { /* must be at least A.ttf */
		return false;
	    } else {
		return(name.startsWith(".ttf", offset) ||
		       name.startsWith(".TTF", offset) ||
		       name.startsWith(".ttc", offset) ||
		       name.startsWith(".TTC", offset));
	    }
        }
    }			 

    public static class T1Filter implements FilenameFilter {
        public boolean accept(File dir,String name) {
	    /* all conveniently have the same suffix length */
	    int offset = name.length()-4;
	    if (offset <= 0) { /* must be at least A.pfa */
		return false;
	    } else {
		return(name.startsWith(".pfa", offset) ||
		       name.startsWith(".pfb", offset) ||
		       name.startsWith(".PFA", offset) ||
		       name.startsWith(".PFB", offset));
	    }
        }
    }

    /* No need to keep consing up new instances - reuse a singleton.
     * The trade-off is that these objects don't get GC'd.
     */
    public static final TTFilter ttFilter = new TTFilter();
    public static final T1Filter t1Filter = new T1Filter();

    /* The majority of the register functions in this class are
     * registering platform fonts in the JRE's font maps.
     * The next one is opposite in function as it registers the JRE
     * fonts as platform fonts. If subsequent to calling this
     * your implementation enumerates platform fonts in a way that
     * would return these fonts too you may get duplicates.
     * This function is primarily used to install the JRE fonts
     * so that the native platform can access them.
     * It is intended to be overridden by platform subclasses
     * Currently minimal use is made of this as generally
     * Java 2D doesn't need the platform to be able to
     * use its fonts and platforms which already have matching
     * fonts registered (possibly even from other different JRE
     * versions) generally can't be guaranteed to use the
     * one registered by this JRE version in response to
     * requests from this JRE.
     */
    protected void registerJREFontsWithPlatform(String pathName) {
	return;
    }

    /* Called from FontManager - has Solaris specific implementation */
    public void register1dot0Fonts() {
	java.security.AccessController.doPrivileged(
	    		    new java.security.PrivilegedAction() {
	    public Object run() {
		String type1Dir = "/usr/openwin/lib/X11/fonts/Type1";
		registerFontsInDir(type1Dir, true, Font2D.TYPE1_RANK,
				   false, false);
		return null;
	    }
	});
    }

    protected void registerFontDirs(String pathName) {
	return;
    }

    /* Called to register fall back fonts */
    public void registerFontsInDir(String dirName) {
	registerFontsInDir(dirName, true, Font2D.JRE_RANK, true, false);
    }

    private void registerFontsInDir(String dirName, boolean useJavaRasterizer,
				    int fontRank,
				    boolean defer, boolean resolveSymLinks) {
	File pathFile = new File(dirName);
	addDirFonts(dirName, pathFile, ttFilter,
		    FontManager.FONTFORMAT_TRUETYPE, useJavaRasterizer,
		    fontRank==Font2D.UNKNOWN_RANK ?
		    Font2D.TTF_RANK : fontRank,
		    defer, resolveSymLinks);
	addDirFonts(dirName, pathFile, t1Filter,
		    FontManager.FONTFORMAT_TYPE1, useJavaRasterizer,
		    fontRank==Font2D.UNKNOWN_RANK ?
		    Font2D.TYPE1_RANK : fontRank,
		    defer, resolveSymLinks);	
    }

    private void registerFontsOnPath(String pathName, 
				     boolean useJavaRasterizer, int fontRank,
				     boolean defer, boolean resolveSymLinks) {

        StringTokenizer parser = new StringTokenizer(pathName, 
                                                     File.pathSeparator);
        try {
            while (parser.hasMoreTokens()) {
		registerFontsInDir(parser.nextToken(),
				   useJavaRasterizer, fontRank,
				   defer, resolveSymLinks);
            }
        } catch (NoSuchElementException e) {
        }
    }

    protected void registerFontFile(String fontFileName, String[] nativeNames,
                                    int fontRank, boolean defer) {
        // REMIND: case compare depends on platform
        if (registeredFontFiles.contains(fontFileName)) {
            return;
        }
        int fontFormat;
        if (ttFilter.accept(null, fontFileName)) {
            fontFormat = FontManager.FONTFORMAT_TRUETYPE;
        } else if (t1Filter.accept(null, fontFileName)) {
            fontFormat = FontManager.FONTFORMAT_TYPE1;
        } else {
	    fontFormat = FontManager.FONTFORMAT_NATIVE;
	}
	registeredFontFiles.add(fontFileName);
	if (defer) {
	    FontManager.registerDeferredFont(fontFileName,
					     fontFileName, nativeNames,
					     fontFormat, false, fontRank);
	} else {
	    FontManager.registerFontFile(fontFileName, nativeNames,
					 fontFormat, false, fontRank);
	}
    }

    protected void registerFontDir(String path) {
    }

    protected String[] getNativeNames(String fontFileName,
				      String platformName) {
	return null;
    }

    /*
     * helper function for registerFonts
     */
    private void addDirFonts(String dirName, File dirFile,
			     FilenameFilter filter,
			     int fontFormat, boolean useJavaRasterizer,
			     int fontRank,
			     boolean defer, boolean resolveSymLinks) {
        String[] ls = dirFile.list(filter);
        if (ls == null || ls.length == 0) {
            return;
        }
	String[] fontNames = new String[ls.length];
	String[][] nativeNames = new String[ls.length][];
	int fontCount = 0;

        for (int i=0; i < ls.length; i++ ) {
            File theFile = new File(dirFile, ls[i]);
	    String fullName = null;
	    if (resolveSymLinks) {
		try {	
		    fullName = theFile.getCanonicalPath();
		} catch (IOException e) {
		}
	    }
	    if (fullName == null) {
		fullName = dirName + File.separator + ls[i];
	    }

            // REMIND: case compare depends on platform
            if (registeredFontFiles.contains(fullName)) {
                continue;
            }

	    if (badFonts != null && badFonts.contains(fullName)) {
		if (debugFonts) {
		    logger.warning("skip bad font " + fullName);  
		}
		continue; // skip this font file.
	    }

            registeredFontFiles.add(fullName);

            if (debugFonts && logger.isLoggable(Level.INFO)) {
                String message = "Registering font " + fullName;
		String[] natNames = getNativeNames(fullName, null);
		if (natNames == null) {
		    message += " with no native name";
		} else {
                    message += " with native name(s) " + natNames[0];
		    for (int nn = 1; nn < natNames.length; nn++) {
			message += ", " + natNames[nn];
		    }
		}
                logger.info(message);
	    }
	    fontNames[fontCount] = fullName;
	    nativeNames[fontCount++] = getNativeNames(fullName, null);
        }
        FontManager.registerFonts(fontNames, nativeNames, fontCount,
				  fontFormat, useJavaRasterizer, fontRank,
				  defer);
        return;
    }

    /**
     * Creates this environment's FontConfiguration.
     */
    protected abstract FontConfiguration createFontConfiguration();

    public abstract FontConfiguration
	createFontConfiguration(boolean preferLocaleFonts,
				boolean preferPropFonts);

    /*
     * This method asks the font configuration API for all platform names
     * used as components of composite/logical fonts and iterates over these
     * looking up their corresponding file name and registers these fonts.
     * It also ensures that the fonts are accessible via platform APIs.
     * The composites themselves are then registered.
     */
    private void initCompositeFonts(FontConfiguration fontConfig,
				    Hashtable altNameCache) {

	int numCoreFonts = fontConfig.getNumberCoreFonts();
	String[] fcFonts = fontConfig.getPlatformFontNames();
	for (int f=0; f<fcFonts.length; f++) {
            String platformFontName = fcFonts[f];
	    String fontFileName =
		getFileNameFromPlatformName(platformFontName);
	    String[] nativeNames = null;
	    if (fontFileName == null) {
		/* No file located, so register using the platform name,
		 * i.e. as a native font.
		 */
		fontFileName = platformFontName;
	    } else {
		if (f < numCoreFonts) {
		    /* If platform APIs also need to access the font, add it
		     * to a set to be registered with the platform too.
		     * This may be used to add the parent directory to the X11
		     * font path if its not already there. See the docs for the
		     * subclass implementation.
		     * This is now mainly for the benefit of X11-based AWT
		     * But for historical reasons, 2D initialisation code
		     * makes these calls.
		     * If the fontconfiguration file is properly set up
		     * so that all fonts are mapped to files and all their
		     * appropriate directories are specified, then this
		     * method will be low cost as it will return after
		     * a test that finds a null lookup map.
		     */
                    addFontToPlatformFontPath(platformFontName);
		}
		nativeNames = getNativeNames(fontFileName, platformFontName);
	    }
	    /* Uncomment these two lines to "generate" the XLFD->filename
	     * mappings needed to speed start-up on Solaris.
	     * Augment this with the appendedpathname and the mappings
	     * for native (F3) fonts
	     */
	    //String platName = platformFontName.replaceAll(" ", "_");
	    //System.out.println("filename."+platName+"="+fontFileName);
	    registerFontFile(fontFileName, nativeNames,
			     Font2D.FONT_CONFIG_RANK, true);


	}
	/* This registers accumulated paths from the calls to
	 * addFontToPlatformFontPath(..) and any specified by
	 * the font configuration. Rather than registering
	 * the fonts it puts them in a place and form suitable for
	 * the Toolkit to pick up and use if a toolkit is initialised,
	 * and if it uses X11 fonts.
	 */
	registerPlatformFontsUsedByFontConfiguration();
   
        CompositeFontDescriptor[] compositeFontInfo
                = fontConfig.get2DCompositeFontInfo();
        for (int i = 0; i < compositeFontInfo.length; i++) {
            CompositeFontDescriptor descriptor = compositeFontInfo[i];
	    String[] componentFileNames = descriptor.getComponentFileNames();
	    String[] componentFaceNames = descriptor.getComponentFaceNames();

	    /* FontConfiguration needs to convey how many fonts it has added
	     * as fallback component fonts which should not affect metrics.
	     * The core component count will be the number of metrics slots.
	     * This does not preclude other mechanisms for adding 
	     * fall back component fonts to the composite.
	     */
	    if (altNameCache != null) {
		FontManager.registerCompositeFont(
		    descriptor.getFaceName(),
                    componentFileNames, componentFaceNames,
		    descriptor.getCoreComponentCount(),
                    descriptor.getExclusionRanges(),
                    descriptor.getExclusionRangeLimits(),
		    true,
                    altNameCache);
	    } else {
		FontManager.registerCompositeFont(
		    descriptor.getFaceName(),
                    componentFileNames, componentFaceNames,
		    descriptor.getCoreComponentCount(),
                    descriptor.getExclusionRanges(),
                    descriptor.getExclusionRangeLimits(),
                    true);		
	    }
	    if (debugFonts) {
		logger.info("registered " + descriptor.getFaceName());
            }
        }
    }

    /**
     * Notifies graphics environment that the logical font configuration
     * uses the given platform font name. The graphics environment may
     * use this for platform specific initialization.
     */
    protected void addFontToPlatformFontPath(String platformFontName) {
    }

    protected void registerPlatformFontsUsedByFontConfiguration() {
    }

    /**
     * Determines whether the given font is a logical font.
     */
    public static boolean isLogicalFont(Font f) {
        return FontConfiguration.isLogicalFontFamilyName(f.getFamily());
    }

    /**
     * Return the default font configuration.
     */
    public FontConfiguration getFontConfiguration() {
	return fontConfig;
    }

    /**
     * Return the bounds of a GraphicsDevice, less its screen insets.
     * See also java.awt.GraphicsEnvironment.getUsableBounds();
     */
    public static Rectangle getUsableBounds(GraphicsDevice gd) {
        GraphicsConfiguration gc = gd.getDefaultConfiguration();
        Insets insets = Toolkit.getDefaultToolkit().getScreenInsets(gc);
        Rectangle usableBounds = gc.getBounds();

        usableBounds.x += insets.left;
        usableBounds.y += insets.top;
        usableBounds.width -= (insets.left + insets.right);
        usableBounds.height -= (insets.top + insets.bottom);

        return usableBounds;
    }

    /**
     * This method is provided for internal and exclusive use by Swing.
     * This method should no longer be called, instead directly call
     * FontManager.fontSupportsDefaultEncoding(Font).
     * This method will be removed once Swing is updated to no longer
     * call it.
     */
    public static boolean fontSupportsDefaultEncoding(Font font) {
	return FontManager.fontSupportsDefaultEncoding(font);
    }

    public static void useAlternateFontforJALocales() {
	FontManager.useAlternateFontforJALocales();
    }

    /*
     * This invocation is not in a privileged block because
     * all privileged operations (reading files and properties)
     * was conducted on the creation of the GE
     */
    public void createCompositeFonts(Hashtable altNameCache,
				     boolean preferLocale,
				     boolean preferProportional) {
	
	FontConfiguration fontConfig =
	    createFontConfiguration(preferLocale, preferProportional);
	initCompositeFonts(fontConfig, altNameCache);
    }

    /* If (as we do on X11) need to set a platform font path,
     * then the needed path may be specified by the platform
     * specific FontConfiguration class & data file. Such platforms
     * (ie X11) need to override this method to retrieve this information
     * into suitable data structures.
     */
    protected void getPlatformFontPathFromFontConfig() {
    }
}
