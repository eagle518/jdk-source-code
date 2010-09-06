/*
 * @(#)FontManager.java	1.17 04/14/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.FontFormatException;
import java.io.File;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.plaf.FontUIResource;

import sun.awt.AppContext;
import sun.awt.FontConfiguration;

import sun.java2d.FontSupport;
import sun.java2d.HeadlessGraphicsEnvironment;
import sun.java2d.SunGraphicsEnvironment;

/*
 * Interface between Java Fonts (java.awt.Font) and the underlying
 * font files/native font resources and the Java and native font scalers.
 */
public final class FontManager {

    public static final int FONTFORMAT_NONE      = -1;   
    public static final int FONTFORMAT_TRUETYPE  = 0;   
    public static final int FONTFORMAT_TYPE1     = 1;
    public static final int FONTFORMAT_T2K	 = 2;
    public static final int FONTFORMAT_TTC	 = 3;
    public static final int FONTFORMAT_COMPOSITE = 4;
    public static final int FONTFORMAT_NATIVE	 = 5;

    public static final int NO_FALLBACK         = 0;
    public static final int PHYSICAL_FALLBACK   = 1;
    public static final int LOGICAL_FALLBACK    = 2;

    public static final int QUADPATHTYPE = 1;
    public static final int CUBICPATHTYPE = 2;

    /* Pool of 20 font file channels chosen because some UTF-8 locale
     * composite fonts can use up to 16 platform fonts (including the
     * Lucida fall back). This should prevent channel thrashing when
     * dealing with one of these fonts.
     * The pool array stores the fonts, rather than directly referencing
     * the channels, as the font needs to do the open/close work.
     */
    private static final int CHANNELPOOLSIZE = 20;
    private static int lastPoolIndex = 0;
    private static int poolSize = 0;
    private static FileFont fontFileCache[] = new FileFont[CHANNELPOOLSIZE];

    /* Need to implement a simple linked list scheme for fast
     * traversal and lookup.
     * Also want to "fast path" dialog so there's minimal overhead.
     */
    /* There are at exactly 20 composite fonts: 5 faces (but some are not
     * usually different), in 4 styles. The array may be auto-expanded
     * later if more are needed, eg for user-defined composites or locale
     * variants.
     */
    private static int maxCompFont = 0;
    private static CompositeFont [] compFonts = new CompositeFont[20];
    private static Hashtable compositeFonts = new Hashtable();
    private static Hashtable physicalFonts = new Hashtable();
    /* given a full name find the Font. Remind: there's duplication
     * here in that this contains the content of compositeFonts +
     * physicalFonts.
     */
    private static Hashtable fullNameToFont = new Hashtable();

    /* TrueType fonts have localised names. Support searching all
     * of these before giving up on a name.
     */
    private static HashMap localeFullNamesToFont;

    private static PhysicalFont defaultPhysicalFont;

    /* deprecated, unsupported hack - actually invokes a bug! */
    private static boolean usePlatformFontMetrics = false;
   
    public static Logger logger = null;
    public static boolean logging;
    static boolean longAddresses;
    static String osName;
    static boolean useT2K;
    static boolean isWindows;
    static boolean isSolaris;
    public static boolean isSolaris8; // needed to check for JA wavedash fix.
    private static boolean loaded1dot0Fonts = false;
    static SunGraphicsEnvironment sgEnv;
    static boolean loadedAllFonts = false;
    static TrueTypeFont eudcFont;
    static HashMap<String,String> jreFontMap;

    /* Used to indicate required return type from toArray(..); */
    private static String[] STR_ARRAY = new String[0];

    static {

	if (SunGraphicsEnvironment.debugFonts) {
	    logger = Logger.getLogger("sun.java2d", null);
	    logging = logger.getLevel() != Level.OFF;
	}

	/* Key is familyname+style value as an int.
	 * Value is filename containing the font.
	 * If no mapping exists, it means there is no font file for the style
	 * If the mapping exists but the file doesn't exist in the deferred
	 * list then it means its not installed.
	 * This looks like a lot of code and strings but if it saves even
	 * a single file being opened at JRE start-up there's a big payoff.
	 * Lucida Sans is probably the only important case as the others
	 * are rarely used. Consider removing the other mappings if there's
	 * no evidence they are useful in practice.
	 */
	jreFontMap = new HashMap<String,String>();
	/* Lucida Sans Family */
	jreFontMap.put("lucida sans0",   "LucidaSansRegular.ttf");
	jreFontMap.put("lucida sans1",   "LucidaSansDemiBold.ttf");
	/* Lucida Sans full names (map Bold and DemiBold to same file) */
	jreFontMap.put("lucida sans regular0", "LucidaSansRegular.ttf");
	jreFontMap.put("lucida sans regular1", "LucidaSansDemiBold.ttf");
	jreFontMap.put("lucida sans bold1", "LucidaSansDemiBold.ttf");
	jreFontMap.put("lucida sans demibold1", "LucidaSansDemiBold.ttf");

	/* Lucida Sans Typewriter Family */
	jreFontMap.put("lucida sans typewriter0", "LucidaTypewriter.ttf");
	jreFontMap.put("lucida sans typewriter1", "LucidaTypewriterBold.ttf");
	/* Typewriter full names (map Bold and DemiBold to same file) */
	jreFontMap.put("lucida sans typewriter regular0",
		       "LucidaTypewriter.ttf");
	jreFontMap.put("lucida sans typewriter regular1",
		       "LucidaTypewriterBold.ttf");
	jreFontMap.put("lucida sans typewriter bold1",
		       "LucidaTypewriterBold.ttf");
	jreFontMap.put("lucida sans typewriter demibold1",
		       "LucidaTypewriterBold.ttf");

	/* Lucida Bright Family */
	jreFontMap.put("lucida bright0", "LucidaBrightRegular.ttf");
	jreFontMap.put("lucida bright1", "LucidaBrightDemiBold.ttf");
	jreFontMap.put("lucida bright2", "LucidaBrightItalic.ttf");
	jreFontMap.put("lucida bright3", "LucidaBrightDemiItalic.ttf");
	/* Lucida Bright full names (map Bold and DemiBold to same file) */
	jreFontMap.put("lucida bright regular0", "LucidaBrightRegular.ttf");
	jreFontMap.put("lucida bright regular1", "LucidaBrightDemiBold.ttf");
	jreFontMap.put("lucida bright regular2", "LucidaBrightItalic.ttf");
	jreFontMap.put("lucida bright regular3", "LucidaBrightDemiItalic.ttf");
	jreFontMap.put("lucida bright bold1", "LucidaBrightDemiBold.ttf");
	jreFontMap.put("lucida bright bold3", "LucidaBrightDemiItalic.ttf");
	jreFontMap.put("lucida bright demibold1", "LucidaBrightDemiBold.ttf");
	jreFontMap.put("lucida bright demibold3","LucidaBrightDemiItalic.ttf");
	jreFontMap.put("lucida bright italic2", "LucidaBrightItalic.ttf");
	jreFontMap.put("lucida bright italic3", "LucidaBrightDemiItalic.ttf");
	jreFontMap.put("lucida bright bold italic3",
		       "LucidaBrightDemiItalic.ttf");
	jreFontMap.put("lucida bright demibold italic3",
		       "LucidaBrightDemiItalic.ttf");

	java.security.AccessController.doPrivileged(
				    new java.security.PrivilegedAction() {
  	   public Object run() {
	       /* REMIND do we really have to load awt here? */
	       System.loadLibrary("awt");
	       System.loadLibrary("fontmanager");

	       // JNI throws an exception if a class/method/field is not found,
	       // so there's no need to do anything explicit here.
	       initIDs();

	       switch (StrikeCache.nativeAddressSize) {
	       case 8: longAddresses = true; break;
	       case 4: longAddresses = false; break;
	       default: throw new RuntimeException("Unexpected address size");
	       }

	       osName = System.getProperty("os.name", "unknownOS");
	       isSolaris = osName.startsWith("SunOS");

	       if (isSolaris) {
		   String t2kStr= System.getProperty("sun.java2d.font.scaler");
                   useT2K = "t2k".equals(t2kStr);
		   String version = System.getProperty("os.version", "unk");
		   isSolaris8 = version.equals("5.8");
	       } else {
		   isWindows = osName.startsWith("Windows");
		   if (isWindows) {
		       String eudcFile =
			   SunGraphicsEnvironment.eudcFontFileName;
		       if (eudcFile != null) {
			   try {
			       eudcFont = new TrueTypeFont(eudcFile, null, 0,
							   true);
			   } catch (FontFormatException e) {
			   }
		       }
		       String prop =
			   System.getProperty("java2d.font.usePlatformFont");
		       if (("true".equals(prop) || getPlatformFontVar())) {
			   usePlatformFontMetrics = true;
			   System.out.println("Enabling platform font metrics for win32. This is an unsupported option.");
			   System.out.println("This yields incorrect composite font metrics as reported by 1.1.x releases.");
			   System.out.println("It is appropriate only for use by applications which do not use any Java 2");
			   System.out.println("functionality. This property will be removed in a later release.");
		       }
		   }
	       }
	       return null;
	   }
        });

    }

