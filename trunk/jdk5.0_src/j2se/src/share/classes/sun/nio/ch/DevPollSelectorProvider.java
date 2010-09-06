/*
 * @(#)DevPollSelectorProvider.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;

public class DevPollSelectorProvider
    extends SelectorProviderImpl
{
    public AbstractSelector openSelector() throws IOException {
	return new DevPollSelectorImpl(this);
    }

    public Channel inheritedChannel() throws IOException {
	return InheritedChannel.getChannel();
    }
}
