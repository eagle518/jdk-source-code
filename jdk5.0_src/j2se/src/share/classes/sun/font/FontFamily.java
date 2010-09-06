/*
 * @(#)FontFamily.java	1.4 01/06/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.Font;
import java.util.HashMap;
import java.util.Locale;

public class FontFamily {

    private static HashMap familyNameMap = new HashMap();
    private static HashMap allLocaleNames;

    protected String familyName;
    protected Font2D plain;
    protected Font2D bold;
    protected Font2D italic;
    protected Font2D bolditalic;
    protected boolean logicalFont = false;
    protected int familyRank;

    public static FontFamily getFamily(String name) {
	synchronized (familyNameMap) {
	    return
	       (FontFamily)familyNameMap.get(name.toLowerCase(Locale.ENGLISH));
	}
    }

    public static String[] getAllFamilyNames() {
	return null;
    }

    /* Only for use by FontManager.deRegisterBadFont(..).
     * If this was the only font in the family, the family is removed
     * from the map
     */
    static void remove(Font2D font2D) {
	
	String name = font2D.getFamilyName(Locale.ENGLISH);
	FontFamily family = getFamily(name);
	if (family == null) {
	    return;
	}
	if (family.plain == font2D) {
	    family.plain = null;
	}
	if (family.bold == font2D) {
	    family.bold = null;
	}
	if (family.italic == font2D) {
	    family.italic = null;
	}
	if (family.bolditalic == font2D) {
	    family.bolditalic = null;
	}
	if (family.plain == null && family.bold == null &&
	    family.plain == null && family.bold == null) {
	    synchronized (familyNameMap) {
		familyNameMap.remove(name);
	    }
	}
    }

    public FontFamily(String name, boolean isLogFont, int rank) {
	logicalFont = isLogFont;
	familyName = name;
	familyRank = rank;
	synchronized (familyNameMap) {
	    familyNameMap.put(name.toLowerCase(Locale.ENGLISH), this);
	}
    }

    public String getFamilyName() {
	return familyName;
    }

    public int getRank() {
	return familyRank;
    }

    public void setFont(Font2D font, int style) {
	if (font.getRank() > familyRank) {
	    if (FontManager.logging) {
		FontManager.logger.warning("Rejecting adding " + font +
					   " of lower rank " + font.getRank() +
					   " to family " + this +
					   " of rank " + familyRank);
	    }
	    return;
	}

	switch (style) {

	case Font.PLAIN:
	    plain = font;
	    break;

	case Font.BOLD:
	    bold = font;
	    break;

	case Font.ITALIC:
	    italic = font;
	    break;
	    
	case Font.BOLD|Font.ITALIC:
	    bolditalic = font;
	    break;

	default:
	    break;
	}
    }

    public Font2D getFontWithExactStyleMatch(int style) {

	switch (style) {

	case Font.PLAIN:
	    return plain;

	case Font.BOLD:
	    return bold;

	case Font.ITALIC:
	    return italic;
   
	case Font.BOLD|Font.ITALIC:
	    return bolditalic;

	default:
	    return null;
	}
    }

    /* REMIND: if the callers of this method are operating in an
     * environment in which not all fonts are registered, the returned
     * font may be a algorithmically styled one, where in fact if loadfonts
     * were executed, a styled font may be located. Our present "solution"
     * to this is to register all fonts in a directory and assume that this
     * registered all the styles of a font, since they would all be in the
     * same location.
     */
    public Font2D getFont(int style) {

	switch (style) {

	case Font.PLAIN:
	    return plain;

	case Font.BOLD:
	    if (bold != null) {
		return bold;
	    } else if (plain != null && plain.canDoStyle(style)) {
		    return plain;
	    } else {
		return null;
	    }

	case Font.ITALIC:
	    if (italic != null) {
		return italic;
	    } else if (plain != null && plain.canDoStyle(style)) {
		    return plain;
	    } else {
		return null;
	    }
   
	case Font.BOLD|Font.ITALIC:
	    if (bolditalic != null) {
		return bolditalic;
	    } else if (italic != null && italic.canDoStyle(style)) {
		    return italic;
	    } else if (bold != null && bold.canDoStyle(style)) {
		    return italic;
	    } else if (plain != null && plain.canDoStyle(style)) {
		    return plain;
	    } else {
		return null;
	    }
	default:
	    return null;
	}
    }

    /* Only to be called if getFont(style) returns null
     * This method will only return null if the family is completely empty!
     * Note that it assumes the font of the style you need isn't in the
     * family. The logic here is that if we must substitute something
     * it might as well be from the same family.
     */
     Font2D getClosestStyle(int style) {

	switch (style) {
	    /* if you ask for a plain font try to return a non-italic one,
	     * then a italic one, finally a bold italic one */
        case Font.PLAIN:
	    if (bold != null) {
		return bold;
	    } else if (italic != null) {
		return italic;
	    } else {
		return bolditalic;
	    }
	    
	    /* if you ask for a bold font try to return a non-italic one,
	     * then a bold italic one, finally an italic one */
	case Font.BOLD:
	    if (plain != null) {
		return plain;
	    } else if (bolditalic != null) {
		return bolditalic;
	    } else {
		return italic;
	    }
	    
	    /* if you ask for a italic font try to return a  bold italic one,
	     * then a plain one, finally an bold one */
	case Font.ITALIC:
	    if (bolditalic != null) {
		return bolditalic;
	    } else if (plain != null) {
		return plain;
	    } else {
		return bold;
	    }
	
	case Font.BOLD|Font.ITALIC:
	    if (italic != null) {
		return italic;
	    } else if (bold != null) {
		return bold;
	    } else {
		return plain;
	    }
	}
	return null;
    }

    /* Font may have localized names. Store these in a separate map, so
     * that only clients who use these names need be affected.
     */
    static synchronized void addLocaleNames(FontFamily family, String[] names){
	if (allLocaleNames == null) {
	    allLocaleNames = new HashMap();
	}
	for (int i=0; i<names.length; i++) {
	    allLocaleNames.put(names[i].toLowerCase(), family);
	}
    }

    public static synchronized FontFamily getLocaleFamily(String name) {
	if (allLocaleNames == null) {
	    return null;
	}
	return (FontFamily)allLocaleNames.get(name.toLowerCase());
    }

    public String toString() {
	return
	    "Font family: " + familyName +
	    " plain="+plain+
	    " bold=" + bold +
	    " italic=" + italic +
	    " bolditalic=" + bolditalic;
	    
    }

}
