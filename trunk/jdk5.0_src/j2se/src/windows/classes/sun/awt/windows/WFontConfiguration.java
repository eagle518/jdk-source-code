/*
 * @(#)WFontConfiguration.java	1.16 04/02/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.util.HashMap;
import java.util.Hashtable;
import sun.awt.FontDescriptor;
import sun.awt.FontConfiguration;
import sun.io.CharacterEncoding;
import sun.io.CharToByteConverter;
import sun.java2d.SunGraphicsEnvironment;

public class WFontConfiguration extends FontConfiguration {

    // whether compatibility fallbacks for TimesRoman and Co. are used
    private boolean useCompatibilityFallbacks;
    
    public WFontConfiguration(SunGraphicsEnvironment environment) {
	super(environment);
        useCompatibilityFallbacks = "windows-1252".equals(encoding);
        initTables(encoding);
    }
    
    public WFontConfiguration(SunGraphicsEnvironment environment,
			      boolean preferLocaleFonts,
			      boolean preferPropFonts) {
	super(environment, preferLocaleFonts, preferPropFonts);
        useCompatibilityFallbacks = "windows-1252".equals(encoding);
    }

    protected void initReorderMap() {
	if (encoding.equalsIgnoreCase("windows-31j")) {
	    localeMap = new Hashtable();
	    /* Substitute Mincho for Gothic in this one case.
	     * Note the windows fontconfig files already contain the mapping:
	     * filename.MS_Mincho=MSMINCHO.TTC
	     * which isn't essential to this usage but avoids a call
	     * to loadfonts in the event MSMINCHO.TTC has not otherwise
	     * been opened and its fonts loaded.
	     * Also note this usage is only enabled if a private flag is set.
	     */
            if ("98".equals(osName) || "Me".equals(osName)) {
                localeMap.put("dialoginput.plain.japanese", "\uff2d\uff33 \u660e\u671d");
		localeMap.put("dialoginput.bold.japanese", "\uff2d\uff33 \u660e\u671d");
		localeMap.put("dialoginput.italic.japanese", "\uff2d\uff33 \u660e\u671d");
		localeMap.put("dialoginput.bolditalic.japanese", "\uff2d\uff33 \u660e\u671d");
	    } else {

                localeMap.put("dialoginput.plain.japanese", "MS Mincho");
		localeMap.put("dialoginput.bold.japanese", "MS Mincho");
		localeMap.put("dialoginput.italic.japanese", "MS Mincho");
		localeMap.put("dialoginput.bolditalic.japanese", "MS Mincho");
            }
	}
	reorderMap = new HashMap();
	reorderMap.put("UTF-8.hi", "devanagari");
	reorderMap.put("windows-1255", "hebrew");
	reorderMap.put("x-windows-874", "thai");
	reorderMap.put("windows-31j", "japanese");
	reorderMap.put("x-windows-949", "korean");
	reorderMap.put("GBK", "chinese-ms936");
	reorderMap.put("GB18030", "chinese-gb18030");
	reorderMap.put("x-windows-950", "chinese-ms950");
	reorderMap.put("x-MS950-HKSCS", split("chinese-ms950,chinese-hkscs"));
