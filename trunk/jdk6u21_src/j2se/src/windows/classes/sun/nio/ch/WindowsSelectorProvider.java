/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)WindowsSelectorProvider.java	1.6 10/03/23
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.spi.AbstractSelector;

/*
 * SelectorProvider for sun.nio.ch.WindowsSelectorImpl.
 *
 * @author Konstantin Kladko
 * @version 1.6, 03/23/10
 * @since 1.4
 */

public class WindowsSelectorProvider extends SelectorProviderImpl {
    
    public AbstractSelector openSelector() throws IOException {
        return new WindowsSelectorImpl(this);
    }
}
