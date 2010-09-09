package com.sun.xml.internal.rngom.ast.builder;

import com.sun.xml.internal.rngom.ast.om.Location;
import com.sun.xml.internal.rngom.ast.om.ParsedElementAnnotation;
import com.sun.xml.internal.rngom.ast.om.ParsedPattern;

/**
 * Used to build foreign element annotations.
 */
public interface ElementAnnotationBuilder<
    P extends ParsedPattern,
    E extends ParsedElementAnnotation,
    L extends Location,
    A extends Annotations<E,L,CL>,
    CL extends CommentList<L>> extends Annotations<E,L,CL> {

    /**
     * Called when a child text is found.
     */
    void addText(String value, L loc, CL comments) throws BuildException;

    /**
     * Called at the end to build an application data structure.
     */
    E makeElementAnnotation() throws BuildException;
}
