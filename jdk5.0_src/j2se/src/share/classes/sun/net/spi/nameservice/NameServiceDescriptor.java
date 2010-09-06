/*
 * @(#)NameServiceDescriptor.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.spi.nameservice;

public interface NameServiceDescriptor {
    /**
     * Create a new instance of the corresponding name service.
     */
    public NameService createNameService () throws Exception ;

    /**
     * Returns this service provider's name
     *
     */
    public String getProviderName();

    /**
     * Returns this name service type
     * "dns" "nis" etc
     */
    public String getType();
}
