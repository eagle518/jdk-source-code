/*
 * @(#)FontResolver.java	1.6 03/08/18
 * (C) Copyright IBM Corp. 1999,  All rights reserved.
 */

package sun.font;

import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.font.TextAttribute;
import java.util.Map;
import sun.text.CodePointIterator;

/**
 * This class maps an individual character to a Font family which can 
 * display it.  The character-to-Font mapping does not depend on the
 * character's context, so a particular character will be mapped to the
 * same font family each time.
 * <p>
 * Typically, clients will call getIndexFor(char) for each character
 * in a style run.  When getIndexFor() returns a different value from
 * ones seen previously, the characters up to that point will be assigned
 * a font obtained from getFont().
 */
public final class FontResolver {
    
    // An array of all fonts available to the runtime.  The fonts
    // will be searched in order.
    private Font[] allFonts;

    // Default size of Fonts (if created from an empty Map, for instance).
    private static final float DEFAULT_SIZE = 12; // from Font
    
    // The results of previous lookups are cached in a two-level
    // table.  The value for a character c is found in:
    //     blocks[c>>SHIFT][c&MASK]
    // although the second array is only allocated when needed.
    // A 0 value means the character's font has not been looked up.
    // A positive value means the character's font is in the allFonts
    // array at index (value-1).
    private static final int SHIFT = 9;
    private static final int BLOCKSIZE = 1<<(16-SHIFT);
    private static final int MASK = BLOCKSIZE-1;
    private int[][] blocks = new int[1<<SHIFT][];
    
    private FontResolver() {
        allFonts = GraphicsEnvironment.getLocalGraphicsEnvironment().getAllFonts();
        for (int i=0; i < allFonts.length; i++) {
            allFonts[i] = allFonts[i].deriveFont(DEFAULT_SIZE); 
        }
    }
    
    /**
     * Search fonts in order, and return the index of the first font
     * which can display the given character, plus 1.
     */
    private int getIndexFor(char c) {
        
        for (int i=0; i < allFonts.length; i++) {
            if (allFonts[i].canDisplay(c)) {
                return i+1;
            }
        }
        return 1;
    }

    private int getIndexFor(int cp) {
        if (cp < 0x10000) {
            return getIndexFor((char)cp);
        }

        for (int i = 0; i < allFonts.length; i++) {
            if (allFonts[i].canDisplay(cp)) {
                return i+1;
            }
        }
        return 1;
    }

    /**
     * Return an index for the given character.  The index identifies a
     * font family to getFont(), and has no other inherent meaning.
     * @param c the character to map
     * @return a value for consumption by getFont()
     * @see #getFont
     */
    public int getFontIndex(char c) {
        
        int blockIndex = c>>SHIFT;
        int[] block = blocks[blockIndex];
        if (block == null) {
            block = new int[BLOCKSIZE];
            blocks[blockIndex] = block;
        }
        
        int index = c & MASK;
        if (block[index] == 0) {
            block[index] = getIndexFor(c);
        }
        return block[index];
    }

    public int getFontIndex(int cp) {
        if (cp < 0x10000) {
            return getFontIndex((char)cp);
        }
        return getIndexFor(cp); // !!! note: terribly slow, currently
    }

    /**
     * Determines the font index for the code point at the current position in the
     * iterator, then advances the iterator to the first code point that has
     * a different index or until the iterator is DONE, and returns the font index.
     * @param iter a code point iterator, this will be advanced past any code
     *             points that have the same font index
     * @return the font index for the initial code point found, or 1 if the iterator
     * was empty.
     */
    public int nextFontRunIndex(CodePointIterator iter) {
	int cp = iter.next();
	int fontIndex = 1;
	if (cp != CodePointIterator.DONE) {
	    fontIndex = getFontIndex(cp);

	    while ((cp = iter.next()) != CodePointIterator.DONE) {
		if (getFontIndex(cp) != fontIndex) {
		    iter.prev();
		    break;
		}
	    }
	}
	return fontIndex;
    }

    /**
     * Return a Font from a given font index with properties
     * from attributes.  The font index, which should have been produced
     * by getFontIndex(), determines a font family.  The size and style
     * of the Font reflect the properties in attributes.  Any Font or
     * font family specifications in attributes are ignored, on the 
     * assumption that clients have already handled them.
     * @param index an index from getFontIndex() which determines the
     *        font family
     * @param attributes a Map from which the size and style of the Font
     *        are determined.  The default size is 12 and the default style
     *        is Font.PLAIN
     * @see #getFontIndex
     */
    public Font getFont(int index, Map attributes) {
        
        Font font = allFonts[index-1];
        return font.deriveFont(attributes);
    }
    
    private static FontResolver INSTANCE;
    
    /**
     * Return a shared instance of FontResolver.
     */
    public static FontResolver getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new FontResolver();
        }
        return INSTANCE;
    }
}
