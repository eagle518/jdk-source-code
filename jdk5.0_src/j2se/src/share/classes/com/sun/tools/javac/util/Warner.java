/**
 * @(#)Warner.java	1.15 04/04/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

/**
 * An interface to support optional warnings, needed for support of
 * unchecked conversions and unchecked casts.
 *
 * <p>Nothing described in this source file is part of any supported
 * API.  If you write code that depends on this, you do so at your own
 * risk.  This code and its internal interfaces are subject to change
 * or deletion without notice.
 */
public class Warner {
    public static final Warner noWarnings = new Warner();

    private int pos = Position.NOPOS;
    public boolean warned = false;
    public boolean unchecked = false;

    public int pos() {
        return pos;
    }

    public void warnUnchecked() {
        warned = true;
        unchecked = true;
    }
    public void silentUnchecked() {
        unchecked = true;
    }

    public Warner(int pos) {
        this.pos = pos;
    }

    public Warner() {
        this(Position.NOPOS);
    }
}
