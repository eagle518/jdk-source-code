package com.sun.xml.internal.rngom.ast.builder;

import com.sun.xml.internal.rngom.ast.om.Location;
import com.sun.xml.internal.rngom.ast.om.ParsedPattern;
import com.sun.xml.internal.rngom.ast.om.ParsedElementAnnotation;

import com.sun.xml.internal.rngom.parse.IllegalSchemaException;
import com.sun.xml.internal.rngom.parse.Parseable;

public interface Include<
    P extends ParsedPattern,
    E extends ParsedElementAnnotation,
    L extends Location,
    A extends Annotations<E,L,CL>,
    CL extends CommentList<L>> extends GrammarSection<P,E,L,A,CL> {
    /**
     * @param current
     *      The current document we are parsing.
     *      This is the document that contains an include.
     */
  void endInclude(Parseable current, String uri, String ns,
                  L loc, A anno) throws BuildException, IllegalSchemaException;
}
