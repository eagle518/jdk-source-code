/*
 * @(#)LocaleElements_en_ZA.java	1.13 03/12/19
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

public class LocaleElements_en_ZA extends ListResourceBundle {
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
                    { "ZA", "South Africa" }
                }
            },
            { "NumberPatterns",
                new String[] {
                    "#,##0.###;-#,##0.###", // decimal pattern
                    "\u00A4 #,##0.00;\u00A4-#,##0.00", // currency pattern
                    "#,##0%" // percent pattern
                }
            },
            { "CurrencySymbols",
                new String[][] {
                   {"ZAR", "R"}
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "hh:mm:ss a", // full time pattern
                    "hh:mm:ss", // long time pattern
                    "hh:mm:ss", // medium time pattern
                    "hh:mm", // short time pattern
                    "dd MMMM yyyy", // full date pattern
                    "dd MMMM yyyy", // long date pattern
                    "yyyy/MM/dd", // medium date pattern
                    "yy/MM/dd", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            }
        };
    }
}
