/*
 * @(#)SSLContextSpi.java	1.16 10/03/23
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

import java.util.*;
import java.security.*;

/** 
 * This class defines the <i>Service Provider Interface</i> (<b>SPI</b>)
 * for the <code>SSLContext</code> class.
 *
 * <p> All the abstract methods in this class must be implemented by each
 * cryptographic service provider who wishes to supply the implementation
 * of a particular SSL context.
 *
 * @since 1.4
 * @see SSLContext
 * @version 1.16
 */
public abstract class SSLContextSpi
{

    public SSLContextSpi() { }

    /** 
     * Initializes this context.
     *
     * @param km the sources of authentication keys
     * @param tm the sources of peer authentication trust decisions
     * @param sr the source of randomness
     * @throws KeyManagementException if this operation fails
     * @see SSLContext#init(KeyManager [], TrustManager [], SecureRandom)
     */
    protected abstract void engineInit(KeyManager[] km, TrustManager[] tm,
        SecureRandom sr) throws KeyManagementException;

    /** 
     * Returns a <code>SocketFactory</code> object for this
     * context.
     *
     * @return the <code>SocketFactory</code> object
     * @throws IllegalStateException if the SSLContextImpl requires
     *         initialization and the <code>engineInit()</code>
     *         has not been called
     * @see javax.net.ssl.SSLContext#getSocketFactory()
     */
    protected abstract SSLSocketFactory engineGetSocketFactory();

    /** 
     * Returns a <code>ServerSocketFactory</code> object for
     * this context.
     *
     * @return the <code>ServerSocketFactory</code> object
     * @throws IllegalStateException if the SSLContextImpl requires
     *         initialization and the <code>engineInit()</code>
     *         has not been called
     * @see javax.net.ssl.SSLContext#getServerSocketFactory()
     */
    protected abstract SSLServerSocketFactory engineGetServerSocketFactory();

    /** 
     * Creates a new <code>SSLEngine</code> using this context.
     * <P>
     * Applications using this factory method are providing no hints
     * for an internal session reuse strategy. If hints are desired,
     * {@link #engineCreateSSLEngine(String, int)} should be used
     * instead.
     * <P>
     * Some cipher suites (such as Kerberos) require remote hostname
     * information, in which case this factory method should not be used.
     *
     * @return  the <code>SSLEngine</code> Object
     * @throws IllegalStateException if the SSLContextImpl requires
     *         initialization and the <code>engineInit()</code>
     *         has not been called
     *
     * @see     SSLContext#createSSLEngine()
     *
     * @since   1.5
     */
    protected abstract SSLEngine engineCreateSSLEngine();

    /** 
     * Creates a <code>SSLEngine</code> using this context.
     * <P>
     * Applications using this factory method are providing hints
     * for an internal session reuse strategy.
     * <P>
     * Some cipher suites (such as Kerberos) require remote hostname
     * information, in which case peerHost needs to be specified.
     *
     * @param host the non-authoritative name of the host
     * @param port the non-authoritative port
     * @return  the <code>SSLEngine</code> Object
     * @throws IllegalStateException if the SSLContextImpl requires
     *         initialization and the <code>engineInit()</code>
     *         has not been called
     *
     * @see     SSLContext#createSSLEngine(String, int)
     *
     * @since   1.5
     */
    protected abstract SSLEngine engineCreateSSLEngine(String host, int port);

    /** 
     * Returns a server <code>SSLSessionContext</code> object for
     * this context.
     *
     * @return the <code>SSLSessionContext</code> object
     * @see javax.net.ssl.SSLContext#getServerSessionContext()
     */
    protected abstract SSLSessionContext engineGetServerSessionContext();

    /** 
     * Returns a client <code>SSLSessionContext</code> object for
     * this context.
     *
     * @return the <code>SSLSessionContext</code> object
     * @see javax.net.ssl.SSLContext#getClientSessionContext()
     */
    protected abstract SSLSessionContext engineGetClientSessionContext();

    /** 
     * Returns a copy of the SSLParameters indicating the default
     * settings for this SSL context.
     *
     * <p>The parameters will always have the ciphersuite and protocols
     * arrays set to non-null values.
     *
     * <p>The default implementation obtains the parameters from an
     * SSLSocket created by calling the
     * {@linkplain javax.net.SocketFactory#createSocket
     * SocketFactory.createSocket()} method of this context's SocketFactory.
     *
     * @return a copy of the SSLParameters object with the default settings
     * @throws UnsupportedOperationException if the default SSL parameters
     *   could not be obtained.
     *
     * @since 1.6
     */
    protected SSLParameters engineGetDefaultSSLParameters() {
        return null;
    }

    /** 
     * Returns a copy of the SSLParameters indicating the maximum supported
     * settings for this SSL context.
     *
     * <p>The parameters will always have the ciphersuite and protocols
     * arrays set to non-null values.
     *
     * <p>The default implementation obtains the parameters from an
     * SSLSocket created by calling the
     * {@linkplain javax.net.SocketFactory#createSocket
     * SocketFactory.createSocket()} method of this context's SocketFactory.
     *
     * @return a copy of the SSLParameters object with the maximum supported 
     *   settings
     * @throws UnsupportedOperationException if the supported SSL parameters
     *   could not be obtained.
     *
     * @since 1.6
     */
    protected SSLParameters engineGetSupportedSSLParameters() {
        return null;
    }
}
