/*
 * @(#)AbstractMemberBuilder.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.builders;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.tools.doclets.internal.toolkit.util.*;
import java.util.*;

/**
 * The superclass for all member builders.  Member builders are only executed
 * within Class Builders.  They essentially build sub-components.  For example,
 * method documentation is a sub-component of class documentation.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */
public abstract class AbstractMemberBuilder extends AbstractBuilder {
    
    /**
     * Construct a SubBuilder.
     * @param configuration the configuration used in this run
     *        of the doclet.
     */
    public AbstractMemberBuilder(Configuration configuration) {
        super(configuration);
    }
    
    /**
     * This method is not supported by sub-builders.
     * 
     * @throws DocletAbortException this method will always throw a 
     * DocletAbortException because it is not supported.
     */
    public void build() throws DocletAbortException {
        //You may not call the build method in a subbuilder.
        throw new DocletAbortException();
    }
    
    
    /**
     * Build the sub component if there is anything to document.
     *
     * @param elements {@inheritDoc}
     */
    public void build(List elements) {
        if (hasMembersToDocument()) {
            super.build(elements);
        }        
    }
    
    /**
     * Return true if this subbuilder has anything to document.
     *
     * @return true if this subbuilder has anything to document.
     */
    public abstract boolean hasMembersToDocument();
}

