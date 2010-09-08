/*
 * @(#)FontKey1.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.Font;

/* Used by printing to locate a device scaled font in a cache.
   Since we need to create these fonts on the fly, a cache
   avoids creating excessive garbage.
   Also Fonts are finalizable. That really should be fixed but won't
   matter here any more if we eliminate the creation of most of them.
 */ 

class FontKey1 {

    private Font font;
    private int angle;
    private float scaledSize;
    private float averageWidth;
    private int hash;

    FontKey1() {
    }
    
    void init(Font font, int angle, float scaledSize, float width) {
	this.font = font;
	this.angle = angle;
	this.scaledSize = scaledSize;
	this.averageWidth = width;
	this.hash = font.hashCode() + angle +
	    (int)scaledSize + (int)averageWidth;
    }

    FontKey1 copy() {
	FontKey1 key = new FontKey1();
	key.init(font, angle, scaledSize, averageWidth);
	return key;
    }

    public boolean equals(Object o) {
	try {
	    FontKey1 key = (FontKey1)o;
	    if (key == null) {
		return false;
	    }
	    return
		font.equals(key.font) &&
		angle == key.angle &&
		scaledSize == key.scaledSize &&
		averageWidth == key.averageWidth;
	} catch (ClassCastException e) {
	return false;
	}
    }

    public int hashCode() {
	return hash;
    }
}
