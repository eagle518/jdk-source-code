/*
 * @(#)SelectionKeyImpl.java	1.15 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;


/**
 * An implementation of SelectionKey for Solaris.
 */

class SelectionKeyImpl
    extends AbstractSelectionKey
{

    final SelChImpl channel;				// package-private
    final SelectorImpl selector;			// package-private

    // Index for a pollfd array in Selector that this key is registered with
    private int index;

    private volatile int interestOps;
    private int readyOps;

    SelectionKeyImpl(SelChImpl ch, SelectorImpl sel) {
	channel = ch;
	selector = sel;
    }

    public SelectableChannel channel() {
	return (SelectableChannel)channel;
    }

    public Selector selector() {
	return selector;
    }

    int getIndex() {					// package-private
        return index;
    }

    void setIndex(int i) {				// package-private
        index = i;
    }

    private void ensureValid() {
        if (!isValid())
            throw new CancelledKeyException();
    }

    public int interestOps() {
        ensureValid();
        return interestOps;
    }

    public SelectionKey interestOps(int ops) {
        ensureValid();
        return nioInterestOps(ops);
    }

    public int readyOps() {
	ensureValid();
        return readyOps;
    }

    // The nio versions of these operations do not care if a key
    // has been invalidated. They are for internal use by nio code.

    void nioReadyOps(int ops) {			// package-private
        readyOps = ops;
    }

    int nioReadyOps() {			        // package-private
        return readyOps;
    }

    SelectionKey nioInterestOps(int ops) {	// package-private
        if ((ops & ~channel().validOps()) != 0)
            throw new IllegalArgumentException();
        channel.translateAndSetInterestOps(ops, this);
        interestOps = ops;
	return this;
    }

    int nioInterestOps() {			 // package-private
        return interestOps;
    }

}
