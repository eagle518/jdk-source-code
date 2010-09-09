package com.sun.xml.internal.rngom.ast.builder;

import com.sun.xml.internal.rngom.ast.om.Location;
import com.sun.xml.internal.rngom.ast.om.ParsedPattern;
import com.sun.xml.internal.rngom.ast.om.ParsedElementAnnotation;

public interface Scope<
    P extends ParsedPattern,
    E extends ParsedElementAnnotation,
    L extends Location,
    A extends Annotations<E,L,CL>,
    CL extends CommentList<L> > extends GrammarSection<P,E,L,A,CL> {
    P makeParentRef(String name, L loc, A anno) throws BuildException;
    P makeRef(String name, L loc, A anno) throws BuildException;
}
