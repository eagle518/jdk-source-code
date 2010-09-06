/*
 * @(#)LocaleElements_no_NO_NY.java	1.15 03/12/19
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

public class LocaleElements_no_NO_NY extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "no", "norsk" }
                }
            },
            { "DayNames",
                new String[] {
                    "sundag", // Sunday
                    "m\u00e5ndag", // Monday
                    "tysdag", // Tuesday
                    "onsdag", // Wednesday
                    "torsdag", // Thursday
                    "fredag", // Friday
                    "laurdag" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "su", // abb Sunday
                    "m\u00e5", // abb Monday
                    "ty", // abb Tuesday
                    "on", // abb Wednesday
                    "to", // abb Thursday
                    "fr", // abb Friday
                    "lau" // abb Saturday
                }
            }
        };
    }
}
