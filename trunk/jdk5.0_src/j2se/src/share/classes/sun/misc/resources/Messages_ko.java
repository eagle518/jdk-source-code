/*
 * @(#)Messages_ko.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc.resources;

/**
 * <p> This class represents the <code>ResourceBundle</code>
 * for sun.misc.
 * 
 * @author Michael Colburn
 * @version 1.00, 02/08/01
 */

public class Messages_ko extends java.util.ListResourceBundle {

    /**
     * Returns the contents of this <code>ResourceBundle</code>.     
     * <p>   
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
        
    private static final Object[][] contents = {
	{ "optpkg.versionerror", "\uc624\ub958: {0} JAR \ud30c\uc77c\uc5d0 \uc798\ubabb\ub41c \ubc84\uc804 \ud615\uc2dd\uc774 \uc0ac\uc6a9\ub418\uc5c8\uc2b5\ub2c8\ub2e4. \uc124\uba85\uc11c\ub97c \ucc38\uc870\ud558\uc5ec \uc9c0\uc6d0\ub418\ub294 \ubc84\uc804 \ud615\uc2dd\uc744 \ud655\uc778\ud558\uc2ed\uc2dc\uc624." },
	{ "optpkg.attributeerror", "\uc624\ub958: \ud544\uc694\ud55c {0} JAR \ud45c\uc2dc \uc18d\uc131\uc774 {1} JAR \ud30c\uc77c\uc5d0 \uc124\uc815\ub418\uc5b4 \uc788\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." },
	{ "optpkg.attributeserror", "\uc624\ub958: \ud544\uc694\ud55c JAR \ud45c\uc2dc \uc18d\uc131 \uc77c\ubd80\uac00 {0} JAR \ud30c\uc77c\uc5d0 \uc124\uc815\ub418\uc5b4 \uc788\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4." }
    };
    
}
