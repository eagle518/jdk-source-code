/*
 * @(#)ThaiInputMethodDescriptor.java	1.3 03/12/19
 *
 * (C) Copyright IBM Corp. 2000 - All Rights Reserved
 *
 * Portions Copyright 2004 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 *
 */

package com.sun.inputmethods.internal.thaiim;

import java.awt.Image;
import java.awt.im.spi.InputMethod;
import java.awt.im.spi.InputMethodDescriptor;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

public class ThaiInputMethodDescriptor implements InputMethodDescriptor {

    static final Locale THAI = new Locale("th");

    public ThaiInputMethodDescriptor() {
    }

    /**
     * @see java.awt.im.spi.InputMethodDescriptor#getAvailableLocales
     */
    public Locale[] getAvailableLocales() {
        return new Locale[] { THAI };
    }

    /**
     * @see java.awt.im.spi.InputMethodDescriptor#hasDynamicLocaleList
     */
    public boolean hasDynamicLocaleList() {
        return false;
    }

    /**
     * @see java.awt.im.spi.InputMethodDescriptor#getInputMethodDisplayName
     */
    public synchronized String getInputMethodDisplayName(Locale inputLocale, Locale displayLanguage) {
	try {
	    ResourceBundle resources = ResourceBundle.getBundle(
		"com.sun.inputmethods.internal.thaiim.resources.DisplayNames", displayLanguage);
            return resources.getString("DisplayName.Thai");
	} catch (MissingResourceException mre) {
	    return "Thai Input Method";
	}
    }

    /**
     * @see java.awt.im.spi.InputMethodDescriptor#getInputMethodIcon
     */
    public Image getInputMethodIcon(Locale inputLocale) {
        return null;
    }

    /**
     * @see java.awt.im.spi.InputMethodDescriptor#createInputMethod
     */
    public InputMethod createInputMethod() throws Exception {
        ThaiInputMethodImpl impl = new ThaiInputMethodImpl(); 
        return new ThaiInputMethod(THAI, impl);
    }

    public String toString() {
        return getClass().getName();
    }
}

