/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)LocaleElements_th_TH_TH.java	1.4 03/12/19
 */

package sun.text.resources;

import java.util.ListResourceBundle;

/*
 * This class implements the th_TH_TH requirement that native Thai
 * digits be used in the corresponding locale.
 * 
 * @author John O'Conner
 * @since 1.4.1
 *
 */

public class LocaleElements_th_TH_TH extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {

            { "NumberElements",
                new String[] {
                    ".", // decimal separator
                    ",", // group (thousands) separator
                    ";", // list separator
                    "%", // percent sign
                    "\u0E50", // native 0 digit
                    "#", // pattern digit
                    "-", // minus sign
                    "E", // exponential
                    "\u2030", // per mille
                    "\u221e", // infinity
                    "\ufffd" // NaN
                }
            },
	    
        };
    }
}