    /* Initialise ptrs used by JNI methods */
    private static native void initIDs();

    public static void addToPool(FileFont font) {
	boolean added = false;
	synchronized (fontFileCache) {
	    /* use poolSize to quickly detect if there's any free slots.
	     * This is a performance tweak based on the assumption that
	     * if this is executed at all often, its because there are many
	     * fonts being used and the pool will be full, and we will save
	     * a fruitless iteration
	     */
	    if (poolSize < CHANNELPOOLSIZE) {
		for (int i=0; i<CHANNELPOOLSIZE; i++) {
		    if (fontFileCache[i] == null) {
			fontFileCache[i] = font;
			poolSize++;
			added = true;
			break;
		    }
		}
		assert added;
	    } else {
		// is it possible for this to be the same font?
		assert fontFileCache[lastPoolIndex] != font;
		/* replace with new font,  poolSize is unchanged. */
		fontFileCache[lastPoolIndex].close();
		fontFileCache[lastPoolIndex] = font;
		/* lastPoolIndex is updated so that the least recently opened
		 * file will be closed next.
		 */
		lastPoolIndex = (lastPoolIndex+1) % CHANNELPOOLSIZE;
	    }
	}
    }

    /*
     * In the normal course of events, the pool of fonts can remain open
     * ready for quick access to their contents. The pool is sized so
     * that it is not an excessive consumer of system resources whilst
     * facilitating performance by providing ready access to the most
     * recently used set of font files.
     * The only reason to call removeFromPool(..) is for a Font that
     * you want to to have GC'd. Currently this would apply only to fonts
     * created with java.awt.Font.createFont(..).
     * In this case, the caller is expected to have arranged for the file
     * to be closed.
     * REMIND: consider how to know when a createFont created font should
     * be closed.
     */
    public static void removeFromPool(FileFont font) {
	synchronized (fontFileCache) {
	    for (int i=0; i<CHANNELPOOLSIZE; i++) {
		if (fontFileCache[i] == font) {
		    fontFileCache[i] = null;
		    poolSize--;
		}
	    }
	}
    }

    /**
     * This method is provided for internal and exclusive use by Swing.
     *
     * @param font representing a physical font.
     * @return true if the underlying font is a TrueType or OpenType font
     * that claims to support the Microsoft Windows encoding corresponding to 
     * the default file.encoding property of this JRE instance.
     * This narrow value is useful for Swing to decide if the font is useful
     * for the the Windows Look and Feel, or, if a  composite font should be
     * used instead.
     * The information used to make the decision is obtained from
     * the ulCodePageRange fields in the font.
     * A caller can use isLogicalFont(Font) in this class before calling
     * this method and would not need to call this method if that
     * returns true.
     */
//     static boolean fontSupportsDefaultEncoding(Font font) {
// 	String encoding =
// 	    (String) java.security.AccessController.doPrivileged(
//                new sun.security.action.GetPropertyAction("file.encoding"));
	
// 	if (encoding == null || font == null) {
// 	    return false;
// 	}

// 	encoding = encoding.toLowerCase(Locale.ENGLISH);

// 	return FontManager.fontSupportsEncoding(font, encoding);
//     }

    /* Revise the implementation to in fact mean "font is a composite font.
     * This ensures that Swing components will always benefit from the
     * fall back fonts
     */
    public static boolean fontSupportsDefaultEncoding(Font font) {
	return getFont2D(font) instanceof CompositeFont;
    }

    /**
     * This method is provided for internal and exclusive use by Swing.
     *
     * It may be used in conjunction with fontSupportsDefaultEncoding(Font)
     * In the event that a desktop properties font doesn't directly
     * support the default encoding, (ie because the host OS supports
     * adding support for the current locale automatically for native apps),
     * then Swing calls this method to get a font which  uses the specified
     * font for the code points it covers, but also supports this locale
     * just as the standard composite fonts do.
     * Note: this will over-ride any setting where an application
     * specifies it prefers locale specific composite fonts.
     * The logic for this, is that this method is used only where the user or
     * application has specified that the native L&F be used, and that
     * we should honour that request to use the same font as native apps use.
     *
     * The behaviour of this method is to construct a new composite
     * Font object that uses the specified physical font as its first
     * component, and adds all the components of "dialog" as fall back
     * components.
     * The method currently assumes that only the size and style attributes
     * are set on the specified font. It doesn't copy the font transform or
     * other attributes because they aren't set on a font created from
     * the desktop. This will need to be fixed if use is broadened.
     *
     * Operations such as Font.deriveFont will work properly on the
     * font returned by this method for deriving a different point size,
     * but not for deriving a different style. That's not expected to be
     * how Swing uses it, and adding that requirement would mean additional
     * logic in deriveFont to install a new CompositeFont with properly
     * derived components.
     * Also operations such as new Font(font.getFontName(..), Font.PLAIN, 14);
     * will NOT yield the same result, as the new underlying CompositeFont
     * cannot be "looked up" in the font registry.
     * This returns a FontUIResource as that is the Font sub-class needed
     * by Swing.
     * Suggested usage is something like :
     * FontUIResource fuir;
     * Font desktopFont = getDesktopFont(..);
     * // NOTE even if fontSupportsDefaultEncoding returns true because
     * // you get Tahoma and are running in an English locale, you may
     * // still want to just call getCompositeFontUIResource() anyway
     * // as only then will you get fallback fonts - eg for CJK.
     * if (FontManager.fontSupportsDefaultEncoding(desktopFont)) {
     *   fuir = new FontUIResource(..);
     * } else {
     *   fuir = FontManager.getCompositeFontUIResource(desktopFont);
     * }
     * return fuir;
     */ 
    public static FontUIResource getCompositeFontUIResource(Font font) {

        FontUIResource fuir =
            new FontUIResource(font.getName(),font.getStyle(),font.getSize());
	Font2D font2D = getFont2D(font);

	if (!(font2D instanceof PhysicalFont)) {
	    /* Swing should only be calling this when a font is obtained
             * from desktop properties, so should generally be a physical font,
             * an exception might be for names like "MS Serif" which are
             * automatically mapped to "Serif", so there's no need to do
             * anything special in that case. But note that suggested usage
             * is first to call fontSupportsDefaultEncoding(Font) and this
             * method should not be called if that were to return true.
             */
             return fuir;
	}

	CompositeFont dialog2D =
	  (CompositeFont)findFont2D("dialog", font.getStyle(), NO_FALLBACK);
	if (dialog2D == null) { /* shouldn't happen */
	    return fuir;
	}
	if (dialog2D.getSlotFont(0) == font2D) {
	  /* The physical font is the same as that used as component 0 of
	   * the composite font we've found.
	   * This is quite common as the desktop properties might say "Arial"
	   * which is slot 0 of many JDK fonts, so in that case we can just
	   * return dialog.
	   */
	  return fuir;
	}
	PhysicalFont physicalFont = (PhysicalFont)font2D;
	CompositeFont compFont = new CompositeFont(physicalFont, dialog2D);
	compFont.isStdComposite = false;
	setFont2D(fuir, compFont.handle);
	return fuir;
    }

    public static native void setFont2D(Font font, Font2DHandle font2DHandle);

    public static void registerCompositeFont(String compositeName,
					     String[] componentFileNames,
					     String[] componentNames,
					     int numMetricsSlots,
					     int[] exclusionRanges,
					     int[] exclusionMaxIndex,
					     boolean defer) {

	CompositeFont cf = new CompositeFont(compositeName,
					     componentFileNames,
					     componentNames,
					     numMetricsSlots,
					     exclusionRanges,
					     exclusionMaxIndex, defer);
	addCompositeToFontList(cf, Font2D.FONT_CONFIG_RANK);
	synchronized (compFonts) {
	    compFonts[maxCompFont++] = cf;
	}
    }

    /* This variant is used only when the application specifies
     * a variant of composite fonts which prefers locale specific or
     * proportional fonts.
     */
    public static void registerCompositeFont(String compositeName,
					     String[] componentFileNames,
					     String[] componentNames,
					     int numMetricsSlots,
					     int[] exclusionRanges,
					     int[] exclusionMaxIndex,
					     boolean defer,
					     Hashtable altNameCache) {

	CompositeFont cf = new CompositeFont(compositeName,
					     componentFileNames,
					     componentNames,
					     numMetricsSlots,
					     exclusionRanges,
					     exclusionMaxIndex, defer);
	/* if the cache has an existing composite for this case, make
	 * its handle point to this new font.
	 * This ensures that when the altNameCache that is passed in
	 * is the global mapNameCache - ie we are running as an application -
	 * that any statically created java.awt.Font instances which already
	 * have a Font2D instance will have that re-directed to the new Font
	 * on subsequent uses. This is particularly important for "the"
	 * default font instance, or similar cases where a UI toolkit (eg
	 * Swing) has cached a java.awt.Font. Note that if Swing is using
	 * a custom composite APIs which update the standard composites have
	 * no effect - this is typically the case only when using the Windows
	 * L&F where these APIs would conflict with that L&F anyway.
	 */
	Font2D oldFont = (Font2D)
	    altNameCache.get(compositeName.toLowerCase(Locale.ENGLISH));
	if (oldFont instanceof CompositeFont) {
	    oldFont.handle.font2D = cf;
	}
	altNameCache.put(compositeName.toLowerCase(Locale.ENGLISH), cf);
    }

