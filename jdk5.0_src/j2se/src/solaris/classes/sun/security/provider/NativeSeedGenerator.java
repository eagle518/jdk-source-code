/*
 * @(#)NativeSeedGenerator.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider;

import java.io.IOException;

/**
 * Native seed generator for Unix systems. Inherit everything from
 * URLSeedGenerator.
 *
 * @version 1.3, 12/19/03
 */
class NativeSeedGenerator extends SeedGenerator.URLSeedGenerator {

    NativeSeedGenerator() throws IOException {
	super();
    }

}
