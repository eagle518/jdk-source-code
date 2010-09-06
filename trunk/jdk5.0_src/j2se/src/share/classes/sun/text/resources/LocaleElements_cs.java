/*
 * @(#)LocaleElements_cs.java	1.22 03/12/19
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

public class LocaleElements_cs extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "cs", "\u010de\u0161tina" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "CZ", "\u010cesk\u00e1 republika" }
                }
            },
            { "MonthNames",
                new String[] {
                    "leden", // january
                    "\u00fanor", // february
                    "b\u0159ezen", // march
                    "duben", // april
                    "kv\u011bten", // may
                    "\u010derven", // june
                    "\u010dervenec", // july
                    "srpen", // august
                    "z\u00e1\u0159\u00ed", // september
                    "\u0159\u00edjen", // october
                    "listopad", // november
                    "prosinec", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "I", // abb january
                    "II", // abb february
                    "III", // abb march
                    "IV", // abb april
                    "V", // abb may
                    "VI", // abb june
                    "VII", // abb july
                    "VIII", // abb august
                    "IX", // abb september
                    "X", // abb october
                    "XI", // abb november
                    "XII", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "Ned\u011ble", // Sunday
                    "Pond\u011bl\u00ed", // Monday
                    "\u00dater\u00fd", // Tuesday
                    "St\u0159eda", // Wednesday
                    "\u010ctvrtek", // Thursday
                    "P\u00e1tek", // Friday
                    "Sobota" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "Ne", // abb Sunday
                    "Po", // abb Monday
                    "\u00dat", // abb Tuesday
                    "St", // abb Wednesday
                    "\u010ct", // abb Thursday
                    "P\u00e1", // abb Friday
                    "So" // abb Saturday
                }
            },
            { "AmPmMarkers",
                new String[] {
                    "dop.", // am marker
                    "odp." // pm marker
                }
            },
            { "Eras",
                new String[] { // era strings
                    "p\u0159.Kr.",
                    "po Kr."
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
                    "EEEE, d. MMMM yyyy", // full date pattern
                    "d. MMMM yyyy", // long date pattern
                    "d.M.yyyy", // medium date pattern
                    "d.M.yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "DateTimeElements",
                new String[] {
                    "2", // first day of week
                    "1" // min days in first week
                }
            },
            { "CollationElements",
                /* for CS_CZ, default sorting except for the following: */
                /* add d<stroke> between d and e. */
                /* add ch "ligature" between h and i */
                /* add l<stroke> between l and m. */
                /* add z<abovedot> after z.       */
                "& \u0361 ; \u0308 = \u030d"  // & inv-breve ; umlaut = vt-line-above
                + "& C < c\u030c , C\u030c"  // c < c-caron
                + "& H < ch , cH , Ch , CH " // h < ch ligatures
                + "& R < r\u030c , R\u030c"  // r < r-caron
                + "& S < s\u030c , S\u030c"  // s < s-caron
                + "& Z < z\u030c , Z\u030c"  // z < z-caron
            }
        };
    }
}
