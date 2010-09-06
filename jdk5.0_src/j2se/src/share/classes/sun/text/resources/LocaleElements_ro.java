/*
 * @(#)LocaleElements_ro.java	1.17 03/12/19
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

public class LocaleElements_ro extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "ro", "rom\u00e2n\u0103" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "RO", "Rom\u00e2nia" }
                }
            },
            { "MonthNames",
                new String[] {
                    "ianuarie", // january
                    "februarie", // february
                    "martie", // march
                    "aprilie", // april
                    "mai", // may
                    "iunie", // june
                    "iulie", // july
                    "august", // august
                    "septembrie", // september
                    "octombrie", // october
                    "noiembrie", // november
                    "decembrie", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "Ian", // abb january
                    "Feb", // abb february
                    "Mar", // abb march
                    "Apr", // abb april
                    "Mai", // abb may
                    "Iun", // abb june
                    "Iul", // abb july
                    "Aug", // abb august
                    "Sep", // abb september
                    "Oct", // abb october
                    "Nov", // abb november
                    "Dec", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "duminic\u0103", // Sunday
                    "luni", // Monday
                    "mar\u0163i", // Tuesday
                    "miercuri", // Wednesday
                    "joi", // Thursday
                    "vineri", // Friday
                    "s\u00eemb\u0103t\u0103" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "D", // abb Sunday
                    "L", // abb Monday
                    "Ma", // abb Tuesday
                    "Mi", // abb Wednesday
                    "J", // abb Thursday
                    "V", // abb Friday
                    "S" // abb Saturday
                }
            },
            { "Eras",
                new String[] { // era strings
                    "d.C.",
                    "\u00ee.d.C."
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
                    "dd MMMM yyyy", // full date pattern
                    "dd MMMM yyyy", // long date pattern
                    "dd.MM.yyyy", // medium date pattern
                    "dd.MM.yyyy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CollationElements",
                "& A < a\u0306 , A\u0306 "       // a < a-breve
                + "& D < \u0111, \u0110 "        // d < d-stroke
                + "& I < i\u0302 , I\u0302 "     // i < i-circumflex
                + "& S < s\u0327 , S\u0327 "     // s < s-cedilla
                + "& \u00de < t\u0327 , T\u0327" // thorn < t-cedilla
                + "& Z < z\u0307 , Z\u0307 "     // tal : ezh-tail < z-dot-above
            }
        };
    }
}
