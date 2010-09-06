/*
 * @(#)NameService.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.spi.nameservice;

import java.net.UnknownHostException;

public interface NameService {
    public byte[][] lookupAllHostAddr(String host) throws UnknownHostException;
    public String getHostByAddr(byte[] addr) throws UnknownHostException;
}
