/*
 * @(#)TrustManager.java	1.9 04/02/16
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

/** 
 * This is the base interface for JSSE trust managers.
 * <P>
 * <code>TrustManager</code>s are responsible for managing the trust material
 * that is used when making trust decisions, and for deciding whether
 * credentials presented by a peer should be accepted.
 * <P>
 * <code>TrustManager</code>s are created by either
 * using a <code>TrustManagerFactory</code>,
 * or by implementing one of the <code>TrustManager</code> subclasses.
 *
 * @see TrustManagerFactory
 * @since 1.4
 * @version 1.11
 */
public interface TrustManager
{
}
