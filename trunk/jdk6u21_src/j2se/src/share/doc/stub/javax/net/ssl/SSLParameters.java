/*
 * @(#)SSLParameters.java	1.3 10/03/23
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
 * Encapsulates parameters for an SSL/TLS connection. The parameters
 * are the list of ciphersuites to be accepted in an SSL/TLS handshake,
 * the list of protocols to be allowed, and whether SSL/TLS servers should
 * request or require client authentication.
 *
 * <p>SSLParameters can be created via the constructors in this class.
 * Objects can also be obtained using the <code>getSSLParameters()</code>
 * methods in 
 * {@link SSLSocket#getSSLParameters SSLSocket} and 
 * {@link SSLEngine#getSSLParameters SSLEngine} or the
 * {@link SSLContext#getDefaultSSLParameters getDefaultSSLParameters()} and
 * {@link SSLContext#getSupportedSSLParameters getSupportedSSLParameters()} 
 * methods in <code>SSLContext</code>.
 *
 * <P>SSLParameters can be applied to a connection via the methods 
 * {@link SSLSocket#setSSLParameters SSLSocket.setSSLParameters()} and
 * {@link SSLEngine#setSSLParameters SSLEngine.getSSLParameters()}.
 *
 * @see SSLSocket
 * @see SSLEngine
 * @see SSLContext
 *
 * @since 1.6
 */
public class SSLParameters
{

    /** 
     * Constructs SSLParameters.
     *
     * <p>The cipherSuites and protocols values are set to <code>null</code>,
     * wantClientAuth and needClientAuth are set to <code>false</code>.
     */
    public SSLParameters() { }

    /** 
     * Constructs SSLParameters from the specified array of ciphersuites.
     * Calling this constructor is equivalent to calling the no-args
     * constructor followed by
     * <code>setCipherSuites(cipherSuites);</code>.
     *
     * @param cipherSuites the array of ciphersuites (or null)
     */
    public SSLParameters(String[] cipherSuites) { }

    /** 
     * Constructs SSLParameters from the specified array of ciphersuites
     * and protocols.
     * Calling this constructor is equivalent to calling the no-args
     * constructor followed by
     * <code>setCipherSuites(cipherSuites); setProtocols(protocols);</code>.
     *
     * @param cipherSuites the array of ciphersuites (or null)
     * @param protocols the array of protocols (or null)
     */
    public SSLParameters(String[] cipherSuites, String[] protocols) { }

    /** 
     * Returns a copy of the array of ciphersuites or null if none
     * have been set.
     *
     * @return a copy of the array of ciphersuites or null if none
     * have been set.
     */
    public String[] getCipherSuites() {
        return null;
    }

    /** 
     * Sets the array of ciphersuites.
     *
     * @param cipherSuites the array of ciphersuites (or null)
     */
    public void setCipherSuites(String[] cipherSuites) { }

    /** 
     * Returns a copy of the array of protocols or null if none
     * have been set.
     *
     * @return a copy of the array of protocols or null if none
     * have been set.
     */
    public String[] getProtocols() {
        return null;
    }

    /** 
     * Sets the array of protocols.
     *
     * @param protocols the array of protocols (or null)
     */
    public void setProtocols(String[] protocols) { }

    /** 
     * Returns whether client authentication should be requested.
     *
     * @return whether client authentication should be requested.
     */
    public boolean getWantClientAuth() {
        return false;
    }

    /** 
     * Sets whether client authentication should be requested. Calling
     * this method clears the <code>needClientAuth</code> flag.
     *
     * @param wantClientAuth whether client authentication should be requested
     */
    public void setWantClientAuth(boolean wantClientAuth) { }

    /** 
     * Returns whether client authentication should be required.
     *
     * @return whether client authentication should be required.
     */
    public boolean getNeedClientAuth() {
        return false;
    }

    /** 
     * Sets whether client authentication should be required. Calling
     * this method clears the <code>wantClientAuth</code> flag.
     *
     * @param needClientAuth whether client authentication should be required
     */
    public void setNeedClientAuth(boolean needClientAuth) { }
}
