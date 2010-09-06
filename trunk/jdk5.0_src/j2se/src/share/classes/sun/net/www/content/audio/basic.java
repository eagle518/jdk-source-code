/*
 * @(#)basic.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Basic .au and .snd audio handler.
 * @version 1.7, 12/19/03
 * @author  Jeff Nisewanger
 */
package sun.net.www.content.audio;

import java.net.*;
import java.io.IOException;
import sun.applet.AppletAudioClip;

/**
 * Returns an AppletAudioClip object.
 * This provides backwards compatibility with the behavior
 * of ClassLoader.getResource().getContent() on JDK1.1.
 */
public class basic extends ContentHandler {
    public Object getContent(URLConnection uc) throws IOException {
	return new AppletAudioClip(uc);
    }
}
