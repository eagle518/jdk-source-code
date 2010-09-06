/*
 * @(#)AppletIOException.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

import java.io.IOException;

/**
 * An applet IO exception.
 *
 * @version 	03/12/19
 * @author 	Koji Uno
 */
public 
class AppletIOException extends IOException {
    private String key = null;
    private Object msgobj = null;

    public AppletIOException(String key) {
        super(key);
        this.key = key;
        
    }
    public AppletIOException(String key, Object arg) {
        this(key);
        msgobj = arg;
    }

    public String getLocalizedMessage() {
        if( msgobj != null)
            return amh.getMessage(key, msgobj);
        else
            return amh.getMessage(key);
    }

    private static AppletMessageHandler amh = new AppletMessageHandler("appletioexception");

}
