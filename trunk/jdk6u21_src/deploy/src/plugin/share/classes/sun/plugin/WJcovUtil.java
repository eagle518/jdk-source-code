/*
 * @(#)WJcovUtil.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
