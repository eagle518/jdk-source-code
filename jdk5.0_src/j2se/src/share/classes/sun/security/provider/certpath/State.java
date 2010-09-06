/*
 * @(#)State.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.io.IOException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.cert.CertPathValidatorException;

/** 
 * A specification of a PKIX validation state
 * which is initialized by each build and updated each time a 
 * certificate is added to the current path.
 *
 * @version 	1.6 12/19/03
 * @since	1.4
 * @author      Sean Mullan
 * @author      Yassir Elley
 */

interface State extends Cloneable {

    /**
     * Update the state with the next certificate added to the path.
     *
     * @param cert the certificate which is used to update the state
     */
    public void updateState(X509Certificate cert) 
	throws CertificateException, IOException, CertPathValidatorException;

    /**
     * Creates and returns a copy of this object
     */
    public Object clone();

    /**
     * Returns a boolean flag indicating if the state is initial 
     * (just starting)
     *
     * @return boolean flag indicating if the state is initial (just starting)
     */
    public boolean isInitial();

    /**
     * Returns a boolean flag indicating if a key lacking necessary key
     * algorithm parameters has been encountered.
     *
     * @return boolean flag indicating if key lacking parameters encountered.
     */
    public boolean keyParamsNeeded();
}
