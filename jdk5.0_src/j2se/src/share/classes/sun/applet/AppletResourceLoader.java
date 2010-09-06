/*
 * @(#)AppletResourceLoader.java	1.45 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

import java.net.URL;
import java.awt.Image;
import sun.misc.Ref;

/**
 * Part of this class still remains only to support legacy, 100%-impure
 * applications such as HotJava 1.0.1.
 */
public class AppletResourceLoader {
    public static Image getImage(URL url) {
	return AppletViewer.getCachedImage(url);
    }

    public static Ref getImageRef(URL url) {
	return AppletViewer.getCachedImageRef(url);
    }

    public static void flushImages() {
	AppletViewer.flushImageCache();
    }
}
