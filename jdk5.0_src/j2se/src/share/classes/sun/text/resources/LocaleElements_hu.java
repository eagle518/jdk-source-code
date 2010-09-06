/*
 * @(#)LocaleElements_hu.java	1.18 03/12/19
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

public class LocaleElements_hu extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "hu", "magyar" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "HU", "Magyarorsz\u00e1g" }
                }
            },
            { "MonthNames",
                new String[] {
                    "janu\u00e1r", // january
                    "febru\u00e1r", // february
                    "m\u00e1rcius", // march
                    "\u00e1prilis", // april
                    "m\u00e1jus", // may
                    "j\u00fanius", // june
                    "j\u00falius", // july
                    "augusztus", // august
                    "szeptember", // september
                    "okt\u00f3ber", // october
                    "november", // november
                    "december", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "jan.", // abb january
                    "febr.", // abb february
                    "m\u00e1rc.", // abb march
                    "\u00e1pr.", // abb april
                    "m\u00e1j.", // abb may
                    "j\u00fan.", // abb june
                    "j\u00fal.", // abb july
                    "aug.", // abb august
                    "szept.", // abb september
                    "okt.", // abb october
                    "nov.", // abb november
                    "dec.", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "vas\u00e1rnap", // Sunday
                    "h\u00e9tf\u0151", // Monday
                    "kedd", // Tuesday
                    "szerda", // Wednesday
                    "cs\u00fct\u00f6rt\u00f6k", // Thursday
                    "p\u00e9ntek", // Friday
                    "szombat" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "V", // abb Sunday
                    "H", // abb Monday
                    "K", // abb Tuesday
                    "Sze", // abb Wednesday
                    "Cs", // abb Thursday
                    "P", // abb Friday
                    "Szo" // abb Saturday
                }
            },
            { "AmPmMarkers",
                new String[] {
                    "DE", // am marker
                    "DU" // pm marker
                }
            },
            { "Eras",
                new String[] { // era strings
                    "i.e.",
                    "i.u."
                }
            },
            { "NumberElements",
                new String[] {
                    ",", // decimal separator
                    "\u00a0", // group (thousands) separator
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
                    "H:mm:ss z", // full time pattern
                    "H:mm:ss z", // long time pattern
                    "H:mm:ss", // medium time pattern
                    "H:mm", // short time pattern
                    "yyyy. MMMM d.", // full date pattern
                    "yyyy. MMMM d.", // long date pattern
                    "yyyy.MM.dd.", // medium date pattern
                    "yyyy.MM.dd.", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CollationElements",
                /* for HU_HU, default sorting except for the following: */
                /* add cs "ligature" between c and d. */
                /* add d<stroke> between d and e. */
                /* add gy "ligature" between g and h. */
                /* add ly "ligature" between l and l<stroke>. */
                /* add l<stroke> between l and m. */
                /* add sz "ligature" between s and t. */
                /* add zs "ligature" between z and z<abovedot> */
                /* add z<abovedot> after z.       */
                "& C < cs , cS , Cs , CS " // cs ligatures
                + "& D < \u0111, \u0110 "    // tal : african d < d-stroke
                + "& G < gy, Gy, gY, GY "    // gy ligatures
                + "& L < ly, Ly, lY, LY "    // l < ly
                + "& O < o\u0308 , O\u0308 " // O < o-umlaut
                + "< o\u030b , O\u030b "     // o-double-accute
                + "& S < sz , sZ , Sz , SZ " // s < sz ligature
                + "& U < u\u0308 , U\u0308 " // u < u-umlaut
                + "< u\u030b , U\u030b "     // u-double-accute
                + "& Z < zs , zS , Zs , ZS " // stop-stroke < zs ligature
            }
        };
    }
}
