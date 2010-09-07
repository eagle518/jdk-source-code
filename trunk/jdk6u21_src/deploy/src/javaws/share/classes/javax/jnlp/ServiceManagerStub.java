/*
 * @(#)ServiceManagerStub.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

/**
 * A <code>ServiceManagerStub</code> object implements the particular
 * lookup of JNLP services by the JNLP Client. The object
 * is registered with the <code>ServiceManager</code> using the
 * <code>{@link ServiceManager#setServiceManagerStub}</code> method.
 *
 * @since 1.0
 *
 * @see     javax.jnlp.ServiceManager
 */
public interface ServiceManagerStub {
   
    /** See description for {@link ServiceManager#lookup} */
    public Object lookup(String name) throws UnavailableServiceException;
    
    /** See description for {@link ServiceManager#getServiceNames} */
    public String[] getServiceNames();
}
