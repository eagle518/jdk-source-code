/*
 * @(#)CommandObject.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package javax.activation;

import java.io.IOException;

/**
 * JavaBeans components that are Activation Framework aware implement
 * this interface to find out which command verb they're being asked
 * to perform, and to obtain the DataHandler representing the
 * data they should operate on.  JavaBeans that don't implement
 * this interface may be used as well.  Such commands may obtain
 * the data using the Externalizable interface, or using an
 * application-specific method.<p>
 *
 * @since 1.6
 */
public interface CommandObject {

    /**
     * Initialize the Command with the verb it is requested to handle
     * and the DataHandler that describes the data it will
     * operate on. <b>NOTE:</b> it is acceptable for the caller
     * to pass <i>null</i> as the value for <code>DataHandler</code>.
     *
     * @param verb The Command Verb this object refers to.
     * @param dh The DataHandler.
     */
    public void setCommandContext(String verb, DataHandler dh)
						throws IOException;
}
