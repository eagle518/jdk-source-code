/*
 * @(#)ServiceConfigurationError.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;


/**
 * Error thrown when something goes wrong while looking up service providers.
 * In particular, this error will be thrown in the following situations:
 *
 *   <ul>
 *   <li> A concrete provider class cannot be found,
 *   <li> A concrete provider class cannot be instantiated,
 *   <li> The format of a provider-configuration file is illegal, or
 *   <li> An IOException occurs while reading a provider-configuration file.
 *   </ul>
 *
 * @author Mark Reinhold
 * @version 1.7, 03/12/19
 * @since 1.3
 */

public class ServiceConfigurationError extends Error {

    /**
     * Constructs a new instance with the specified detail string.
     */
    public ServiceConfigurationError(String msg) {
	super(msg);
    }

    /**
     * Constructs a new instance that wraps the specified throwable.
     */
    public ServiceConfigurationError(Throwable x) {
	super(x);
    }

}
