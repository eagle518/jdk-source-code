/*
 * @(#)WJcovUtil.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * This class is here to provide a hook to call into the Windows
 * Native code to dump all the jcov data
 *
 * @version 1.1
 * @author
 *
 */

public class WJcovUtil
{
    public static native boolean dumpJcovData();
}
