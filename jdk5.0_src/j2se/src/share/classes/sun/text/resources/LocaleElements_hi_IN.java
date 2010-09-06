/*
 * @(#)LocaleElements_hi_IN.java	1.10 03/12/19
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

/**
 * The locale elements for Hindi.
 *                                                                 
 */
public class LocaleElements_hi_IN extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "hi", "\u0939\u093f\u0902\u0926\u0940" },
                    { "en", "\u0905\u0901\u0917\u094d\u0930\u0947\u091c\u093c\u0940" }
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "IN", "\u092d\u093e\u0930\u0924" },
                    { "US", "\u0938\u0902\u092f\u0941\u0915\u094d\u0924"
                            +" \u0930\u093e\u091c\u094d\u092f"
                            +" \u0905\u092e\u0947\u0930\u093f\u0915\u093e" }
                }
            },
            { "MonthNames",
                new String[] {
                    "\u091c\u0928\u0935\u0930\u0940", // january
                    "\u092b\u093c\u0930\u0935\u0930\u0940", // february
                    "\u092e\u093e\u0930\u094d\u091a", // march
                    "\u0905\u092a\u094d\u0930\u0948\u0932", // april
                    "\u092e\u0908", // may
                    "\u091c\u0942\u0928", // june
                    "\u091c\u0941\u0932\u093e\u0908", // july
                    "\u0905\u0917\u0938\u094d\u0924", // august
                    "\u0938\u093f\u0924\u0902\u092c\u0930", // september
                    "\u0905\u0915\u094d\u200d\u0924\u0942\u092c\u0930", // october
                    "\u0928\u0935\u0902\u092c\u0930", // november
                    "\u0926\u093f\u0938\u0902\u092c\u0930", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",   // These are same as the long ones.
                new String[] {
                    "\u091c\u0928\u0935\u0930\u0940", // abb january
                    "\u092b\u093c\u0930\u0935\u0930\u0940", // abb february
                    "\u092e\u093e\u0930\u094d\u091a", // abb march
                    "\u0905\u092a\u094d\u0930\u0948\u0932", // abb april
                    "\u092e\u0908", // abb may
                    "\u091c\u0942\u0928", // abb june
                    "\u091c\u0941\u0932\u093e\u0908", // abb july
                    "\u0905\u0917\u0938\u094d\u0924", // abb august
                    "\u0938\u093f\u0924\u0902\u092c\u0930", // abb september
                    "\u0905\u0915\u094d\u200d\u0924\u0942\u092c\u0930", // abb october
                    "\u0928\u0935\u0902\u092c\u0930", // abb november
                    "\u0926\u093f\u0938\u0902\u092c\u0930", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "\u0930\u0935\u093f\u0935\u093e\u0930", // Sunday
                    "\u0938\u094b\u092e\u0935\u093e\u0930", // Monday
                    "\u092e\u0902\u0917\u0932\u0935\u093e\u0930", // Tuesday
                    "\u092c\u0941\u0927\u0935\u093e\u0930", // Wednesday
                    "\u0917\u0941\u0930\u0941\u0935\u093e\u0930", // Thursday
                    "\u0936\u0941\u0915\u094d\u0930\u0935\u093e\u0930", // Friday
                    "\u0936\u0928\u093f\u0935\u093e\u0930" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "\u0930\u0935\u093f", // abb Sunday
                    "\u0938\u094b\u092e", // abb Monday
                    "\u092e\u0902\u0917\u0932", // abb Tuesday
                    "\u092c\u0941\u0927", // abb Wednesday
                    "\u0917\u0941\u0930\u0941", // abb Thursday
                    "\u0936\u0941\u0915\u094d\u0930", // abb Friday
                    "\u0936\u0928\u093f" // abb Saturday
                }
            },
            { "AmPmMarkers",
                new String[] {
                    "\u092a\u0942\u0930\u094d\u0935\u093e\u0939\u094d\u0928", // am marker
                    "\u0905\u092a\u0930\u093e\u0939\u094d\u0928" // pm marker
                }
            },
            { "Eras",
                new String[] { // era strings
                    "\u0908\u0938\u093e\u092a\u0942\u0930\u094d\u0935",
                    "\u0938\u0928"
                }
            },
            { "NumberElements", 
                new String[] {
                    ".", // decimal separator
                    ",", // group (thousands) separator
                    ";", // list separator
                    "%", // percent sign
                    "\u0966", // native 0 digit
                    "#", // pattern digit
                    "-", // minus sign
                    "E", // exponential
                    "\u2030", // per mille
                    "\u221e", // infinity
                    "\ufffd" // NaN
                }
            },
            { "CurrencySymbols",
                new String[][] {
                   {"INR", "\u0930\u0942"}
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
          { "CollationElements", "" 
                    + "< \u0901 < \u0902 < \u0903 < \u0905"
                    + "< \u0906 < \u0907 < \u0908 < \u0909"
                    + "< \u090a < \u090b < \u0960 < \u090e"
                    + "< \u090f < \u090c < \u0961 < \u0910"
                    + "< \u090d < \u0912 < \u0913 < \u0914"
                    + "< \u0911 < \u0915 < \u0958 < \u0916"
                    + "< \u0959 < \u0917 < \u095a < \u0918"
                    + "< \u0919 < \u091a < \u091b < \u091c"
                    + "< \u095b < \u091d < \u091e < \u091f"
                    + "< \u0920 < \u0921 < \u095c < \u0922"
                    + "< \u095d < \u0923 < \u0924 < \u0925"
                    + "< \u0926 < \u0927 < \u0928 < \u0929"
                    + "< \u092a < \u092b < \u095e < \u092c"
                    + "< \u092d < \u092e < \u092f < \u095f"
                    + "< \u0930 < \u0931 < \u0932 < \u0933"
                    + "< \u0934 < \u0935 < \u0936 < \u0937"
                    + "< \u0938 < \u0939 < \u093e < \u093f"
                    + "< \u0940 < \u0941 < \u0942 < \u0943"
                    + "< \u0944 < \u0946 < \u0947 < \u0948"
                    + "< \u0945 < \u094a < \u094b < \u094c"
                    + "< \u0949 < \u094d < \u093c < \u093d"
                    + "< \u0950 < \u0951 < \u0952 < \u0953"
                    + "< \u0954 < \u0962 < \u0963 < \u0964"
                    + "< \u0965 < \u0966 < \u0967 < \u0968"
                    + "< \u0969 < \u096a < \u096b < \u096c"
                    + "< \u096d < \u096e < \u096f < \u0970"
            },
        };
    }
}
