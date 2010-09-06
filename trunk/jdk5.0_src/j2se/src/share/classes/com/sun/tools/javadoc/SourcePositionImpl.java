/**
 * @(#)SourcePositionImpl.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.javadoc.SourcePosition;
import com.sun.tools.javac.util.Position;
import com.sun.tools.javac.tree.Tree;
import com.sun.tools.javac.tree.Tree.TopLevel;
import java.io.File;

/**
 * A source position: filename, line number, and column number.
 *
 * @since J2SE1.4
 * @author Neal M Gafter
 */
class SourcePositionImpl implements SourcePosition {
    String filename;
    int position;

    /** The source file. Returns null if no file information is 
     *  available. */
    public File file() {
	return (filename == null) ? null : new File(filename);
    }

    /** The line in the source file. The first line is numbered 1;
     *  0 means no line number information is available. */
    public int line() {
	return Position.line(position);
    }

    /** The column in the source file. The first column is
     *  numbered 1; 0 means no column information is available.
     *  Columns count characters in the input stream; a tab
     *  advances the column number to the next 8-column tab stop.
     */
    public int column() {
	return Position.column(position);
    }

    private SourcePositionImpl(String file, int position) {
	super();
	this.filename = file;
	this.position = position;
    }

    public static SourcePosition make(String file, int pos) {
	if (file == null) return null;
	return new SourcePositionImpl(file, pos);
    }

    public String toString() {
	if (position == Position.NOPOS)
	    return filename;
	else
	    return filename + ":" + Position.line(position);
    }
}