    private static void addCompositeToFontList(CompositeFont f, int rank) {

	if (logging) {
	    logger.info("Add to Family "+ f.familyName +
			", Font " + f.fullName + " rank="+rank);
	}
	f.setRank(rank);
	compositeFonts.put(f.fullName, f);
	fullNameToFont.put(f.fullName.toLowerCase(Locale.ENGLISH), f);

	FontFamily family = FontFamily.getFamily(f.familyName);
	if (family == null) {
	    family = new FontFamily(f.familyName, true, rank);
	}
	family.setFont(f, f.style);
    }

    /* 
     * Systems may have fonts with the same name.
     * We want to register only one of such fonts (at least until
     * such time as there might be APIs which can accommodate > 1).
     * Rank is 1) font configuration fonts, 2) JRE fonts, 3) OT/TT fonts,
     * 4) Type1 fonts, 5) native fonts.
     * 
     * If the new font has the same name as the old font, the higher
     * ranked font gets added, replacing the lower ranked one.
     * If the fonts are of equal rank, then make a special case of
     * font configuration rank fonts, which are on closer inspection,
     * OT/TT fonts such that the larger font is registered. This is
     * a heuristic since a font may be "larger" in the sense of more
     * code points, or be a larger "file" because it has more bitmaps.
     * So it is possible that using filesize may lead to less glyphs, and
     * using glyphs may lead to lower quality display. Probably number
     * of glyphs is the ideal, but filesize is information we already
     * have and is good enough for the known cases.
     * Also don't want to register fonts that match JRE font families
     * but are coming from a source other than the JRE.
     * This will ensure that we will algorithmically style the JRE
     * plain font and get the same set of glyphs for all styles.
     *
     * Note that this method returns a value
     * if it returns the same object as its argument that means this
     * font was newly registered.
     * If it returns a different object it means this font already exists,
     * and you should use that one.
     * If it returns null means this font was not registered and none
     * in that name is registered. The caller must find a substitute
     */
    private static PhysicalFont addToFontList(PhysicalFont f, int rank) {

	String fontName = f.fullName;
	String familyName = f.familyName;
	if (fontName == null || "".equals(fontName)) {
	    return null;
	}
	if (compositeFonts.containsKey(fontName)) {
	    /* Don't register any font that has the same name as a composite */
	    return null;
	}
	f.setRank(rank);
	if (!physicalFonts.containsKey(fontName)) {
	    if (logging) {
		logger.info("Add to Family "+familyName +
			    ", Font " + fontName + " rank="+rank);
	    }
	    physicalFonts.put(fontName, f);
	    FontFamily family = FontFamily.getFamily(familyName);
	    if (family == null) {
		family = new FontFamily(familyName, false, rank);
		family.setFont(f, f.style);
	    } else if (family.getRank() >= rank) {
		family.setFont(f, f.style);
	    }
	    fullNameToFont.put(fontName.toLowerCase(Locale.ENGLISH), f);
	    return f;
	} else {
	    PhysicalFont newFont = f;
	    PhysicalFont oldFont = (PhysicalFont)physicalFonts.get(fontName);
	    if (oldFont == null) {
		return null;
	    }
	    /* If the new font is of an equal or higher rank, it is a
	     * candidate to replace the current one, subject to further tests.
	     */
	    if (oldFont.getRank() >= rank) {

		/* All fonts initialise their mapper when first
		 * used. If the mapper is non-null then this font
		 * has been accessed at least once. In that case
		 * do not replace it. This may be overly stringent,
		 * but its probably better not to replace a font that
		 * someone is already using without a compelling reason.
		 * Additionally the primary case where it is known
		 * this behaviour is important is in certain composite
		 * fonts, and since all the components of a given
		 * composite are usually initialised together this
		 * is unlikely. For this to be a problem, there would
		 * have to be a case where two different composites used
		 * different versions of the same-named font, and they
		 * were initialised and used at separate times.
		 * In that case we continue on and allow the new font to
		 * be installed, but replaceFont will continue to allow
		 * the original font to be used in Composite fonts.
		 */
		if (oldFont.mapper != null && rank > Font2D.FONT_CONFIG_RANK) {
		    return oldFont;
		}
		
		/* Normally we require a higher rank to replace a font,
		 * but as a special case, if the two fonts are the same rank,
		 * and are instances of TrueTypeFont we want the
		 * more complete (larger) one.
		 */
                if (oldFont.getRank() == rank) {
		    if (oldFont instanceof TrueTypeFont &&
			newFont instanceof TrueTypeFont) {
			TrueTypeFont oldTTFont = (TrueTypeFont)oldFont;
			TrueTypeFont newTTFont = (TrueTypeFont)newFont;
			if (oldTTFont.fileSize >= newTTFont.fileSize) {
			    return oldFont;
			}
		    } else {
			return oldFont;
		    }
		}
		/* Don't replace ever JRE fonts.
		 * This test is in case a font configuration references
		 * a Lucida font, which has been mapped to a Lucida
		 * from the host O/S. The assumption here is that any
		 * such font configuration file is probably incorrect, or
		 * the host O/S version is for the use of AWT.
		 * In other words if we reach here, there's a possible
		 * problem with our choice of font configuration fonts.
		 */
		if (oldFont.platName.startsWith(
		           SunGraphicsEnvironment.jreFontDirName)) {
		    if (logging) {
			logger.warning("Unexpected attempt to replace a JRE " +
				       " font " + fontName + " from " +
				        oldFont.platName +
				       " with " + newFont.platName);
		    }
		    return oldFont;
                }

		if (logging) {
		    logger.info("Replace in Family " + familyName +
				",Font " + fontName + " new rank="+rank +
				" from " + oldFont.platName +
				" with " + newFont.platName);
		}
		replaceFont(oldFont, newFont);
		physicalFonts.put(fontName, newFont);
		fullNameToFont.put(fontName.toLowerCase(Locale.ENGLISH),
                                   newFont);

		FontFamily family = FontFamily.getFamily(familyName);
		if (family == null) {
		    family = new FontFamily(familyName, false, rank);
		    family.setFont(newFont, newFont.style);
		} else if (family.getRank() >= rank) {
		    family.setFont(newFont, newFont.style);
		}
		return newFont;
	    } else {
		return oldFont;
	    }
	}
    }

    public static Font2D[] getRegisteredFonts() {
	PhysicalFont[] physFonts = getPhysicalFonts();
	int mcf = maxCompFont; /* for MT-safety */
	Font2D[] regFonts = new Font2D[physFonts.length+mcf];
	System.arraycopy(compFonts, 0, regFonts, 0, mcf);
	System.arraycopy(physFonts, 0, regFonts, mcf, physFonts.length);
	return regFonts;
    }

    public static PhysicalFont[] getPhysicalFonts() {
	return
	(PhysicalFont[])physicalFonts.values().toArray(new PhysicalFont[0]);
    }


    /* The class FontRegistrationInfo is used when a client says not
     * to register a font immediately. This mechanism is used to defer
     * initialisation of all the components of composite fonts at JRE
     * start-up. The CompositeFont class is "aware" of this and when it
     * is first used it asks for the registration of its components.
     * Also in the event that any physical font is requested the
     * deferred fonts are initialised before triggering a search of the
     * system.
     * Two Hashtables are used. One to track the deferred fonts. The
     * other to track the fonts that have been initialised through this
     * mechanism.
     */

    private static final class FontRegistrationInfo {

	String fontFilePath;
	String[] nativeNames;
	int fontFormat;
	boolean javaRasterizer;
	int fontRank;

	FontRegistrationInfo(String fontPath, String[] names, int format,
			     boolean useJavaRasterizer, int rank) {
	    this.fontFilePath = fontPath;
	    this.nativeNames = names;
	    this.fontFormat = format;
	    this.javaRasterizer = useJavaRasterizer;
	    this.fontRank = rank;
	}
    }

    private static final Hashtable deferredFontFiles = new Hashtable();
    private static final Hashtable initialisedFonts = new Hashtable();

    /* Remind: possibly enhance initialiseDeferredFonts() to be
     * optionally given a name and a style and it could stop when it
     * finds that font - but this would be a problem if two of the
     * fonts reference the same font face name (cf the Solaris
     * euro fonts).
     */
    public static synchronized void initialiseDeferredFonts() {
	String[] fileNames =
	    (String[])deferredFontFiles.keySet().toArray(STR_ARRAY);
	for (int i=0; i<fileNames.length; i++) {
	    initialiseDeferredFont(fileNames[i]);
	}
    }

