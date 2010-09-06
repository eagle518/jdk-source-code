/*
 * @(#)LocaleElements_hr.java	1.17 03/12/19
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

public class LocaleElements_hr extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "hr", "hrvatski" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "HR", "Hrvatska" }
                }
            },
            { "MonthNames",
                new String[] {
                    "sije\u010danj", // january
                    "velja\u010da", // february
                    "o\u017eujak", // march
                    "travanj", // april
                    "svibanj", // may
                    "lipanj", // june
                    "srpanj", // july
                    "kolovoz", // august
                    "rujan", // september
                    "listopad", // october
                    "studeni", // november
                    "prosinac", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "sij", // abb january
                    "vel", // abb february
                    "o\u017eu", // abb march
                    "tra", // abb april
                    "svi", // abb may
                    "lip", // abb june
                    "srp", // abb july
                    "kol", // abb august
                    "ruj", // abb september
                    "lis", // abb october
                    "stu", // abb november
                    "pro", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "nedjelja", // Sunday
                    "ponedjeljak", // Monday
                    "utorak", // Tuesday
                    "srijeda", // Wednesday
                    "\u010detvrtak", // Thursday
                    "petak", // Friday
                    "subota" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "ned", // abb Sunday
                    "pon", // abb Monday
                    "uto", // abb Tuesday
                    "sri", // abb Wednesday
                    "\u010det", // abb Thursday
                    "pet", // abb Friday
                    "sub" // abb Saturday
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
                    "yyyy. MMMM dd", // full date pattern
                    "yyyy. MMMM dd", // long date pattern
                    "yyyy.MM.dd", // medium date pattern
                    "yyyy.MM.dd", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CollationElements",
                /* for HR_HR, default sorting except for the following: */

                /* add dz "ligature" between d and d<stroke>. */
                /* add d<stroke> between d and e. */
                /* add lj "ligature" between l and l<stroke>. */
                /* add l<stroke> between l and m. */
                /* add nj "ligature" between n and o. */
                /* add z<abovedot> after z.       */
                "& \u200f = \u030c "
                + "& \u0306 = \u030d "
                + "& C < c\u030c , C\u030c "  // C < c-caron
                + "< c\u0301 , C\u0301 "      // c-acute
                + "& D < \u01f3 , \u01f2 , \u01f1 "  // dz
                + "< dz , dZ , Dz , DZ "      // dz ligature
                + "< \u01c6 , \u01c5 , \u01c4 "  // dz-caron
                + "< \u0111 , \u0110 "           // d-stroke
                + "& L < lj , lJ , Lj , LJ " // l < lj ligature
                + "& N < nj , nJ , Nj , NJ " // n < nj ligature
                + "& S < s\u030c , S\u030c "  // s < s-caron
                + "& Z < z\u030c , Z\u030c "  // z < z-caron
            }
        };
    }
}
