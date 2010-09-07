/*
 * @(#)ResultID.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

/** A token used to identify a result from one particular JavaScript
    -> Java call. */
public class ResultID {
    private int id;
    
    public ResultID(int id) {
        this.id = id;
    }

    public int getID() {
        return id;
    }

    public boolean equals(Object o) {
        if ((o == null) || (getClass() != o.getClass()))
            return false;

        return (id == ((ResultID) o).id);
    }

    public int hashCode() {
        return id;
    }

    public String toString() {
        return "[ResultID " + id + "]";
    }
}