    private static PhysicalFont findDeferredFont(String name, int style) {

	PhysicalFont physicalFont;

	String nameAndStyle = name.toLowerCase(Locale.ENGLISH) + style;
	String fileName = jreFontMap.get(nameAndStyle);
	if (fileName != null) {
	    initSGEnv(); /* ensure jreFontDirName is initialised */
	    fileName = SunGraphicsEnvironment.jreFontDirName +
		File.separator + fileName;
	    if (deferredFontFiles.get(fileName) != null) {
		physicalFont = initialiseDeferredFont(fileName);
		if (physicalFont != null &&
		    (physicalFont.getFontName(null).equals(name) ||
		     physicalFont.getFamilyName(null).equals(name)) &&
		    physicalFont.style == style) {
		    return physicalFont;
		}
	    }
	}

	String[] fileNames =
	    (String[])deferredFontFiles.keySet().toArray(STR_ARRAY);
	for (int i=0; i<fileNames.length; i++) {
	    physicalFont = initialiseDeferredFont(fileNames[i]);
	    if (physicalFont != null &&
		(physicalFont.getFontName(null).equals(name) ||
		physicalFont.getFamilyName(null).equals(name)) &&
		physicalFont.style == style) {
		return physicalFont;
	    }
	}
	return null;
    }

    public static void registerDeferredFont(String fileNameKey,
					    String fullPathName,
					    String[] nativeNames,
					    int fontFormat,
					    boolean useJavaRasterizer,
					    int fontRank) {
	FontRegistrationInfo regInfo =
	    new FontRegistrationInfo(fullPathName, nativeNames, fontFormat,
				     useJavaRasterizer, fontRank);
	deferredFontFiles.put(fileNameKey, regInfo);
    }


    public static synchronized
	 PhysicalFont initialiseDeferredFont(String fileNameKey) {

	if (fileNameKey == null) {
	    return null;
	}
	if (logging) {
	    logger.info("Opening deferred font file " + fileNameKey);
	}

	PhysicalFont physicalFont;
	FontRegistrationInfo regInfo = 
	    (FontRegistrationInfo)deferredFontFiles.get(fileNameKey);
	if (regInfo != null) {
	    deferredFontFiles.remove(fileNameKey);
	    physicalFont = registerFontFile(regInfo.fontFilePath,
					    regInfo.nativeNames,
					    regInfo.fontFormat,
					    regInfo.javaRasterizer,
					    regInfo.fontRank);


	    if (physicalFont != null) {
		/* Store the handle, so that if a font is bad, we
		 * retrieve the substituted font.
		 */
		initialisedFonts.put(fileNameKey, physicalFont.handle);
	    } else {
		initialisedFonts.put(fileNameKey,
				     getDefaultPhysicalFont().handle);
	    }
	} else {
	    Font2DHandle handle =
		(Font2DHandle)initialisedFonts.get(fileNameKey);
	    if (handle == null) {
		/* Probably shouldn't happen, but just in case */
		physicalFont = getDefaultPhysicalFont();
	    } else {
		physicalFont = (PhysicalFont)(handle.font2D);
	    }
	}
	return physicalFont;
    }
    
    /* Note that the return value from this method is not always
     * derived from this file, and may be null. See addToFontList for
     * some explanation of this.
     */
    public static PhysicalFont registerFontFile(String fileName,
						String[] nativeNames,
						int fontFormat,
						boolean useJavaRasterizer,
						int fontRank) {
	PhysicalFont physicalFont = null;
	try {
	    String name;
	    
	    switch (fontFormat) {

	    case FontManager.FONTFORMAT_TRUETYPE:
		int fn = 0;
		TrueTypeFont ttf;
		do {
		    ttf = new TrueTypeFont(fileName, nativeNames, fn++,
					   useJavaRasterizer);
		    PhysicalFont pf = addToFontList(ttf, fontRank);
		    if (physicalFont == null) {
			physicalFont = pf;
                    }
		}
		while (fn < ttf.getFontCount());
		break;
	
	    case FontManager.FONTFORMAT_TYPE1:
		Type1Font t1f = new Type1Font(fileName, nativeNames);
		physicalFont = addToFontList(t1f, fontRank);
		break;

	    case FontManager.FONTFORMAT_NATIVE:
		NativeFont nf = new NativeFont(fileName, false);
		physicalFont = addToFontList(nf, fontRank);
	    default:

	    }
	    if (logging) {
		logger.info("Registered file " + fileName + " as font " +
			    physicalFont + " rank="  + fontRank);
	    }
	} catch (FontFormatException ffe) {
	    if (logging) {
		logger.warning("Unusable font: " + 
			       fileName + " " + ffe.toString());
	    }
	}
	return physicalFont;
    }

    public static void registerFonts(String[] fileNames,
				     String[][] nativeNames,
				     int fontCount,
				     int fontFormat,
				     boolean useJavaRasterizer,
				     int fontRank, boolean defer) {

	for (int i=0; i < fontCount; i++) {
	    if (defer) {
		registerDeferredFont(fileNames[i],fileNames[i], nativeNames[i],
				     fontFormat, useJavaRasterizer, fontRank);
	    } else {
		registerFontFile(fileNames[i], nativeNames[i],
				 fontFormat, useJavaRasterizer, fontRank);
	    }
	}
    }

    /*
     * This is the Physical font used when some other font on the system
     * can't be located. There has to be at least one font or the font
     * system is not useful and the graphics environment cannot sustain
     * the Java platform.
     */
    public static PhysicalFont getDefaultPhysicalFont() {
	if (defaultPhysicalFont == null) {
	    /* findFont2D will load all fonts before giving up the search.
	     * If the JRE Lucida isn't found (eg because the JRE fonts
	     * directory is missing), it could find another version of Lucida
	     * from the host system. This is OK because at that point we are
	     * trying to gracefully handle/recover from a system
	     * misconfiguration and this is probably a reasonable substitution.
	     */
	    defaultPhysicalFont = (PhysicalFont)
		findFont2D("Lucida Sans Regular", Font.PLAIN, NO_FALLBACK);
	    if (defaultPhysicalFont == null) {
		defaultPhysicalFont = (PhysicalFont)
		    findFont2D("Arial", Font.PLAIN, NO_FALLBACK);
	    }
	    if (defaultPhysicalFont == null) {
		/* Because of the findFont2D call above, if we reach here, we
		 * know all fonts have already been loaded, just accept any
		 * match at this point. If this fails we are in real trouble
		 * and I don't know how to recover from there being absolutely
		 * no fonts anywhere on the system.
		 */
		Iterator i = physicalFonts.values().iterator();
		if (i.hasNext()) {
		    defaultPhysicalFont = (PhysicalFont)i.next();
		} else {
		    throw new Error("Probable fatal error:No fonts found.");
		}
	    }
	}
	return defaultPhysicalFont;
    }

    public static CompositeFont getDefaultLogicalFont(int style) {
	return (CompositeFont)findFont2D("dialog", style, NO_FALLBACK);
    }

    /*
     * return String representation of style prepended with "."
     * This is useful for performance to avoid unnecessary string operations.
     */
    private static String dotStyleStr(int num) {
	switch(num){
	  case Font.BOLD:
            return ".bold";
	  case Font.ITALIC:
            return ".italic";
	  case Font.ITALIC | Font.BOLD:
            return ".bolditalic";
	  default:
            return ".plain";
	}
    }

    static void initSGEnv() {
	if (sgEnv == null) {
	    GraphicsEnvironment ge =
		GraphicsEnvironment.getLocalGraphicsEnvironment();
	    if (ge instanceof HeadlessGraphicsEnvironment) {
		HeadlessGraphicsEnvironment hgEnv =
		    (HeadlessGraphicsEnvironment)ge;
		sgEnv = (SunGraphicsEnvironment)
		    hgEnv.getSunGraphicsEnvironment();
	    } else {
		sgEnv = (SunGraphicsEnvironment)ge;
	    }
	}
    }
	
     /* Hashtable is a synchronized class, so may be a small MP bottleneck.
      * Revisit optimum capacity/load factor. Also since Hashtable is
      * synchronised this is a bottleneck. Can't use HashMap because
      * multiple threads access this table - updating it!
      */
    private static Hashtable fontNameCache = new Hashtable(10, (float) 0.75);

