/*
 * @(#)Krb5CredElement.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import sun.security.krb5.*;
import java.security.Provider;

/**
 * Provides type safety for Krb5 credential elements.
 *
 * @author Mayank Upadhyay
 * @version 1.7, 12/19/03
 * @since 1.4
 */
interface Krb5CredElement 
    extends GSSCredentialSpi {
}
