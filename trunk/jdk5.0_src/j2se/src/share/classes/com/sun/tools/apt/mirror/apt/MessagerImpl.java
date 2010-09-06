/*
 * @(#)MessagerImpl.java	1.3 04/07/13
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.apt;

import com.sun.mirror.apt.Messager;
import com.sun.tools.apt.mirror.util.SourcePositionImpl;
import com.sun.mirror.util.SourcePosition;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Name;
import com.sun.tools.javac.util.Position;
import com.sun.tools.apt.util.Bark;

/**
 * Implementation of Messager.
 */

public class MessagerImpl implements Messager {
    private final Bark bark;

    private static final Context.Key<MessagerImpl> messagerKey =
	    new Context.Key<MessagerImpl>();

    public static MessagerImpl instance(Context context) {
	MessagerImpl instance = context.get(messagerKey);
	if (instance == null) {
	    instance = new MessagerImpl(context);
	}
	return instance;
    }

    private MessagerImpl(Context context) {
	context.put(messagerKey, this);
	bark = Bark.instance(context);
    }


    /**
     * {@inheritDoc}
     */
    public void printError(String msg) {
	bark.error(Position.NOPOS, "Messager", msg);
    }

    /**
     * {@inheritDoc}
     */
    public void printError(SourcePosition pos, String msg) {
	if (pos instanceof SourcePositionImpl) {
	    SourcePositionImpl posImpl = (SourcePositionImpl) pos;
	    Name prev = bark.useSource(posImpl.getName());
	    bark.error(posImpl.getJavacPosition(), "Messager", msg);
	    bark.useSource(prev);
	} else
	    printError(msg);
    }

    /**
     * {@inheritDoc}
     */
    public void printWarning(String msg) {
	bark.warning(Position.NOPOS,  "Messager", msg);
    }

    /**
     * {@inheritDoc}
     */
    public void printWarning(SourcePosition pos, String msg) {
	if (pos instanceof SourcePositionImpl) {
	    SourcePositionImpl posImpl = (SourcePositionImpl) pos;
	    Name prev = bark.useSource(posImpl.getName());
	    bark.warning(posImpl.getJavacPosition(), "Messager", msg);
	    bark.useSource(prev);
	} else
	    printWarning(msg);
    }

    /**
     * {@inheritDoc}
     */
    public void printNotice(String msg) {
	bark.note("Messager", msg);
    }

    /**
     * {@inheritDoc}
     */
    public void printNotice(SourcePosition pos, String msg) {
	if (pos instanceof SourcePositionImpl) {
	    SourcePositionImpl posImpl = (SourcePositionImpl) pos;
	    Name prev = bark.useSource(posImpl.getName());
	    bark.note(posImpl.getJavacPosition(), "Messager", msg);	
	    bark.useSource(prev);
	} else
	    printNotice(msg);
    }
}
