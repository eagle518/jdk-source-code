/*
 * @(#)AppletID.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

/** Identifies a running applet. Note that we mainly use this for
    error checking because there are no "invalid" int values. */

public class AppletID {
    private int id;
    
    public AppletID(int id) {
        this.id = id;
    }

    public int getID() {
        return id;
    }

    public boolean equals(Object o) {
        if ((o == null) || (getClass() != o.getClass()))
            return false;

        return (id == ((AppletID) o).id);
    }

    public int hashCode() {
        return id;
    }

    public String toString() {
        return "[AppletID " + id + "]";
    }
}
