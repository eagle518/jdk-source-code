/*
 * @(#)SourcePositionImpl.java	1.3 04/07/13
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.util;


import java.io.File;

import com.sun.mirror.util.SourcePosition;
import com.sun.tools.javac.util.Name;
import com.sun.tools.javac.util.Position;


/**
 * Implementation of SourcePosition
 */

public class SourcePositionImpl implements SourcePosition {

    private Name sourcefile;
    private int pos;		// file position, in javac's internal format


    public SourcePositionImpl(Name sourcefile, int pos) {
	this.sourcefile = sourcefile;
	this.pos = pos;
	assert sourcefile != null;
    }

    public int getJavacPosition() {
	return pos;
    }

    public Name getName() {
	return sourcefile;
    }

    /**
     * Returns a string representation of this position in the
     * form "sourcefile:line", or "sourcefile" if no line number is available.
     */
    public String toString() {
	int ln = line();
	return (ln == Position.NOPOS)
		? sourcefile.toString()
		: sourcefile + ":" + ln;
    }

    /**
     * {@inheritDoc}
     */
    public File file() {
	return new File(sourcefile.toString());
    }

    /**
     * {@inheritDoc}
     */
    public int line() {
	return Position.line(pos);
    }

    /**
     * {@inheritDoc}
     */
    public int column() {
	return Position.column(pos);
    }
}
