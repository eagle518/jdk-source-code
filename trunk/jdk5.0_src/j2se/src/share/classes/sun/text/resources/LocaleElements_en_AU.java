/*
 * @(#)LocaleElements_en_AU.java	1.15 03/12/19
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

public class LocaleElements_en_AU extends ListResourceBundle {
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
                   {"AUD", "$"}
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "hh:mm:ss a z", // full time pattern
                    "H:mm:ss", // long time pattern
                    "HH:mm:ss", // medium time pattern
                    "HH:mm", // short time pattern
                    "EEEE, d MMMM yyyy", // full date pattern
                    "d MMMM yyyy", // long date pattern
                    "d/MM/yyyy", // medium date pattern
                    "d/MM/yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            }
        };
    }
}
