/*
 * @(#)FontKey2.java	1.3 10/03/23
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

class FontKey2 {

    private String name;
    private int style;
    private int size;
    private int angle;
    private float averageWidth;
    private int hash;

    FontKey2() {
    }
    
    void init(String name, int style, int size, int angle, float width) {
	this.name = name;
	this.style = style;
	this.size = size;
	this.angle = angle;
	this.averageWidth = width;
	this.hash = name.hashCode() + style + size + angle + (int)averageWidth;
    }

    FontKey2 copy() {
	FontKey2 key = new FontKey2();
	key.init(name, style, size, angle, averageWidth);
	return key;
    }

    public boolean equals(Object o) {
	try {
	    FontKey2 key = (FontKey2)o;
	    if (key == null) {
		return false;
	    }
	    return
		name.equals(key.name) &&
		style == key.style &&
		size == key.size &&
		angle == key.angle &&
		averageWidth == key.averageWidth;
	} catch (ClassCastException e) {
	return false;
	}
    }

    public int hashCode() {
	return hash;
    }
}
