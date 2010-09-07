/*
 * @(#)HttpsURLConnection.java	1.13 04/02/16
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

import java.net.URL;
import java.net.HttpURLConnection;
import java.security.Principal;
import java.security.cert.X509Certificate;
import javax.security.auth.x500.X500Principal;

/** 
 * <code>HttpsURLConnection</code> extends <code>HttpURLConnection</code>
 * with support for https-specific features.
 * <P>
 * See <A HREF="http://www.w3.org/pub/WWW/Protocols/">
 * http://www.w3.org/pub/WWW/Protocols/</A> and
 * <A HREF="http://www.ietf.org/"> RFC 2818 </A>
 * for more details on the
 * https specification.
 * <P>
 * This class uses <code>HostnameVerifier</code> and
 * <code>SSLSocketFactory</code>.
 * There are default implementations defined for both classes.
 * However, the implementations can be replaced on a per-class (static) or
 * per-instance basis.  All new <code>HttpsURLConnection</code>s instances
 * will be assigned
 * the "default" static values at instance creation, but they can be overriden
 * by calling the appropriate per-instance set method(s) before
 * <code>connect</code>ing.
 *
 * @since 1.4
 * @version 1.25
 */
public abstract class HttpsURLConnection extends HttpURLConnection
{
    /** 
     * The <code>hostnameVerifier</code> for this object.
     */
    protected HostnameVerifier hostnameVerifier;

    /** 
     * Creates an <code>HttpsURLConnection</code> using the
     * URL specified.
     *
     * @param url the URL
     */
    protected HttpsURLConnection(URL url) { }

    /** 
     * Returns the cipher suite in use on this connection.
     *
     * @return the cipher suite
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     */
    public abstract String getCipherSuite();

    /** 
     * Returns the certificate(s) that were sent to the server during
     * handshaking.
     * <P>
     * Note: This method is useful only when using certificate-based
     * cipher suites.
     * <P>
     * When multiple certificates are available for use in a
     * handshake, the implementation chooses what it considers the
     * "best" certificate chain available, and transmits that to
     * the other side.  This method allows the caller to know
     * which certificate chain was actually sent.
     *
     * @return an ordered array of certificates,
     *		with the client's own certificate first followed by any
     *		certificate authorities.  If no certificates were sent,
     *		then null is returned.
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     * @see #getLocalPrincipal()
     */
    public abstract java.security.cert.Certificate[] getLocalCertificates();

    /** 
     * Returns the server's certificate chain which was established
     * as part of defining the session.
     * <P>
     * Note: This method can be used only when using certificate-based
     * cipher suites; using it with non-certificate-based cipher suites,
     * such as Kerberos, will throw an SSLPeerUnverifiedException.
     *
     * @return an ordered array of server certificates,
     *		with the peer's own certificate first followed by
     *		any certificate authorities.
     * @throws SSLPeerUnverifiedException if the peer is not verified.
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     * @see #getPeerPrincipal()
     */
    public abstract java.security.cert.Certificate[] getServerCertificates()
        throws SSLPeerUnverifiedException;

    /** 
     * Returns the server's principal which was established as part of
     * defining the session.
     * <P>
     * Note: Subclasses should override this method. If not overridden, it
     * will default to returning the X500Principal of the server's end-entity
     * certificate for certificate-based ciphersuites, or throw an
     * SSLPeerUnverifiedException for non-certificate based ciphersuites,
     * such as Kerberos.
     *
     * @return the server's principal. Returns an X500Principal of the
     * end-entity certiticate for X509-based cipher suites, and
     * KerberosPrincipal for Kerberos cipher suites.
     *
     * @throws SSLPeerUnverifiedException if the peer was not verified
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     *
     * @see #getServerCertificates()
     * @see #getLocalPrincipal()
     *
     * @since 1.5
     */
    public Principal getPeerPrincipal() throws SSLPeerUnverifiedException {
        return null;
    }

