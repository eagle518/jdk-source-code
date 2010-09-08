/*
 * @(#)SSLSessionBindingEvent.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.net.ssl;

import java.util.EventObject;

/** 
 * This event is propagated to a SSLSessionBindingListener.
 * When a listener object is bound or unbound to an SSLSession by
 * {@link SSLSession#putValue(String, Object)}
 * or {@link SSLSession#removeValue(String)}, objects which
 * implement the SSLSessionBindingListener will be receive an
 * event of this type.  The event's <code>name</code> field is the
 * key in which the listener is being bound or unbound.
 *
 * @see SSLSession
 * @see SSLSessionBindingListener
 *
 * @since 1.4
 * @author Nathan Abramson
 * @author David Brownell
 * @version 1.15
 */
public class SSLSessionBindingEvent extends EventObject
{
    /** 
     * @serial The name to which the object is being bound or unbound
     */
    private String name;

    /** 
     * Constructs a new SSLSessionBindingEvent.
     *
     * @param session the SSLSession acting as the source of the event
     * @param name the name to which the object is being bound or unbound
     * @exception  IllegalArgumentException  if <code>session</code> is null.
     */
    public SSLSessionBindingEvent(SSLSession session, String name) { }

    /** 
     * Returns the name to which the object is being bound, or the name
     * from which the object is being unbound.
     *
     * @return the name to which the object is being bound or unbound
     */
    public String getName() {
        return null;
    }

    /** 
     * Returns the SSLSession into which the listener is being bound or
     * from which the listener is being unbound.
     *
     * @return the <code>SSLSession</code>
     */
    public SSLSession getSession() {
        return null;
    }
}
