/*
 * @(#)TrueTypeGlyphMapper.java	1.5 03/18/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.nio.ByteBuffer;
import java.util.Locale;

public class TrueTypeGlyphMapper extends CharToGlyphMapper {

    static final char REVERSE_SOLIDUS = 0x005c; // the backslash char.
    static final char JA_YEN = 0x00a5;
    static final char JA_FULLWIDTH_TILDE_CHAR = 0xff5e;
    static final char JA_WAVE_DASH_CHAR = 0x301c;

    /* if running on Solaris and default Locale is ja_JP then
     * we map need to remap reverse solidus (backslash) to Yen as 
     * apparently expected there.
     */
    static final boolean isJAlocale = Locale.JAPAN.equals(Locale.getDefault());
    private final boolean needsJAremapping;
    private boolean remapJAWaveDash;

    TrueTypeFont font;
    CMap cmap;
    int numGlyphs;

    public TrueTypeGlyphMapper(TrueTypeFont font) {
	this.font = font;
	try {
	    cmap = CMap.initialize(font);
	} catch (Exception e) {
	    cmap = null;
	}
	if (cmap == null) {
	    if (cmap == null) {
		if (FontManager.logging) {
		    FontManager.logger.severe("Null Cmap for " + font +
					      "substituting for this font");
		}
		FontManager.deRegisterBadFont(font);
		/* The next line is not really a solution, but might
		 * reduce the exceptions until references to this font2D
		 * are gone.
		 */
		cmap = CMap.theNullCmap;
	    }
	}
	missingGlyph = 0; /* standard for TrueType fonts */
        ByteBuffer buffer = font.getTableBuffer(TrueTypeFont.maxpTag);
	numGlyphs = buffer.getChar(4); // offset 4 bytes in MAXP table.
	if (FontManager.isSolaris && isJAlocale && font.supportsJA()) {
	    needsJAremapping = true;
	    if (FontManager.isSolaris8 &&
		cmap.getGlyph(JA_WAVE_DASH_CHAR) == missingGlyph) {
		remapJAWaveDash = true;
	    }
	} else {
	    needsJAremapping = false;
	}
    }

    public int getNumGlyphs() {
	return numGlyphs;
    }

    private final char remapJAChar(char unicode) {
	switch (unicode) {
	case REVERSE_SOLIDUS:
	    return JA_YEN;
	    /* This is a workaround for bug 4533422.
	     * Japanese wave dash missing from Solaris JA TrueType fonts.
	     */
	case JA_WAVE_DASH_CHAR:
	    if (remapJAWaveDash) {
		return JA_FULLWIDTH_TILDE_CHAR;
	    }
	default: return unicode;
	}
    }
    private final int remapJAIntChar(int unicode) {
	switch (unicode) {
	case REVERSE_SOLIDUS:
	    return JA_YEN;
	    /* This is a workaround for bug 4533422.
	     * Japanese wave dash missing from Solaris JA TrueType fonts.
	     */
	case JA_WAVE_DASH_CHAR:
	    if (remapJAWaveDash) {
		return JA_FULLWIDTH_TILDE_CHAR;
	    }
	default: return unicode;
	}
    }

    public int charToGlyph(char unicode) {
	if (needsJAremapping) {
	    unicode = remapJAChar(unicode);
	}
	int glyph = cmap.getGlyph(unicode);
	if (font.useNatives && glyph < font.glyphToCharMap.length) {
	    font.glyphToCharMap[glyph] = unicode;
	}
        return glyph;
    }

    public int charToGlyph(int unicode) {
	if (needsJAremapping) {
	    unicode = remapJAIntChar(unicode);
	}	
	int glyph = cmap.getGlyph(unicode);
	if (font.useNatives && glyph < font.glyphToCharMap.length) {
	    font.glyphToCharMap[glyph] = (char)unicode;
	}
        return glyph;
    }

    public void charsToGlyphs(int count, int[] unicodes, int[] glyphs) {
	if (cmap != null) {
	    for (int i=0;i<count;i++) {
		if (needsJAremapping) {
		    glyphs[i] = cmap.getGlyph(remapJAIntChar(unicodes[i]));
		} else {		    
		    glyphs[i] = cmap.getGlyph(unicodes[i]);
		}
		if (font.useNatives && glyphs[i] < font.glyphToCharMap.length){
		    font.glyphToCharMap[glyphs[i]] = (char)unicodes[i];
		}
	    }
	}
    }

    public void charsToGlyphs(int count, char[] unicodes, int[] glyphs) {

	for (int i=0; i<count; i++) {
	    int code;
	    if (needsJAremapping) {
		code = remapJAChar(unicodes[i]);
	    } else {
		code = unicodes[i]; // char is unsigned.
	    }
	    
	    if (code >= HI_SURROGATE_START && 
		code <= HI_SURROGATE_END && i < count - 1) {
		char low = unicodes[i + 1];
		
		if (low >= LO_SURROGATE_START &&
		    low <= LO_SURROGATE_END) {
		    code = (code - HI_SURROGATE_START) * 
			0x400 + low - LO_SURROGATE_START + 0x10000;

		    glyphs[i] = cmap.getGlyph(code);
		    i += 1; // Empty glyph slot after surrogate
		    glyphs[i] = INVISIBLE_GLYPH_ID;
		    continue;
		}
	    }
	    glyphs[i] = cmap.getGlyph(code);

	    if (font.useNatives && glyphs[i] < font.glyphToCharMap.length) {
		font.glyphToCharMap[glyphs[i]] = (char)code;
	    }
	    
	}
    }

    /* This variant checks if shaping is needed and immediately
     * returns true if it does. A caller of this method should be expecting
     * to check the return type because it needs to know how to handle
     * the character data for display.
     */
    public boolean charsToGlyphsNS(int count, char[] unicodes, int[] glyphs) {

	for (int i=0; i<count; i++) {
	    int code;
	    if (needsJAremapping) {
		code = remapJAChar(unicodes[i]);
	    } else {
		code = unicodes[i]; // char is unsigned.
	    }
	    
	    if (code >= HI_SURROGATE_START && 
		code <= HI_SURROGATE_END && i < count - 1) {
		char low = unicodes[i + 1];
		
		if (low >= LO_SURROGATE_START &&
		    low <= LO_SURROGATE_END) {
		    code = (code - HI_SURROGATE_START) * 
			0x400 + low - LO_SURROGATE_START + 0x10000;
		    glyphs[i + 1] = INVISIBLE_GLYPH_ID;
		}
	    }

	    glyphs[i] = cmap.getGlyph(code);
	    if (font.useNatives && glyphs[i] < font.glyphToCharMap.length) {
		font.glyphToCharMap[glyphs[i]] = (char)code;
	    }

	    if (code < 0x0590) {
		continue;
	    }
	    else if (code <= 0x05ff) { // Hebrew 0x0590->0x05ff
		return true;
	    }
	    else if (code >= 0x0600 && code <= 0x06ff) { // Arabic
		return true;
	    }
	    else if (code >= 0x0900 && code <= 0x0d7f) {
		// if Indic, assume shaping for conjuncts, reordering:
		// 0900 - 097F Devanagari
		// 0980 - 09FF Bengali
		// 0A00 - 0A7F Gurmukhi
		// 0A80 - 0AFF Gujarati
		// 0B00 - 0B7F Oriya
		// 0B80 - 0BFF Tamil
		// 0C00 - 0C7F Telugu
		// 0C80 - 0CFF Kannada
		// 0D00 - 0D7F Malayalam
		return true;
	    }
	    else if (code >= 0x0e00 && code <= 0x0e7f) {
		// if Thai, assume shaping for vowel, tone marks
		return true;
	    }
	    else if (code >= 0x200c && code <= 0x200d) { //  zwj or zwnj
		return true;
	    }
	    else if (code >= 0x202a && code <= 0x202e) { // directional control
		return true;
	    }
	    else if (code >= 0x206a && code <= 0x206f) { // directional control
		return true;
	    }
	    else if (code >= 0x10000) {
		i += 1; // Empty glyph slot after surrogate
		continue;
	    }
	}

	return false;
    }
}
