/*
 * @(#)LocaleElements_sl.java	1.18 03/12/19
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

public class LocaleElements_sl extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "sl", "Sloven\u0161\u010dina" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "SI", "Slovenija" }
                }
            },
            { "MonthNames",
                new String[] {
                    "januar", // january
                    "februar", // february
                    "marec", // march
                    "april", // april
                    "maj", // may
                    "junij", // june
                    "julij", // july
                    "avgust", // august
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
                    "avg", // abb august
                    "sep", // abb september
                    "okt", // abb october
                    "nov", // abb november
                    "dec", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "Nedelja", // Sunday
                    "Ponedeljek", // Monday
                    "Torek", // Tuesday
                    "Sreda", // Wednesday
                    "\u010cetrtek", // Thursday
                    "Petek", // Friday
                    "Sobota" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "Ned", // abb Sunday
                    "Pon", // abb Monday
                    "Tor", // abb Tuesday
                    "Sre", // abb Wednesday
                    "\u010cet", // abb Thursday
                    "Pet", // abb Friday
                    "Sob" // abb Saturday
                }
            },
            { "Eras",
                new String[] { // era strings
                    "pr.n.\u0161.",
                    "po Kr."
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
                    "H:mm:ss z", // full time pattern
                    "H:mm:ss z", // long time pattern
                    "H:mm:ss", // medium time pattern
                    "H:mm", // short time pattern
                    "EEEE, d MMMM yyyy", // full date pattern
                    "EEEE, d MMMM yyyy", // long date pattern
                    "d.M.yyyy", // medium date pattern
                    "d.M.y", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CollationElements",
                /* for sl_SI, default sorting except for the following: */

                /* add d<stroke> between d and e. */
                /* add l<stroke> between l and m. */
                /* add nj "ligature" between n and o. */
                /* add z<abovedot> after z.       */
                "& C < c\u030c , C\u030c "           // C < c-caron
                + "< c\u0301 , C\u0301 "             // c-acute
                + "& D < \u01f3 , \u01f2 , \u01f1 "  // dz
                + "< \u01c6 , \u01c5 , \u01c4 "      // dz-caron
                + "< \u0111 , \u0110 "               // d-stroke
                + "& L < \u0142 , \u0141 "           // l < l-stroke
                + "& N < nj , nJ , Nj , NJ "         // ligature updated
                + "& S < s\u030c , S\u030c "         // s < s-caron
                + "< s\u0301, S\u0301 "              // s-acute
                + "& Z < z\u030c , Z\u030c "         // z < z-caron
                + "< z\u0301 , Z\u0301 "             // z-acute
                + "< z\u0307 , Z\u0307 "             // z-dot-above
            }
        };
    }
}
