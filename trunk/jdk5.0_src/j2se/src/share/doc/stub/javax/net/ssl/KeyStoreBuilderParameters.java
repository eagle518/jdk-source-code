/*
 * @(#)KeyStoreBuilderParameters.java	1.5 04/02/16
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

import java.util.*;
import java.security.*;

/** 
 * A parameters object for X509KeyManagers that encapsulates a List
 * of KeyStore.Builders.
 *
 * @see java.security.KeyStore.Builder
 * @see X509KeyManager
 *
 * @author  Andreas Sterbenz
 * @version 1.3, 12/09/03
 * @since   1.5
 */
public class KeyStoreBuilderParameters implements ManagerFactoryParameters
{

    /** 
     * Construct new KeyStoreBuilderParameters from the specified
     * {@linkplain java.security.KeyStore.Builder}.
     *
     * @param builder the Builder object
     * @exception NullPointerException if builder is null
     */
    public KeyStoreBuilderParameters(KeyStore.Builder builder) { }

    /** 
     * Construct new KeyStoreBuilderParameters from a List
     * of {@linkplain java.security.KeyStore.Builder}s. Note that the list
     * is cloned to protect against subsequent modification.
     *
     * @param parameters the List of Builder objects
     * @exception NullPointerException if parameters is null
     * @exception IllegalArgumentException if parameters is an empty list
     */
    public KeyStoreBuilderParameters(List parameters) { }

    /** 
     * Return the unmodifiable List of the
     * {@linkplain java.security.KeyStore.Builder}s
     * encapsulated by this object.
     *
     * @return the unmodifiable List of the
     * {@linkplain java.security.KeyStore.Builder}s
     * encapsulated by this object.
     */
    public List getParameters() {
        return null;
    }
}
