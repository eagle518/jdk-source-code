/*
 * @(#)x_xbitmap.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.content.image;

import java.net.*;
import sun.awt.image.*;
import java.awt.Image;
import java.awt.Toolkit;

public class x_xbitmap extends ContentHandler {
    public Object getContent(URLConnection urlc) throws java.io.IOException {
	return new URLImageSource(urlc);
    }

    public Object getContent(URLConnection urlc, Class[] classes) throws java.io.IOException {
	for (int i = 0; i < classes.length; i++) {
	  if (classes[i].isAssignableFrom(URLImageSource.class)) {
		return new URLImageSource(urlc);
	  }
	  if (classes[i].isAssignableFrom(Image.class)) {
	    Toolkit tk = Toolkit.getDefaultToolkit();
	    return tk.createImage(new URLImageSource(urlc));
	  }
	}
	return null;
    }
}
