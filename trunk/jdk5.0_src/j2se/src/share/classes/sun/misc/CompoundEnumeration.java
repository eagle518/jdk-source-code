/*
 * @(#)CompoundEnumeration.java	1.8 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.util.Enumeration;
import java.util.NoSuchElementException;

/*
 * A useful utility class that will enumerate over an array of
 * enumerations.
 */
public class CompoundEnumeration<E> implements Enumeration<E> {
    private Enumeration[] enums;
    private int index = 0;

    public CompoundEnumeration(Enumeration[] enums) {
	this.enums = enums;
    }

    private boolean next() {
	while (index < enums.length) {
	    if (enums[index] != null && enums[index].hasMoreElements()) {
		return true;
	    }
	    index++;
	}
	return false;
    }

    public boolean hasMoreElements() {
	return next();
    }

    public E nextElement() {
	if (!next()) {
	    throw new NoSuchElementException();
	}
	return (E)enums[index].nextElement();
    }
}