    /*
     * The client supplies a name and a style.
     * The name could be a family name, or a full name.
     * A font may exist with the specified style, or it may
     * exist only in some other style. For non-native fonts the scaler
     * may be able to emulate the required style.
     */
    public static Font2D findFont2D(String name, int style, int fallback) {
	String lowerCaseName = name.toLowerCase(Locale.ENGLISH);
        String mapName = lowerCaseName + dotStyleStr(style);
	Font2D font;

	/* If preferLocaleFonts() or preferProportionalFonts() has been
	 * called we may be using an alternate set of composite fonts in this
	 * app context. The presence of a pre-built name map indicates whether
	 * this is so, and gives access to the alternate composite for the
	 * name.
	 */
	if (usingPerAppContextComposites) {
	    Hashtable altNameCache =
		(Hashtable)AppContext.getAppContext().get(CompositeFont.class);
	    if (altNameCache != null) {
		font = (Font2D)altNameCache.get(mapName);
	    } else {
		font = null;
	    }
	} else {
	    font = (Font2D)fontNameCache.get(mapName);
	}
	if (font != null) {
	    return font;
	}

        // The check below is just so that the bitmap fonts being set by
        // AWT and Swing thru the desktop properties do not trigger the
        // the load fonts case. The two bitmap fonts are now mapped to
        // appropriate equivalents for serif and sansserif.
	// Note that the cost of this comparison is only for the first
	// call until the map is filled.
	if (isWindows) {
	    if (lowerCaseName.equals("ms sans serif")) {
		name = "sansserif";
	    } else if (lowerCaseName.equals("ms serif")) {
		name = "serif";
	    }
	}
	
	/* This isn't intended to support a client passing in the
	 * string default, but if a client passes in null for the name
	 * the java.awt.Font class internally substitutes this name.
	 * So we need to recognise it here to prevent a loadFonts
	 * on the unrecognised name. The only potential problem with
	 * this is it would hide any real font called "default"!
	 * But that seems like a potential problem we can ignore for now.
	 */
	if (lowerCaseName.equals("default")) {
	    name = "dialog";
	}

	/* First see if its a family name. */
	FontFamily family = FontFamily.getFamily(name);
	if (family != null) {
	    font = family.getFontWithExactStyleMatch(style);
	    if (font == null) {
		font = findDeferredFont(name, style);
	    }
	    if (font == null) {
		font = family.getFont(style); 
	    }
	    if (font == null) {
		font = family.getClosestStyle(style);
	    }
	    if (font != null) {
		fontNameCache.put(mapName, font);
		return font;
	    }
	}

	/* If it wasn't a family name, it should be a full name of
	 * either a composite, or a physical font
	 */
	font = (Font2D)fullNameToFont.get(lowerCaseName);
	if (font != null) {	
	    /* Check that the requested style matches the matched font's style.
	     * But also match style automatically if the requested style is
	     * "plain". This because the existing behaviour is that the fonts
	     * listed via getAllFonts etc always list their style as PLAIN.
	     * This does lead to non-commutative behaviours where you might
	     * start with "Lucida Sans Regular" and ask for a BOLD version
	     * and get "Lucida Sans DemiBold" but if you ask for the PLAIN
	     * style of "Lucida Sans DemiBold" you get "Lucida Sans DemiBold".
	     * This consistent however with what happens if you have a bold
	     * version of a font and no plain version exists - alg. styling
	     * doesn't "unbolden" the font.
	     */
	    if (font.style == style || style == Font.PLAIN) {
		fontNameCache.put(mapName, font);
		return font;
	    } else {
		/* If it was a full name like "Lucida Sans Regular", but
		 * the style requested is "bold", then we want to see if
		 * there's the appropriate match against another font in
		 * that family before trying to load all fonts, or applying a
		 * algorithmic styling
		 */
		family = FontFamily.getFamily(font.getFamilyName(null));
		if (family != null) {
		    Font2D familyFont = family.getFont(style|font.style);
		    /* We exactly matched the requested style, use it! */
		    if (familyFont != null) {
			fontNameCache.put(mapName, familyFont);	
			return familyFont;
		    } else {
			/* This next call is designed to support the case
			 * where bold italic is requested, and if we must
			 * style, then base it on either bold or italic -
			 * not on plain!
			 */
			familyFont = family.getClosestStyle(style|font.style);
			if (familyFont != null) {
			    /* The next check is perhaps one
			     * that shouldn't be done. ie if we get this
			     * far we have probably as close a match as we
			     * are going to get. We could load all fonts to
			     * see if somehow some parts of the family are
			     * loaded but not all of it.
			     */
			    if (familyFont.canDoStyle(style|font.style)) { 
				fontNameCache.put(mapName, familyFont);	
				return familyFont;
			    }
			}
		    }
		}
	    }
	}
	
	/* If reach here its possible that this is in a client which never
	 * loaded the GraphicsEnvironment, so we haven't even loaded ANY of
	 * the fonts from the environment. Do so now and recurse.
	 */
	if (sgEnv == null) {
	    initSGEnv();
	    return findFont2D(name, style, fallback);
	}

	/* If reach here and no match has been located, then if there are
	 * uninitialised deferred fonts, load as many of those as needed
	 * to find the deferred font. If none is found through that
	 * search continue on.
	 * There is possibly a minor issue when more than one
	 * deferred font implements the same font face. Since deferred
	 * fonts are only those in font configuration files, this is a
	 * controlled situation, the known case being Solaris euro_fonts
	 * versions of Arial, Times New Roman, Courier New. However
	 * the larger font will transparently replace the smaller one
	 *  - see addToFontList() - when it is needed by the composite font.
	 */
	if (deferredFontFiles.size() > 0) {
	    font = findDeferredFont(name, style);
	    if (font != null) {
		fontNameCache.put(mapName, font);
		return font;
	    }
	}

	/* Some apps use deprecated 1.0 names such as helvetica and courier. On
	 * Solaris these are Type1 fonts in /usr/openwin/lib/X11/fonts/Type1.
	 * If running on Solaris will register all the fonts in this
	 * directory.
	 * May as well register the whole directory without actually testing
	 * the font name is one of the deprecated names as the next step would
	 * load all fonts which are in this directory anyway.
	 * In the event that this lookup is successful it potentially "hides"
	 * TrueType versions of such fonts that are elsewhere but since they
	 * do not exist on Solaris this is not a problem.
	 * Set a flag to indicate we've done this registration to avoid
	 * repetition and more seriously, to avoid recursion.
	 */
	if (isSolaris&&!loaded1dot0Fonts) {
	    /* "timesroman" is a special case since that's not the
	     * name of any known font on Solaris or elsewhere.
	     */
	    if (lowerCaseName.equals("timesroman")) {
		font = findFont2D("serif", style, fallback);
		fontNameCache.put(mapName, font);
	    }
	    sgEnv.register1dot0Fonts();
	    loaded1dot0Fonts = true;
	    Font2D ff = findFont2D(name, style, fallback);
	    return ff;
	}

	/* If reach here and no match has been located, then if all fonts
	 * are not yet loaded, do so, and then recurse.
	 */
	if (!loadedAllFonts) {
	    sgEnv.loadFonts();
	    loadedAllFonts = true;
	    return findFont2D(name, style, fallback);
	}

	/* The primary name is the locale default - ie not US/English but
	 * whatever is the default in this locale. This is the way it always
	 * has been but may be surprising to some developers if "Arial Regular"
	 * were hard-coded in their app and yet "Arial Regular" was not the
	 * default name. Fortunately for them, as a consequence of the JDK
	 * supporting returning names and family names for arbitrary locales,
	 * we also need to support searching all localised names for a match.
	 * But because this case of the name used to reference a font is not
	 * the same as the default for this locale is rare, it makes sense to
	 * search a much shorter list of default locale names and only go to
	 * a longer list of names in the event that no match was found.
	 * So add here code which searches localised names too.
	 * As in 1.4.x this happens only after loading all fonts, which
	 * is probably the right order.
	 */
	if ((font = findFont2DAllLocales(name, style)) != null) {
	    fontNameCache.put(mapName, font);
	    return font;
	}

	/* Perhaps its a "compatibility" name - timesroman, helvetica,
	 * or courier, which 1.0 apps used for logical fonts.
	 * We look for these "late" after a loadFonts as we must not
	 * hide real fonts of these names.
	 * Map these appropriately:
	 * On windows this means according to the rules specified by the
	 * FontConfiguration : do it only for encoding==Cp1252
	 *
	 * REMIND: this is something we plan to remove.
	 */
	if (isWindows) {
	    String compatName =
		sgEnv.getFontConfiguration().getFallbackFamilyName(name, null);
	    if (compatName != null) {
		font = findFont2D(compatName, style, fallback);
		fontNameCache.put(mapName, font);
		return font;
	    }
	} else if (lowerCaseName.equals("timesroman")) {
	    font = findFont2D("serif", style, fallback);
	    fontNameCache.put(mapName, font);
	    return font;
	} else if (lowerCaseName.equals("helvetica")) {
	    font = findFont2D("sansserif", style, fallback);
	    fontNameCache.put(mapName, font);
	    return font;
	} else if (lowerCaseName.equals("courier")) {
	    font = findFont2D("monospaced", style, fallback);
	    fontNameCache.put(mapName, font);
	    return font;
	}

	switch (fallback) {
	case PHYSICAL_FALLBACK: return getDefaultPhysicalFont();
	case LOGICAL_FALLBACK: return getDefaultLogicalFont(style);
	default: return null;
	}
    }

