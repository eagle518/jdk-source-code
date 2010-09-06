/*
 * @(#)LocaleElements_en_GB.java	1.21 04/01/20
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

public class LocaleElements_en_GB extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "en", "English" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "US", "United States" },
                    { "GB", "United Kingdom" },
                    { "CA", "Canada" },
                    { "IE", "Ireland" },
                    { "AU", "Australia" },
                    { "NZ", "New Zealand" }
                }
            },
            { "CurrencySymbols",
                new String[][] {
                   {"EUR", "\u20AC"},
                   {"GBP", "\u00A3"}
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "HH:mm:ss 'o''clock' z", // full time pattern
                    "HH:mm:ss z", // long time pattern
                    "HH:mm:ss", // medium time pattern
                    "HH:mm", // short time pattern
                    "dd MMMM yyyy", // full date pattern
                    "dd MMMM yyyy", // long date pattern
                    "dd-MMM-yyyy", // medium date pattern
                    "dd/MM/yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "DateTimeElements",
                new String[] {
                    "2", // first day of week
                    "4" // min days in first week
                }
            }
        };
    }
}
