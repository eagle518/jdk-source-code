/*
 * @(#)LocaleElements_is.java	1.21 03/12/19
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright Taligent, Inc. 1996, 1997 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - 1998 - All Rights Reserved
 *
 * The original version of this source code and documentation
 * is copyrighted and owned by Taligent, Inc., a wholly-owned
 * subsidiary of IBM. These materials are provided under terms
 * of a License Agreement between Taligent and Sun. This technology
 * is protected by multiple US and International patents.
 *
 * This notice and attribution to Taligent may not be removed.
 * Taligent is a registered trademark of Taligent, Inc.
 *
 */

package sun.text.resources;

import java.util.ListResourceBundle;

public class LocaleElements_is extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "is", "\u00edslenska" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "IS", "\u00cdsland" }
                }
            },
            { "MonthNames",
                new String[] {
                    "jan\u00faar", // january
                    "febr\u00faar", // february
                    "mars", // march
                    "apr\u00edl", // april
                    "ma\u00ed", // may
                    "j\u00fan\u00ed", // june
                    "j\u00fal\u00ed", // july
                    "\u00e1g\u00fast", // august
                    "september", // september
                    "okt\u00f3ber", // october
                    "n\u00f3vember", // november
                    "desember", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "jan.", // abb january
                    "feb.", // abb february
                    "mar.", // abb march
                    "apr.", // abb april
                    "ma\u00ed", // abb may
                    "j\u00fan.", // abb june
                    "j\u00fal.", // abb july
                    "\u00e1g\u00fa.", // abb august
                    "sep.", // abb september
                    "okt.", // abb october
                    "n\u00f3v.", // abb november
                    "des.", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "sunnudagur", // Sunday
                    "m\u00e1nudagur", // Monday
                    "\u00feri\u00f0judagur", // Tuesday
                    "mi\u00f0vikudagur", // Wednesday
                    "fimmtudagur", // Thursday
                    "f\u00f6studagur", // Friday
                    "laugardagur" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "sun.", // abb Sunday
                    "m\u00e1n.", // abb Monday
                    "\u00feri.", // abb Tuesday
                    "mi\u00f0.", // abb Wednesday
                    "fim.", // abb Thursday
                    "f\u00f6s.", // abb Friday
                    "lau." // abb Saturday
                }
            },
            { "NumberElements",
                new String[] {
                    ",", // decimal separator
                    ".", // group (thousands) separator
                    ";", // list separator
                    "%", // percent sign
                    "0", // native 0 digit
                    "#", // pattern digit
                    "-", // minus sign
                    "E", // exponential
                    "\u2030", // per mille
                    "\u221e", // infinity
                    "\ufffd" // NaN
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "HH:mm:ss z", // full time pattern
                    "HH:mm:ss z", // long time pattern
                    "HH:mm:ss", // medium time pattern
                    "HH:mm", // short time pattern
                    "d. MMMM yyyy", // full date pattern
                    "d. MMMM yyyy", // long date pattern
                    "d.M.yyyy", // medium date pattern
                    "d.M.yyyy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CollationElements",
                /* for IS_IS, accents sorted backwards plus the following: */

                "@"                                           /* sort accents bkwd */
                /* assuming that in the default collation we add:                   */
                /*  thorn, ae ligature, o-diaeresis, and o-slash                    */
                /*  ....in this order...and ditto for the uppercase of these....    */
                /* to be treated as characters (not accented characters) after z    */
                /* then we don't have to add anything here. I've just added it here */
                /* just in case it gets overlooked.                                 */
                + "& A < a\u0301, A\u0301 "       // nt : A < a-acute
                + "& D < \u00f0, \u00d0"          // nt : d < eth
                + "& E < e\u0301, E\u0301 "       // nt : e < e-acute
                + "& I < i\u0301, I\u0301 "       // nt : i < i-acute
                + "& O < o\u0301, O\u0301 "       // nt : o < o-acute
                + "& U < u\u0301, U\u0301 "       // nt : u < u-acute
                + "& Y < y\u0301, Y\u0301 "       // nt : y < y-acute
                + "& Z < \u00fe, \u00de < \u00e6, \u00c6" // nt : z < thron < a-e-ligature
                + "< o\u0308, O\u0308 ; \u00f8, \u00d8" // nt : o-umlaut ; o-stroke
            }
        };
    }
}
