/*
 * @(#)LocaleElements_da.java	1.20 04/01/20
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

public class LocaleElements_da extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "da", "dansk" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "DK", "Danmark" }
                }
            },
            { "MonthNames",
                new String[] {
                    "januar", // january
                    "februar", // february
                    "marts", // march
                    "april", // april
                    "maj", // may
                    "juni", // june
                    "juli", // july
                    "august", // august
                    "september", // september
                    "oktober", // october
                    "november", // november
                    "december", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "jan", // abb january
                    "feb", // abb february
                    "mar", // abb march
                    "apr", // abb april
                    "maj", // abb may
                    "jun", // abb june
                    "jul", // abb july
                    "aug", // abb august
                    "sep", // abb september
                    "okt", // abb october
                    "nov", // abb november
                    "dec", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "s\u00f8ndag", // Sunday
                    "mandag", // Monday
                    "tirsdag", // Tuesday
                    "onsdag", // Wednesday
                    "torsdag", // Thursday
                    "fredag", // Friday
                    "l\u00f8rdag" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "s\u00f8", // abb Sunday
                    "ma", // abb Monday
                    "ti", // abb Tuesday
                    "on", // abb Wednesday
                    "to", // abb Thursday
                    "fr", // abb Friday
                    "l\u00f8" // abb Saturday
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
                    "yyyy-MM-dd", // medium date pattern
                    "yy-MM-dd", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "DateTimeElements",
                new String[] {
                    "2", // first day of week
                    "4" // min days in first week
                }
            },
            { "CollationElements",
	        "& D <\u00D0,\u00F0"      // eth
                + "& ss,\u00DF"             // s-zet
                + "& th, \u00DE &th, \u00FE "     // thorn
                + "& A < \u00e6\u0301 , \u00c6\u0301" // A < ae-acute
                + "& Z < \u00e6 , \u00c6" // Z < ae
                + "; a\u0308 , A\u0308 "       // a-umlaut
                + "< \u00f8 , \u00d8 ; o\u0308 , O\u0308 "  // o-stroke ; o-umlaut
                + "; o\u030b, O\u030b"        // nt :  o-double-acute
                + "< a\u030a , A\u030a"       // a-ring
                + ", aa , aA , Aa , AA "      // after a-ring
                + "& oe, \u0152 &oe, \u0153 " // oe ligature
                + " & V, w, W "
                + "& Y ; u\u0308 , U\u0308"   // j1.1.4 : u-umlaut is eq. to y.
            }
        };
    }
}