    /* This method can be more efficient as it will only need to
     * do the lookup once, and subsequent calls on the java.awt.Font
     * instance can utilise the cached Font2D on that object.
     * Its unfortunate it needs to be a native method, but the font2D
     * variable has to be private.
     */
    public static native Font2D getFont2D(Font font);

    /* Stuff below was in NativeFontWrapper and needed a new home */

    /*
     * Workaround for apps which are dependent on a font metrics bug
     * in JDK 1.1. This is an unsupported win32 private setting.
     */
    public static boolean usePlatformFontMetrics() {
        return usePlatformFontMetrics;
    }
    
    static native boolean getPlatformFontVar();

    private static final short US_LCID = 0x0409;  // US English - default
    private static Map lcidMap;

    // Return a Microsoft LCID from the given Locale.
    // Used when getting localized font data.

    public static short getLCIDFromLocale(Locale locale) {    
        // optimize for common case
        if (locale.equals(Locale.US)) {
            return US_LCID;
        }
            
        if (lcidMap == null) {
            createLCIDMap();
        }
    
        String key = locale.toString();
        while (!"".equals(key)) {
            Short lcidObject = (Short) lcidMap.get(key);
            if (lcidObject != null) {
                return lcidObject.shortValue();
            }
            int pos = key.lastIndexOf('_');
            if (pos < 1) {
                return US_LCID;
            }
            key = key.substring(0, pos);
        }
        
        return US_LCID;
    }

        
    private static void addLCIDMapEntry(Map map, String key, short value) {
        map.put(key, new Short(value));
    }

    private static synchronized void createLCIDMap() {
        if (lcidMap != null) {
            return;
        }
        
        Map map = new HashMap(200);
        
        // the following statements are derived from the langIDMap
        // in src/win32/native/common/locale_str.h using the following
        // awk script:
        //    $1~/\/\*/   { next}
        //    $3~/\?\?/   { next }
        //    $3!~/_/     { next }
        //    $1~/0x0409/ { next }
        //    $1~/0x0c0a/ { next }
        //    $1~/0x042c/ { next }
        //    $1~/0x0443/ { next }
        //    $1~/0x0812/ { next }
        //    $1~/0x04/   { print "        addLCIDMapEntry(map, " substr($3, 0, 3) "\", (short) " substr($1, 0, 6) ");" ; next }
        //    $3~/,/      { print "        addLCIDMapEntry(map, " $3  " (short) " substr($1, 0, 6) ");" ; next }
        //                { print "        addLCIDMapEntry(map, " $3 ", (short) " substr($1, 0, 6) ");" ; next }
        // The lines of this script:
        // - eliminate comments
        // - eliminate questionable locales
        // - eliminate language-only locales
        // - eliminate the default LCID value
        // - eliminate a few other unneeded LCID values
        // - print language-only locale entries for x04* LCID values
        //   (apparently Microsoft doesn't use language-only LCID values -
        //   see http://www.microsoft.com/OpenType/otspec/name.htm
        // - print complete entries for all other LCID values
        // Run
        //     awk -f awk-script langIDMap > statements
        addLCIDMapEntry(map, "ar", (short) 0x0401);
        addLCIDMapEntry(map, "bg", (short) 0x0402);
        addLCIDMapEntry(map, "ca", (short) 0x0403);
        addLCIDMapEntry(map, "zh", (short) 0x0404);
        addLCIDMapEntry(map, "cs", (short) 0x0405);
        addLCIDMapEntry(map, "da", (short) 0x0406);
        addLCIDMapEntry(map, "de", (short) 0x0407);
        addLCIDMapEntry(map, "el", (short) 0x0408);
        addLCIDMapEntry(map, "es", (short) 0x040a);
        addLCIDMapEntry(map, "fi", (short) 0x040b);
        addLCIDMapEntry(map, "fr", (short) 0x040c);
        addLCIDMapEntry(map, "iw", (short) 0x040d);
        addLCIDMapEntry(map, "hu", (short) 0x040e);
        addLCIDMapEntry(map, "is", (short) 0x040f);
        addLCIDMapEntry(map, "it", (short) 0x0410);
        addLCIDMapEntry(map, "ja", (short) 0x0411);
        addLCIDMapEntry(map, "ko", (short) 0x0412);
        addLCIDMapEntry(map, "nl", (short) 0x0413);
        addLCIDMapEntry(map, "no", (short) 0x0414);
        addLCIDMapEntry(map, "pl", (short) 0x0415);
        addLCIDMapEntry(map, "pt", (short) 0x0416);
        addLCIDMapEntry(map, "rm", (short) 0x0417);
        addLCIDMapEntry(map, "ro", (short) 0x0418);
        addLCIDMapEntry(map, "ru", (short) 0x0419);
        addLCIDMapEntry(map, "hr", (short) 0x041a);
        addLCIDMapEntry(map, "sk", (short) 0x041b);
        addLCIDMapEntry(map, "sq", (short) 0x041c);
        addLCIDMapEntry(map, "sv", (short) 0x041d);
        addLCIDMapEntry(map, "th", (short) 0x041e);
        addLCIDMapEntry(map, "tr", (short) 0x041f);
        addLCIDMapEntry(map, "in", (short) 0x0421);
        addLCIDMapEntry(map, "uk", (short) 0x0422);
        addLCIDMapEntry(map, "be", (short) 0x0423);
        addLCIDMapEntry(map, "sl", (short) 0x0424);
        addLCIDMapEntry(map, "et", (short) 0x0425);
        addLCIDMapEntry(map, "lv", (short) 0x0426);
        addLCIDMapEntry(map, "lt", (short) 0x0427);
        addLCIDMapEntry(map, "fa", (short) 0x0429);
        addLCIDMapEntry(map, "vi", (short) 0x042a);
        addLCIDMapEntry(map, "hy", (short) 0x042b);
        addLCIDMapEntry(map, "eu", (short) 0x042d);
        addLCIDMapEntry(map, "mk", (short) 0x042f);
        addLCIDMapEntry(map, "tn", (short) 0x0432);
        addLCIDMapEntry(map, "af", (short) 0x0436);
        addLCIDMapEntry(map, "ka", (short) 0x0437);
        addLCIDMapEntry(map, "fo", (short) 0x0438);
        addLCIDMapEntry(map, "hi", (short) 0x0439);
        addLCIDMapEntry(map, "mt", (short) 0x043a);
        addLCIDMapEntry(map, "gd", (short) 0x043c);
        addLCIDMapEntry(map, "ms", (short) 0x043e);
        addLCIDMapEntry(map, "kk", (short) 0x043f);
        addLCIDMapEntry(map, "sw", (short) 0x0441);
        addLCIDMapEntry(map, "ta", (short) 0x0449);
        addLCIDMapEntry(map, "mr", (short) 0x044e);
        addLCIDMapEntry(map, "sa", (short) 0x044f);
        addLCIDMapEntry(map, "ar_IQ", (short) 0x0801);
        addLCIDMapEntry(map, "zh_CN", (short) 0x0804);
        addLCIDMapEntry(map, "de_CH", (short) 0x0807);
        addLCIDMapEntry(map, "en_GB", (short) 0x0809);
        addLCIDMapEntry(map, "es_MX", (short) 0x080a);
        addLCIDMapEntry(map, "fr_BE", (short) 0x080c);
        addLCIDMapEntry(map, "it_CH", (short) 0x0810);
        addLCIDMapEntry(map, "nl_BE", (short) 0x0813);
        addLCIDMapEntry(map, "no_NO_NY", (short) 0x0814);
        addLCIDMapEntry(map, "pt_PT", (short) 0x0816);
        addLCIDMapEntry(map, "ro_MD", (short) 0x0818);
        addLCIDMapEntry(map, "ru_MD", (short) 0x0819);
        addLCIDMapEntry(map, "sr_CS", (short) 0x081a);
        addLCIDMapEntry(map, "sv_FI", (short) 0x081d);
        addLCIDMapEntry(map, "az_AZ", (short) 0x082c);
        addLCIDMapEntry(map, "ga_IE", (short) 0x083c);
        addLCIDMapEntry(map, "ms_BN", (short) 0x083e);
        addLCIDMapEntry(map, "uz_UZ", (short) 0x0843);
        addLCIDMapEntry(map, "ar_EG", (short) 0x0c01);
        addLCIDMapEntry(map, "zh_HK", (short) 0x0c04);
        addLCIDMapEntry(map, "de_AT", (short) 0x0c07);
        addLCIDMapEntry(map, "en_AU", (short) 0x0c09);
        addLCIDMapEntry(map, "fr_CA", (short) 0x0c0c);
        addLCIDMapEntry(map, "sr_CS", (short) 0x0c1a);
        addLCIDMapEntry(map, "ar_LY", (short) 0x1001);
        addLCIDMapEntry(map, "zh_SG", (short) 0x1004);
        addLCIDMapEntry(map, "de_LU", (short) 0x1007);
        addLCIDMapEntry(map, "en_CA", (short) 0x1009);
        addLCIDMapEntry(map, "es_GT", (short) 0x100a);
        addLCIDMapEntry(map, "fr_CH", (short) 0x100c);
        addLCIDMapEntry(map, "ar_DZ", (short) 0x1401);
        addLCIDMapEntry(map, "zh_MO", (short) 0x1404);
        addLCIDMapEntry(map, "de_LI", (short) 0x1407);
        addLCIDMapEntry(map, "en_NZ", (short) 0x1409);
        addLCIDMapEntry(map, "es_CR", (short) 0x140a);
        addLCIDMapEntry(map, "fr_LU", (short) 0x140c);
        addLCIDMapEntry(map, "ar_MA", (short) 0x1801);
        addLCIDMapEntry(map, "en_IE", (short) 0x1809);
        addLCIDMapEntry(map, "es_PA", (short) 0x180a);
        addLCIDMapEntry(map, "fr_MC", (short) 0x180c);
        addLCIDMapEntry(map, "ar_TN", (short) 0x1c01);
        addLCIDMapEntry(map, "en_ZA", (short) 0x1c09);
        addLCIDMapEntry(map, "es_DO", (short) 0x1c0a);
        addLCIDMapEntry(map, "ar_OM", (short) 0x2001);
        addLCIDMapEntry(map, "en_JM", (short) 0x2009);
        addLCIDMapEntry(map, "es_VE", (short) 0x200a);
        addLCIDMapEntry(map, "ar_YE", (short) 0x2401);
        addLCIDMapEntry(map, "es_CO", (short) 0x240a);
        addLCIDMapEntry(map, "ar_SY", (short) 0x2801);
        addLCIDMapEntry(map, "en_BZ", (short) 0x2809);
        addLCIDMapEntry(map, "es_PE", (short) 0x280a);
        addLCIDMapEntry(map, "ar_JO", (short) 0x2c01);
        addLCIDMapEntry(map, "en_TT", (short) 0x2c09);
        addLCIDMapEntry(map, "es_AR", (short) 0x2c0a);
        addLCIDMapEntry(map, "ar_LB", (short) 0x3001);
        addLCIDMapEntry(map, "en_ZW", (short) 0x3009);
        addLCIDMapEntry(map, "es_EC", (short) 0x300a);
        addLCIDMapEntry(map, "ar_KW", (short) 0x3401);
        addLCIDMapEntry(map, "en_PH", (short) 0x3409);
        addLCIDMapEntry(map, "es_CL", (short) 0x340a);
        addLCIDMapEntry(map, "ar_AE", (short) 0x3801);
        addLCIDMapEntry(map, "es_UY", (short) 0x380a);
        addLCIDMapEntry(map, "ar_BH", (short) 0x3c01);
        addLCIDMapEntry(map, "es_PY", (short) 0x3c0a);
        addLCIDMapEntry(map, "ar_QA", (short) 0x4001);
        addLCIDMapEntry(map, "es_BO", (short) 0x400a);
        addLCIDMapEntry(map, "es_SV", (short) 0x440a);
        addLCIDMapEntry(map, "es_HN", (short) 0x480a);
        addLCIDMapEntry(map, "es_NI", (short) 0x4c0a);
        addLCIDMapEntry(map, "es_PR", (short) 0x500a);
        
        lcidMap = map;
    }

