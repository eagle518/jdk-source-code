/*
 * @(#)DateFormatZoneData_hi_IN.java	1.9 03/12/19
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (c) 1998 International Business Machines.
 * All Rights Reserved.
 *
 */

package sun.text.resources;

import java.util.ListResourceBundle;


// Indian DateFormatZoneData
//
public final class DateFormatZoneData_hi_IN extends DateFormatZoneData
{
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            {"Asia/Calcutta",
                new String[] {
		    "\u092d\u093e\u0930\u0924\u0940\u092f \u0938\u092e\u092f", "IST",
                    "\u092d\u093e\u0930\u0924\u0940\u092f \u0938\u092e\u092f", "IST"
                }
            },
            {"localPatternChars", "GyMdkHmsSEDFwWahKzZ"},
        };
    }
}