    /** 
     * Returns the principal that was sent to the server during handshaking.
     * <P>
     * Note: Subclasses should override this method. If not overridden, it
     * will default to returning the X500Principal of the end-entity certificate
     * that was sent to the server for certificate-based ciphersuites or,
     * return null for non-certificate based ciphersuites, such as Kerberos.
     *
     * @return the principal sent to the server. Returns an X500Principal
     * of the end-entity certificate for X509-based cipher suites, and
     * KerberosPrincipal for Kerberos cipher suites. If no principal was
     * sent, then null is returned.
     *
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     *
     * @see #getLocalCertificates()
     * @see #getPeerPrincipal()
     *
     * @since 1.5
     */
    public Principal getLocalPrincipal() {
        return null;
    }

    /** 
     * Sets the default <code>HostnameVerifier</code> inherited by a
     * new instance of this class.
     * <P>
     * If this method is not called, the default
     * <code>HostnameVerifier</code> assumes the connection should not
     * be permitted.
     *
     * @param v the default host name verifier
     * @throws IllegalArgumentException if the <code>HostnameVerifier</code>
     *		parameter is null.
     * @see #getDefaultHostnameVerifier()
     */
    public static void setDefaultHostnameVerifier(HostnameVerifier v) { }

    /** 
     * Gets the default <code>HostnameVerifier</code> that is inherited
     * by new instances of this class.
     *
     * @return the default host name verifier
     * @see #setDefaultHostnameVerifier(HostnameVerifier)
     */
    public static HostnameVerifier getDefaultHostnameVerifier() {
        return null;
    }

    /** 
     * Sets the <code>HostnameVerifier</code> for this instance.
     * <P>
     * New instances of this class inherit the default static hostname
     * verifier set by {@link #setDefaultHostnameVerifier(HostnameVerifier)
     * setDefaultHostnameVerifier}.  Calls to this method replace
     * this object's <code>HostnameVerifier</code>.
     *
     * @param v the host name verifier
     * @throws IllegalArgumentException if the <code>HostnameVerifier</code>
     *	parameter is null.
     * @see #getHostnameVerifier()
     * @see #setDefaultHostnameVerifier(HostnameVerifier)
     */
    public void setHostnameVerifier(HostnameVerifier v) { }

    /** 
     * Gets the <code>HostnameVerifier</code> in place on this instance.
     *
     * @return the host name verifier
     * @see #setHostnameVerifier(HostnameVerifier)
     * @see #setDefaultHostnameVerifier(HostnameVerifier)
     */
    public HostnameVerifier getHostnameVerifier() {
        return null;
    }

    /** 
     * Sets the default <code>SSLSocketFactory</code> inherited by new
     * instances of this class.
     * <P>
     * The socket factories are used when creating sockets for secure
     * https URL connections.
     *
     * @param sf the default SSL socket factory
     * @throws IllegalArgumentException if the SSLSocketFactory
     *		parameter is null.
     * @see #getDefaultSSLSocketFactory()
     */
    public static void setDefaultSSLSocketFactory(SSLSocketFactory sf) { }

    /** 
     * Gets the default static <code>SSLSocketFactory</code> that is
     * inherited by new instances of this class.
     * <P>
     * The socket factories are used when creating sockets for secure
     * https URL connections.
     *
     * @return the default <code>SSLSocketFactory</code>
     * @see #setDefaultSSLSocketFactory(SSLSocketFactory)
     */
    public static SSLSocketFactory getDefaultSSLSocketFactory() {
        return null;
    }

    /** 
     * Sets the <code>SSLSocketFactory</code> to be used when this instance
     * creates sockets for secure https URL connections.
     * <P>
     * New instances of this class inherit the default static
     * <code>SSLSocketFactory</code> set by
     * {@link #setDefaultSSLSocketFactory(SSLSocketFactory)
     * setDefaultSSLSocketFactory}.  Calls to this method replace
     * this object's <code>SSLSocketFactory</code>.
     *
     * @param sf the SSL socket factory
     * @throws IllegalArgumentException if the <code>SSLSocketFactory</code>
     *		parameter is null.
     * @see #getSSLSocketFactory()
     */
    public void setSSLSocketFactory(SSLSocketFactory sf) { }

    /** 
     * Gets the SSL socket factory to be used when creating sockets
     * for secure https URL connections.
     *
     * @return the <code>SSLSocketFactory</code>
     * @see #setSSLSocketFactory(SSLSocketFactory)
     */
    public SSLSocketFactory getSSLSocketFactory() {
        return null;
    }
}