    public static int getNumFonts() {
	return physicalFonts.size()+maxCompFont;
    }

    private static boolean fontSupportsEncoding(Font font, String encoding) {
	return getFont2D(font).supportsEncoding(encoding);
    }

    public synchronized static native String getFontPath(boolean noType1Fonts);
    public synchronized static native void setNativeFontPath(String fontPath);


    public static Font2D createFont2D(File fontFile, int fontFormat,
                                      boolean isCopy) 
	throws FontFormatException {

	String fontFilePath = fontFile.getPath();
	FileFont font2D = null;
	final File fFile = fontFile;
	try {
	    switch (fontFormat) {
	    case Font.TRUETYPE_FONT: 
		font2D = new TrueTypeFont(fontFilePath, null, 0, true);
		break;
	    case Font.TYPE1_FONT:
		font2D = new Type1Font(fontFilePath, null);
		break;
	    default:
		throw new FontFormatException("Unrecognised Font Format");
	    }
	} catch (FontFormatException e) {
            if (isCopy) {
	        java.security.AccessController.doPrivileged(
	             new java.security.PrivilegedAction() {
	                  public Object run() {
		              fFile.delete();
			      return null;
		          }
	        });
            }
	    throw(e);
	}
        if (isCopy) {
	    font2D.setFileToRemove(fontFile);
        }
	return font2D;
    }

    /* remind: used in X11GraphicsEnvironment and called often enough
     * that we ought to obsolete this code
     */
    public synchronized static String getFullNameByFileName(String fileName) {
	PhysicalFont[] physFonts = getPhysicalFonts();
	for (int i=0;i<physFonts.length;i++) {
	    if (physFonts[i].platName.equals(fileName)) {
		return (physFonts[i].getFontName(null));
	    }
	}
	return null;
    }

    /*
     * This is called when font is determined to be invalid/bad.
     * It designed to be called (for example) by the font scaler
     * when in processing a font file it is discovered to be incorrect.
     * This is different than the case where fonts are discovered to
     * be incorrect during initial verification, as such fonts are
     * never registered.
     * Handles to this font held are re-directed to a default font.
     * This default may not be an ideal substitute buts it better than
     * crashing This code assumes a PhysicalFont parameter as it doesn't
     * make sense for a Composite to be "bad".
     */
    public static synchronized void deRegisterBadFont(Font2D font2D) {
	if (!(font2D instanceof PhysicalFont)) {
	    /* We should never reach here, but just in case */
	    return;
	} else {
	    if (logging) {
		logger.severe("Deregister bad font: " + font2D);
	    }
	    replaceFont((PhysicalFont)font2D, getDefaultPhysicalFont());
	}
    }

    /*
     * This encapsulates all the work that needs to be done when a
     * Font2D is replaced by a different Font2D.
     */
    public static synchronized void replaceFont(PhysicalFont oldFont,
						PhysicalFont newFont) {

	if (oldFont.handle.font2D != oldFont) {
	    /* already done */
	    return;
	}

	/* eliminate references to this font, so it won't be located
	 * by future callers, and will be eligible for GC when all
	 * references are removed
	 */
	oldFont.handle.font2D = newFont;
	physicalFonts.remove(oldFont.fullName);
	fullNameToFont.remove(oldFont.fullName.toLowerCase(Locale.ENGLISH));
	FontFamily.remove(oldFont);

	if (localeFullNamesToFont != null) {
	    Map.Entry[] mapEntries =
		(Map.Entry[])localeFullNamesToFont.entrySet().
		toArray(new Map.Entry[0]);
	    /* Should I be replacing these, or just I just remove
	     * the names from the map?
	     */
	    for (int i=0; i<mapEntries.length;i++) {
		if (mapEntries[i].getValue() == oldFont) {
		    try {
			mapEntries[i].setValue(newFont);
		    } catch (Exception e) {
			/* some maps don't support this operation.
			 * In this case just give up and remove the entry.
			 */
			localeFullNamesToFont.remove(mapEntries[i].getKey());
		    }
		}
	    }
	}

	for (int i=0; i<maxCompFont; i++) {
	    /* Deferred initialization of composites shouldn't be
	     * a problem for this case, since a font must have been
	     * initialised to be discovered to be bad.
	     * Some JRE composites on Solaris use two versions of the same
	     * font. The replaced font isn't bad, just "smaller" so there's
	     * no need to make the slot point to the new font.
	     * Since composites have a direct reference to the Font2D (not
	     * via a handle) making this substitution is not safe and could
	     * cause an additional problem and so this substitution is
	     * warranted only when a font is truly "bad" and could cause
	     * a crash. So we now replace it only if its being substituted
	     * with some font other than a fontconfig rank font
	     * Since in practice a substitution will have the same rank
	     * this may never happen, but the code is safer even if its
	     * also now a no-op.
	     * The only obvious "glitch" from this stems from the current
	     * implementation that when asked for the number of glyphs in a
	     * composite it lies and returns the number in slot 0 because
	     * composite glyphs aren't contiguous. Since we live with that
	     * we can live with the glitch that depending on how it was
	     * initialised a composite may return different values for this.
	     * Fixing the issues with composite glyph ids is tricky as
	     * there are exclusion ranges and unlike other fonts even the
	     * true "numGlyphs" isn't a contiguous range. Likely the only
	     * solution is an API that returns an array of glyph ranges
	     * which takes precedence over the existing API. That might
	     * also need to address excluding ranges which represent a
	     * code point supported by an earlier component. 
	     */
	    if (newFont.getRank() > Font2D.FONT_CONFIG_RANK) {
		compFonts[i].replaceComponentFont(oldFont, newFont);
	    }
	}
    }

