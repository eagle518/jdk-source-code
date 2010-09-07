/*
 * @(#)HandshakeCompletedListener.java	1.8 04/02/16
 *
 * Copyright (c) 2004 Sun Microsystems, Inc. All Rights Reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.net.ssl;

import java.util.EventListener;

/** 
 * This interface is implemented by any class which wants to receive
 * notifications about the completion of an SSL protocol handshake
 * on a given SSL connection.
 *
 * <P> When an SSL handshake completes, new security parameters will
 * have been established.  Those parameters always include the security
 * keys used to protect messages.  They may also include parameters
 * associated with a new <em>session</em> such as authenticated
 * peer identity and a new SSL cipher suite.
 *
 * @since 1.4
 * @version 1.8
 * @author David Brownell
 */
public interface HandshakeCompletedListener extends EventListener
{

    /** 
     * This method is invoked on registered objects
     * when a SSL handshake is completed.
     *
     * @param event the event identifying when the SSL Handshake
     *		completed on a given SSL connection
     */
    public void handshakeCompleted(HandshakeCompletedEvent event);
}
