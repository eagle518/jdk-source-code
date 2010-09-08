/*
 * @(#)EPollSelectorProvider.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;

public class EPollSelectorProvider
    extends SelectorProviderImpl
{
    public AbstractSelector openSelector() throws IOException {
	return new EPollSelectorImpl(this);
    }

    public Channel inheritedChannel() throws IOException {
	return InheritedChannel.getChannel();
    }
}
