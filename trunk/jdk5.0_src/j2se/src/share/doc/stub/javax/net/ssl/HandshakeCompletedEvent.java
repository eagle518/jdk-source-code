/*
 * @(#)HandshakeCompletedEvent.java	1.12 04/02/16
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

import java.util.EventObject;
import java.security.cert.Certificate;
import java.security.Principal;
import java.security.cert.X509Certificate;
import javax.security.auth.x500.X500Principal;

/** 
 * This event indicates that an SSL handshake completed on a given
 * SSL connection.  All of the core information about that handshake's
 * result is captured through an "SSLSession" object.  As a convenience,
 * this event class provides direct access to some important session
 * attributes.
 *
 * <P> The source of this event is the SSLSocket on which handshaking
 * just completed.
 *
 * @see SSLSocket
 * @see HandshakeCompletedListener
 * @see SSLSession
 *
 * @since 1.4
 * @version 1.23
 * @author David Brownell
 */
public class HandshakeCompletedEvent extends EventObject
{

    /** 
     * Constructs a new HandshakeCompletedEvent.
     *
     * @param sock the SSLSocket acting as the source of the event
     * @param s the SSLSession this event is associated with
     */
    public HandshakeCompletedEvent(SSLSocket sock, SSLSession s) { }

    /** 
     * Returns the session that triggered this event.
     *
     * @return the <code>SSLSession</code> for this handshake
     */
    public SSLSession getSession() {
        return null;
    }

    /** 
     * Returns the cipher suite in use by the session which was produced
     * by the handshake.  (This is a convenience method for
     * getting the ciphersuite from the SSLsession.)
     *
     * @return the name of the cipher suite negotiated during this session.
     */
    public String getCipherSuite() {
        return null;
    }

    /** 
     * Returns the certificate(s) that were sent to the peer during
     * handshaking.
     * Note: This method is useful only when using certificate-based
     * cipher suites.
     *
     * When multiple certificates are available for use in a
     * handshake, the implementation chooses what it considers the
     * "best" certificate chain available, and transmits that to
     * the other side.  This method allows the caller to know
     * which certificate chain was actually used.
     *
     * @return an ordered array of certificates, with the local
     *		certificate first followed by any
     *		certificate authorities.  If no certificates were sent,
     *		then null is returned.
     * @see #getLocalPrincipal()
     */
    public Certificate[] getLocalCertificates() {
        return null;
    }

    /** 
     * Returns the identity of the peer which was established as part
     * of defining the session.
     * Note: This method can be used only when using certificate-based
     * cipher suites; using it with non-certificate-based cipher suites,
     * such as Kerberos, will throw an SSLPeerUnverifiedException.
     *
     * @return an ordered array of the peer certificates,
     *		with the peer's own certificate first followed by
     *		any certificate authorities.
     * @exception SSLPeerUnverifiedException if the peer is not verified.
     * @see #getPeerPrincipal()
     */
    public Certificate[] getPeerCertificates() throws SSLPeerUnverifiedException
    {
        return null;
    }

    /** 
     * Returns the identity of the peer which was identified as part
     * of defining the session.
     * Note: This method can be used only when using certificate-based
     * cipher suites; using it with non-certificate-based cipher suites,
     * such as Kerberos, will throw an SSLPeerUnverifiedException.
     *
     * <p><em>Note: this method exists for compatibility with previous
     * releases. New applications should use
     * {@link #getPeerCertificates} instead.</em></p>
     *
     * @return an ordered array of peer X.509 certificates,
     *		with the peer's own certificate first followed by any
     *		certificate authorities.  (The certificates are in
     *		the original JSSE
     *		{@link javax.security.cert.X509Certificate} format).
     * @exception SSLPeerUnverifiedException if the peer is not verified.
     * @see #getPeerPrincipal()
     */
    public javax.security.cert.X509Certificate[] getPeerCertificateChain()
        throws SSLPeerUnverifiedException
    {
        return null;
    }

    /** 
     * Returns the identity of the peer which was established as part of
     * defining the session.
     *
     * @return the peer's principal. Returns an X500Principal of the
     * end-entity certiticate for X509-based cipher suites, and
     * KerberosPrincipal for Kerberos cipher suites.
     *
     * @throws SSLPeerUnverifiedException if the peer's identity has not
     *		been verified
     *
     * @see #getPeerCertificates()
     * @see #getLocalPrincipal()
     *
     * @since 1.5
     */
    public Principal getPeerPrincipal() throws SSLPeerUnverifiedException {
        return null;
    }

    /** 
     * Returns the principal that was sent to the peer during handshaking.
     *
     * @return the principal sent to the peer. Returns an X500Principal
     * of the end-entity certificate for X509-based cipher suites, and
     * KerberosPrincipal for Kerberos cipher suites. If no principal was
     * sent, then null is returned.
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
     * Returns the socket which is the source of this event.
     * (This is a convenience function, to let applications
     * write code without type casts.)
     *
     * @return the socket on which the connection was made.
     */
    public SSLSocket getSocket() {
        return null;
    }
}
