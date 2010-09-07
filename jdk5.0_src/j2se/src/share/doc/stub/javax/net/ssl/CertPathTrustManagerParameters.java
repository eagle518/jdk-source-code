/*
 * @(#)CertPathTrustManagerParameters.java	1.4 04/02/16
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

import java.security.cert.CertPathParameters;

/** 
 * A wrapper for CertPathParameters. This class is used to pass validation
 * settings to CertPath based {@link TrustManager}s using the
 * {@link TrustManagerFactory#init(ManagerFactoryParameters)
 * TrustManagerFactory.init()} method.
 *
 * <p>Instances of this class are immutable.
 *
 * @see X509TrustManager
 * @see TrustManagerFactory
 * @see java.security.cert.CertPathParameters
 *
 * @since   1.5
 * @version 1.2, 08/05/03
 * @author  Andreas Sterbenz
 */
public class CertPathTrustManagerParameters implements ManagerFactoryParameters
{

    /** 
     * Construct new CertPathTrustManagerParameters from the specified
     * parameters. The parameters are cloned to protect against subsequent
     * modification.
     *
     * @param parameters the CertPathParameters to be used
     *
     * @throws NullPointerException if parameters is null
     */
    public CertPathTrustManagerParameters(CertPathParameters parameters) { }

    /** 
     * Return a clone of the CertPathParameters encapsulated by this class.
     *
     * @return a clone of the CertPathParameters encapsulated by this class.
     */
    public CertPathParameters getParameters() {
        return null;
    }
}
