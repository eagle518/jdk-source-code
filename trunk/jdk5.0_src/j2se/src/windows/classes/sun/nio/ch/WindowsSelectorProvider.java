/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)WindowsSelectorProvider.java	1.4 03/12/19
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.spi.AbstractSelector;

/*
 * SelectorProvider for sun.nio.ch.WindowsSelectorImpl.
 *
 * @author Konstantin Kladko
 * @version 1.4, 12/19/03
 * @since 1.4
 */

public class WindowsSelectorProvider extends SelectorProviderImpl {
    
    public AbstractSelector openSelector() throws IOException {
        return new WindowsSelectorImpl(this);
    }
}
