/*
 * @(#)CertParseError.java	1.17 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

/**
 * CertException indicates one of a variety of certificate problems.
 * @deprecated use one of the Exceptions defined in the
 * java.security.cert package.
 *
 * @version 1.17
 * @author David Brownell
 */
@Deprecated
class CertParseError extends CertException
{
    private static final long serialVersionUID = -4559645519017017804L;

    CertParseError (String where)
    {
	super (CertException.verf_PARSE_ERROR, where);
    }
}