// 	reorderMap.put("windows-1252", "alphabetic");
    }

    protected void setOsNameAndVersion(){
        super.setOsNameAndVersion();
        if (osName.startsWith("Windows")){
            int p, q;
            p = osName.indexOf(' ');
            if (p == -1){
                osName = null;
            }
            else{
                q = osName.indexOf(' ', p + 1);
                if (q == -1){
                    osName = osName.substring(p + 1);
		}
                else{
                    osName = osName.substring(p + 1, q);
                }
	    }
            osVersion = null;
        }
    }

    // overrides FontConfiguration.getFallbackFamilyName
    public String getFallbackFamilyName(String fontName, String defaultFallback) {
        // maintain compatibility with old font.properties files, where
        // default file had aliases for timesroman & Co, while others didn't.
        if (useCompatibilityFallbacks) {
            String compatibilityName = getCompatibilityFamilyName(fontName);
            if (compatibilityName != null) {
                return compatibilityName;
            }
        }
        return defaultFallback;
    }

    protected String makeAWTFontName(String platformFontName, String characterSubsetName) {
        String windowsCharset = (String) subsetCharsetMap.get(characterSubsetName);
        if (windowsCharset == null) {
            windowsCharset = "DEFAULT_CHARSET";
        }
        return platformFontName + "," + windowsCharset;
    }
    
    protected String getEncoding(String awtFontName, String characterSubsetName) {
        String encoding = (String) subsetEncodingMap.get(characterSubsetName);
        if (encoding == null) {
            encoding = "default";
        }
        return encoding;
    }

    protected CharToByteConverter getDefaultFontCharset(String fontName) {
	return new WDefaultFontCharset(fontName);
    }

    public String getFaceNameFromComponentFontName(String componentFontName) {
        // for Windows, the platform name is the face name
        return componentFontName;
    }

    protected String getFileNameFromComponentFontName(String componentFontName) {
	return getFileNameFromPlatformName(componentFontName);
    }

    /**
     * Returns the component font name (face name plus charset) of the
     * font that should be used for AWT text components. May return null.
     */
    public String getTextComponentFontName(String familyName, int style) {
        FontDescriptor[] fontDescriptors = getFontDescriptors(familyName, style);
        String fontName = findFontWithCharset(fontDescriptors, textInputCharset);
        if (fontName == null) {
            fontName = findFontWithCharset(fontDescriptors, "DEFAULT_CHARSET");
        }
        return fontName;
    }
    
    private String findFontWithCharset(FontDescriptor[] fontDescriptors, String charset) {
        String fontName = null;
        for (int i = 0; i < fontDescriptors.length; i++) {
            String componentFontName = fontDescriptors[i].getNativeName();
            if (componentFontName.endsWith(charset)) {
                fontName = componentFontName;
            }
        }
        return fontName;
    }
    
    private static HashMap subsetCharsetMap = new HashMap();
    private static HashMap subsetEncodingMap = new HashMap();
    private static String textInputCharset;
    
    private void initTables(String defaultEncoding) {
        subsetCharsetMap.put("alphabetic", "ANSI_CHARSET");
        subsetCharsetMap.put("alphabetic/1252", "ANSI_CHARSET");
        subsetCharsetMap.put("alphabetic/default", "DEFAULT_CHARSET");
        subsetCharsetMap.put("arabic", "ARABIC_CHARSET");
        subsetCharsetMap.put("chinese-ms936", "GB2312_CHARSET");
        subsetCharsetMap.put("chinese-gb18030", "GB2312_CHARSET");
        subsetCharsetMap.put("chinese-ms950", "CHINESEBIG5_CHARSET");
        subsetCharsetMap.put("chinese-hkscs", "CHINESEBIG5_CHARSET");
        subsetCharsetMap.put("cyrillic", "RUSSIAN_CHARSET");
        subsetCharsetMap.put("devanagari", "DEFAULT_CHARSET");
        subsetCharsetMap.put("dingbats", "SYMBOL_CHARSET");
        subsetCharsetMap.put("greek", "GREEK_CHARSET");
        subsetCharsetMap.put("hebrew", "HEBREW_CHARSET");
        subsetCharsetMap.put("japanese", "SHIFTJIS_CHARSET");
        subsetCharsetMap.put("korean", "HANGEUL_CHARSET");
        subsetCharsetMap.put("latin", "ANSI_CHARSET");
        subsetCharsetMap.put("symbol", "SYMBOL_CHARSET");
        subsetCharsetMap.put("thai", "THAI_CHARSET");

        subsetEncodingMap.put("alphabetic", "default");
        subsetEncodingMap.put("alphabetic/1252", "sun.io.CharToByteCp1252");
        subsetEncodingMap.put("alphabetic/default",
                    "sun.io.CharToByte" + CharacterEncoding.aliasName(defaultEncoding));
        subsetEncodingMap.put("arabic", "sun.io.CharToByteCp1256");
        subsetEncodingMap.put("chinese-ms936", "sun.io.CharToByteGBK");
        subsetEncodingMap.put("chinese-gb18030", "sun.io.CharToByteGB18030");
        if ("x-MS950-HKSCS".equals(defaultEncoding)) {
            subsetEncodingMap.put("chinese-ms950", "sun.io.CharToByteMS950_HKSCS");
        } else {
            subsetEncodingMap.put("chinese-ms950", "sun.io.CharToByteMS950");
        }
        subsetEncodingMap.put("chinese-hkscs", "sun.io.CharToByteHKSCS");
        subsetEncodingMap.put("cyrillic", "sun.io.CharToByteCp1251");
        subsetEncodingMap.put("devanagari", "sun.io.CharToByteUnicodeLittle");
        subsetEncodingMap.put("dingbats", "sun.awt.windows.CharToByteWingDings");
        subsetEncodingMap.put("greek", "sun.io.CharToByteCp1253");
        subsetEncodingMap.put("hebrew", "sun.io.CharToByteCp1255");
        subsetEncodingMap.put("japanese", "sun.io.CharToByteMS932");
        subsetEncodingMap.put("korean", "sun.io.CharToByteMS949");
        subsetEncodingMap.put("latin", "sun.io.CharToByteCp1252");
        subsetEncodingMap.put("symbol", "sun.awt.CharToByteSymbol");
        subsetEncodingMap.put("thai", "sun.io.CharToByteMS874");
        
        if ("windows-1256".equals(defaultEncoding)) {
            textInputCharset = "ARABIC_CHARSET";
        } else if ("GBK".equals(defaultEncoding)) {
            textInputCharset = "GB2312_CHARSET";
        } else if ("GB18030".equals(defaultEncoding)) {
            textInputCharset = "GB2312_CHARSET";
        } else if ("x-windows-950".equals(defaultEncoding)) {
            textInputCharset = "CHINESEBIG5_CHARSET";
        } else if ("x-MS950-HKSCS".equals(defaultEncoding)) {
            textInputCharset = "CHINESEBIG5_CHARSET";
        } else if ("windows-1251".equals(defaultEncoding)) {
            textInputCharset = "RUSSIAN_CHARSET";
        } else if ("UTF-8".equals(defaultEncoding)) {
            textInputCharset = "DEFAULT_CHARSET";
        } else if ("windows-1253".equals(defaultEncoding)) {
            textInputCharset = "GREEK_CHARSET";
        } else if ("windows-1255".equals(defaultEncoding)) {
            textInputCharset = "HEBREW_CHARSET";
        } else if ("windows-31j".equals(defaultEncoding)) {
            textInputCharset = "SHIFTJIS_CHARSET";
        } else if ("x-windows-949".equals(defaultEncoding)) {
            textInputCharset = "HANGEUL_CHARSET";
        } else if ("x-windows-874".equals(defaultEncoding)) {
            textInputCharset = "THAI_CHARSET";
        } else {
            textInputCharset = "DEFAULT_CHARSET";
        }
    }
}


