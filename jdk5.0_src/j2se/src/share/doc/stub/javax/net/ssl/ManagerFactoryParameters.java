/*
 * @(#)ManagerFactoryParameters.java	1.7 04/02/16
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
 * This class is the base interface for providing
 * algorithm-specific information to a KeyManagerFactory or
 * TrustManagerFactory.
 * <P>
 * In some cases, initialization parameters other than keystores
 * may be needed by a provider.  Users of that particular provider
 * are expected to pass an implementation of the appropriate
 * sub-interface of this class as defined by the
 * provider.  The provider can then call the specified methods in
 * the <CODE>ManagerFactoryParameters</CODE> implementation to obtain the
 * needed information.
 *
 * @author Brad R. Wetmore
 * @version 1.3, 06/24/03
 * @since 1.4
 */
public interface ManagerFactoryParameters
{
}
