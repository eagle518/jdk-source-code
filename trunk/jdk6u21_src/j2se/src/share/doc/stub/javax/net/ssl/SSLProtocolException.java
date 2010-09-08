/*
 * @(#)SSLProtocolException.java	1.9 10/03/23
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

/** 
 * Reports an error in the operation of the SSL protocol.  Normally
 * this indicates a flaw in one of the protocol implementations.
 *
 * @since 1.4
 * @version 1.11
 * @author David Brownell
 */
public class SSLProtocolException extends SSLException
{

    /** 
     * Constructs an exception reporting an SSL protocol error
     * detected by an SSL subsystem.
     *
     * @param reason describes the problem.
     */
    public SSLProtocolException(String reason) { }
}
