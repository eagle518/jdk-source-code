/*
 * @(#)LocaleElements_en_IN.java	1.9 03/12/19
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (c) 1999 International Business Machines.
 * All Rights Reserved.
 *
 */

package sun.text.resources;

import java.util.ListResourceBundle;


/**
 * The locale elements for English in India.
 *                                                                 
 */
public class LocaleElements_en_IN extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "en", "English" },
                    { "hi", "Hindi" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "IN", "India" },
		    { "GB", "United Kingdom"},
                    { "US", "United States" }
                }
            },
            { "NumberElements", 
                new String[] {
                    ".", // decimal separator
                    ",", // group (thousands) separator
                    ";", // list separator
                    "%", // percent sign
		    "\u0030", // native 0 digit
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
                    "h:mm:ss a z", // full time pattern
                    "h:mm:ss a z", // long time pattern
                    "h:mm:ss a", // medium time pattern
                    "h:mm a", // short time pattern
                    "EEEE, d MMMM, yyyy", // full date pattern
                    "d MMMM, yyyy", // long date pattern
                    "d MMM, yyyy", // medium date pattern
                    "d/M/yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "CurrencySymbols",
                new String[][] {
                   {"INR", "Rs."}
                }
            },
        };
    }
}
