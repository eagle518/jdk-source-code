/*
 * @(#)Applet2Status.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.applet.*;

/** A class representing a synchronous snapshot of an applet's status
    -- the applet, if created, and the error state, if any. This
    mirrors some methods in the Applet2Manager and is designed to
    avoid race conditions when querying this state and deciding to add
    an Applet2Listener in response. */

public class Applet2Status {
    private Applet    applet;
    private boolean   errorOccurred;
    private String    errorMessage;
    private Throwable errorException;

    public Applet2Status(Applet    applet,
                         boolean   errorOccurred,
                         String    errorMessage,
                         Throwable errorException) {
        this.applet = applet;
        this.errorOccurred = errorOccurred;
        this.errorMessage = errorMessage;
        this.errorException = errorException;
    }

    public Applet    getApplet()         { return applet;         }
    public boolean   isInErrorState()    { return errorOccurred;  }
    public String    getErrorMessage()   { return errorMessage;   }
    public Throwable getErrorException() { return errorException; }
}