    private static synchronized void loadLocaleNames() {
	if (localeFullNamesToFont != null) {
	    return;
	}
	localeFullNamesToFont = new HashMap();
	Font2D[] fonts = getRegisteredFonts();
	for (int i=0; i<fonts.length; i++) {
	    if (fonts[i] instanceof TrueTypeFont) {
		TrueTypeFont ttf = (TrueTypeFont)fonts[i];
		String[] fullNames = ttf.getAllFullNames();
		for (int n=0; n<fullNames.length; n++) {
		    localeFullNamesToFont.put(fullNames[n], ttf);
		}
		FontFamily family = FontFamily.getFamily(ttf.familyName);
		if (family != null) {
		    FontFamily.addLocaleNames(family, ttf.getAllFamilyNames());
		}
	    }
	}
    }

    /* This replicate the core logic of findFont2D but operates on
     * all the locale names. This hasn't been merged into findFont2D to
     * keep the logic simpler and reduce overhead, since this case is
     * almost never used. The main case in which it is called is when
     * a bogus font name is used and we need to check all possible names
     * before returning the default case.
     */
    private static Font2D findFont2DAllLocales(String name, int style) {

	/* If reach here and no match has been located, then if we have
	 * not yet built the map of localeFullNamesToFont for TT fonts, do so
	 * now. This method must be called after all fonts have been loaded.
	 */
	if (localeFullNamesToFont == null) {
	    loadLocaleNames();
	}
	String lowerCaseName = name.toLowerCase();
	Font2D font = null;

	/* First see if its a family name. */
	FontFamily family = FontFamily.getLocaleFamily(lowerCaseName);
	if (family != null) {
	  font = family.getFont(style);
	  if (font == null) {
	    font = family.getClosestStyle(style);
	  }
	  if (font != null) {
	      return font;
	  }
	}

	/* If it wasn't a family name, it should be a full name. */
	synchronized (FontManager.class) {
	    font = (Font2D)localeFullNamesToFont.get(name);
	}
	if (font != null) {
	    if (font.style == style || style == Font.PLAIN) {	
		return font;
	    } else {
		family = FontFamily.getFamily(font.getFamilyName(null));
		if (family != null) {
		    Font2D familyFont = family.getFont(style);
		    /* We exactly matched the requested style, use it! */
		    if (familyFont != null) {
			return familyFont;
		    } else {
			familyFont = family.getClosestStyle(style);
			if (familyFont != null) {
			    /* The next check is perhaps one
			     * that shouldn't be done. ie if we get this
			     * far we have probably as close a match as we
			     * are going to get. We could load all fonts to
			     * see if somehow some parts of the family are
			     * loaded but not all of it.
			     * This check is commented out for now.
			     */
			    if (!familyFont.canDoStyle(style)) {
				familyFont = null;
			    }    
			    return familyFont;
			}
		    }
		}
	    }
	}
	return font;
    }

    /* Supporting "alternate" composite fonts on 2D graphics objects
     * is accessed by the application by calling methods on the local
     * GraphicsEnvironment. The overall implementation is described
     * in one place, here, since otherwise the implementation is spread
     * around it may be difficult to track.
     * The methods below call into SunGraphicsEnvironment which creates a
     * new FontConfiguration instance. The FontConfiguration class,
     * and its platform sub-classes are updated to take parameters requesting
     * these behaviours. This is then used to create new composite font
     * instances. Since this calls the initCompositeFont method in
     * SunGraphicsEnvironment it performs the same initialization as is
     * performed normally. There may be some duplication of effort, but
     * that code is already written to be able to perform properly if called
     * to duplicate work. The main difference is that if we detect we are
     * running in an applet/browser/Java plugin environment these new fonts
     * are not placed in the "default" maps but into an AppContext instance.
     * The font lookup mechanism in java.awt.Font.getFont2D() is also updated
     * so that look-up for composite fonts will in that case always
     * do a lookup rather than returning a cached result.
     * This is inefficient but necessary else singleton java.awt.Font
     * instances would not retrieve the correct Font2D for the appcontext.
     * sun.font.FontManager.findFont2D is also updated to that it uses
     * a name map cache specific to that appcontext.
     * 
     * Getting an AppContext is expensive, so there is a global variable
     * that records whether these methods have ever been called and can
     * avoid the expense for almost all applications. Once the correct
     * CompositeFont is associated with the Font, everything should work
     * through existing mechanisms.
     * A special case is that GraphicsEnvironment.getAllFonts() must
     * return an AppContext specific list.
     *
     * Calling the methods below is "heavyweight" but it is expected that
     * these methods will be called very rarely.
     *
     * If usingPerAppContextComposites is true, we are in "applet"
     * (eg browser) enviroment and at least one context has selected
     * an alternate composite font behaviour.
     * If usingAlternateComposites is true, we are not in an "applet"
     * environment and the (single) application has selected
     * an alternate composite font behaviour.
     *
     * - Printing: The implementation delegates logical fonts to an AWT
     * mechanism which cannot use these alternate configurations.
     * We can detect that alternate fonts are in use and back-off to 2D, but
     * that uses outlines. Much of this can be fixed with additional work
     * but that may have to wait. The results should be correct, just not
     * optimal.
     */
    private static final Object altJAFontKey       = new Object();
    private static final Object localeFontKey       = new Object();
    private static final Object proportionalFontKey = new Object();
    public static boolean usingPerAppContextComposites = false;
    private static boolean usingAlternateComposites = false;

    /* These values are used only if we are running as a standalone
     * application, as determined by maybeMultiAppContext();
     */
    private static boolean gAltJAFont = false;
    private static boolean gLocalePref = false;
    private static boolean gPropPref = false;

    public static boolean usingAlternateCompositeFonts() {
	return (usingAlternateComposites ||
		(usingPerAppContextComposites &&
		AppContext.getAppContext().get(CompositeFont.class) != null));
    }

    private static boolean maybeMultiAppContext() {
	Boolean appletSM = (Boolean)
	    java.security.AccessController.doPrivileged(
	        new java.security.PrivilegedAction() {
			public Object run() {
			    SecurityManager sm = System.getSecurityManager();
			    return new Boolean
				(sm instanceof sun.applet.AppletSecurity);
			}
		    });
	return appletSM.booleanValue();
    }

    /* Modifies the behaviour of a subsequent call to preferLocaleFonts()
     * to use Mincho instead of Gothic for dialoginput in JA locales
     * on windows. Not needed on other platforms.
     */
    public static synchronized void useAlternateFontforJALocales() {

	if (!isWindows) {
	    return;
	}

	initSGEnv();
	if (!maybeMultiAppContext()) {
	    gAltJAFont = true;
	} else {
	    AppContext appContext = AppContext.getAppContext();
	    appContext.put(altJAFontKey, altJAFontKey);
	}
    }

    public static boolean usingAlternateFontforJALocales() {
	if (!maybeMultiAppContext()) {
	    return gAltJAFont;
	} else {
	    AppContext appContext = AppContext.getAppContext();
	    return appContext.get(altJAFontKey) == altJAFontKey;
	}	
    }

    public static synchronized void preferLocaleFonts() {

	initSGEnv();

	/* Test if re-ordering will have any effect */
	if (!FontConfiguration.willReorderForStartupLocale()) {
	    return;
	}

	if (!maybeMultiAppContext()) {
	    if (gLocalePref == true) {
		return;
	    }
	    gLocalePref = true;
	    sgEnv.createCompositeFonts(fontNameCache, gLocalePref, gPropPref);
	    usingAlternateComposites = true;
	} else {
	    AppContext appContext = AppContext.getAppContext();
	    if (appContext.get(localeFontKey) == localeFontKey) {
		return;
	    }
	    appContext.put(localeFontKey, localeFontKey);
	    boolean acPropPref =
		appContext.get(proportionalFontKey) == proportionalFontKey;
	    Hashtable altNameCache = new Hashtable();
	    /* If there is an existing hashtable, we can drop it. */
	    appContext.put(CompositeFont.class, altNameCache);
	    usingPerAppContextComposites = true;
	    sgEnv.createCompositeFonts(altNameCache, true, acPropPref);
	}
    }

    public static synchronized void preferProportionalFonts() {
	
	/* If no proportional fonts are configured, there's no need
	 * to take any action.
	 */
	if (!FontConfiguration.hasMonoToPropMap()) {
	    return;
	}

	initSGEnv();

	if (!maybeMultiAppContext()) {
	    if (gPropPref == true) {
		return;
	    }
	    gPropPref = true;
	    sgEnv.createCompositeFonts(fontNameCache, gLocalePref, gPropPref);
	    usingAlternateComposites = true;
	} else {
	    AppContext appContext = AppContext.getAppContext();
	    if (appContext.get(proportionalFontKey) == proportionalFontKey) {
		return;
	    }
	    appContext.put(proportionalFontKey, proportionalFontKey);
	    boolean acLocalePref =
		appContext.get(localeFontKey) == localeFontKey;
	    Hashtable altNameCache = new Hashtable();
	    /* If there is an existing hashtable, we can drop it. */
	    appContext.put(CompositeFont.class, altNameCache);
	    usingPerAppContextComposites = true;
	    sgEnv.createCompositeFonts(altNameCache, acLocalePref, true);
	}
    }
}
