/*
 * @(#)LocaleElements_en_CA.java	1.20 03/12/19
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

public class LocaleElements_en_CA extends ListResourceBundle {
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
                   {"CAD", "$"},
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "h:mm:ss 'o''clock' a z", // full time pattern
                    "h:mm:ss z a", // long time pattern
                    "h:mm:ss a", // medium time pattern
                    "h:mm a", // short time pattern
                    "EEEE, MMMM d, yyyy", // full date pattern
                    "MMMM d, yyyy", // long date pattern
                    "d-MMM-yyyy", // medium date pattern
                    "dd/MM/yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            }
        };
    }
}
