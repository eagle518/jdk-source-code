/*
 * @(#)AscendingMonitorComparator.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.util.*;
import sun.jvmstat.monitor.*;

/**
 * Class to compare two Monitor objects by name in ascending order.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
class AscendingMonitorComparator implements Comparator {
    public int compare(Object o1, Object o2) {
        String name1 = ((Monitor)o1).getName();
        String name2 = ((Monitor)o2).getName();
        return name1.compareTo(name2);
    }
}
